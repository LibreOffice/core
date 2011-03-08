/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
//+-------------------------------------------------------------------------
//
//  File:       propspec.hxx
//
//  Contents:   C++ wrapper(s) for FULLPROPSPEC
//
//-------------------------------------------------------------------------
#pragma once
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <ole2.h>
#include <ntquery.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
//+-------------------------------------------------------------------------
//
//  Declare:    CLSID_SummaryInforation, GUID
//              CLSID_Storage, GUID
//
//  Contents:   Definitions of OpenOffice.org Document properties
//
//--------------------------------------------------------------------------

//extern GUID CLSID_Storage;
//
//extern GUID CLSID_SummaryInformation;
//const PID_TITLE     = PIDSI_TITLE; // 2;
//const PID_SUBJECT   = PIDSI_SUBJECT; // 3;
//const PID_AUTHOR    = PIDSI_AUTHOR; // 4;
//const PID_KEYWORDS  = PIDSI_KEYWORDS; // 5;
//const PID_COMMENTS  = PIDSI_COMMENTS; //6;
//const PID_REVNUMBER = PIDSI_REVNUMBER; //9;
//const PID_WORDCOUNT = PIDSI_WORDCOUNT; //f;
//+-------------------------------------------------------------------------
//
//  Class:      CFullPropertySpec
//
//  Purpose:    Describes full (PropertySet\Property) name of a property.
//
//--------------------------------------------------------------------------

class CFullPropSpec
{
public:
    CFullPropSpec();
    CFullPropSpec( GUID const & guidPropSet, PROPID pidProperty );
    CFullPropSpec( GUID const & guidPropSet, WCHAR const * wcsProperty );
    // Validity check
    inline BOOL IsValid() const;

    // Copy constructors/assignment/clone
    CFullPropSpec( CFullPropSpec const & Property );
    CFullPropSpec & operator=( CFullPropSpec const & Property );
    ~CFullPropSpec();
    // Memory allocation
    void * operator new( size_t size );
    inline void * operator new( size_t size, void * p );
    void   operator delete( void * p );
    inline FULLPROPSPEC * CastToStruct();
    inline FULLPROPSPEC const * CastToStruct() const;
    // Comparators
    int operator==( CFullPropSpec const & prop ) const;
    int operator!=( CFullPropSpec const & prop ) const;
    // Member variable access
    inline void SetPropSet( GUID const & guidPropSet );
    inline GUID const & GetPropSet() const;

    void SetProperty( PROPID pidProperty );
    BOOL SetProperty( WCHAR const * wcsProperty );
    inline WCHAR const * GetPropertyName() const;
    inline PROPID GetPropertyPropid() const;
    inline PROPSPEC GetPropSpec() const;
    inline BOOL IsPropertyName() const;
    inline BOOL IsPropertyPropid() const;
private:
    GUID     _guidPropSet;
    PROPSPEC _psProperty;
};
// Inline methods for CFullPropSpec
inline void * CFullPropSpec::operator new( size_t size )
{
    void * p = CoTaskMemAlloc( size );
    return( p );
}
inline void * CFullPropSpec::operator new( size_t /*size*/, void * p )
{
    return( p );
}
inline void CFullPropSpec::operator delete( void * p )
{
    if ( p )
        CoTaskMemFree( p );
}
inline BOOL CFullPropSpec::IsValid() const
{
    return ( _psProperty.ulKind == PRSPEC_PROPID ||
             0 != _psProperty.lpwstr );
}
inline void CFullPropSpec::SetPropSet( GUID const & guidPropSet )
{
    _guidPropSet = guidPropSet;
}
inline GUID const & CFullPropSpec::GetPropSet() const
{
    return( _guidPropSet );
}
inline PROPSPEC CFullPropSpec::GetPropSpec() const
{
    return( _psProperty );
}
inline WCHAR const * CFullPropSpec::GetPropertyName() const
{
    return( _psProperty.lpwstr );
}
inline PROPID CFullPropSpec::GetPropertyPropid() const
{
    return( _psProperty.propid );
}
inline BOOL CFullPropSpec::IsPropertyName() const
{
    return( _psProperty.ulKind == PRSPEC_LPWSTR );
}
inline BOOL CFullPropSpec::IsPropertyPropid() const
{
    return( _psProperty.ulKind == PRSPEC_PROPID );
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
