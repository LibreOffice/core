/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: surrogate.hxx,v $
 * $Revision: 1.4 $
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

#include <uno/dispatcher.h>
#include <uno/mapping.hxx>
#include <uno/environment.h>
#include <uno/data.h>
#include <osl/diagnose.h>
#include <osl/interlck.h>

#include <com/sun/star/uno/Reference.hxx>

/*
//==================================================================================================
struct UnoMediator : public uno_Interface
{
    oslInterlockedCount         nRef;
    uno_Interface *             pDest;

    UnoMediator( uno_Interface * pDest );
    ~UnoMediator();
};

//--------------------------------------------------------------------------------------------------
inline static void SAL_CALL UnoMediator_acquire( uno_Interface * pUnoI )
{
    osl_incrementInterlockedCount( &((UnoMediator *)pUnoI)->nRef );
}
//--------------------------------------------------------------------------------------------------
inline static void SAL_CALL UnoMediator_release( uno_Interface * pUnoI )
{
    if (! osl_decrementInterlockedCount( &((UnoMediator *)pUnoI)->nRef ))
        delete (UnoMediator *)pUnoI;
}
//--------------------------------------------------------------------------------------------------
inline static void SAL_CALL UnoMediator_dispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    (*((UnoMediator *)pUnoI)->pDest->pDispatcher)(
        ((UnoMediator *)pUnoI)->pDest, pMemberType, pReturn, pArgs, ppException );
}

//__________________________________________________________________________________________________
UnoMediator::UnoMediator( uno_Interface * pDest_ )
    : nRef( 0 )
    , pDest( pDest_ )
{
    (*pDest->acquire)( pDest );
    uno_Interface::acquire = UnoMediator_acquire;
    uno_Interface::release = UnoMediator_release;
    uno_Interface::pDispatcher = UnoMediator_dispatch;
}
//__________________________________________________________________________________________________
UnoMediator::~UnoMediator()
{
    (*pDest->release)( pDest );
}
*/

//##################################################################################################

template< class T >
inline sal_Bool makeSurrogate( com::sun::star::uno::Reference< T > & rOut,
                               const com::sun::star::uno::Reference< T > & rOriginal )
{
    rOut.clear();

    typelib_TypeDescription * pTD = 0;
    const com::sun::star::uno::Type & rType = ::getCppuType( &rOriginal );
    TYPELIB_DANGER_GET( &pTD, rType.getTypeLibType() );
    OSL_ENSURE( pTD, "### cannot get typedescription!" );
    if (pTD)
    {
        uno_Environment * pCppEnv1 = 0;
        uno_Environment * pCppEnv2 = 0;

        ::rtl::OUString aCppEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
        uno_getEnvironment( &pCppEnv1, aCppEnvTypeName.pData, 0 );
        uno_createEnvironment( &pCppEnv2, aCppEnvTypeName.pData, 0 ); // anonymous

        ::com::sun::star::uno::Mapping aMapping( pCppEnv1, pCppEnv2, ::rtl::OUString::createFromAscii("prot") );
        T * p = (T *)aMapping.mapInterface( rOriginal.get(), (typelib_InterfaceTypeDescription *)pTD );
        if (p)
        {
            rOut = p;
            p->release();
        }

        (*pCppEnv2->release)( pCppEnv2 );
        (*pCppEnv1->release)( pCppEnv1 );

        TYPELIB_DANGER_RELEASE( pTD );
    }
/*
    ::com::sun::star::uno::Mapping aCpp2Uno( CPPU_CURRENT_LANGUAGE_BINDING_NAME, UNO_LB_UNO );
    ::com::sun::star::uno::Mapping aUno2Cpp( UNO_LB_UNO, CPPU_CURRENT_LANGUAGE_BINDING_NAME );
    OSL_ENSURE( aCpp2Uno.is() && aUno2Cpp.is(), "### cannot get mappings!" );
    if (aCpp2Uno.is() && aUno2Cpp.is())
    {
        typelib_TypeDescription * pTD = 0;
        const com::sun::star::uno::Type & rType = ::getCppuType( &rOriginal );
        TYPELIB_DANGER_GET( &pTD, rType.getTypeLibType() );
        OSL_ENSURE( pTD, "### cannot get typedescription!" );
        if (pTD)
        {
            uno_Interface * pUno = (uno_Interface *)aCpp2Uno.mapInterface(
                rOriginal.get(), (typelib_InterfaceTypeDescription *)pTD );
            if (pUno)
            {
                UnoMediator * pPseudo = new UnoMediator( pUno );
                (*pPseudo->acquire)( pPseudo );
                OSL_ENSURE( uno_equals( &pUno, &pPseudo, pTD, 0 ), "### interfaces don't belong to same object, but they do!?" );
                (*pUno->release)( pUno );

                T * pCpp = (T *)aUno2Cpp.mapInterface(
                    pPseudo, (typelib_InterfaceTypeDescription *)pTD );
                (*pPseudo->release)( pPseudo );

                if (pCpp)
                {
                    rOut = pCpp;
                    pCpp->release();
                }
            }
            TYPELIB_DANGER_RELEASE( pTD );
        }
    }
*/
    return rOut.is();
}
