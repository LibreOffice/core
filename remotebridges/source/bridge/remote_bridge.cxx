/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: remote_bridge.cxx,v $
 * $Revision: 1.11 $
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
#include <stdio.h>

#include "remote_bridge.hxx"
#include "bridge_connection.hxx"
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/bridge/BridgeExistsException.hpp>

#define IMPLEMENTATION_NAME "com.sun.star.comp.remotebridges.Bridge.various"


using namespace ::rtl;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;

namespace remotebridges_bridge
{
    rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

    ORemoteBridge::ORemoteBridge() :
        OComponentHelper( m_mutex ),
        m_pContext( 0 ),
        m_pEnvRemote(0 )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
        remote_DisposingListener::acquire = thisAcquire;
        remote_DisposingListener::release = thisRelease;
        remote_DisposingListener::disposing = thisDisposing;
    }

    ORemoteBridge::~ORemoteBridge()
    {
        if( m_pContext )
        {
            m_pContext->aBase.release( (uno_Context *) m_pContext );
        }
        if( m_pEnvRemote )
        {
            m_pEnvRemote->release( m_pEnvRemote );
        }
        g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
    }

    void ORemoteBridge::objectMappedSuccesfully()
    {
        MutexGuard guard( m_mutex );

        if( m_pEnvRemote )
        {
            m_pEnvRemote->release( m_pEnvRemote );
            m_pEnvRemote = 0;
        }
    }

    Any ORemoteBridge::queryInterface( const  Type & aType ) throw(RuntimeException)
    {
        Any a = ::cppu::queryInterface(
            aType ,
            SAL_STATIC_CAST( XInitialization * , this ) ,
            SAL_STATIC_CAST( XBridge * , this ),
            SAL_STATIC_CAST( XTypeProvider * ,  this ) );
        if( a.hasValue() )
        {
            return a;
        }

        return OComponentHelper::queryInterface( aType );
    }


    void ORemoteBridge::initialize( const Sequence< Any >& aArguments )
            throw( Exception, RuntimeException)
    {

        MutexGuard guard( m_mutex );

        if( 4 != aArguments.getLength() )
        {
            throw IllegalArgumentException( rtl::OUString::createFromAscii("wrong number of arguments") ,
                                            Reference< XInterface >(),
                                            0 );
        }

        OUString swName;
        OUString swProtocol;
        Reference < XConnection > rConnection;
        Reference < XInstanceProvider > rProvider;

        aArguments.getConstArray()[0] >>= swName;
        aArguments.getConstArray()[1] >>= swProtocol;
        aArguments.getConstArray()[2] >>= rConnection;
        aArguments.getConstArray()[3] >>= rProvider;

        if( ! rConnection.is() )
        {
            throw IllegalArgumentException( rtl::OUString::createFromAscii("connection is missing") ,
                                            Reference < XInterface > (),
                                            2 );
        }

        remote_Connection *pConnection = new OConnectionWrapper( rConnection );
        remote_InstanceProvider *pProvider = 0;
        if( rProvider.is( ))
        {
            pProvider = new OInstanceProviderWrapper( rProvider , this );
        }

        OUString sName = swName;
        m_sDescription = swProtocol;
        m_sDescription += OUString( RTL_CONSTASCII_USTRINGPARAM(":"));
        m_sDescription += rConnection->getDescription();

        if( 0 == sName.getLength() )
        {
            sName = m_sDescription;
        }
        else
        {
            m_sName = sName;
        }

        m_pContext = remote_createContext( pConnection,
                                           sName.pData,
                                           m_sDescription.pData,
                                           swProtocol.pData,
                                           pProvider );
        if( ! m_pContext )
        {
            throw BridgeExistsException();
        }

        m_pContext->addDisposingListener( m_pContext ,
                                          (remote_DisposingListener *) this );

        // environment will be released by the first succesfull mapping
        OUString sRemoteEnv;
        if( swProtocol.indexOf( ',') == -1 )
        {
            sRemoteEnv = swProtocol;
        }
        else
        {
                sRemoteEnv = swProtocol.copy( 0 , swProtocol.indexOf( ',' ) );
        }
        m_sProtocol = sRemoteEnv;
        uno_getEnvironment( &m_pEnvRemote ,
                            sRemoteEnv.pData ,
                            m_pContext );
        if( ! m_pEnvRemote )
        {
            m_pContext->removeDisposingListener( m_pContext ,
                                                 (remote_DisposingListener*) this );
            m_pContext->aBase.release( (uno_Context * ) m_pContext );
            m_pContext = 0;

            // forgotten exception when specifying the interface
            throw RuntimeException( rtl::OUString::createFromAscii("couldn't create uno-remote-environment") ,
                                    Reference < XInterface > () );
        }
    }

    Reference< XInterface > ORemoteBridge::getInstance( const ::rtl::OUString& sInstanceName )
            throw(::com::sun::star::uno::RuntimeException)
    {
        Reference < XInterface > rReturn;

        remote_Context *pContext = 0;
        {
            MutexGuard guard( m_mutex );
            if( m_pContext && m_pContext->getRemoteInstance )
            {
                pContext = m_pContext;
                pContext->aBase.acquire( (uno_Context*)pContext );
            }
        }
        if( pContext )
        {
            // get the appropriate remote environment
            uno_Environment *pEnvRemote = 0;
            uno_getEnvironment( &pEnvRemote , m_sProtocol.pData , pContext );

            if( ! pEnvRemote )
            {
                pContext->aBase.release( (uno_Context*) pContext );
                throw RuntimeException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "RemoteBridge: bridge already disposed" ) ),
                    Reference< XInterface > () );
            }

            Type type = getCppuType( (Reference < XInterface > * ) 0 );

            remote_Interface *pRemoteI = 0;
            uno_Any exception;
            uno_Any *pException = &exception;

            pContext->getRemoteInstance(
                pEnvRemote,
                &pRemoteI,
                sInstanceName.pData,
                type.getTypeLibType(),
                &pException );
            pContext->aBase.release( (uno_Context*) pContext );
            pContext = 0;

            uno_Environment *pEnvCpp =0;
            OUString sCppuName( RTL_CONSTASCII_USTRINGPARAM( CPPU_CURRENT_LANGUAGE_BINDING_NAME ) );
            uno_getEnvironment( &pEnvCpp ,
                                sCppuName.pData ,
                                0 );
            Mapping map( pEnvRemote , pEnvCpp );

            pEnvCpp->release( pEnvCpp );
            pEnvRemote->release( pEnvRemote );

            if( pException )
            {
                typelib_CompoundTypeDescription * pCompType = 0 ;
                getCppuType( (Exception*)0 ).getDescription( (typelib_TypeDescription **) &pCompType );

                if( ! ((typelib_TypeDescription *)pCompType)->bComplete )
                {
                    typelib_typedescription_complete( (typelib_TypeDescription**) &pCompType );
                }
                XInterface *pXInterface = (XInterface *) map.mapInterface(
                    *(remote_Interface**) ( ((char*)pException->pData)+pCompType->pMemberOffsets[1] ),
                    getCppuType( (Reference< XInterface > *)0 ) );
                RuntimeException myException(
                    *((rtl_uString **)pException->pData),
                    Reference< XInterface > ( pXInterface , SAL_NO_ACQUIRE) );
                uno_any_destruct( pException , 0 );

                throw myException;
            }
            else if( pRemoteI )
            {
                // got an interface !
                XInterface * pCppI = ( XInterface * ) map.mapInterface( pRemoteI, type );
                rReturn = Reference<XInterface > ( pCppI, SAL_NO_ACQUIRE );
                pRemoteI->release( pRemoteI );
                objectMappedSuccesfully();
            }
        }
        else
        {
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "RemoteBridge: bridge already disposed." ) ),
                Reference< XInterface > () );
        }

        return rReturn;
    }


    ::rtl::OUString SAL_CALL ORemoteBridge::getName(  )
            throw(::com::sun::star::uno::RuntimeException)
    {
        return m_sName;
    }

    ::rtl::OUString SAL_CALL ORemoteBridge::getDescription(  )
            throw(::com::sun::star::uno::RuntimeException)
    {
        return m_sDescription;
    }

    // XTypeProvider
    Sequence< Type > SAL_CALL ORemoteBridge::getTypes(void) throw( RuntimeException )
    {
        static OTypeCollection *pCollection = 0;
        if( ! pCollection )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( ! pCollection )
            {
                static OTypeCollection collection(
                    getCppuType( (Reference< XTypeProvider> * )0),
                    getCppuType( (Reference< XBridge > * ) 0 ),
                    getCppuType( (Reference< XInitialization > * ) 0 ),
                    OComponentHelper::getTypes() );
                pCollection = &collection;
            }
        }

        return (*pCollection).getTypes();
    }

    Sequence< sal_Int8 > SAL_CALL ORemoteBridge::getImplementationId(  ) throw( RuntimeException)
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

    void ORemoteBridge::disposing()
    {
        MutexGuard guard( m_mutex );
        if( m_pContext )
        {
            m_pContext->removeDisposingListener( m_pContext , ( remote_DisposingListener * )this);
            if( ! m_pEnvRemote )
            {
                if( m_pContext->m_pConnection )
                {
                    sal_Int32 nIndex = 0;
                    OUString sProtocol = OUString( m_pContext->m_pProtocol ).getToken( 0 , ',' , nIndex );
                    uno_getEnvironment( &m_pEnvRemote , sProtocol.pData , m_pContext );
                    OSL_ASSERT( m_pEnvRemote );
                }
                else
                {
                    // within disposing from the context, no further dispose necessary !
                }
            }

            if( m_pEnvRemote )
            {
                m_pEnvRemote->dispose( m_pEnvRemote );
                m_pEnvRemote->release( m_pEnvRemote );
                m_pEnvRemote = 0;
            }

            m_pContext->aBase.release( (uno_Context*)m_pContext );
            m_pContext = 0;
        }
    }


    //----------------------
    // static methods
    //----------------------
    void ORemoteBridge::thisAcquire( remote_DisposingListener *p )
    {
        ORemoteBridge *m = (ORemoteBridge * ) p;
        m->acquire();
    }

    void ORemoteBridge::thisRelease( remote_DisposingListener *p )
    {
        ORemoteBridge *m = (ORemoteBridge * ) p;
        m->release();
    }

    void ORemoteBridge::thisDisposing( remote_DisposingListener * p,
                                       rtl_uString * )
    {
        ORemoteBridge *m  = (ORemoteBridge * ) p;
        m->dispose();
    }

    //---------------------------------
    //
    //---------------------------------
    Reference< XInterface > SAL_CALL CreateInstance( const Reference< XComponentContext > &)
    {
        return Reference< XInterface > ( ( OWeakObject * ) new ORemoteBridge );
    }

    OUString getImplementationName()
    {
        static OUString *pImplName = 0;
        if( ! pImplName )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( ! pImplName )
            {
                static OUString implName(
                    RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
                pImplName = &implName;
            }
        }
        return *pImplName;
    }

    Sequence< OUString > getSupportedServiceNames()
    {
        static Sequence < OUString > *pNames = 0;
        if( ! pNames )
        {
            MutexGuard guard( Mutex::getGlobalMutex() );
            if( !pNames )
            {
                static Sequence< OUString > seqNames(3);
                seqNames.getArray()[0] = OUString::createFromAscii( "com.sun.star.bridge.Bridge" );
                seqNames.getArray()[1] = OUString::createFromAscii( "com.sun.star.bridge.IiopBridge" );
                seqNames.getArray()[2] = OUString::createFromAscii( "com.sun.star.bridge.UrpBridge" );

                pNames = &seqNames;
            }
        }
        return *pNames;
    }
}

using namespace remotebridges_bridge;

static struct ImplementationEntry g_entries[] =
{
    {
        remotebridges_bridge::CreateInstance, remotebridges_bridge::getImplementationName,
        remotebridges_bridge::getSupportedServiceNames, createSingleComponentFactory,
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





