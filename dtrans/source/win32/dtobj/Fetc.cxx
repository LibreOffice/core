/*************************************************************************
 *
 *  $RCSfile: Fetc.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-02 14:31:02 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _FETC_HXX_
#include "Fetc.hxx"
#endif

#ifndef _IMPLHELPER_HXX_
#include "..\misc\ImplHelper.hxx"
#endif

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

