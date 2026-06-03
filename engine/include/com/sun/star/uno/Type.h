/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#ifndef INCLUDED_COM_SUN_STAR_UNO_TYPE_H
#define INCLUDED_COM_SUN_STAR_UNO_TYPE_H

#include "typelib/typedescription.h"
#include "com/sun/star/uno/TypeClass.hdl"
#include "rtl/ustring.hxx"
#include "rtl/alloc.h"


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** Enum defining UNO_TYPE_NO_ACQUIRE for type description reference transfer.
*/
enum UnoType_NoAcquire
{
    /** This enum value can be used for creating a Type object granting a given type description
        reference, i.e. transferring ownership to it.
    */
    UNO_TYPE_NO_ACQUIRE
};

/** C++ class representing an IDL meta type. This class is used to represent a type,
    i.e. a type name and its type class.
    Internally the type holds a C type description reference of the runtime.
    You can obtain a full type description of a type by calling member function getDescription().

    @see typelib_TypeDescriptionReference
*/
class SAL_WARN_UNUSED SAL_DLLPUBLIC_RTTI Type
{
    /** the C typelib reference pointer
    */
    typelib_TypeDescriptionReference * _pType;

public:
    /** Default Constructor: Type is set to void.
    */
    inline Type();

    /** Constructor: Type is constructed by given name and type class.

        @param eTypeClass type class of type
        @param rTypeName name of type
    */
    inline Type( TypeClass eTypeClass, const ::rtl::OUString & rTypeName );

    /** Constructor: Type is constructed by given name and type class.

        @param eTypeClass type class of type
        @param pTypeName name of type
    */
    inline Type( TypeClass eTypeClass, const char * pTypeName );

    /** Constructor: Type is (copy) constructed by given C type description reference.

        @param pType C type description reference
    */
    inline Type( typelib_TypeDescriptionReference * pType );

    /** Constructor: Type is (copy) constructed by given C type description reference
        without acquiring it.

        @param pType C type description reference
        @param dummy UNO_TYPE_NO_ACQUIRE to force obvious distinction to other constructors
    */
    inline Type( typelib_TypeDescriptionReference * pType, UnoType_NoAcquire dummy );
    /** Constructor: Type is (copy) constructed by given C type description reference
        without acquiring it.

        @param pType C type description reference
        @param dummy SAL_NO_ACQUIRE to force obvious distinction to other constructors
    */
    inline Type( typelib_TypeDescriptionReference * pType, __sal_NoAcquire dummy );

    /** Copy constructor: Type is copy constructed by given type.

        @param rType another type
    */
    inline Type( const Type & rType );

    /** Destructor: Releases acquired C type description reference.
    */
    ~Type()
        { ::typelib_typedescriptionreference_release( _pType ); }

    /** Assignment operator: Acquires right side type and releases previously set type.

        @param rType another type (right side)
        @return this type
    */
    inline Type & SAL_CALL operator = ( const Type & rType );

    inline Type & SAL_CALL operator = ( Type && );

    /** Gets the type class of set type.

        @return type class of set type
    */
    TypeClass SAL_CALL getTypeClass() const
        { return static_cast<TypeClass>(_pType->eTypeClass); }

    /** Gets the name of the set type.

        @return name of the set type
    */
    inline ::rtl::OUString SAL_CALL getTypeName() const;

    /** Obtains a full type description of set type.

        @param ppDescr [inout] type description
    */
    void SAL_CALL getDescription( typelib_TypeDescription ** ppDescr ) const
        { ::typelib_typedescriptionreference_getDescription( ppDescr, _pType ); }

    /** Gets the C typelib type description reference pointer. Does not acquire the reference!

        @return UNacquired type description reference
    */
    typelib_TypeDescriptionReference * SAL_CALL getTypeLibType() const
        { return _pType; }

    /** Tests if values of this reflected type can be assigned by values of given type.
        This includes widening conversion (e.g., long assignable from short), as long as there
        is no data loss.

        @param rType another type
        @return true if values of this type can be assigned from values of given type,
                false otherwise
    */
    bool SAL_CALL isAssignableFrom( const Type & rType ) const
        { return ::typelib_typedescriptionreference_isAssignableFrom( _pType, rType._pType ); }

    /** Compares two types.

        @param rType another type
        @return true if both types refer the same type, false otherwise
    */
    bool SAL_CALL equals( const Type & rType ) const
        { return ::typelib_typedescriptionreference_equals( _pType, rType._pType ); }
    /** Equality operator: Compares two types.

        @param rType another type
        @return true if both types refer the same type, false otherwise
    */
    bool SAL_CALL operator == ( const Type & rType ) const
        { return ::typelib_typedescriptionreference_equals( _pType, rType._pType ); }
    /** Inequality operator: Compares two types.

        @param rType another type
        @return false if both types refer the same type, true otherwise
    */
    bool SAL_CALL operator != ( const Type & rType ) const
        { return (! ::typelib_typedescriptionreference_equals( _pType, rType._pType )); }
};

/** Helper class to specify a type pointer for idl arrays.
*/
template< class T >
class Array
{
public:
    static typelib_TypeDescriptionReference * s_pType;
};

}
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
