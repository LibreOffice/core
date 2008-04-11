/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: smartid.cxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <vcl/smartid.hxx>

struct ImplSmartIdData
{
    String aUId;
    ULONG nUId;
    BOOL bHasStringId;
    BOOL bHasNumericId;
};


ImplSmartIdData* SmartId::GetSmartIdData()
{
    if ( !mpData )
    {
        mpData = new ImplSmartIdData;
//        mpData->aUId = "";
        mpData->nUId = 0;
        mpData->bHasStringId = FALSE;
        mpData->bHasNumericId = FALSE;
    }
    return mpData;
}


SmartId::SmartId( const String& rId )
: mpData( NULL )
{
    GetSmartIdData()->aUId = rId;
    GetSmartIdData()->bHasStringId = TRUE;
}

SmartId::SmartId( ULONG nId )
: mpData( NULL )
{
    GetSmartIdData()->nUId = nId;
    GetSmartIdData()->bHasNumericId = TRUE;
}

SmartId::SmartId( const String& rId, ULONG nId )
: mpData( NULL )
{
    GetSmartIdData()->aUId = rId;
    GetSmartIdData()->bHasStringId = TRUE;
    GetSmartIdData()->nUId = nId;
    GetSmartIdData()->bHasNumericId = TRUE;
}

SmartId::SmartId()
: mpData( NULL )
{}

SmartId::SmartId( const SmartId& rId )
: mpData( NULL )
{
    if ( rId.mpData )
    {
        GetSmartIdData();
        mpData->aUId = rId.mpData->aUId;
        mpData->bHasStringId = rId.mpData->bHasStringId;
        mpData->nUId = rId.mpData->nUId;
        mpData->bHasNumericId = rId.mpData->bHasNumericId;
    }
}

SmartId& SmartId::operator = ( const SmartId& rId )
{
    if ( rId.mpData )
        GetSmartIdData();
    else
    {
        delete mpData;
        mpData = NULL;
    }
    if ( mpData && rId.mpData )
    {
        mpData->aUId = rId.mpData->aUId;
        mpData->bHasStringId = rId.mpData->bHasStringId;
        mpData->nUId = rId.mpData->nUId;
        mpData->bHasNumericId = rId.mpData->bHasNumericId;
    }
    return *this;
}

SmartId::~SmartId()
{
    if ( mpData )
        delete mpData;
#ifdef DBG_UTIL
    if ( mpData )
        mpData = (ImplSmartIdData*)0xDeadBeef;
#endif
}

void SmartId::UpdateId( const SmartId& rId, SmartIdUpdateMode aMode )
{
    // Check if ImplData is needed
    if ( aMode != SMART_SET_SMART || ( rId.HasString() || rId.HasNumeric() ) )
        GetSmartIdData();

    if ( aMode == SMART_SET_STR || aMode == SMART_SET_ALL || ( aMode == SMART_SET_SMART && rId.HasString() ) )
    {
        GetSmartIdData()->aUId = rId.GetStr();
        GetSmartIdData()->bHasStringId = rId.HasString();
    }
    if ( aMode == SMART_SET_NUM || aMode == SMART_SET_ALL || ( aMode == SMART_SET_SMART && rId.HasNumeric() ) )
    {
        GetSmartIdData()->nUId = rId.GetNum();
        GetSmartIdData()->bHasNumericId = rId.HasNumeric();
    }

    // remove ImplData when no IDs are set. This is Important because Implementation of Equals() Matches and HasAny relies on it
    if ( mpData && !mpData->bHasStringId && !mpData->bHasNumericId )
    {
        delete mpData;
        mpData = NULL;
    }
}

BOOL SmartId::HasNumeric() const
{
    if ( !mpData )
        return FALSE;
    else
        return mpData->bHasNumericId;
}

BOOL SmartId::HasString() const
{
    if ( !mpData )
        return FALSE;
    else
        return mpData->bHasStringId;
}

BOOL SmartId::HasAny() const
{
    return mpData != NULL;
}

ULONG SmartId::GetNum() const
{
    if ( !mpData )
        return 0;
    else
        return mpData->nUId;
}

String SmartId::GetStr() const
{
    if ( !mpData )
        return String();
    else
        return mpData->aUId;
}


String SmartId::GetText() const   // return String for UI usage
{
    String aRes;
    if ( HasNumeric() )
        aRes = String::CreateFromInt64( GetNum() );
    if ( HasString() )
    {
        if ( HasNumeric() )
            aRes.AppendAscii( "/" );
        aRes.Append( GetStr() );
    }
    return aRes;
}

BOOL SmartId::Matches( const String &rId )const
{
    if ( HasString() )
        return GetStr().EqualsIgnoreCaseAscii( rId );
    else
        return FALSE;
}

BOOL SmartId::Matches( const ULONG nId ) const
{
    if ( HasNumeric() )
        return GetNum() == nId;
    else
        return FALSE;
}

/******************************************************************************
If Both Ids have nither Strings nor Numbers they don't match
If both Ids have Strings the result of Matching these is returned.
Numbers are then Ignored.
Else Matching Numbers is attempted.
******************************************************************************/
BOOL SmartId::Matches( const SmartId &rId ) const
{
    if ( !mpData || !rId.mpData )
        return FALSE;
    else if ( HasString() && rId.HasString() )
        return Matches( rId.GetStr() );
    else
        return rId.HasNumeric() && Matches( rId.GetNum() );
}

BOOL SmartId::Equals( const SmartId &rId ) const
{
    if ( mpData && rId.mpData )
        return mpData->aUId.EqualsIgnoreCaseAscii( rId.mpData->aUId )
            && mpData->bHasStringId == rId.mpData->bHasStringId
            && mpData->nUId == rId.mpData->nUId
            && mpData->bHasNumericId == rId.mpData->bHasNumericId;
    else if ( !mpData && !rId.mpData )
        return TRUE;
    else
        return FALSE;
}

BOOL SmartId::operator == ( const SmartId& rRight ) const
{
    return Equals( rRight );
}

BOOL SmartId::operator <  ( const SmartId& rRight ) const
{
    if ( HasString() && rRight.HasString() && GetStr() != rRight.GetStr() )
        return GetStr() < rRight.GetStr();
    else if ( HasNumeric() && rRight.HasNumeric() && GetNum() != rRight.GetNum() )
        return GetNum() < rRight.GetNum();
    else
    {   // Sort Strings to Front
        if ( HasString() )
            return rRight.HasString() && rRight.HasNumeric();
        else
            return rRight.HasString() || !HasNumeric() && rRight.HasNumeric();
    }
}
