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

#include <rsc/rscsfx.hxx>

#include "stylehelper.hxx"
#include "global.hxx"
#include "globstr.hrc"

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

struct ScDisplayNameMap
{
    OUString  aDispName;
    OUString  aProgName;
};

static const ScDisplayNameMap* lcl_GetStyleNameMap( sal_uInt16 nType )
{
    if ( nType == SFX_STYLE_FAMILY_PARA )
    {
        static bool bCellMapFilled = false;
        static ScDisplayNameMap aCellMap[6];
        if ( !bCellMapFilled )
        {
            aCellMap[0].aDispName = ScGlobal::GetRscString( STR_STYLENAME_STANDARD );
            aCellMap[0].aProgName = OUString( SC_STYLE_PROG_STANDARD );

            aCellMap[1].aDispName = ScGlobal::GetRscString( STR_STYLENAME_RESULT );
            aCellMap[1].aProgName = OUString( SC_STYLE_PROG_RESULT );

            aCellMap[2].aDispName = ScGlobal::GetRscString( STR_STYLENAME_RESULT1 );
            aCellMap[2].aProgName = OUString( SC_STYLE_PROG_RESULT1 );

            aCellMap[3].aDispName = ScGlobal::GetRscString( STR_STYLENAME_HEADLINE );
            aCellMap[3].aProgName = OUString( SC_STYLE_PROG_HEADLINE );

            aCellMap[4].aDispName = ScGlobal::GetRscString( STR_STYLENAME_HEADLINE1 );
            aCellMap[4].aProgName = OUString( SC_STYLE_PROG_HEADLINE1 );

            //  last entry remains empty

            bCellMapFilled = true;
        }
        return aCellMap;
    }
    else if ( nType == SFX_STYLE_FAMILY_PAGE )
    {
        static bool bPageMapFilled = false;
        static ScDisplayNameMap aPageMap[3];
        if ( !bPageMapFilled )
        {
            aPageMap[0].aDispName = ScGlobal::GetRscString( STR_STYLENAME_STANDARD );
            aPageMap[0].aProgName = OUString( SC_STYLE_PROG_STANDARD );

            aPageMap[1].aDispName = ScGlobal::GetRscString( STR_STYLENAME_REPORT );
            aPageMap[1].aProgName = OUString( SC_STYLE_PROG_REPORT );

            //  last entry remains empty

            bPageMapFilled = true;
        }
        return aPageMap;
    }
    OSL_FAIL("invalid family");
    return NULL;
}

//  programmatic name suffix for display names that match other programmatic names
//  is " (user)" including a space

#define SC_SUFFIX_USER      " (user)"
#define SC_SUFFIX_USER_LEN  7

static bool lcl_EndsWithUser( const OUString& rString )
{
    return rString.endsWith(SC_SUFFIX_USER);
}

OUString ScStyleNameConversion::DisplayToProgrammaticName( const OUString& rDispName, sal_uInt16 nType )
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

OUString ScStyleNameConversion::ProgrammaticToDisplayName( const OUString& rProgName, sal_uInt16 nType )
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
