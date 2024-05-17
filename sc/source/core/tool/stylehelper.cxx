/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <string_view>

#include <svl/style.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>

#include <stylehelper.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

namespace {

struct ScDisplayNameMap
{
    OUString  aDispName;
    OUString  aProgName;
};

}

static const ScDisplayNameMap* lcl_GetStyleNameMap( SfxStyleFamily nType )
{
    if ( nType == SfxStyleFamily::Para )
    {
        static ScDisplayNameMap const aCellMap[]
        {
            // Standard builtin styles from configuration.
            // Defined in sc/res/xml/styles.xml
            // Installed to "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/calc/styles.xml"
            // e.g. /usr/lib64/libreoffice/share/calc/styles.xml
            // or instdir/share/calc/styles.xml
            { ScResId( STR_STYLENAME_HEADING ),   u"Heading"_ustr },
            { ScResId( STR_STYLENAME_HEADING_1 ), u"Heading 1"_ustr },
            { ScResId( STR_STYLENAME_HEADING_2 ), u"Heading 2"_ustr },
            { ScResId( STR_STYLENAME_TEXT ),      u"Text"_ustr },
            { ScResId( STR_STYLENAME_NOTE ),      u"Note"_ustr },
            { ScResId( STR_STYLENAME_FOOTNOTE ),  u"Footnote"_ustr },
            { ScResId( STR_STYLENAME_HYPERLINK ), u"Hyperlink"_ustr },
            { ScResId( STR_STYLENAME_STATUS ),    u"Status"_ustr },
            { ScResId( STR_STYLENAME_GOOD ),      u"Good"_ustr },
            { ScResId( STR_STYLENAME_NEUTRAL ),   u"Neutral"_ustr },
            { ScResId( STR_STYLENAME_BAD ),       u"Bad"_ustr },
            { ScResId( STR_STYLENAME_WARNING ),   u"Warning"_ustr },
            { ScResId( STR_STYLENAME_ERROR ),     u"Error"_ustr },
            { ScResId( STR_STYLENAME_ACCENT ),    u"Accent"_ustr },
            { ScResId( STR_STYLENAME_ACCENT_1 ),  u"Accent 1"_ustr },
            { ScResId( STR_STYLENAME_ACCENT_2 ),  u"Accent 2"_ustr },
            { ScResId( STR_STYLENAME_ACCENT_3 ),  u"Accent 3"_ustr },
            { ScResId( STR_STYLENAME_RESULT ),    u"Result"_ustr },
            // API compatibility programmatic names after.
            { ScResId( STR_STYLENAME_STANDARD ), SC_STYLE_PROG_STANDARD },
            { ScResId( STR_STYLENAME_RESULT ), SC_STYLE_PROG_RESULT },
            { ScResId( STR_STYLENAME_RESULT1 ), SC_STYLE_PROG_RESULT1 },
            { ScResId( STR_STYLENAME_HEADING ), SC_STYLE_PROG_HEADING },
            { ScResId( STR_STYLENAME_HEADING_1 ), SC_STYLE_PROG_HEADING1 },
            // Pivot table styles.
            { ScResId( STR_PIVOT_STYLENAME_INNER ), SC_PIVOT_STYLE_PROG_INNER },
            { ScResId( STR_PIVOT_STYLENAME_RESULT ), SC_PIVOT_STYLE_PROG_RESULT },
            { ScResId( STR_PIVOT_STYLENAME_CATEGORY ), SC_PIVOT_STYLE_PROG_CATEGORY },
            { ScResId( STR_PIVOT_STYLENAME_TITLE ), SC_PIVOT_STYLE_PROG_TITLE },
            { ScResId( STR_PIVOT_STYLENAME_FIELDNAME ), SC_PIVOT_STYLE_PROG_FIELDNAME },
            { ScResId( STR_PIVOT_STYLENAME_TOP ), SC_PIVOT_STYLE_PROG_TOP },
            //  last entry remains empty
            { OUString(), OUString() },
        };
        return aCellMap;
    }
    else if ( nType == SfxStyleFamily::Page )
    {
        static ScDisplayNameMap const aPageMap[]
        {
            { ScResId( STR_STYLENAME_STANDARD ), SC_STYLE_PROG_STANDARD },
            { ScResId( STR_STYLENAME_REPORT ),   SC_STYLE_PROG_REPORT },
            //  last entry remains empty
            { OUString(), OUString() },
        };
        return aPageMap;
    }
    else if ( nType == SfxStyleFamily::Frame )
    {
        static ScDisplayNameMap const aGraphicMap[]
        {
            { ScResId( STR_STYLENAME_STANDARD ), SC_STYLE_PROG_STANDARD },
            { ScResId( STR_STYLENAME_NOTE ), u"Note"_ustr },
            //  last entry remains empty
            { OUString(), OUString() },
        };
        return aGraphicMap;
    }
    OSL_FAIL("invalid family");
    return nullptr;
}

//  programmatic name suffix for display names that match other programmatic names
//  is " (user)" including a space

constexpr OUString SC_SUFFIX_USER = u" (user)"_ustr;

static bool lcl_EndsWithUser( std::u16string_view rString )
{
    return o3tl::ends_with(rString, SC_SUFFIX_USER);
}

OUString ScStyleNameConversion::DisplayToProgrammaticName( const OUString& rDispName, SfxStyleFamily nType )
{
    bool bDisplayIsProgrammatic = false;

    const ScDisplayNameMap* pNames = lcl_GetStyleNameMap( nType );
    if (pNames)
    {
        do
        {
            if (pNames->aDispName == rDispName)
                return pNames->aProgName;
            else if (pNames->aProgName == rDispName)
                bDisplayIsProgrammatic = true;          // display name matches any programmatic name
        }
        while( !(++pNames)->aDispName.isEmpty() );
    }

    if ( bDisplayIsProgrammatic || lcl_EndsWithUser( rDispName ) )
    {
        //  add the (user) suffix if the display name matches any style's programmatic name
        //  or if it already contains the suffix
        return rDispName + SC_SUFFIX_USER;
    }

    return rDispName;
}

OUString ScStyleNameConversion::ProgrammaticToDisplayName( const OUString& rProgName, SfxStyleFamily nType )
{
    if ( lcl_EndsWithUser( rProgName ) )
    {
        //  remove the (user) suffix, don't compare to map entries
        return rProgName.copy( 0, rProgName.getLength() - SC_SUFFIX_USER.getLength() );
    }

    const ScDisplayNameMap* pNames = lcl_GetStyleNameMap( nType );
    if (pNames)
    {
        do
        {
            if (pNames->aProgName == rProgName)
                return pNames->aDispName;
        }
        while( !(++pNames)->aDispName.isEmpty() );
    }
    return rProgName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
