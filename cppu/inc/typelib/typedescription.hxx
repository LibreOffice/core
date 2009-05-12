/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: typedescription.hxx,v $
 * $Revision: 1.9 $
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
#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#define _TYPELIB_TYPEDESCRIPTION_HXX_

#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Type.h>
#ifndef _TYPELIB_TYPEDESCRIPTION_H
#include <typelib/typedescription.h>
#endif


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
    // these are here to force memory de/allocation to sal lib.
    /** @internal */
    inline static void * SAL_CALL operator new ( size_t nSize ) SAL_THROW( () )
        { return ::rtl_allocateMemory( nSize ); }
    /** @internal */
    inline static void SAL_CALL operator delete ( void * pMem ) SAL_THROW( () )
        { ::rtl_freeMemory( pMem ); }
    /** @internal */
    inline static void * SAL_CALL operator new ( size_t, void * pMem ) SAL_THROW( () )
        { return pMem; }
    /** @internal */
    inline static void SAL_CALL operator delete ( void *, void * ) SAL_THROW( () )
        {}

    /** Constructor:

        @param pTypeDescr a type description
    */
    inline TypeDescription( typelib_TypeDescription * pTypeDescr = 0 ) SAL_THROW( () );
    /** Constructor:

        @param pTypeDescrRef a type description reference
    */
    inline TypeDescription( typelib_TypeDescriptionReference * pTypeDescrRef ) SAL_THROW( () );
    /** Constructor:

        @param rType a type
    */
    inline TypeDescription( const ::com::sun::star::uno::Type & rType ) SAL_THROW( () );
    /** Copy constructor:

        @param rDescr another TypeDescription
    */
    inline TypeDescription( const TypeDescription & rDescr ) SAL_THROW( () );
    /** Constructor:

        @param pTypeName a type name
    */
    inline TypeDescription( rtl_uString * pTypeName ) SAL_THROW( () );
    /** Constructor:

        @param rTypeName a type name
    */
    inline TypeDescription( const ::rtl::OUString & rTypeName ) SAL_THROW( () );
    /** Destructor: releases type description
    */
    inline ~TypeDescription() SAL_THROW( () );

    /** Assignment operator: acquires given type description and releases a set one.

        @param pTypeDescr another type description
        @return this TypeDescription
    */
    inline TypeDescription & SAL_CALL operator = ( typelib_TypeDescription * pTypeDescr ) SAL_THROW( () );
    /** Assignment operator: acquires given type description and releases a set one.

        @param rTypeDescr another type description
        @return this TypeDescription
    */
    inline TypeDescription & SAL_CALL operator =( const TypeDescription & rTypeDescr ) SAL_THROW( () )
        { return this->operator =( rTypeDescr.get() ); }

    /** Tests whether two type descriptions are equal.

        @param pTypeDescr another type description
        @return true, if both type descriptions are equal, false otherwise
    */
    inline sal_Bool SAL_CALL equals( const typelib_TypeDescription * pTypeDescr ) const SAL_THROW( () );
    /** Tests whether two type descriptions are equal.

        @param rTypeDescr another type description
        @return true, if both type descriptions are equal, false otherwise
    */
    inline sal_Bool SAL_CALL equals( const TypeDescription & rTypeDescr ) const SAL_THROW( () )
        { return equals( rTypeDescr._pTypeDescr ); }

    /** Makes stored type description complete.
    */
    inline void SAL_CALL makeComplete() const SAL_THROW( () );

    /** Gets the UNacquired type description pointer.

        @return stored pointer of type description
    */
    inline typelib_TypeDescription * SAL_CALL get() const SAL_THROW( () )
        { return _pTypeDescr; }
    /** Tests if a type description is set.

        @return true, if a type description is set, false otherwise
    */
    inline sal_Bool SAL_CALL is() const SAL_THROW( () )
        { return (_pTypeDescr != 0); }
};
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( typelib_TypeDescription * pTypeDescr ) SAL_THROW( () )
    : _pTypeDescr( pTypeDescr )
{
    if (_pTypeDescr)
        typelib_typedescription_acquire( _pTypeDescr );
}
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( typelib_TypeDescriptionReference * pTypeDescrRef ) SAL_THROW( () )
    : _pTypeDescr( 0 )
{
    if (pTypeDescrRef)
        typelib_typedescriptionreference_getDescription( &_pTypeDescr, pTypeDescrRef );
}
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( const ::com::sun::star::uno::Type & rType ) SAL_THROW( () )
    : _pTypeDescr( 0 )
{
    if (rType.getTypeLibType())
        typelib_typedescriptionreference_getDescription( &_pTypeDescr, rType.getTypeLibType() );
}
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( const TypeDescription & rTypeDescr ) SAL_THROW( () )
    : _pTypeDescr( rTypeDescr._pTypeDescr )
{
    if (_pTypeDescr)
        typelib_typedescription_acquire( _pTypeDescr );
}
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( rtl_uString * pTypeName ) SAL_THROW( () )
    : _pTypeDescr( 0 )
{
    typelib_typedescription_getByName( &_pTypeDescr , pTypeName );
}
//__________________________________________________________________________________________________
inline TypeDescription::TypeDescription( const ::rtl::OUString & rTypeName ) SAL_THROW( () )
    : _pTypeDescr( 0 )
{
    typelib_typedescription_getByName( &_pTypeDescr , rTypeName.pData );
}
//__________________________________________________________________________________________________
inline TypeDescription::~TypeDescription() SAL_THROW( () )
{
    if (_pTypeDescr)
        typelib_typedescription_release( _pTypeDescr );
}
//__________________________________________________________________________________________________
inline TypeDescription & TypeDescription::operator = ( typelib_TypeDescription * pTypeDescr ) SAL_THROW( () )
{
    if (pTypeDescr)
        typelib_typedescription_acquire( pTypeDescr );
    if (_pTypeDescr)
        typelib_typedescription_release( _pTypeDescr );
    _pTypeDescr = pTypeDescr;
    return *this;
}
//__________________________________________________________________________________________________
inline sal_Bool TypeDescription::equals( const typelib_TypeDescription * pTypeDescr ) const SAL_THROW( () )
{
    return (_pTypeDescr && pTypeDescr &&
            typelib_typedescription_equals( _pTypeDescr, pTypeDescr ));
}
//__________________________________________________________________________________________________
inline void TypeDescription::makeComplete() const SAL_THROW( () )
{
    if (_pTypeDescr && !_pTypeDescr->bComplete)
        ::typelib_typedescription_complete( &_pTypeDescr );
}

}
}
}
}

#endif
