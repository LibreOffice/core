/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "scdllapi.h"
#include "address.hxx"

#include <vector>
#include <memory>

namespace weld {
    class TreeIter;
    class TreeView;
}

/**
 * Parameter used during call to ScOrcusFilters::loadXMLStructure().
 */
struct ScOrcusXMLTreeParam
{
    enum EntryType { ElementDefault, ElementRepeat, Attribute };

    /** Custom data stored with each tree item. */
    struct EntryData
    {
        size_t mnNamespaceID; /// numerical ID for xml namespace
        EntryType meType;
        ScAddress maLinkedPos; /// linked cell position (invalid if unlinked)
        bool mbRangeParent:1;
        bool mbLeafNode:1; /// Leaf if it has no child elements. Child Attributes don't count.

        SC_DLLPUBLIC EntryData(EntryType eType);
    };

    typedef std::vector<std::unique_ptr<EntryData>> UserDataStoreType;

    OUString maImgElementDefault;
    OUString maImgElementRepeat;
    OUString maImgAttribute;

    /**
     * Store all custom data instances since the tree control doesn't manage
     * the life cycle of user data.
     */
    UserDataStoreType m_UserDataStore;

    static EntryData* getUserData(const weld::TreeView& rControl, const weld::TreeIter& rEntry);
};

struct ScOrcusImportXMLParam
{
    struct CellLink
    {
        ScAddress maPos;
        OString maPath;

        CellLink(const ScAddress& rPos, const OString& rPath);
    };

    struct RangeLink
    {
        ScAddress maPos;
        std::vector<OString> maFieldPaths;
        std::vector<OString> maRowGroups;
    };

    std::vector<size_t>    maNamespaces;
    std::vector<CellLink>  maCellLinks;
    std::vector<RangeLink> maRangeLinks;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
