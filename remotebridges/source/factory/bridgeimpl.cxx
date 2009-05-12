/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bridgeimpl.cxx,v $
 * $Revision: 1.10 $
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

#include "bridgeimpl.hxx"

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::bridge;

namespace remotebridges_factory {


    OBridge::OBridge( remote_Context *pContext ) :
        OComponentHelper( m_mutex ),
        m_pContext( pContext )
    {
        g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );

        remote_DisposingListener::acquire = thisAcquire;
        remote_DisposingListener::release = thisRelease;
        remote_DisposingListener::disposing = thisDisposing;

        m_pContext->aBase.acquire( (uno_Context*)m_pContext );
        m_pContext->addDisposingListener( m_pContext, ( remote_DisposingListener * ) this );
    }

    OBridge::~OBridge()
    {
        if( m_pContext )
        {
            m_pContext->aBase.release( (uno_Context * ) m_pContext );
        }
        g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
    }

    ::com::sun::star::uno::Any OBridge::queryInterface( const ::com::sun::star::uno::Type & aType ) throw(RuntimeException)
    {
        Any a = ::cppu::queryInterface(
            aType ,
            SAL_STATIC_CAST( XBridge * , this ),
            SAL_STATIC_CAST( XTypeProvider * ,  this ) );
        if( a.hasValue() )
        {
            return a;
        }

        return OComponentHelper::queryInterface( aType );
    }

    void OBridge::disposing()
    {
        if( m_pContext )
        {
            m_pContext->removeDisposingListener( m_pContext , ( remote_DisposingListener * )this);

            uno_Environment *pEnvRemote = 0;
            if( m_pContext->m_pConnection )
            {
                sal_Int32 nIndex = 0;
                OUString sProtocol = OUString( m_pContext->m_pProtocol ).getToken( 0 , ',' , nIndex );
                uno_getEnvironment( &pEnvRemote , sProtocol.pData , m_pContext );
                OSL_ASSERT( pEnvRemote );
            }
            else
            {
                // within disposing from the context, no further dispose necessary !
            }

            if( pEnvRemote )
            {
                pEnvRemote->dispose( pEnvRemote );
                pEnvRemote->release( pEnvRemote );
                pEnvRemote = 0;
            }

            m_pContext->aBase.release( (uno_Context*)m_pContext );
            m_pContext = 0;
        }
    }


    Reference< XInterface > OBridge::getInstance( const ::rtl::OUString& sInstanceName )
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
            sal_Int32 nIndex = 0;
            OUString sProtocol = OUString( m_pContext->m_pProtocol ).getToken( 0 , ',' , nIndex );

            // get the appropriate remote environment
            uno_Environment *pEnvRemote = 0;
            uno_getEnvironment( &pEnvRemote , sProtocol.pData , pContext );

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

    ::rtl::OUString SAL_CALL OBridge::getName(  )
                throw(::com::sun::star::uno::RuntimeException)

    {
        return OUString( m_pContext->m_pName );
    }

    ::rtl::OUString OBridge::getDescription(  )
            throw(::com::sun::star::uno::RuntimeException)
    {
        return OUString( m_pContext->m_pDescription  );
    }

        // XTypeProvider
    Sequence< Type > SAL_CALL OBridge::getTypes(void) throw( RuntimeException )
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
                    OComponentHelper::getTypes() );
                pCollection = &collection;
            }
        }

        return (*pCollection).getTypes();
    }

    Sequence< sal_Int8 > SAL_CALL OBridge::getImplementationId(  ) throw( RuntimeException)
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

    //----------------------
    // static methods
    //----------------------
    void OBridge::thisAcquire( remote_DisposingListener *p )
    {
        OBridge *m = (OBridge * ) p;
        m->acquire();
    }

    void OBridge::thisRelease( remote_DisposingListener *p )
    {
        OBridge *m = (OBridge * ) p;
        m->release();
    }

    void OBridge::thisDisposing( remote_DisposingListener * p, rtl_uString * )
    {
        OBridge *m  = (OBridge * ) p;
        m->dispose();
    }
}
