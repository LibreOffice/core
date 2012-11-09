/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_ORCUSXML_HXX__
#define __SC_ORCUSXML_HXX__

#include "scdllapi.h"
#include "address.hxx"
#include "vcl/image.hxx"

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

class SvTreeListEntry;

/**
 * Parameter used during call to ScOrcusFilters::loadXMLStructure().
 */
struct ScOrcusXMLTreeParam
{
    enum EntryType { ElementDefault, ElementRepeat, Attribute };

    /** Custom data stored with each tree item. */
    struct EntryData
    {
        EntryType meType;
        ScAddress maLinkedPos; /// linked cell position (invalid if unlinked)
        bool mbRangeParent;

        SC_DLLPUBLIC EntryData(EntryType eType);
    };

    typedef boost::ptr_vector<EntryData> UserDataStoreType;

    Image maImgElementDefault;
    Image maImgElementRepeat;
    Image maImgAttribute;

    /**
     * Store all custom data instances since the tree control doesn't manage
     * the life cycle of user datas.
     */
    UserDataStoreType maUserDataStore;

    static SC_DLLPUBLIC EntryData* getUserData(SvTreeListEntry& rEntry);
    static SC_DLLPUBLIC const EntryData* getUserData(const SvTreeListEntry& rEntry);
};

struct ScOrcusImportXMLParam
{
    struct CellLink
    {
        ScAddress maPos;
        rtl::OUString maPath;

        CellLink(const ScAddress& rPos, const rtl::OUString& rPath);
    };

    struct RangeLink
    {
        ScAddress maPos;
        std::vector<rtl::OUString> maFieldPaths;
    };

    std::vector<CellLink> maCellLinks;
    std::vector<RangeLink> maRangeLinks;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
