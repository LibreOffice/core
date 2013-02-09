/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "editattributemap.hxx"

#include "editeng/eeitem.hxx"
#include "editeng/memberids.hrc"

namespace {

struct {
    const char* mpXMLName;
    sal_uInt16 mnItemID;
    sal_uInt8 mnFlag;
} Entries[] = {
    { "color", EE_CHAR_COLOR, 0 },
    { "font-weight", EE_CHAR_WEIGHT, MID_WEIGHT },
    { "font-weight-asian", EE_CHAR_WEIGHT_CJK, MID_WEIGHT },
    { "font-weight-complex", EE_CHAR_WEIGHT_CTL, MID_WEIGHT },
    { "font-size", EE_CHAR_FONTHEIGHT, MID_FONTHEIGHT },
    { "font-size-asian", EE_CHAR_FONTHEIGHT_CJK, MID_FONTHEIGHT },
    { "font-size-complex", EE_CHAR_FONTHEIGHT_CTL, MID_FONTHEIGHT },
    { "font-style", EE_CHAR_ITALIC, MID_POSTURE },
    { "font-style-asian", EE_CHAR_ITALIC_CJK, MID_POSTURE },
    { "font-style-complex", EE_CHAR_ITALIC_CTL, MID_POSTURE },
};

}

ScXMLEditAttributeMap::Entry::Entry(sal_uInt16 nItemID, sal_uInt8 nFlag) :
    mnItemID(nItemID), mnFlag(nFlag) {}

ScXMLEditAttributeMap::ScXMLEditAttributeMap()
{
    size_t n = sizeof(Entries) / sizeof(Entries[0]);
    for (size_t i = 0; i < n; ++i)
    {
        maEntries.insert(
            EntriesType::value_type(
                OUString::createFromAscii(Entries[i].mpXMLName),
                Entry(Entries[i].mnItemID, Entries[i].mnFlag)));
    }
}

const ScXMLEditAttributeMap::Entry* ScXMLEditAttributeMap::getEntry(const OUString& rXMLName) const
{
    EntriesType::const_iterator it = maEntries.find(rXMLName);
    return it == maEntries.end() ? NULL : &it->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
