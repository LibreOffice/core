/*************************************************************************
 *
 *  $RCSfile: proxyfac.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-08 09:40:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <osl/diagnose.h>
#include <osl/interlck.h>

#include <uno/dispatcher.h>
#include <uno/data.h>
#include <uno/any2.h>
#include <uno/mapping.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <vector>

#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/reflection/XProxyFactory.hpp>

using namespace std;
using namespace rtl;
using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::registry;

#define SERVICENAME "com.sun.star.reflection.ProxyFactory"
#define IMPLNAME    "com.sun.star.comp.reflection.ProxyFactory"

namespace stoc_proxyfac
{

//--------------------------------------------------------------------------------------------------
static inline uno_Interface * uno_queryInterface(
    uno_Interface * pUnoI, typelib_InterfaceTypeDescription * pTypeDescr )
{
    uno_Interface * pRet = 0;

    void * pArgs[1];

    typelib_InterfaceTypeDescription * pTXInterfaceDescr = 0;
    const Type & rXIType = ::getCppuType( (const Reference<XInterface > *)0 );
    TYPELIB_DANGER_GET( (typelib_TypeDescription **)&pTXInterfaceDescr, rXIType.getTypeLibType() );
    OSL_ASSERT( pTXInterfaceDescr->ppAllMembers );
    typelib_TypeDescription * pMTqueryInterface = 0;
    TYPELIB_DANGER_GET( &pMTqueryInterface, pTXInterfaceDescr->ppAllMembers[0] );

    Type aType( ((typelib_TypeDescription *)pTypeDescr)->pWeakRef );
    pArgs[0] = &aType;

    uno_Any aRetI, aExc;
    uno_Any * pExc = &aExc;

    (*((uno_Interface *)pUnoI)->pDispatcher)(
        (uno_Interface *)pUnoI, pMTqueryInterface, &aRetI, pArgs, &pExc );

    OSL_ENSHURE( !pExc, "### Exception occured during queryInterface()!" );
    if (pExc) // cleanup exception
    {
        uno_any_destruct( pExc, 0 );
    }
    else
    {
        if (aRetI.pType->eTypeClass == typelib_TypeClass_INTERFACE)
        {
            pRet = *(uno_Interface **)aRetI.pData;
            (*pRet->acquire)( pRet );
        }
        uno_any_destruct( &aRetI, 0 );
    }

    TYPELIB_DANGER_RELEASE( pMTqueryInterface );
    TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *)pTXInterfaceDescr );
    return pRet;
}

struct ProxyRoot;
struct FactoryImpl;

//==================================================================================================
struct uno_Proxy : public uno_Interface
{
    ProxyRoot *                 pRoot;
    uno_Interface *             pTarget;
    typelib_InterfaceTypeDescription * pTypeDescr;
};
typedef vector< uno_Proxy * > t_InterfaceVector;

//==================================================================================================
struct ProxyRoot : public OWeakAggObject
{
    FactoryImpl *               pFactory;
    Mutex                       aMutex;
    t_InterfaceVector           aInterfaces;

    uno_Interface *             pTarget;

    inline ProxyRoot( FactoryImpl * pFactory_, const Reference< XInterface > & xTarget_ );
    virtual ~ProxyRoot();

    virtual Any SAL_CALL queryAggregation( const Type & rType ) throw (RuntimeException);
};
//==================================================================================================
struct FactoryImpl : public WeakImplHelper2< XServiceInfo, XProxyFactory >
{
    Mapping     aUno2Cpp;
    Mapping     aCpp2Uno;

    FactoryImpl();

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);

    // XProxyFactory
    virtual Reference< XAggregation > SAL_CALL createProxy( const Reference< XInterface > & xTarget ) throw (RuntimeException);
};

extern "C"
{
//__________________________________________________________________________________________________
static void SAL_CALL uno_proxy_dispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberType,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    uno_Proxy * pThis = static_cast< uno_Proxy * >( pUnoI );

    try
    {
        switch (((typelib_InterfaceMemberTypeDescription *)pMemberType)->nPosition)
        {
        case 0: // queryInterface()
        {
            Any aRet( pThis->pRoot->queryInterface( * reinterpret_cast< const Type * >( pArgs[0] ) ) );
            const Type & rAnyType = ::getCppuType( &aRet );
            uno_type_copyAndConvertData(
                pReturn, &aRet, rAnyType.getTypeLibType(), pThis->pRoot->pFactory->aCpp2Uno.get() );
            *ppException = 0; // no exc
            break;
        }
        case 1: // acquire()
            pThis->pRoot->acquire();
            *ppException = 0; // no exc
            break;
        case 2: // release()
            pThis->pRoot->release();
            *ppException = 0; // no exc
            break;
        default:
            (*pThis->pTarget->pDispatcher)( pThis->pTarget, pMemberType, pReturn, pArgs, ppException );
        }
    }
    catch (...)
    {
        RuntimeException aExc;
        aExc.Message = OUString( RTL_CONSTASCII_USTRINGPARAM("unexpected exception occured!" ) );
        aExc.Context = (XAggregation *)pThis->pRoot;
        const Type & rExcType = ::getCppuType( &aExc );
        uno_type_any_constructAndConvert( *ppException, &aExc, rExcType.getTypeLibType(),
                                          pThis->pRoot->pFactory->aCpp2Uno.get() );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL uno_proxy_acquire( uno_Interface * pUnoI )
{
    static_cast< uno_Proxy * >( pUnoI )->pRoot->acquire();
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL uno_proxy_release( uno_Interface * pUnoI )
{
    static_cast< uno_Proxy * >( pUnoI )->pRoot->release();
}
}

//__________________________________________________________________________________________________
inline ProxyRoot::ProxyRoot( FactoryImpl * pFactory_, const Reference< XInterface > & xTarget_ )
    : pFactory( pFactory_ )
    , pTarget( 0 )
{
    pFactory->acquire();
    pFactory->aCpp2Uno.mapInterface( (void **)&pTarget, xTarget_.get(), ::getCppuType( &xTarget_ ) );
    OSL_ENSHURE( pTarget, "### mapping interface failed!" );
    aInterfaces.reserve( 8 );
}
//__________________________________________________________________________________________________
ProxyRoot::~ProxyRoot()
{
    for ( t_InterfaceVector::const_iterator iPos( aInterfaces.begin() );
          iPos != aInterfaces.end(); ++iPos )
    {
        uno_Proxy * p = *iPos;
        (*p->pTarget->release)( p->pTarget );
        typelib_typedescription_release( (typelib_TypeDescription *)p->pTypeDescr );
        delete p;
    }
    (*pTarget->release)( pTarget );
    pFactory->release();
}

//--------------------------------------------------------------------------------------------------
static inline sal_Bool type_equals(
    const Type & rType, typelib_InterfaceTypeDescription * pTypeDescr )
{
    typelib_TypeDescriptionReference * p1 = rType.getTypeLibType();
    typelib_TypeDescriptionReference * p2 = (typelib_TypeDescriptionReference *)pTypeDescr;

    return (p1 == p2 ||
            (p1->pTypeName->length == p2->pTypeName->length &&
             rtl_ustr_compare( p1->pTypeName->buffer, p2->pTypeName->buffer ) == 0));
}
//__________________________________________________________________________________________________
Any ProxyRoot::queryAggregation( const Type & rType )
    throw (RuntimeException)
{
    Any aRet( OWeakAggObject::queryAggregation( rType ) );
    if (! aRet.hasValue())
    {
        // query existing interfaces
        MutexGuard aGuard( aMutex );
        for ( t_InterfaceVector::const_iterator iPos( aInterfaces.begin() );
              iPos != aInterfaces.end(); ++iPos )
        {
            uno_Proxy * p = *iPos;
            typelib_InterfaceTypeDescription * pTypeDescr = p->pTypeDescr;
            while (pTypeDescr)
            {
                if (type_equals( rType, pTypeDescr ))
                {
                    Reference< XInterface > xRet;
                    pFactory->aUno2Cpp.mapInterface( (void **)&xRet, (uno_Interface *)p, pTypeDescr );
                    aRet.setValue( &xRet, (typelib_TypeDescription *)pTypeDescr );
                    return aRet;
                }
                pTypeDescr = pTypeDescr->pBaseTypeDescription;
            }
        }
        // else perform query
        typelib_InterfaceTypeDescription * pTypeDescr = 0;
        TYPELIB_DANGER_GET( (typelib_TypeDescription **)&pTypeDescr, rType.getTypeLibType() );
        uno_Interface * pProxyTarget = uno_queryInterface( pTarget, pTypeDescr );
        if (pProxyTarget)
        {
            uno_Proxy * p = new uno_Proxy();
            p->acquire = uno_proxy_acquire;
            p->release = uno_proxy_release;
            p->pDispatcher = uno_proxy_dispatch;
            //
            p->pRoot = this;
            p->pTarget = pProxyTarget;
            typelib_typedescription_acquire( (typelib_TypeDescription *)pTypeDescr );
            p->pTypeDescr = pTypeDescr;

            Reference< XInterface > xRet;
            pFactory->aUno2Cpp.mapInterface( (void **)&xRet, (uno_Interface *)p, pTypeDescr );
            OSL_ENSHURE( xRet.is(), "### mapping interface failed!" );
            aInterfaces.push_back( p );
            aRet.setValue( &xRet, (typelib_TypeDescription *)pTypeDescr );
        }
        TYPELIB_DANGER_RELEASE( (typelib_TypeDescription *)pTypeDescr );
    }
    return aRet;
}

//##################################################################################################
//##################################################################################################

//--------------------------------------------------------------------------------------------------
inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
}

//__________________________________________________________________________________________________
FactoryImpl::FactoryImpl()
    : aUno2Cpp( OUString( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) ),
                OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ) )
    , aCpp2Uno( OUString( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) ),
                OUString( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) ) )
{
    OSL_ENSHURE( aUno2Cpp.is(), "### cannot get bridge uno <-> C++!" );
    OSL_ENSHURE( aCpp2Uno.is(), "### cannot get bridge C++ <-> uno!" );
}

// XProxyFactory
//__________________________________________________________________________________________________
Reference< XAggregation > FactoryImpl::createProxy( const Reference< XInterface > & xTarget )
    throw (RuntimeException)
{
    return new ProxyRoot( this, xTarget );
}
// XServiceInfo
//__________________________________________________________________________________________________
OUString FactoryImpl::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}
//__________________________________________________________________________________________________
sal_Bool FactoryImpl::supportsService( const OUString & rServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    const Sequence< OUString > & rSNL = getSupportedServiceNames();
    const OUString * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (pArray[nPos] == rServiceName)
            return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > FactoryImpl::getSupportedServiceNames()
    throw(::com::sun::star::uno::RuntimeException)
{
    return stoc_proxyfac::getSupportedServiceNames();
}

//==================================================================================================
static Reference< XInterface > SAL_CALL FactoryImpl_create( const Reference< XMultiServiceFactory > & xMgr )
    throw(::com::sun::star::uno::Exception)
{
    return Reference< XInterface >( *new FactoryImpl() );
}

}


//##################################################################################################
//##################################################################################################
//##################################################################################################


extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );

            const Sequence< OUString > & rSNL = stoc_proxyfac::getSupportedServiceNames();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            stoc_proxyfac::FactoryImpl_create,
            stoc_proxyfac::getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}

