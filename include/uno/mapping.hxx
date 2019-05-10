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
#ifndef INCLUDED_UNO_MAPPING_HXX
#define INCLUDED_UNO_MAPPING_HXX

#include "sal/config.h"

#include <cstddef>

#include "uno/lbnames.h"
#include "rtl/alloc.h"
#include "rtl/ustring.hxx"
#include "osl/diagnose.h"
#include "uno/mapping.h"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "cppu/unotype.hxx"
#include "uno/environment.hxx"

typedef struct _typelib_TypeDescription typelib_TypeDescription;
typedef struct _typelib_InterfaceTypeDescription typelib_InterfaceTypeDescription;
typedef struct _uno_Interface uno_Interface;

namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** C++ wrapper for C uno_Mapping.

    @see uno_Mapping
*/
class Mapping
{
    uno_Mapping * _pMapping;

public:
    // these are here to force memory de/allocation to sal lib.
    /// @cond INTERNAL
    static void * SAL_CALL operator new ( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete ( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * SAL_CALL operator new ( size_t, void * pMem )
        { return pMem; }
    static void SAL_CALL operator delete ( void *, void * )
        {}
    /// @endcond

    /** Holds a mapping from the specified source to the specified destination by environment
        type names.

        @param rFrom        type name of source environment
        @param rTo          type name of destination environment
        @param rAddPurpose  additional purpose
    */
    inline Mapping(
        const ::rtl::OUString & rFrom, const ::rtl::OUString & rTo,
        const ::rtl::OUString & rAddPurpose = ::rtl::OUString() );

    /** Holds a mapping from the specified source to the specified destination.

        @param pFrom        source environment
        @param pTo          destination environment
        @param rAddPurpose  additional purpose
    */
    inline Mapping(
        uno_Environment * pFrom, uno_Environment * pTo,
        const ::rtl::OUString & rAddPurpose = ::rtl::OUString() );

    /** Holds a mapping from the specified source to the specified destination
        environment.

        @param rFrom         source environment
        @param rTo           destination environment
        @param rAddPurpose  additional purpose
    */
    inline Mapping(const Environment & rFrom, const Environment & rTo,
                   const ::rtl::OUString & rAddPurpose = ::rtl::OUString() );

    /** Constructor.

        @param pMapping another mapping
    */
    inline Mapping( uno_Mapping * pMapping = NULL );

    /** Copy constructor.

        @param rMapping another mapping
    */
    inline Mapping( const Mapping & rMapping );

#if defined LIBO_INTERNAL_ONLY
    Mapping(Mapping && other): _pMapping(other._pMapping)
    { other._pMapping = nullptr; }
#endif

    /** Destructor.
    */
    inline ~Mapping();

    /** Sets a given mapping.

        @param pMapping another mapping
        @return this mapping
    */
    inline Mapping & SAL_CALL operator = ( uno_Mapping * pMapping );
    /** Sets a given mapping.

        @param rMapping another mapping
        @return this mapping
    */
    Mapping & SAL_CALL operator = ( const Mapping & rMapping )
        { return operator = ( rMapping._pMapping ); }

#if defined LIBO_INTERNAL_ONLY
    Mapping & operator =(Mapping && other) {
        if (_pMapping != nullptr) {
            (*_pMapping->release)(_pMapping);
        }
        _pMapping = other._pMapping;
        other._pMapping = nullptr;
        return *this;
    }
#endif

    /** Provides a pointer to the C mapping. The returned mapping is NOT acquired!

        @return UNacquired C mapping
    */
    uno_Mapping * SAL_CALL get() const
        { return _pMapping; }

    /** Tests if a mapping is set.

        @return true if a mapping is set
    */
    bool SAL_CALL is() const
        { return (_pMapping != NULL); }

    /** Releases a set mapping.
    */
    inline void SAL_CALL clear();

    /** Maps an interface from one environment to another.

        @param pInterface       source interface
        @param pTypeDescr       type description of interface
        @return                 mapped interface
    */
    inline void * SAL_CALL mapInterface( void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr ) const;
    /** Maps an interface from one environment to another.

        @param pInterface       source interface
        @param pTypeDescr       type description of interface
        @return                 mapped interface
    */
    void * SAL_CALL mapInterface( void * pInterface, typelib_TypeDescription * pTypeDescr ) const
        { return mapInterface( pInterface, reinterpret_cast<typelib_InterfaceTypeDescription *>(pTypeDescr) ); }

    /** Maps an interface from one environment to another.

        @param pInterface       source interface
        @param rType            type of interface
        @return                 mapped interface
    */
    inline void * SAL_CALL mapInterface(
        void * pInterface, const css::uno::Type & rType ) const;

    /** Maps an interface from one environment to another.

        @param ppOut            inout mapped interface
        @param pInterface       source interface
        @param pTypeDescr       type description of interface
    */
    void SAL_CALL mapInterface( void ** ppOut, void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr ) const
        { (*_pMapping->mapInterface)( _pMapping, ppOut, pInterface, pTypeDescr ); }
    /** Maps an interface from one environment to another.

        @param ppOut            inout mapped interface
        @param pInterface       source interface
        @param pTypeDescr       type description of interface
    */
    void SAL_CALL mapInterface( void ** ppOut, void * pInterface, typelib_TypeDescription * pTypeDescr ) const
        { (*_pMapping->mapInterface)( _pMapping, ppOut, pInterface, reinterpret_cast<typelib_InterfaceTypeDescription *>(pTypeDescr) ); }

    /** Maps an interface from one environment to another.

        @param ppOut            inout mapped interface
        @param pInterface       source interface
        @param rType            type of interface to be mapped
     */
    inline void SAL_CALL mapInterface( void ** ppOut, void * pInterface, const css::uno::Type & rType ) const;
};

inline Mapping::Mapping(
    const ::rtl::OUString & rFrom, const ::rtl::OUString & rTo, const ::rtl::OUString & rAddPurpose )
    : _pMapping( NULL )
{
    uno_getMappingByName( &_pMapping, rFrom.pData, rTo.pData, rAddPurpose.pData );
}

inline Mapping::Mapping(
    uno_Environment * pFrom, uno_Environment * pTo, const ::rtl::OUString & rAddPurpose )
    : _pMapping( NULL )
{
    uno_getMapping( &_pMapping, pFrom, pTo, rAddPurpose.pData );
}

inline Mapping::Mapping(
    const Environment & rFrom, const Environment & rTo, const ::rtl::OUString & rAddPurpose )
        : _pMapping(NULL)
{
    uno_getMapping( &_pMapping, rFrom.get(), rTo.get(), rAddPurpose.pData );
}

inline Mapping::Mapping( uno_Mapping * pMapping )
    : _pMapping( pMapping )
{
    if (_pMapping)
        (*_pMapping->acquire)( _pMapping );
}

inline Mapping::Mapping( const Mapping & rMapping )
    : _pMapping( rMapping._pMapping )
{
    if (_pMapping)
        (*_pMapping->acquire)( _pMapping );
}

inline Mapping::~Mapping()
{
    if (_pMapping)
        (*_pMapping->release)( _pMapping );
}

inline void Mapping::clear()
{
    if (_pMapping)
    {
        (*_pMapping->release)( _pMapping );
        _pMapping = NULL;
    }
}

inline Mapping & Mapping::operator = ( uno_Mapping * pMapping )
{
    if (pMapping)
        (*pMapping->acquire)( pMapping );
    if (_pMapping)
        (*_pMapping->release)( _pMapping );
    _pMapping = pMapping;
    return *this;
}

inline void Mapping::mapInterface(
    void ** ppOut, void * pInterface, const css::uno::Type & rType ) const
{
    typelib_TypeDescription * pTD = NULL;
    TYPELIB_DANGER_GET( &pTD, rType.getTypeLibType() );
    if (pTD)
    {
        (*_pMapping->mapInterface)( _pMapping, ppOut, pInterface, reinterpret_cast<typelib_InterfaceTypeDescription *>(pTD) );
        TYPELIB_DANGER_RELEASE( pTD );
    }
}

inline void * Mapping::mapInterface(
    void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr ) const
{
    void * pOut = NULL;
    (*_pMapping->mapInterface)( _pMapping, &pOut, pInterface, pTypeDescr );
    return pOut;
}

inline void * Mapping::mapInterface(
    void * pInterface, const css::uno::Type & rType ) const
{
    void * pOut = NULL;
    mapInterface( &pOut, pInterface, rType );
    return pOut;
}

/** Deprecated. This function DOES NOT WORK with Purpose Environments
    (http://wiki.openoffice.org/wiki/Uno/Binary/Spec/Purpose Environments)

    Maps an binary C UNO interface to be used in the currently used compiler environment.

    @tparam C interface type
    @param ppRet inout returned interface pointer
    @param pUnoI binary C UNO interface
    @return true if successful, false otherwise

    @deprecated
*/
template< class C >
SAL_DEPRECATED("use uno_Mapping")
inline bool mapToCpp( Reference< C > * ppRet, uno_Interface * pUnoI )
{
    Mapping aMapping( UNO_LB_UNO, CPPU_CURRENT_LANGUAGE_BINDING_NAME );
    OSL_ASSERT( aMapping.is() );
    aMapping.mapInterface(
            reinterpret_cast<void **>(ppRet), pUnoI, ::cppu::getTypeFavourUnsigned( ppRet ) );
    return (0 != *ppRet);
}
/** Deprecated. This function DOES NOT WORK with Purpose Environments
    (http://wiki.openoffice.org/wiki/Uno/Binary/Spec/Purpose Environments)

    Maps a UNO interface of the currently used compiler environment to binary C UNO.

    @tparam C interface type
    @param ppRet inout returned interface pointer
    @param x interface reference
    @return true if successful, false otherwise

    @deprecated
*/
template< class C >
SAL_DEPRECATED("use uno_Mapping")
inline bool mapToUno( uno_Interface ** ppRet, const Reference< C > & x )
{
    Mapping aMapping( CPPU_CURRENT_LANGUAGE_BINDING_NAME, UNO_LB_UNO );
    OSL_ASSERT( aMapping.is() );
    aMapping.mapInterface(
            reinterpret_cast<void **>(ppRet), x.get(), ::cppu::getTypeFavourUnsigned( &x ) );
    return (NULL != *ppRet);
}

}
}
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
