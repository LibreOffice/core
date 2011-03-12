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
#include <boost/unordered_map.hpp>

#include <rtl/alloc.h>

#include <uno/mapping.hxx>
#include <osl/mutex.hxx>

#include <bridges/remote/context.h>

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/typeprovider.hxx>
#include "cppuhelper/unourl.hxx"
#include "rtl/malformeduriexception.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/bridge/XBridgeFactory.hpp>

#include <com/sun/star/registry/XRegistryKey.hpp>

#include <com/sun/star/container/XContentEnumerationAccess.hpp>


#include "bridgeimpl.hxx"

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::std;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::container;

#define SERVICE_NAME "com.sun.star.bridge.BridgeFactory"
#define IMPLEMENTATION_NAME "com.sun.star.comp.remotebridges.BridgeFactory"

namespace remotebridges_factory
{
    rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

    struct hashOUString
    {
        size_t operator()(const OUString & s) const
            { return s.hashCode(); }
    };

    struct equalOUString
    {
        sal_Bool operator()(const OUString & s1 , const OUString &s2 ) const
            {
                return s1 == s2;
            }
    };

    typedef ::boost::unordered_map
    <
    OUString,
    WeakReference< XBridge >,
    hashOUString,
    equalOUString
    > BridgeHashMap;


    typedef ::boost::unordered_map
    <
    OUString,
    OUString,
    hashOUString,
    equalOUString
    > ServiceHashMap;

    class OBridgeFactory :
        public MyMutex,
        public OComponentHelper,
        public XBridgeFactory,
        public XServiceInfo
    {
    public:
        OBridgeFactory( const Reference < XComponentContext > &rCtx );
        ~OBridgeFactory();

    public: // XInterface
        ::com::sun::star::uno::Any      SAL_CALL
            queryInterface( const ::com::sun::star::uno::Type & aType ) throw(RuntimeException);

        void        SAL_CALL acquire() throw()
            { OComponentHelper::acquire(); }
        void        SAL_CALL release() throw()
            { OComponentHelper::release(); }

    public:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::bridge::XBridge > SAL_CALL
                  createBridge(
                      const ::rtl::OUString& sName,
                      const ::rtl::OUString& sProtocol,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::connection::XConnection >& aConnection,
                      const ::com::sun::star::uno::Reference< ::com::sun::star::bridge::XInstanceProvider >& anInstanceProvider )
            throw(::com::sun::star::bridge::BridgeExistsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::bridge::XBridge > SAL_CALL
            getBridge( const ::rtl::OUString& sName )
              throw(::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::bridge::XBridge > > SAL_CALL
              getExistingBridges(  ) throw(::com::sun::star::uno::RuntimeException);

    public: //XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
                     getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
                     getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    public: //XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

    public:
        static OUString getImplementationNameStatic(  ) SAL_THROW( () );
        static Sequence< OUString >  getSupportedServiceNamesStatic() SAL_THROW( () );

    private:
        void init();
        OUString getServiceNameForProtocol( const OUString &sProtocol );

    private:
        Reference < XMultiComponentFactory > m_rSMgr;
        Reference < XComponentContext > m_rCtx;
        BridgeHashMap m_mapBridge;
        ServiceHashMap m_mapProtocolToService;
        sal_Bool m_bInitialized;
        ::osl::Mutex m_mutexInit;
    };

    OBridgeFactory::OBridgeFactory( const Reference < XComponentContext > &rCtx )
        : OComponentHelper( m_mutex )
        , m_rSMgr( rCtx->getServiceManager() )
        , m_rCtx( rCtx )
        , m_bInitialized( sal_False )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    }

    OBridgeFactory::~OBridgeFactory()
    {
        g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
    }

    void OBridgeFactory::init()
    {
        MutexGuard guard( m_mutexInit );
        if( ! m_bInitialized )
        {
            Reference< XContentEnumerationAccess > rContent( m_rSMgr , UNO_QUERY );
            if( rContent.is() )
            {
                OUString sMetaService = OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.bridge.Bridge" ) );
                Reference < XEnumeration > rEnum =
                    rContent->createContentEnumeration( sMetaService );
                if ( rEnum.is() )
                    while( rEnum->hasMoreElements() )
                    {
                        Any a = rEnum->nextElement();
                        Reference <XServiceInfo> rInfo;
                        if( a >>= rInfo )
                        {
                            Sequence< OUString > seq = rInfo->getSupportedServiceNames();
                            sal_Int32 i;
                            for( i = 0 ; i < seq.getLength() ; i ++ )
                            {
                                if( seq.getConstArray()[i] != sMetaService )
                                {
                                    sal_Int32 nIndex = seq.getConstArray()[i].lastIndexOf( '.' );
                                    OUString sSuffix = seq.getConstArray()[i].copy(nIndex+1);
                                    if( sSuffix.getLength() > 6  &&
                                        0 == sSuffix.copy( sSuffix.getLength() - 6 ).compareToAscii( "Bridge" ) )
                                    {
                                        OUString sProtocol = sSuffix.copy(0 , sSuffix.getLength()-6 ).toAsciiLowerCase();
                                        m_mapProtocolToService[ sProtocol ] = seq.getConstArray()[i];
                                    }
                                }
                            }
                        }
                    }
            }
            m_bInitialized = sal_True;
        }
    }

    OUString OBridgeFactory::getServiceNameForProtocol( const OUString & sProtocol )
    {
        init();
        OUString sService;
        OUString sProtocolName;
        try
        {
            sProtocolName = cppu::UnoUrlDescriptor(sProtocol).getName();
        }
        catch (rtl::MalformedUriException &)
        {
            OSL_FAIL("MalformedUriException");
        }
        ServiceHashMap::iterator ii = m_mapProtocolToService.find( sProtocolName );
        if( ii != m_mapProtocolToService.end() )
        {
            sService = (*ii).second;
        }
        else
        {
            // fallback to the old solution, deprecated, should be removed !
            sService = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.Bridge."));
            sService += sProtocolName;
        }
        return sService;
    }

    Any OBridgeFactory::queryInterface( const Type &aType ) throw(RuntimeException)
    {
        Any a = ::cppu::queryInterface(
            aType ,
            ( XBridgeFactory * ) this );
        if( a.hasValue() )
        {
            return a;
        }

        return OComponentHelper::queryInterface( aType );

    }

    Reference< XBridge > OBridgeFactory::createBridge(
        const OUString& sName,
        const OUString& sProtocol,
        const Reference< XConnection >& aConnection,
        const Reference< XInstanceProvider >& anInstanceProvider )
        throw(::com::sun::star::bridge::BridgeExistsException,
              ::com::sun::star::lang::IllegalArgumentException,
              ::com::sun::star::uno::RuntimeException)
    {
        OUString sService = getServiceNameForProtocol( sProtocol );

        Reference < XInterface > rXInterface =
            m_rSMgr->createInstanceWithContext(sService, m_rCtx );
        Reference < XInitialization > rInit(rXInterface, UNO_QUERY );
        Reference < XBridge > rBridge( rInit , UNO_QUERY );

        if( rInit.is() && rBridge.is() )
        {
            Sequence < Any > seqAny( 4 );
            seqAny.getArray()[0] <<= sName;
            seqAny.getArray()[1] <<= sProtocol;
            seqAny.getArray()[2] <<= aConnection;
            seqAny.getArray()[3] <<= anInstanceProvider;

            // let the Exceptions fly ....
            rInit->initialize( seqAny );
        }
        else
        {
            throw IllegalArgumentException();
        }

        if( sName.getLength() )
        {
            MutexGuard guard( m_mutex );
            // put the bridge into the hashmap
            m_mapBridge[ sName ] = rBridge;
        }
        return rBridge;
    }

    Reference< XBridge > OBridgeFactory::getBridge( const ::rtl::OUString& sName )
        throw(::com::sun::star::uno::RuntimeException )

    {
        MutexGuard guard( m_mutex );
        BridgeHashMap::iterator ii = m_mapBridge.find( sName );

        Reference < XBridge > rBridge;

        if( ii != m_mapBridge.end() )
        {
            rBridge = (*ii).second;
            if( ! rBridge.is() )
            {
                m_mapBridge.erase( ii );
            }
        }

        if( ! rBridge.is() )
        {
            // try to get it via the C-Context
            remote_Context * pRemoteC = remote_getContext( sName.pData );

            if( pRemoteC )
            {
                rBridge = Reference < XBridge > ((XBridge *) new OBridge( pRemoteC ) );
                pRemoteC->aBase.release( (uno_Context * )pRemoteC );
                m_mapBridge[ sName ] = rBridge;
            }
        }
        return rBridge;
    }

    Sequence< Reference< XBridge > > OBridgeFactory::getExistingBridges(  )
        throw(::com::sun::star::uno::RuntimeException)
    {
        MutexGuard guard( m_mutex );

        sal_Int32 nCount;

        rtl_uString **ppName = remote_getContextList(
            &nCount,
            rtl_allocateMemory );

        Sequence < Reference < XBridge > > seq( nCount );
        if( nCount )
        {

            for( sal_Int32 i = 0;
                 i < nCount ;
                 i ++ )
            {
                seq.getArray()[i] = getBridge( OUString( ppName[i]) );
                rtl_uString_release( ppName[i] );
            }
            rtl_freeMemory( ppName );
        }

        return seq;
    }

    // XTypeProvider
    Sequence< Type > SAL_CALL OBridgeFactory::getTypes(void) throw( RuntimeException )
    {
        static OTypeCollection *pCollection = 0;
        if( ! pCollection )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( ! pCollection )
            {
                static OTypeCollection collection(
                    getCppuType( (Reference< XBridgeFactory > * ) 0 ),
                    OComponentHelper::getTypes() );
                pCollection = &collection;
            }
        }
        return (*pCollection).getTypes();
    }

    Sequence< sal_Int8 > SAL_CALL OBridgeFactory::getImplementationId(  ) throw( RuntimeException)
    {
        static OImplementationId *pId = 0;
        if( ! pId )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( ! pId )
            {
                static OImplementationId id( sal_False );
                pId = &id;
            }
        }
        return (*pId).getImplementationId();
    }

    OUString OBridgeFactory::getImplementationNameStatic()
    {
        static OUString *pName = 0;
        if( ! pName )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( ! pName )
            {
                static OUString name( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
                pName = &name;
            }
        }
        return *pName;
    }

    Sequence< OUString > OBridgeFactory::getSupportedServiceNamesStatic()
    {
        static Sequence < OUString > *pNames = 0;
        if( ! pNames )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( !pNames )
            {
                static Sequence< OUString > seqNames(1);
                seqNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ));
                pNames = &seqNames;
            }
        }
        return *pNames;
    }

    OUString OBridgeFactory::getImplementationName(  ) throw(RuntimeException)
    {
        return getImplementationNameStatic();
    }

    sal_Bool SAL_CALL OBridgeFactory::supportsService( const OUString& ServiceName ) throw(RuntimeException)
    {
        Sequence< OUString > seq = getSupportedServiceNamesStatic();
        sal_Bool bReturn = sal_False;
        for( sal_Int32 i = 0 ; i < seq.getLength() ; i ++ )
        {
            if( seq.getConstArray()[i] == ServiceName )
            {
                bReturn = sal_True;
                break;
            }
        }
        return bReturn;
    }

    Sequence< OUString > SAL_CALL OBridgeFactory::getSupportedServiceNames(  ) throw(RuntimeException)
    {
        return getSupportedServiceNamesStatic();
    }


    Reference< XInterface > SAL_CALL CreateInstance(Reference< XComponentContext > const & xContext)
    {
        return Reference < XInterface > ( *new OBridgeFactory( xContext ) );
    }
}

using namespace remotebridges_factory;
static ImplementationEntry g_entries[] =
{
    {   CreateInstance, OBridgeFactory::getImplementationNameStatic,
        OBridgeFactory::getSupportedServiceNamesStatic, createSingleComponentFactory ,
        &g_moduleCount.modCnt , 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{
sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
