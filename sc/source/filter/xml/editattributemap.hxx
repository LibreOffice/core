/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_XML_EDITATTRIBUTEMAP_HXX__
#define __SC_XML_EDITATTRIBUTEMAP_HXX__

#include "rtl/ustring.hxx"

#include <boost/unordered_map.hpp>

/**
 * Provide mapping from ODF text formatting styles to EditEngine's, for
 * rich-text cell content import.
 */
class ScXMLEditAttributeMap
{
public:
    struct Entry
    {
        sal_uInt16 mnItemID;
        sal_uInt8 mnFlag;

        Entry(sal_uInt16 nItemID, sal_uInt8 nFlag);
    };

    ScXMLEditAttributeMap();

    const Entry* getEntry(const OUString& rXMLName) const;

private:
    typedef boost::unordered_map<OUString, Entry, OUStringHash> EntriesType;
    EntriesType maEntries;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
