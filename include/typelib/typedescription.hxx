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
#ifndef INCLUDED_TYPELIB_TYPEDESCRIPTION_HXX
#define INCLUDED_TYPELIB_TYPEDESCRIPTION_HXX

#include "sal/config.h"

#include <cstddef>

#include "rtl/alloc.h"
#include "rtl/ustring.hxx"
#include "com/sun/star/uno/Type.h"
#include "typelib/typedescription.h"


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** C++ wrapper for typelib_TypeDescription.
    Constructors by name, type, type description reference will get the full type description.

    @see typelib_TypeDescription
*/
class TypeDescription
{
    /** C typelib type description
    */
    mutable typelib_TypeDescription * _pTypeDescr;

public:
    /// @cond INTERNAL
    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new ( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete ( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * SAL_CALL operator new ( size_t, void * pMem )
        { return pMem; }
    static void SAL_CALL operator delete ( void *, void * )
        {}
    /// @endcond

    /** Constructor:

        @param pTypeDescr a type description
    */
    inline TypeDescription( typelib_TypeDescription * pTypeDescr = NULL );
    /** Constructor:

        @param pTypeDescrRef a type description reference
    */
    inline TypeDescription( typelib_TypeDescriptionReference * pTypeDescrRef );
    /** Constructor:

        @param rType a type
    */
    inline TypeDescription( const css::uno::Type & rType );
    /** Copy constructor:

        @param rDescr another TypeDescription
    */
    inline TypeDescription( const TypeDescription & rDescr );
#if defined LIBO_INTERNAL_ONLY
    TypeDescription(TypeDescription && other): _pTypeDescr(other._pTypeDescr)
    { other._pTypeDescr = nullptr; }
#endif
    /** Constructor:

        @param pTypeName a type name
    */
    inline TypeDescription( rtl_uString * pTypeName );
    /** Constructor:

        @param rTypeName a type name
    */
    inline TypeDescription( const ::rtl::OUString & rTypeName );
    /** Destructor: releases type description
    */
    inline ~TypeDescription();

    /** Assignment operator: acquires given type description and releases a set one.

        @param pTypeDescr another type description
        @return this TypeDescription
    */
    inline TypeDescription & SAL_CALL operator = ( typelib_TypeDescription * pTypeDescr );
    /** Assignment operator: acquires given type description and releases a set one.

        @param rTypeDescr another type description
        @return this TypeDescription
    */
    TypeDescription & SAL_CALL operator =( const TypeDescription & rTypeDescr )
        { return this->operator =( rTypeDescr.get() ); }

#if defined LIBO_INTERNAL_ONLY
    TypeDescription & operator =(TypeDescription && other) {
        if (_pTypeDescr != nullptr) {
            typelib_typedescription_release(_pTypeDescr);
        }
        _pTypeDescr = other._pTypeDescr;
        other._pTypeDescr = nullptr;
        return *this;
    }
#endif

    /** Tests whether two type descriptions are equal.

        @param pTypeDescr another type description
        @return true, if both type descriptions are equal, false otherwise
    */
    inline bool SAL_CALL equals( const typelib_TypeDescription * pTypeDescr ) const;
    /** Tests whether two type descriptions are equal.

        @param rTypeDescr another type description
        @return true, if both type descriptions are equal, false otherwise
    */
    bool SAL_CALL equals( const TypeDescription & rTypeDescr ) const
        { return equals( rTypeDescr._pTypeDescr ); }

    /** Makes stored type description complete.
    */
    inline void SAL_CALL makeComplete() const;

    /** Gets the UNacquired type description pointer.

        @return stored pointer of type description
    */
    typelib_TypeDescription * SAL_CALL get() const
        { return _pTypeDescr; }
    /** Tests if a type description is set.

        @return true, if a type description is set, false otherwise
    */
    bool SAL_CALL is() const
        { return (_pTypeDescr != NULL); }
};

inline TypeDescription::TypeDescription( typelib_TypeDescription * pTypeDescr )
    : _pTypeDescr( pTypeDescr )
{
    if (_pTypeDescr)
        typelib_typedescription_acquire( _pTypeDescr );
}

inline TypeDescription::TypeDescription( typelib_TypeDescriptionReference * pTypeDescrRef )
    : _pTypeDescr( NULL )
{
    if (pTypeDescrRef)
        typelib_typedescriptionreference_getDescription( &_pTypeDescr, pTypeDescrRef );
}

inline TypeDescription::TypeDescription( const css::uno::Type & rType )
    : _pTypeDescr( NULL )
{
    if (rType.getTypeLibType())
        typelib_typedescriptionreference_getDescription( &_pTypeDescr, rType.getTypeLibType() );
}

inline TypeDescription::TypeDescription( const TypeDescription & rTypeDescr )
    : _pTypeDescr( rTypeDescr._pTypeDescr )
{
    if (_pTypeDescr)
        typelib_typedescription_acquire( _pTypeDescr );
}

inline TypeDescription::TypeDescription( rtl_uString * pTypeName )
    : _pTypeDescr( NULL )
{
    typelib_typedescription_getByName( &_pTypeDescr , pTypeName );
}

inline TypeDescription::TypeDescription( const ::rtl::OUString & rTypeName )
    : _pTypeDescr( NULL )
{
    typelib_typedescription_getByName( &_pTypeDescr , rTypeName.pData );
}

inline TypeDescription::~TypeDescription()
{
    if (_pTypeDescr)
        typelib_typedescription_release( _pTypeDescr );
}

inline TypeDescription & TypeDescription::operator = ( typelib_TypeDescription * pTypeDescr )
{
    if (pTypeDescr)
        typelib_typedescription_acquire( pTypeDescr );
    if (_pTypeDescr)
        typelib_typedescription_release( _pTypeDescr );
    _pTypeDescr = pTypeDescr;
    return *this;
}

inline bool TypeDescription::equals( const typelib_TypeDescription * pTypeDescr ) const
{
    return (_pTypeDescr && pTypeDescr &&
            typelib_typedescription_equals( _pTypeDescr, pTypeDescr ));
}

inline void TypeDescription::makeComplete() const
{
    if (_pTypeDescr && !_pTypeDescr->bComplete)
        ::typelib_typedescription_complete( &_pTypeDescr );
}

}
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
