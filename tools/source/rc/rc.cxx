/*************************************************************************
 *
 *  $RCSfile: rc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _TOOLS_RC_CXX

#include <string.h>

#ifndef _DATE_HXX
#include <date.hxx>
#endif
#ifndef _TIME_HXX
#include <time.hxx>
#endif
#ifndef _INTN_HXX
#include <intn.hxx>
#endif

#ifndef _TOOLS_RC_HXX
#include <rc.hxx>
#endif
#ifndef _TOOLS_RCID_H
#include <rcid.h>
#endif

#pragma hdrstop

// =======================================================================

ImplSVResourceData aResData;

inline ImplSVResourceData* GetResData()
{
    //return &ImplGetSVData()->maResourceData;
    return &aResData;
}

// =======================================================================

Resource::Resource( const ResId& rResId )
{
    GetRes( rResId.SetRT( RSC_RESOURCE ) );
}

// -----------------------------------------------------------------------

void Resource::GetRes( const ResId& rResId )
{
    if ( rResId.GetResMgr() )
        rResId.GetResMgr()->GetResource( rResId, this );
    else
        GetResManager()->GetResource( rResId, this );

    IncrementRes( sizeof( RSHEADER_TYPE ) );
}

// -----------------------------------------------------------------------

void Resource::TestRes()
{
    ImplSVResourceData* pSVInData = GetResData();
    if( pSVInData->pAppResMgr )
        GetResManager()->TestStack( this );
}

// -----------------------------------------------------------------------

void Resource::SetResManager( ResMgr* pNewResMgr )
{
    ImplSVResourceData * pSVInData = GetResData();
    pSVInData->pAppResMgr = pNewResMgr;
}

// -----------------------------------------------------------------------

ResMgr* Resource::GetResManager()
{
    ImplSVResourceData* pSVInData = GetResData();

    return pSVInData->pAppResMgr;
}

// =======================================================================

#ifndef ENABLEUNICODE

String::String( const ResId& rResId )
{
    rResId.SetRT( RSC_STRING );
    ResMgr* pResMgr = rResId.GetResMgr();
    if ( !pResMgr )
        pResMgr = Resource::GetResManager();

    if ( pResMgr->GetResource( rResId ) )
    {
        // String laden
        RSHEADER_TYPE * pResHdr = (RSHEADER_TYPE*)pResMgr->GetClass();
        USHORT nLen = pResHdr->GetLocalOff() - sizeof( RSHEADER_TYPE );

        USHORT nStringLen = strlen( (char*)(pResHdr+1) );
        UniString aWString( (const char*)(pResHdr+1), RTL_TEXTENCODING_UTF8,
                          RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |
                          RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                          RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT );
        InitStringRes( aWString );

        USHORT nSize = sizeof( RSHEADER_TYPE ) + nStringLen + 1;
        nSize += nSize % 2;
        pResMgr->Increment( nSize );
    }
}

#endif

// =======================================================================

UniString::UniString( const ResId& rResId )
{
    rResId.SetRT( RSC_STRING );
    ResMgr* pResMgr = rResId.GetResMgr();
    if ( !pResMgr )
        pResMgr = Resource::GetResManager();

    if ( pResMgr->GetResource( rResId ) )
    {
        // String laden
        RSHEADER_TYPE * pResHdr = (RSHEADER_TYPE*)pResMgr->GetClass();
        USHORT nLen = pResHdr->GetLocalOff() - sizeof( RSHEADER_TYPE );

        USHORT nStringLen = strlen( (char*)(pResHdr+1) );
        InitStringRes( (const char*)(pResHdr+1), nStringLen );

        USHORT nSize = sizeof( RSHEADER_TYPE ) + nStringLen + 1;
        nSize += nSize % 2;
        pResMgr->Increment( nSize );
    }
}

// =======================================================================

Time::Time( const ResId& rResId )
{
    nTime = 0;
    rResId.SetRT( RSC_TIME );
    ResMgr* pResMgr = NULL;

    ResMgr::GetResourceSkipHeader( rResId, &pResMgr );

    USHORT nObjMask = (USHORT)pResMgr->ReadShort();

    if ( 0x01 & nObjMask )
        SetHour( (USHORT)pResMgr->ReadShort() );
    if ( 0x02 & nObjMask )
        SetMin( (USHORT)pResMgr->ReadShort() );
    if ( 0x04 & nObjMask )
        SetSec( (USHORT)pResMgr->ReadShort() );
    if ( 0x08 & nObjMask )
        Set100Sec( (USHORT)pResMgr->ReadShort() );
}

// =======================================================================

Date::Date( const ResId& rResId )
{
    rResId.SetRT( RSC_DATE );
    ResMgr* pResMgr = NULL;

    ResMgr::GetResourceSkipHeader( rResId, &pResMgr );

    USHORT nObjMask = (USHORT)pResMgr->ReadShort();

    if ( 0x01 & nObjMask )
        SetYear( (USHORT)pResMgr->ReadShort() );
    if ( 0x02 & nObjMask )
        SetMonth( (USHORT)pResMgr->ReadShort() );
    if ( 0x04 & nObjMask )
        SetDay( (USHORT)pResMgr->ReadShort() );
}

// =======================================================================

International::International( const ResId& rResId )
{
    rResId.SetRT( RSC_INTERNATIONAL );
    ResMgr* pResMgr = NULL;

    ResMgr::GetResourceSkipHeader( rResId, &pResMgr );

    USHORT nObjMask = (USHORT)pResMgr->ReadShort();

    LanguageType eLangType = LANGUAGE_SYSTEM;
    LanguageType eFormatType = LANGUAGE_SYSTEM;

    if ( 0x0001 & nObjMask )
    {
            eLangType = (LanguageType)(USHORT)pResMgr->ReadShort();
            eFormatType = eLangType;
    }
    if ( 0x0002 & nObjMask )
        eFormatType = (LanguageType)(USHORT)pResMgr->ReadShort();
    Init( eLangType, eFormatType );

    if ( 0x0004 & nObjMask )
        SetDateFormat( (DateFormat)(USHORT)pResMgr->ReadShort() );
    if ( 0x0008 & nObjMask )
            SetDateDayLeadingZero( (BOOL)(USHORT)pResMgr->ReadShort() );
    if ( 0x0010 & nObjMask )
        SetDateMonthLeadingZero( (BOOL)(USHORT)pResMgr->ReadShort() );
    if ( 0x0020 & nObjMask )
        SetDateCentury( (BOOL)(USHORT)pResMgr->ReadShort() );
    if ( 0x0040 & nObjMask )
        SetLongDateFormat( (DateFormat)(USHORT)pResMgr->ReadShort() );
    if ( 0x0080 & nObjMask )
            SetLongDateDayOfWeekFormat( (DayOfWeekFormat)(USHORT)pResMgr->ReadShort() );
    if ( 0x0100 & nObjMask )
        SetLongDateDayOfWeekSep( pResMgr->ReadString() );
    if ( 0x0200 & nObjMask )
        SetLongDateDayLeadingZero( (BOOL)(USHORT)pResMgr->ReadShort() );
    if ( 0x0400 & nObjMask )
        SetLongDateDaySep( pResMgr->ReadString() );
    if ( 0x0800 & nObjMask )
        SetLongDateMonthFormat( (MonthFormat)(USHORT)pResMgr->ReadShort() );
    if ( 0x1000 & nObjMask )
        SetLongDateMonthSep( pResMgr->ReadString() );
    if ( 0x2000 & nObjMask )
        SetLongDateCentury( (BOOL)(USHORT)pResMgr->ReadShort() );
    if ( 0x4000 & nObjMask )
        SetLongDateYearSep( pResMgr->ReadString() );
    if ( 0x8000 & nObjMask )
        SetTimeFormat( (TimeFormat)(USHORT)pResMgr->ReadShort() );

    // Zweite Maske holen
    nObjMask = (USHORT)pResMgr->ReadShort();
    if ( 0x0001 & nObjMask )
        SetTimeLeadingZero( (BOOL)(USHORT)pResMgr->ReadShort() );
    if ( 0x0002 & nObjMask )
        SetTimeAM( pResMgr->ReadString() );
    if ( 0x0004 & nObjMask )
        SetTimePM( pResMgr->ReadString() );
    if ( 0x0008 & nObjMask )
        SetNumLeadingZero( (BOOL)(USHORT)pResMgr->ReadShort() );
    if ( 0x0010 & nObjMask )
        SetNumDigits( (USHORT)pResMgr->ReadShort() );
    if ( 0x0020 & nObjMask )
        SetCurrPositiveFormat( (USHORT)pResMgr->ReadShort() );
    if ( 0x0040 & nObjMask )
        SetCurrNegativeFormat( (USHORT)pResMgr->ReadShort() );
    if ( 0x0080 & nObjMask )
        SetCurrDigits( (USHORT)pResMgr->ReadShort() );
    if ( 0x0100 & nObjMask )
        SetNumTrailingZeros( (BOOL)(USHORT)pResMgr->ReadShort() );
    if ( 0x0200 & nObjMask )
        SetMeasurementSystem( (MeasurementSystem)(USHORT)pResMgr->ReadShort() );
}
