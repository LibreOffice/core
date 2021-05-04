/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

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
        OUString maXMLName;
        const char* mpAPIName;
        sal_uInt16 mnItemID;
        sal_uInt8 mnFlag;

        /* use template and OUStringLiteral so we can init this at compile time */
        template <std::size_t N>
        constexpr Entry(sal_uInt16 nXMLNS, OUStringLiteral<N> aXMLName, const char* pAPIName,
                        sal_uInt16 nItemID, sal_uInt8 nFlag)
            : nmXMLNS(nXMLNS)
            , maXMLName(aXMLName)
            , mpAPIName(pAPIName)
            , mnItemID(nItemID)
            , mnFlag(nFlag)
        {
        }
    };

    ScXMLEditAttributeMap();

    const Entry* getEntryByAPIName(const OUString& rAPIName) const;
    const Entry* getEntryByItemID(sal_uInt16 nItemID) const;

private:
    typedef std::unordered_map<OUString, const Entry*> StrToEntriesType;
    typedef std::unordered_map<sal_uInt16, const Entry*> IndexToEntriesType;
    StrToEntriesType maAPIEntries;
    IndexToEntriesType maItemIDEntries;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
