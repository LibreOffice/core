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

#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <rtl/ustring.hxx>
#include <typelib/typedescription.h>
#include <uno/dispatcher.h>
#include <uno/environment.h>
#include <uno/mapping.h>
#include <uno/lbnames.h>

using ::rtl::OUString;

namespace pseudo_uno
{

//==================================================================================================
struct pseudo_Mapping : public uno_Mapping
{
    oslInterlockedCount     nRef;

    uno_ExtEnvironment *    pFrom;
    uno_ExtEnvironment *    pTo;

    pseudo_Mapping( uno_ExtEnvironment * pFrom_, uno_ExtEnvironment * pTo_ );
    ~pseudo_Mapping();
};

//==== a uno pseudo proxy =============================================================================
struct pseudo_unoInterfaceProxy : public uno_Interface
{
    oslInterlockedCount                 nRef;
    pseudo_Mapping *                    pPseudoMapping;

    // mapping information
    uno_Interface *                     pUnoI; // wrapped interface
    typelib_InterfaceTypeDescription *  pTypeDescr;
    OUString                            oid;

    // ctor
    inline pseudo_unoInterfaceProxy( pseudo_Mapping * pPseudoMapping_,
                                     uno_Interface * pUnoI_,
                                     typelib_InterfaceTypeDescription * pTypeDescr_,
                                     const OUString & rOId_ );
};
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_unoInterfaceProxy_dispatch(
    uno_Interface * pUnoI,
    const typelib_TypeDescription * pMemberType,
    void * pReturn,
    void * pArgs[],
    uno_Any ** ppException )
{
    pseudo_unoInterfaceProxy * pThis = static_cast< pseudo_unoInterfaceProxy * >( pUnoI );
    (*pThis->pUnoI->pDispatcher)( pThis->pUnoI, pMemberType, pReturn, pArgs, ppException );
}

//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_unoInterfaceProxy_free( uno_ExtEnvironment * pEnv, void * pProxy )
{
    pseudo_unoInterfaceProxy * pThis =
        static_cast< pseudo_unoInterfaceProxy * >(
            reinterpret_cast< uno_Interface * >( pProxy ) );
    OSL_ASSERT( pEnv == pThis->pPseudoMapping->pTo );

    (*pThis->pPseudoMapping->pFrom->revokeInterface)( pThis->pPseudoMapping->pFrom, pThis->pUnoI );
    (*pThis->pUnoI->release)( pThis->pUnoI );
    typelib_typedescription_release( (typelib_TypeDescription *)pThis->pTypeDescr );
    (*pThis->pPseudoMapping->release)( pThis->pPseudoMapping );

#if OSL_DEBUG_LEVEL > 1
    *(int *)pProxy = 0xdeadbabe;
#endif
    delete pThis;
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_unoInterfaceProxy_acquire( uno_Interface * pUnoI )
{
    if (1 == osl_atomic_increment( &static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->nRef ))
    {
        // rebirth of proxy zombie
        // register at uno env
        void * pThis = static_cast< uno_Interface * >( pUnoI );
        (*static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->pPseudoMapping->pTo->registerProxyInterface)(
            static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->pPseudoMapping->pTo,
            &pThis, pseudo_unoInterfaceProxy_free,
            static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->oid.pData,
            static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->pTypeDescr );
        OSL_ASSERT( pThis == static_cast< uno_Interface * >( pUnoI ) );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_unoInterfaceProxy_release( uno_Interface * pUnoI )
{
    if (! osl_atomic_decrement( & static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->nRef ))
    {
        // revoke from uno env on last release
        (*static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->pPseudoMapping->pTo->revokeInterface)(
            static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->pPseudoMapping->pTo, pUnoI );
    }
}
//__________________________________________________________________________________________________
inline pseudo_unoInterfaceProxy::pseudo_unoInterfaceProxy(
    pseudo_Mapping * pPseudoMapping_, uno_Interface * pUnoI_,
    typelib_InterfaceTypeDescription * pTypeDescr_, const OUString & rOId_ )
    : nRef( 1 )
    , pPseudoMapping( pPseudoMapping_ )
    , pUnoI( pUnoI_ )
    , pTypeDescr( pTypeDescr_ )
    , oid( rOId_ )
{
    (*pPseudoMapping->acquire)( pPseudoMapping );
    typelib_typedescription_acquire( (typelib_TypeDescription *)pTypeDescr );
    (*pPseudoMapping->pFrom->registerInterface)(
        pPseudoMapping->pFrom, reinterpret_cast< void ** >( &pUnoI ), oid.pData, pTypeDescr );
    (*pUnoI->acquire)( pUnoI );

    // uno_Interface
    uno_Interface::acquire = pseudo_unoInterfaceProxy_acquire;
    uno_Interface::release = pseudo_unoInterfaceProxy_release;
    uno_Interface::pDispatcher = pseudo_unoInterfaceProxy_dispatch;
}

//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_Mapping_mapInterface(
    uno_Mapping * pMapping, void ** ppOut,
    void * pUnoI, typelib_InterfaceTypeDescription * pTypeDescr )
{
    OSL_ASSERT( ppOut && pTypeDescr );
    if (*ppOut)
    {
        (*reinterpret_cast< uno_Interface * >( *ppOut )->release)(
            reinterpret_cast< uno_Interface * >( *ppOut ) );
        *ppOut = 0;
    }
    if (pUnoI && pTypeDescr)
    {
        // get object id of uno interface to be wrapped
        rtl_uString * pOId = 0;
        (*static_cast< pseudo_Mapping * >( pMapping )->pFrom->getObjectIdentifier)(
            static_cast< pseudo_Mapping * >( pMapping )->pFrom, &pOId, pUnoI );
        OSL_ASSERT( pOId );

        if (pOId)
        {
            // try to get any known interface from target environment
            (*static_cast< pseudo_Mapping * >( pMapping )->pTo->getRegisteredInterface)(
                static_cast< pseudo_Mapping * >( pMapping )->pTo, ppOut, pOId, pTypeDescr );
            if (! *ppOut) // no existing interface, register new proxy interface
            {
                // try to publish a new proxy (ref count initially 1)
                void * pProxy = new pseudo_unoInterfaceProxy(
                    static_cast< pseudo_Mapping * >( pMapping ),
                    reinterpret_cast< uno_Interface * >( pUnoI ), pTypeDescr, pOId );

                // proxy may be exchanged during registration
                (*static_cast< pseudo_Mapping * >( pMapping )->pTo->registerProxyInterface)(
                    static_cast< pseudo_Mapping * >( pMapping )->pTo,
                    &pProxy, pseudo_unoInterfaceProxy_free, pOId, pTypeDescr );

                *ppOut = pProxy;
            }
            rtl_uString_release( pOId );
        }
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_Mapping_free( uno_Mapping * pMapping )
{
    delete static_cast< pseudo_Mapping * >( pMapping );
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_Mapping_acquire( uno_Mapping * pMapping )
{
    if (1 == osl_atomic_increment( & static_cast< pseudo_Mapping * >( pMapping )->nRef ))
    {
        OUString aMappingPurpose( RTL_CONSTASCII_USTRINGPARAM("pseudo") );
        uno_registerMapping( &pMapping,
                             pseudo_Mapping_free,
                             (uno_Environment *)((pseudo_Mapping *)pMapping)->pFrom,
                             (uno_Environment *)((pseudo_Mapping *)pMapping)->pTo,
                             aMappingPurpose.pData );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_Mapping_release( uno_Mapping * pMapping )
{
    if (! osl_atomic_decrement( & static_cast< pseudo_Mapping * >( pMapping )->nRef ))
    {
        uno_revokeMapping( pMapping );
    }
}

//__________________________________________________________________________________________________
pseudo_Mapping::pseudo_Mapping( uno_ExtEnvironment * pFrom_, uno_ExtEnvironment * pTo_ )
    : nRef( 1 )
    , pFrom( pFrom_ )
    , pTo( pTo_ )
{
    (*((uno_Environment *)pFrom)->acquire)( (uno_Environment *)pFrom );
    (*((uno_Environment *)pTo)->acquire)( (uno_Environment *)pTo );
    //
    uno_Mapping::acquire = pseudo_Mapping_acquire;
    uno_Mapping::release = pseudo_Mapping_release;
    uno_Mapping::mapInterface = pseudo_Mapping_mapInterface;
}
//__________________________________________________________________________________________________
pseudo_Mapping::~pseudo_Mapping()
{
    (*((uno_Environment *)pTo)->release)( (uno_Environment *)pTo );
    (*((uno_Environment *)pFrom)->release)( (uno_Environment *)pFrom );
}

}

//##################################################################################################
extern "C" void SAL_CALL uno_initEnvironment( uno_Environment * pUnoEnv )
{
    OSL_FAIL( "### no impl: unexpected call!" );
}
//##################################################################################################
extern "C" void SAL_CALL uno_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo )
{
    OSL_ASSERT( ppMapping && pFrom && pTo );
    if (ppMapping && pFrom && pTo && pFrom->pExtEnv && pTo->pExtEnv)
    {
        uno_Mapping * pMapping = 0;

        if (0 == rtl_ustr_ascii_compare( pFrom->pTypeName->buffer, UNO_LB_UNO ) &&
            0 == rtl_ustr_ascii_compare( pTo->pTypeName->buffer, UNO_LB_UNO ))
        {
            OUString aMappingPurpose( RTL_CONSTASCII_USTRINGPARAM("pseudo") );
            // ref count is initially 1
            pMapping = new pseudo_uno::pseudo_Mapping( pFrom->pExtEnv, pTo->pExtEnv );
            uno_registerMapping( &pMapping, pseudo_uno::pseudo_Mapping_free,
                                 (uno_Environment *)pFrom->pExtEnv,
                                 (uno_Environment *)pTo->pExtEnv,
                                 aMappingPurpose.pData );
        }

        if (*ppMapping)
            (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = pMapping;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
