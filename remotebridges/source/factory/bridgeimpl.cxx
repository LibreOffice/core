/*************************************************************************
 *
 *  $RCSfile: bridgeimpl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:56 $
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
    }

    ::com::sun::star::uno::Any OBridge::queryInterface( const ::com::sun::star::uno::Type & aType )
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
                OUString sProtocol = OUString( m_pContext->m_pProtocol ).getToken( 0 , ',' );
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

        if( m_pContext && m_pContext->getRemoteInstance )
        {
            // get the appropriate remote environment
            OUString sRemote( RTL_CONSTASCII_USTRINGPARAM( UNO_LB_REMOTE ));
            uno_Environment *pEnvRemote = 0;
            uno_getEnvironment( &pEnvRemote , sRemote.pData , m_pContext );

            OSL_ASSERT( pEnvRemote );

            Type type = getCppuType( (Reference < XInterface > * ) 0 );

            remote_Interface *pRemoteI = 0;
            m_pContext->getRemoteInstance(
                pEnvRemote,
                &pRemoteI,
                sInstanceName.pData,
                type.getTypeLibType() );


            if( pRemoteI )
            {
                // got an interface !
                OUString sCppu ( RTL_CONSTASCII_USTRINGPARAM( CPPU_CURRENT_LANGUAGE_BINDING_NAME ));
                uno_Environment *pEnvCpp =0;
                uno_getEnvironment( &pEnvCpp , sCppu.pData , 0 );

                Mapping map( pEnvRemote , pEnvCpp );

                XInterface * pCppI = ( XInterface * ) map.mapInterface( pRemoteI,type );

                rReturn = Reference<XInterface > ( pCppI );

                pCppI->release( );

                pRemoteI->release( pRemoteI );
                pEnvCpp->release( pEnvCpp );
            }

            pEnvRemote->release( pEnvRemote );
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

    void OBridge::thisDisposing( remote_DisposingListener * p,
                                     rtl_uString *pString )
    {
        OBridge *m  = (OBridge * ) p;
        m->dispose();
    }


}



