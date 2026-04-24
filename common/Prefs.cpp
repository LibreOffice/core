/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#include <common/FileUtil.hpp>
#include <common/JsonUtil.hpp>
#include <common/Log.hpp>
#include <common/Prefs.hpp>

#include <Poco/JSON/Object.h>

#include <fstream>
#include <sstream>

Prefs::Prefs(std::string path)
    : _path(std::move(path))
{
    std::ifstream stream;
    FileUtil::openFileToIFStream(_path, stream);
    if (!stream.is_open())
        return;

    std::stringstream buf;
    buf << stream.rdbuf();
    const std::string text = buf.str();
    if (text.empty())
        return;

    Poco::JSON::Object::Ptr obj;
    if (!JsonUtil::parseJSON(text, obj))
    {
        LOG_WRN("Prefs: failed to parse '" << _path << "', starting empty");
        return;
    }

    std::vector<std::string> names;
    obj->getNames(names);
    for (const auto& name : names)
        _values.emplace(name, obj->get(name).toString());
}

std::string Prefs::get(const std::string& key) const
{
    std::scoped_lock lock(_mutex);
    const auto it = _values.find(key);
    return it == _values.end() ? std::string{} : it->second;
}

void Prefs::set(const std::string& key, std::string value)
{
    std::scoped_lock lock(_mutex);
    _values[key] = std::move(value);
    save();
}

std::string Prefs::serialize() const
{
    std::scoped_lock lock(_mutex);
    Poco::JSON::Object::Ptr obj = new Poco::JSON::Object();
    for (const auto& [k, v] : _values)
        obj->set(k, v);
    return JsonUtil::jsonToString(obj);
}

void Prefs::save() const
{
    Poco::JSON::Object::Ptr obj = new Poco::JSON::Object();
    for (const auto& [k, v] : _values)
        obj->set(k, v);

    std::ofstream stream;
    FileUtil::openFileToOFStream(_path, stream);
    if (!stream.is_open() || stream.bad())
    {
        LOG_ERR("Prefs: could not open '" << _path << "' for writing");
        return;
    }
    stream << JsonUtil::jsonToString(obj);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
