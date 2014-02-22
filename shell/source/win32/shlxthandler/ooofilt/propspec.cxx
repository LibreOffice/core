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


//+-------------------------------------------------------------------------
//  File:       propspec.cxx
//  Contents:   C++ wrappers for FULLPROPSPEC

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#ifdef _MSC_VER
#pragma warning(disable: 4512)
#endif
#include <filter.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include "propspec.hxx"

//refer to ms-help://MS.VSCC/MS.MSDNVS.2052/com/stgasstg_7agk.htm
//FMTID_SummaryInformation
//GUID CLSID_SummaryInforation = {
//    0xF29F85E0,
//    0x4FF9,
//    0x1068,
//    { 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9 }
//};
//+-------------------------------------------------------------------------
//  Member:     CFullPropSpec::CFullPropSpec, public
//  Synopsis:   Default constructor
//  Effects:    Defines property with null guid and propid 0


CFullPropSpec::CFullPropSpec()
{
    memset( &_guidPropSet, 0, sizeof( _guidPropSet ) );
    _psProperty.ulKind = PRSPEC_PROPID;
    _psProperty.propid = 0;
}
//+-------------------------------------------------------------------------
//  Member:     CFullPropSpec::CFullPropSpec, public
//  Synopsis:   Construct propid based propspec
//  Arguments:  [guidPropSet]  -- Property set
//              [pidProperty] -- Property

CFullPropSpec::CFullPropSpec( GUID const & guidPropSet, PROPID pidProperty ) :
    _guidPropSet( guidPropSet )
{
    _psProperty.ulKind = PRSPEC_PROPID;
    _psProperty.propid = pidProperty;
}
//+-------------------------------------------------------------------------
//  Member:     CFullPropSpec::CFullPropSpec, public
//  Synopsis:   Construct name based propspec
//  Arguments:  [guidPropSet] -- Property set
//              [wcsProperty] -- Property

CFullPropSpec::CFullPropSpec( GUID const & guidPropSet,
                              WCHAR const * wcsProperty ) :
    _guidPropSet( guidPropSet )
{
    _psProperty.ulKind = PRSPEC_PROPID;
    SetProperty( wcsProperty );
}
//+-------------------------------------------------------------------------
//  Member:     CFullPropSpec::CFullPropSpec, public
//  Synopsis:   Copy constructor
//  Arguments:  [src] -- Source property spec

CFullPropSpec::CFullPropSpec( CFullPropSpec const & src ) :
    _guidPropSet( src._guidPropSet )
{
    _psProperty.ulKind = src._psProperty.ulKind;
    if ( _psProperty.ulKind == PRSPEC_LPWSTR )
    {
        if ( src._psProperty.lpwstr )
        {
            _psProperty.ulKind = PRSPEC_PROPID;
            SetProperty( src._psProperty.lpwstr );
        }
        else
            _psProperty.lpwstr = 0;
    }
    else
    {
        _psProperty.propid = src._psProperty.propid;
    }
}
inline void * operator new( size_t /*size*/, void * p )
{
    return( p );
}
//+-------------------------------------------------------------------------
//  Member:     CFullPropSpec::operator=, public
//  Synopsis:   Assignment operator
//  Arguments:  [Property] -- Source property

CFullPropSpec & CFullPropSpec::operator=( CFullPropSpec const & Property )
{
    // Clean up.
    this->CFullPropSpec::~CFullPropSpec();

#ifdef _MSC_VER
#pragma warning( disable : 4291 )           // unmatched operator new
#endif
    new (this) CFullPropSpec( Property );
#ifdef _MSC_VER
#pragma warning( default : 4291 )
#endif
    return *this;
}
CFullPropSpec::~CFullPropSpec()
{
    if ( _psProperty.ulKind == PRSPEC_LPWSTR &&
         _psProperty.lpwstr )
    {
        CoTaskMemFree( _psProperty.lpwstr );
    }
}
void CFullPropSpec::SetProperty( PROPID pidProperty )
{
    if ( _psProperty.ulKind == PRSPEC_LPWSTR &&
         0 != _psProperty.lpwstr )
    {
        CoTaskMemFree( _psProperty.lpwstr );
    }
    _psProperty.ulKind = PRSPEC_PROPID;
    _psProperty.propid = pidProperty;
}
BOOL CFullPropSpec::SetProperty( WCHAR const * wcsProperty )
{
    if ( _psProperty.ulKind == PRSPEC_LPWSTR &&
         0 != _psProperty.lpwstr )
    {
        CoTaskMemFree( _psProperty.lpwstr );
    }
    _psProperty.ulKind = PRSPEC_LPWSTR;
    int len = (int) ( (wcslen( wcsProperty ) + 1) * sizeof( WCHAR ) );
    _psProperty.lpwstr = (WCHAR *)CoTaskMemAlloc( len );
    if ( 0 != _psProperty.lpwstr )
    {
        memcpy( _psProperty.lpwstr,
                wcsProperty,
                len );
        return( TRUE );
    }
    else
    {
        _psProperty.lpwstr = 0;
        return( FALSE );
    }
}
int CFullPropSpec::operator==( CFullPropSpec const & prop ) const
{
    if ( memcmp( &prop._guidPropSet,
                 &_guidPropSet,
                 sizeof( _guidPropSet ) ) != 0 ||
         prop._psProperty.ulKind != _psProperty.ulKind )
    {
        return( 0 );
    }
    switch( _psProperty.ulKind )
    {
    case PRSPEC_LPWSTR:
        return( _wcsicmp( GetPropertyName(), prop.GetPropertyName() ) == 0 );
        break;
    case PRSPEC_PROPID:
        return( GetPropertyPropid() == prop.GetPropertyPropid() );
        break;
    default:
        return( 0 );
        break;
    }
}
int CFullPropSpec::operator!=( CFullPropSpec const & prop ) const
{
    if (*this == prop)
        return( 0 );
    else
        return( 1 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
