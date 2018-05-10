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
#define SC_STYLE_PROG_HEADLINE      "Heading"
#define SC_STYLE_PROG_HEADLINE1     "Heading1"
#define SC_STYLE_PROG_REPORT        "Report"

#define SC_PIVOT_STYLE_PROG_INNER                  "Pivot Table Value"
#define SC_PIVOT_STYLE_PROG_RESULT                 "Pivot Table Result"
#define SC_PIVOT_STYLE_PROG_CATEGORY               "Pivot Table Category"
#define SC_PIVOT_STYLE_PROG_TITLE                  "Pivot Table Title"
#define SC_PIVOT_STYLE_PROG_FIELDNAME              "Pivot Table Field"
#define SC_PIVOT_STYLE_PROG_TOP                    "Pivot Table Corner"

struct ScDisplayNameMap
{
    OUString  aDispName;
    OUString  aProgName;
};

static const ScDisplayNameMap* lcl_GetStyleNameMap( SfxStyleFamily nType )
{
    if ( nType == SfxStyleFamily::Para )
    {
        static bool bCellMapFilled = false;
        static ScDisplayNameMap aCellMap[12];
        if ( !bCellMapFilled )
        {
            aCellMap[0].aDispName = ScResId( STR_STYLENAME_STANDARD );
            aCellMap[0].aProgName = SC_STYLE_PROG_STANDARD;

            aCellMap[1].aDispName = ScResId( STR_STYLENAME_RESULT );
            aCellMap[1].aProgName = SC_STYLE_PROG_RESULT;

            aCellMap[2].aDispName = ScResId( STR_STYLENAME_RESULT1 );
            aCellMap[2].aProgName = SC_STYLE_PROG_RESULT1;

            aCellMap[3].aDispName = ScResId( STR_STYLENAME_HEADLINE );
            aCellMap[3].aProgName = SC_STYLE_PROG_HEADLINE;

            aCellMap[4].aDispName = ScResId( STR_STYLENAME_HEADLINE1 );
            aCellMap[4].aProgName = SC_STYLE_PROG_HEADLINE1;

            aCellMap[5].aDispName = ScResId( STR_PIVOT_STYLENAME_INNER );
            aCellMap[5].aProgName = SC_PIVOT_STYLE_PROG_INNER;

            aCellMap[6].aDispName = ScResId( STR_PIVOT_STYLENAME_RESULT );
            aCellMap[6].aProgName = SC_PIVOT_STYLE_PROG_RESULT;

            aCellMap[7].aDispName = ScResId( STR_PIVOT_STYLENAME_CATEGORY );
            aCellMap[7].aProgName = SC_PIVOT_STYLE_PROG_CATEGORY;

            aCellMap[8].aDispName = ScResId( STR_PIVOT_STYLENAME_TITLE );
            aCellMap[8].aProgName = SC_PIVOT_STYLE_PROG_TITLE;

            aCellMap[9].aDispName = ScResId( STR_PIVOT_STYLENAME_FIELDNAME );
            aCellMap[9].aProgName = SC_PIVOT_STYLE_PROG_FIELDNAME;

            aCellMap[10].aDispName = ScResId( STR_PIVOT_STYLENAME_TOP );
            aCellMap[10].aProgName = SC_PIVOT_STYLE_PROG_TOP;

            //  last entry remains empty

            bCellMapFilled = true;
        }
        return aCellMap;
    }
    else if ( nType == SfxStyleFamily::Page )
    {
        static bool bPageMapFilled = false;
        static ScDisplayNameMap aPageMap[3];
        if ( !bPageMapFilled )
        {
            aPageMap[0].aDispName = ScResId( STR_STYLENAME_STANDARD );
            aPageMap[0].aProgName = SC_STYLE_PROG_STANDARD;

            aPageMap[1].aDispName = ScResId( STR_STYLENAME_REPORT );
            aPageMap[1].aProgName = SC_STYLE_PROG_REPORT;

            //  last entry remains empty

            bPageMapFilled = true;
        }
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
