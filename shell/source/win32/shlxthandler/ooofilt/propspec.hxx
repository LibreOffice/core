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

#ifndef INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_OOOFILT_PROPSPEC_HXX
#define INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_OOOFILT_PROPSPEC_HXX

//+-------------------------------------------------------------------------
//  File:       propspec.hxx
//  Contents:   C++ wrapper(s) for FULLPROPSPEC

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <ole2.h>
#include <ntquery.h>
//+-------------------------------------------------------------------------
//  Declare:    CLSID_SummaryInformation, GUID
//              CLSID_Storage, GUID
//  Contents:   Definitions of OpenOffice.org Document properties


//+-------------------------------------------------------------------------
//  Class:      CFullPropertySpec
//  Purpose:    Describes full (PropertySet\Property) name of a property.


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
    void   operator delete( void * p );
    inline FULLPROPSPEC * CastToStruct();
    inline FULLPROPSPEC const * CastToStruct() const;
    // Comparators
    bool operator==( CFullPropSpec const & prop ) const;
    bool operator!=( CFullPropSpec const & prop ) const;
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
    GUID     _guidPropSet = {};
    PROPSPEC _psProperty;
};
// Inline methods for CFullPropSpec
inline void * CFullPropSpec::operator new( size_t size )
{
    void * p = CoTaskMemAlloc( size );
    return p;
}
inline void CFullPropSpec::operator delete( void * p )
{
    if ( p )
        CoTaskMemFree( p );
}
inline BOOL CFullPropSpec::IsValid() const
{
    return ( _psProperty.ulKind == PRSPEC_PROPID ||
             nullptr != _psProperty.lpwstr );
}
inline void CFullPropSpec::SetPropSet( GUID const & guidPropSet )
{
    _guidPropSet = guidPropSet;
}
inline GUID const & CFullPropSpec::GetPropSet() const
{
    return _guidPropSet;
}
inline PROPSPEC CFullPropSpec::GetPropSpec() const
{
    return _psProperty;
}
inline WCHAR const * CFullPropSpec::GetPropertyName() const
{
    return _psProperty.lpwstr;
}
inline PROPID CFullPropSpec::GetPropertyPropid() const
{
    return _psProperty.propid;
}
inline BOOL CFullPropSpec::IsPropertyName() const
{
    return _psProperty.ulKind == PRSPEC_LPWSTR;
}
inline BOOL CFullPropSpec::IsPropertyPropid() const
{
    return _psProperty.ulKind == PRSPEC_PROPID;
}

#endif // INCLUDED_SHELL_SOURCE_WIN32_SHLXTHANDLER_OOOFILT_PROPSPEC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
