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

#include "vcl/image.hxx"

#include <boost/ptr_container/ptr_vector.hpp>

/**
 * Parameter used during call to ScOrcusFilters::loadXMLStructure().
 */
struct ScOrcusXMLTreeParam
{
    enum EntryType { ElementDefault, ElementRepeat, Attribute };

    /** Custom data stored with each tree item. */
    struct TreeEntryUserData
    {
        EntryType meType;
    };

    typedef boost::ptr_vector<TreeEntryUserData> UserDataStoreType;

    Image maImgElementDefault;
    Image maImgElementRepeat;
    Image maImgAttribute;

    /**
     * Store all custom data instances since the tree control doesn't manage
     * the life cycle of user datas.
     */
    UserDataStoreType maUserDataStore;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
