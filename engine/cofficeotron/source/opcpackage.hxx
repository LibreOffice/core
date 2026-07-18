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

#ifndef OFFICEOTRON_OPCPACKAGE_HXX
#define OFFICEOTRON_OPCPACKAGE_HXX

#include <map>
#include <string>
#include <vector>

#include "zipfile.hxx"

// One Relationship harvested from a .rels part of an OPC package.
struct OOXMLTarget
{
    // The .rels entry this relationship came from, in the same form the
    // relationship spidering probes it ("_rels/.rels" for the package
    // rels, "/word/_rels/document.xml.rels" below that).
    std::string hostPartEntryName;
    // The Type attribute of the Relationship.
    std::string type;
    // The Target attribute of the Relationship, as written.
    std::string name;
    // The content type from [Content_Types].xml overrides; empty when no
    // override names this part.
    std::string mimeType;

    // The text after the last dot of the target, or the whole target
    // when it has no dot.
    std::string extension() const;

    // The text after the last slash of the target.
    std::string filename() const;

    // The part name with the file name removed, ending in a slash.
    std::string targetFolder() const;

    // The target resolved to an absolute part name starting with "/".
    // Relative targets resolve against the folder that holds the .rels
    // part's folder.
    std::string targetAsPartName() const;
};

// The relationship graph of an OPC package: spiders _rels/.rels and
// every .rels part reachable from it, then annotates the collected
// targets with content types from [Content_Types].xml.
class OPCPackage
{
public:
    explicit OPCPackage(const ZipArchive& zip);

    void process();

    const std::vector<OOXMLTarget>& targets() const { return targetList; }

    // Returns null when no collected relationship resolves to that part
    // name.
    const OOXMLTarget* targetByName(const std::string& partName) const;

    // The Default extension-to-content-type declarations of
    // [Content_Types].xml.
    const std::map<std::string, std::string>& defaultTypes() const { return defaultTypeMap; }

private:
    void processRelationships(const std::string& entryName);

    const ZipArchive& zip;
    std::vector<OOXMLTarget> targetList;
    std::map<std::string, size_t> nameIndex;
    std::vector<std::string> partsProbed;
    std::map<std::string, std::string> defaultTypeMap;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
