/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Fetc.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_dtrans.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
#include <osl/diagnose.h>
#include "Fetc.hxx"
#include "..\misc\ImplHelper.hxx"

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc::CFormatEtc(  )
{
    m_FormatEtc.cfFormat = 0;
    m_FormatEtc.ptd      = NULL;
    m_FormatEtc.dwAspect = 0;
    m_FormatEtc.lindex   = -1;
    m_FormatEtc.tymed    = TYMED_NULL;
}

//------------------------------------------------------------------------
// transfer of ownership
//------------------------------------------------------------------------

CFormatEtc::CFormatEtc( const FORMATETC& aFormatEtc )
{
    CopyFormatEtc( &m_FormatEtc, &const_cast< FORMATETC& >( aFormatEtc ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc::~CFormatEtc( )
{
    DeleteTargetDevice( m_FormatEtc.ptd );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc::CFormatEtc( CLIPFORMAT cf, DWORD tymed, DVTARGETDEVICE* ptd, DWORD dwAspect, LONG lindex )
{
    m_FormatEtc.cfFormat = cf;
    m_FormatEtc.ptd      = CopyTargetDevice( ptd );
    m_FormatEtc.dwAspect = dwAspect;
    m_FormatEtc.lindex   = lindex;
    m_FormatEtc.tymed    = tymed;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc::CFormatEtc( const CFormatEtc& theOther )
{
    m_FormatEtc.cfFormat = theOther.m_FormatEtc.cfFormat;
    m_FormatEtc.ptd      = CopyTargetDevice( theOther.m_FormatEtc.ptd );
    m_FormatEtc.dwAspect = theOther.m_FormatEtc.dwAspect;
    m_FormatEtc.lindex   = theOther.m_FormatEtc.lindex;
    m_FormatEtc.tymed    = theOther.m_FormatEtc.tymed;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc& CFormatEtc::operator=( const CFormatEtc& theOther )
{
    if ( this != &theOther )
    {
        DeleteTargetDevice( m_FormatEtc.ptd );

        m_FormatEtc.cfFormat = theOther.m_FormatEtc.cfFormat;
        m_FormatEtc.ptd      = CopyTargetDevice( theOther.m_FormatEtc.ptd );
        m_FormatEtc.dwAspect = theOther.m_FormatEtc.dwAspect;
        m_FormatEtc.lindex   = theOther.m_FormatEtc.lindex;
        m_FormatEtc.tymed    = theOther.m_FormatEtc.tymed;
    }

    return *this;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc::operator FORMATETC*( )
{
    return &m_FormatEtc;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CFormatEtc::operator FORMATETC( )
{
    return m_FormatEtc;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFormatEtc::getFORMATETC( LPFORMATETC lpFormatEtc )
{
    OSL_ASSERT( lpFormatEtc );
    OSL_ASSERT( !IsBadWritePtr( lpFormatEtc, sizeof( FORMATETC ) ) );

    CopyFormatEtc( lpFormatEtc, &m_FormatEtc );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CLIPFORMAT CFormatEtc::getClipformat( ) const
{
    return m_FormatEtc.cfFormat;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

DWORD CFormatEtc::getTymed( ) const
{
    return m_FormatEtc.tymed;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFormatEtc::getTargetDevice( DVTARGETDEVICE** lpDvTargetDevice ) const
{
    OSL_ASSERT( lpDvTargetDevice );
    OSL_ASSERT( !IsBadWritePtr( lpDvTargetDevice, sizeof( DVTARGETDEVICE ) ) );

    *lpDvTargetDevice = NULL;

    if ( m_FormatEtc.ptd )
        *lpDvTargetDevice = CopyTargetDevice( m_FormatEtc.ptd );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

DWORD CFormatEtc::getDvAspect( ) const
{
    return m_FormatEtc.dwAspect;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

LONG CFormatEtc::getLindex( ) const
{
    return m_FormatEtc.lindex;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFormatEtc::setClipformat( CLIPFORMAT cf )
{
    m_FormatEtc.cfFormat = cf;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFormatEtc::setTymed( DWORD tymed )
{
    m_FormatEtc.tymed = tymed;
}

//------------------------------------------------------------------------
// transfer of ownership!
//------------------------------------------------------------------------

void CFormatEtc::setTargetDevice( DVTARGETDEVICE* ptd )
{
    DeleteTargetDevice( m_FormatEtc.ptd );
    m_FormatEtc.ptd = ptd;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFormatEtc::setDvAspect( DWORD dwAspect )
{
    m_FormatEtc.dwAspect = dwAspect;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void CFormatEtc::setLindex( LONG lindex )
{
    m_FormatEtc.lindex = lindex;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Int32 operator==( const CFormatEtc& lhs, const CFormatEtc& rhs )
{
    return CompareFormatEtc( &lhs.m_FormatEtc, &rhs.m_FormatEtc );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Int32 operator!=( const CFormatEtc& lhs, const CFormatEtc& rhs )
{
    return ( ( lhs == rhs ) != 1 );
}

