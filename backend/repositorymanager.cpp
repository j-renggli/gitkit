#include "repositorymanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtGui/QFont>

namespace gitigor
{

RepositoryManager RepositoryManager::s_repositories;

RepositoryManager::RepositoryManager() : active_(-1) {}

RepositoryManager::RepositoryManager(const RepositoryManager& rhs)
{
    Q_ASSERT(false);
}

RepositoryManager& RepositoryManager::instance()
{
    return s_repositories;
}

bool RepositoryManager::add(const QString& name, const QDir& root)
{
    beginResetModel();
    repositories_.push_back(std::unique_ptr<Repository>(new Repository(name, root)));
    if (active_ >= repositories_.size()) {
        active_ = repositories_.size() - 1;
    }
    return repositories_.back()->initialise();
    endResetModel();
}

bool RepositoryManager::remove(size_t index)
{
    if (index >= repositories_.size())
        return false;

    if (active_ >= index)
        --active_;
    repositories_.erase(repositories_.begin() + index);
    return true;
}

bool RepositoryManager::setActive(size_t index)
{
    if (index >= repositories_.size())
        return false;
    beginResetModel();
    active_ = index;
    endResetModel();
    return true;
}

QVariant RepositoryManager::data(const QModelIndex& index, int role) const
{
    const size_t row = index.row();
    if (role == Qt::DisplayRole) {
        if (row < repositories_.size()) {
            if (auto& repo = repositories_.at(row)) {
                switch (index.column()) {
                case 0:
                    return QVariant(repo->name());
                case 1:
                    return QVariant(repo->root().absolutePath());
                }
            }
        }
    } else if (role == Qt::FontRole && row == active_) {
        QFont font;
        font.setBold(true);
        return font;
    }

    return QVariant();
}

QVariant RepositoryManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return QVariant("Name");
        case 1:
            return QVariant("Path");
        }
    }

    return QVariant();
}

QModelIndex RepositoryManager::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
        return QModelIndex();

    return createIndex(row, column, nullptr);
}

bool RepositoryManager::initialise(const QFileInfo& storagePath)
{
    storagePath_ = storagePath;
    /*
    repositories_.push_back(std::unique_ptr<Repository>(new
    Repository(QString("DogFood"), QDir("."))));
    repositories_.back()->initialise();
    active_ = 0;
    */
    return load();
}

bool RepositoryManager::load()
{
    beginResetModel();

    // Empty repos...
    repositories_.clear();

    QFile file(storagePath_.filePath());
    if (!file.exists())
        return true;

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument json(QJsonDocument::fromJson(file.readAll()));
    QJsonObject main = json.object();
    int version = main["version"].toInt();
    Q_ASSERT(version == 1);
    if (version != 1)
        return false;

    QJsonArray repos = main["repositories"].toArray();
    for (int i = 0; i < repos.size(); ++i) {
        QJsonObject item = repos[i].toObject();
        repositories_.push_back(
            std::unique_ptr<Repository>(new Repository(item["name"].toString(), QDir(item["root"].toString()))));
        repositories_.back()->initialise();
    }

    active_ = main["active"].toInt();
    if (active_ >= repositories_.size()) {
        active_ = 0;
    }

    endResetModel();

    return true;
}
/*
void RepositoryManager::reload()
{
        beginResetModel();
        endResetModel();
}
*/
bool RepositoryManager::save()
{
    QFile file(storagePath_.filePath());
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonObject main;
    main["version"] = 1;
    QJsonArray repos;
    for (auto& repo : repositories_) {
        QJsonObject item;
        item["name"] = repo->name();
        item["root"] = repo->root().absolutePath();
        repos.append(item);
    }
    main["repositories"] = repos;
    main["active"] = static_cast<int>(active_);

    QJsonDocument json(main);
    file.write(json.toJson());
    return true;
}

QModelIndex RepositoryManager::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

int RepositoryManager::columnCount(const QModelIndex& parent) const
{
    return 2;
}

int RepositoryManager::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    return static_cast<int>(repositories_.size());
}
} // namespace gitigor
