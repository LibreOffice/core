/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_XML_EDITATTRIBUTEMAP_HXX
#define INCLUDED_SC_SOURCE_FILTER_XML_EDITATTRIBUTEMAP_HXX

#include <rtl/ustring.hxx>

#include <unordered_map>

/**
 * Provide mapping from ODF text formatting styles to EditEngine's, for
 * rich-text cell content import.
 */
class ScXMLEditAttributeMap
{
public:
    struct Entry
    {
        sal_uInt16 nmXMLNS;
        const char* mpXMLName;
        const char* mpAPIName;
        sal_uInt16 mnItemID;
        sal_uInt8 mnFlag;
    };

    ScXMLEditAttributeMap();

    const Entry* getEntryByAPIName(const OUString& rAPIName) const;
    const Entry* getEntryByItemID(sal_uInt16 nItemID) const;

private:
    typedef std::unordered_map<OUString, const Entry*, OUStringHash> StrToEntriesType;
    typedef std::unordered_map<sal_uInt16, const Entry*> IndexToEntriesType;
    StrToEntriesType maAPIEntries;
    IndexToEntriesType maItemIDEntries;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
