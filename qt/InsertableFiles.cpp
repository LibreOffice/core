/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include "InsertableFiles.hpp"

#include <QDir>
#include <QFileInfo>
#include <QStringList>

#include <mutex>

namespace
{
std::mutex WildcardsMutex;
QStringList Wildcards;
}

void setInsertableFileWildcards(const std::string& wildcards)
{
    QStringList globs = QString::fromStdString(wildcards).split(QLatin1Char(';'), Qt::SkipEmptyParts);

    std::lock_guard<std::mutex> lock(WildcardsMutex);
    Wildcards = std::move(globs);
}

bool isInsertableFileName(const QString& path)
{
    const QString name = QFileInfo(path).fileName().toLower();

    std::lock_guard<std::mutex> lock(WildcardsMutex);
    return !name.isEmpty() && QDir::match(Wildcards, name);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
