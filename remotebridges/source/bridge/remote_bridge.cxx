/*************************************************************************
 *
 *  $RCSfile: remote_bridge.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2000-10-27 09:00:57 $
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
#include <stdio.h>

#include "remote_bridge.hxx"
#include "bridge_connection.hxx"

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
    ORemoteBridge::ORemoteBridge() :
        OComponentHelper( m_mutex ),
        m_pContext( 0 ),
        m_pEnvRemote(0 )
    {
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

    Any ORemoteBridge::queryInterface( const  Type & aType )
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

        if( m_pContext && m_pContext->getRemoteInstance )
        {
            // get the appropriate remote environment
            uno_Environment *pEnvRemote = 0;
            uno_getEnvironment( &pEnvRemote , m_sProtocol.pData , m_pContext );

            if( ! pEnvRemote )
            {
                return Reference< XInterface > ();
            }

            Type type = getCppuType( (Reference < XInterface > * ) 0 );

            remote_Interface *pRemoteI = 0;
            m_pContext->getRemoteInstance(
                pEnvRemote,
                &pRemoteI,
                sInstanceName.pData,
                type.getTypeLibType()  );

            if( pRemoteI )
            {
                // got an interface !
                uno_Environment *pEnvCpp =0;
                OUString sCppuName( RTL_CONSTASCII_USTRINGPARAM( CPPU_CURRENT_LANGUAGE_BINDING_NAME ) );
                uno_getEnvironment( &pEnvCpp ,
                                    sCppuName.pData ,
                                    0 );

                Mapping map( pEnvRemote , pEnvCpp );

                XInterface * pCppI = ( XInterface * ) map.mapInterface( pRemoteI, type );

                rReturn = Reference<XInterface > ( pCppI );

                pCppI->release( );

                pRemoteI->release( pRemoteI );
                pEnvCpp->release( pEnvCpp );
                objectMappedSuccesfully();
            }

            pEnvRemote->release( pEnvRemote );
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
                    OUString sProtocol = OUString( m_pContext->m_pProtocol ).getToken( 0 , ',' );
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
                                     rtl_uString *pString )
    {
        ORemoteBridge *m  = (ORemoteBridge * ) p;
        m->dispose();
    }

    //---------------------------------
    //
    //---------------------------------
    Reference< XInterface > SAL_CALL CreateInstance( const Reference< XMultiServiceFactory > &)
    {
        return Reference< XInterface > ( ( OWeakObject * ) new ORemoteBridge );
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
                seqNames.getArray()[1] = OUString::createFromAscii( "com.sun.star.bridge.Iiop" );
                seqNames.getArray()[2] = OUString::createFromAscii( "com.sun.star.bridge.Urp" );

                pNames = &seqNames;
            }
        }
        return *pNames;
    }
}

using namespace remotebridges_bridge;

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
                    OUString::createFromAscii( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL = getSupportedServiceNames();
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

    if (pServiceManager && rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            CreateInstance, getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}




