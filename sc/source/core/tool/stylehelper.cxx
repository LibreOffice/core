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

#include <svl/style.hxx>
#include <osl/diagnose.h>

#include <stylehelper.hxx>
#include <global.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

//  conversion programmatic <-> display (visible) name
//  currently, the core always has the visible names
//  the api is required to use programmatic names for default styles
//  these programmatic names must never change!

#define SC_STYLE_PROG_STANDARD      "Default"
#define SC_STYLE_PROG_RESULT        "Result"
#define SC_STYLE_PROG_RESULT1       "Result2"
#define SC_STYLE_PROG_HEADING       "Heading"
#define SC_STYLE_PROG_HEADING1      "Heading1"
#define SC_STYLE_PROG_REPORT        "Report"

#define SC_PIVOT_STYLE_PROG_INNER                  "Pivot Table Value"
#define SC_PIVOT_STYLE_PROG_RESULT                 "Pivot Table Result"
#define SC_PIVOT_STYLE_PROG_CATEGORY               "Pivot Table Category"
#define SC_PIVOT_STYLE_PROG_TITLE                  "Pivot Table Title"
#define SC_PIVOT_STYLE_PROG_FIELDNAME              "Pivot Table Field"
#define SC_PIVOT_STYLE_PROG_TOP                    "Pivot Table Corner"

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
            { ScResId( STR_STYLENAME_HEADING ),   "Heading" },
            { ScResId( STR_STYLENAME_HEADING_1 ), "Heading 1" },
            { ScResId( STR_STYLENAME_HEADING_2 ), "Heading 2" },
            { ScResId( STR_STYLENAME_TEXT ),      "Text" },
            { ScResId( STR_STYLENAME_NOTE ),      "Note" },
            { ScResId( STR_STYLENAME_FOOTNOTE ),  "Footnote" },
            { ScResId( STR_STYLENAME_HYPERLINK ), "Hyperlink" },
            { ScResId( STR_STYLENAME_STATUS ),    "Status" },
            { ScResId( STR_STYLENAME_GOOD ),      "Good" },
            { ScResId( STR_STYLENAME_NEUTRAL ),   "Neutral" },
            { ScResId( STR_STYLENAME_BAD ),       "Bad" },
            { ScResId( STR_STYLENAME_WARNING ),   "Warning" },
            { ScResId( STR_STYLENAME_ERROR ),     "Error" },
            { ScResId( STR_STYLENAME_ACCENT ),    "Accent" },
            { ScResId( STR_STYLENAME_ACCENT_1 ),  "Accent 1" },
            { ScResId( STR_STYLENAME_ACCENT_2 ),  "Accent 2" },
            { ScResId( STR_STYLENAME_ACCENT_3 ),  "Accent 3" },
            { ScResId( STR_STYLENAME_RESULT ),    "Result" },
            // API compatibility programmatic names after.
            { ScResId( STR_STYLENAME_STANDARD_CELL ), OUString(SC_STYLE_PROG_STANDARD) },
            { ScResId( STR_STYLENAME_RESULT ), OUString(SC_STYLE_PROG_RESULT) },
            { ScResId( STR_STYLENAME_RESULT1 ), OUString(SC_STYLE_PROG_RESULT1) },
            { ScResId( STR_STYLENAME_HEADING ), OUString(SC_STYLE_PROG_HEADING) },
            { ScResId( STR_STYLENAME_HEADING_1 ), OUString(SC_STYLE_PROG_HEADING1) },
            // Pivot table styles.
            { ScResId( STR_PIVOT_STYLENAME_INNER ), OUString(SC_PIVOT_STYLE_PROG_INNER) },
            { ScResId( STR_PIVOT_STYLENAME_RESULT ), OUString(SC_PIVOT_STYLE_PROG_RESULT) },
            { ScResId( STR_PIVOT_STYLENAME_CATEGORY ), OUString(SC_PIVOT_STYLE_PROG_CATEGORY) },
            { ScResId( STR_PIVOT_STYLENAME_TITLE ), OUString(SC_PIVOT_STYLE_PROG_TITLE) },
            { ScResId( STR_PIVOT_STYLENAME_FIELDNAME ), OUString(SC_PIVOT_STYLE_PROG_FIELDNAME) },
            { ScResId( STR_PIVOT_STYLENAME_TOP ), OUString(SC_PIVOT_STYLE_PROG_TOP) },
            //  last entry remains empty
            { OUString(), OUString() },
        };
        return aCellMap;
    }
    else if ( nType == SfxStyleFamily::Page )
    {
        static ScDisplayNameMap const aPageMap[]
        {
            { ScResId( STR_STYLENAME_STANDARD_PAGE ), OUString(SC_STYLE_PROG_STANDARD) },
            { ScResId( STR_STYLENAME_REPORT ),   OUString(SC_STYLE_PROG_REPORT) },
            //  last entry remains empty
            { OUString(), OUString() },
        };
        return aPageMap;
    }
    OSL_FAIL("invalid family");
    return nullptr;
}

//  programmatic name suffix for display names that match other programmatic names
//  is " (user)" including a space

#define SC_SUFFIX_USER      " (user)"
#define SC_SUFFIX_USER_LEN  7

static bool lcl_EndsWithUser( const OUString& rString )
{
    return rString.endsWith(SC_SUFFIX_USER);
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
        return rProgName.copy( 0, rProgName.getLength() - SC_SUFFIX_USER_LEN );
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
