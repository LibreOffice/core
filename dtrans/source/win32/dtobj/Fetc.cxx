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

#include <osl/diagnose.h>
#include "Fetc.hxx"
#include "../misc/ImplHelper.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
