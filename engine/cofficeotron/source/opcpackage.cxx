/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2009-2010 Griffin Brown Digital Publishing Ltd.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "opcpackage.hxx"

#include <algorithm>

#include "xmlutil.hxx"

namespace
{
const char* const NS_PACKAGE_RELATIONSHIPS
    = "http://schemas.openxmlformats.org/package/2006/relationships";
const char* const NS_CONTENT_TYPES = "http://schemas.openxmlformats.org/package/2006/content-types";

// True when the reference has a URI scheme (like "http:"), which makes
// it absolute and exempt from part-name resolution.
bool hasUriScheme(const std::string& reference)
{
    for (size_t i = 0; i < reference.size(); ++i)
    {
        char character = reference[i];
        if (character == ':')
            return i > 0;
        bool schemeCharacter = (character >= 'a' && character <= 'z')
                               || (character >= 'A' && character <= 'Z')
                               || (i > 0
                                   && ((character >= '0' && character <= '9') || character == '+'
                                       || character == '-' || character == '.'));
        if (!schemeCharacter)
            return false;
    }
    return false;
}

// Merges a relative reference with a base path that names a folder
// (empty, or ending in "/"), then removes "." and ".." segments.
std::string resolveReference(const std::string& base, const std::string& reference)
{
    if (hasUriScheme(reference))
        return reference;

    std::string merged = base + reference;

    bool absolute = !merged.empty() && merged[0] == '/';

    std::vector<std::string> segments;
    size_t position = 0;
    while (position <= merged.size())
    {
        size_t slash = merged.find('/', position);
        std::string segment = slash == std::string::npos
                                  ? merged.substr(position)
                                  : merged.substr(position, slash - position);
        if (segment == "..")
        {
            if (!segments.empty())
                segments.pop_back();
        }
        else if (!segment.empty() && segment != ".")
        {
            segments.push_back(segment);
        }
        if (slash == std::string::npos)
            break;
        position = slash + 1;
    }

    std::string resolved;
    for (size_t i = 0; i < segments.size(); ++i)
    {
        if (i > 0 || absolute)
            resolved += "/";
        resolved += segments[i];
    }
    return resolved;
}

std::string zipEntryNameFor(const std::string& name)
{
    return !name.empty() && name[0] == '/' ? name.substr(1) : name;
}

} // namespace

std::string OOXMLTarget::extension() const
{
    size_t dot = name.rfind('.');
    return dot == std::string::npos ? name : name.substr(dot + 1);
}

std::string OOXMLTarget::filename() const
{
    size_t slash = name.rfind('/');
    return slash == std::string::npos ? name : name.substr(slash + 1);
}

std::string OOXMLTarget::targetFolder() const
{
    std::string partName = targetAsPartName();
    size_t slash = partName.rfind('/');
    return slash == std::string::npos ? partName : partName.substr(0, slash + 1);
}

std::string OOXMLTarget::targetAsPartName() const
{
    if (!name.empty() && name[0] == '/')
        return name;

    // The base is the parent of the folder holding the .rels part.
    std::string base = hostPartEntryName;
    size_t relsPosition = base.find("_rels/");
    if (relsPosition != std::string::npos)
        base = base.substr(0, relsPosition);

    std::string resolved = resolveReference(base, name);

    if (!resolved.empty() && resolved[0] != '/' && resolved.find("//:") == std::string::npos)
        resolved = "/" + resolved;

    return resolved;
}

OPCPackage::OPCPackage(const ZipArchive& zip)
    : zip(zip)
{
}

void OPCPackage::process()
{
    processRelationships("_rels/.rels");

    std::string bytes;
    if (!zip.extract("[Content_Types].xml", bytes))
        return;

    std::string parseError;
    XmlDocPtr doc = parseXmlMemory(bytes, "[Content_Types].xml", parseError);
    if (!doc)
        return;

    XmlNode* root = doc->rootElement();
    if (!root)
        return;
    for (const auto& child : root->children)
    {
        const XmlNode* node = child.get();
        if (!node->isElement() || namespaceOf(node) != NS_CONTENT_TYPES)
            continue;

        if (localNameOf(node) == "Override")
        {
            std::string partName;
            std::string contentType;
            getNsAttribute(node, nullptr, "PartName", partName);
            getNsAttribute(node, nullptr, "ContentType", contentType);
            auto it = nameIndex.find(partName);
            if (it != nameIndex.end())
                targetList[it->second].mimeType = contentType;
        }
        else if (localNameOf(node) == "Default")
        {
            std::string extension;
            std::string contentType;
            getNsAttribute(node, nullptr, "Extension", extension);
            getNsAttribute(node, nullptr, "ContentType", contentType);
            defaultTypeMap[extension] = contentType;
        }
    }
}

void OPCPackage::processRelationships(const std::string& entryName)
{
    std::string bytes;
    if (!zip.extract(zipEntryNameFor(entryName), bytes))
        return; // many probes are expected to find nothing

    std::string parseError;
    XmlDocPtr doc = parseXmlMemory(bytes, entryName, parseError);
    if (!doc)
        return;

    std::vector<OOXMLTarget> found;
    XmlNode* root = doc->rootElement();
    if (!root)
        return;
    for (const auto& child : root->children)
    {
        const XmlNode* node = child.get();
        if (!node->isElement() || namespaceOf(node) != NS_PACKAGE_RELATIONSHIPS
            || localNameOf(node) != "Relationship")
            continue;

        OOXMLTarget target;
        target.hostPartEntryName = entryName;
        getNsAttribute(node, nullptr, "Type", target.type);
        getNsAttribute(node, nullptr, "Target", target.name);
        found.push_back(std::move(target));
    }

    for (OOXMLTarget& target : found)
    {
        std::string partName = target.targetAsPartName();
        if (std::find(partsProbed.begin(), partsProbed.end(), partName) != partsProbed.end())
            continue;
        partsProbed.push_back(partName);

        std::string folder = target.targetFolder();
        std::string probe = folder + "_rels/" + target.filename() + ".rels";

        nameIndex.emplace(partName, targetList.size());
        targetList.push_back(target);

        processRelationships(probe);
    }
}

const OOXMLTarget* OPCPackage::targetByName(const std::string& partName) const
{
    auto it = nameIndex.find(partName);
    return it == nameIndex.end() ? nullptr : &targetList[it->second];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
