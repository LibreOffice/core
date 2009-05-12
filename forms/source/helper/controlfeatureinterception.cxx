/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: controlfeatureinterception.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "controlfeatureinterception.hxx"
#include "urltransformer.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/
#include <tools/debug.hxx>

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;

    //====================================================================
    //= ControlFeatureInterception
    //====================================================================
    //--------------------------------------------------------------------
    ControlFeatureInterception::ControlFeatureInterception( const Reference< XMultiServiceFactory >& _rxORB )
        :m_pUrlTransformer( new UrlTransformer( _rxORB ) )
    {
    }

    //--------------------------------------------------------------------
    void SAL_CALL ControlFeatureInterception::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException )
    {
        if ( !_rxInterceptor.is() )
        {
            DBG_ERROR( "ControlFeatureInterception::registerDispatchProviderInterceptor: invalid interceptor!" );
            return;
        }

        if ( m_xFirstDispatchInterceptor.is() )
        {
            // there is already an interceptor; the new one will become its master
            Reference< XDispatchProvider > xFirstProvider( m_xFirstDispatchInterceptor, UNO_QUERY );
            _rxInterceptor->setSlaveDispatchProvider( xFirstProvider );
            m_xFirstDispatchInterceptor->setMasterDispatchProvider( xFirstProvider );
        }

        // we are the master of the chain's first interceptor
        m_xFirstDispatchInterceptor = _rxInterceptor;
        m_xFirstDispatchInterceptor->setMasterDispatchProvider( NULL );
            // it's the first of the interceptor chain
    }

    //--------------------------------------------------------------------
    void SAL_CALL ControlFeatureInterception::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException )
    {
        if ( !_rxInterceptor.is() )
        {
            DBG_ERROR( "ControlFeatureInterception::releaseDispatchProviderInterceptor: invalid interceptor!" );
            return;
        }

        Reference< XDispatchProviderInterceptor >  xChainWalk( m_xFirstDispatchInterceptor );

        if ( m_xFirstDispatchInterceptor == _rxInterceptor )
        {   // our chain will have a new first element
            Reference< XDispatchProviderInterceptor >  xSlave( m_xFirstDispatchInterceptor->getSlaveDispatchProvider(), UNO_QUERY );
            m_xFirstDispatchInterceptor = xSlave;
        }
        // do this before removing the interceptor from the chain as we won't know it's slave afterwards)

        while ( xChainWalk.is() )
        {
            // walk along the chain of interceptors and look for the interceptor that has to be removed
            Reference< XDispatchProviderInterceptor >  xSlave( xChainWalk->getSlaveDispatchProvider(), UNO_QUERY );

            if ( xChainWalk == _rxInterceptor )
            {
                // old master may be an interceptor too
                Reference< XDispatchProviderInterceptor >  xMaster( xChainWalk->getMasterDispatchProvider(), UNO_QUERY );

                // unchain the interceptor that has to be removed
                xChainWalk->setSlaveDispatchProvider( NULL );
                xChainWalk->setMasterDispatchProvider( NULL );

                // reconnect the chain
                if ( xMaster.is() )
                {
                    xMaster->setSlaveDispatchProvider( Reference< XDispatchProvider >::query( xSlave ) );
                }

                // if somebody has registered the same interceptor twice, then we will remove
                // it once per call ...
                break;
            }

            xChainWalk = xSlave;
        }
    }

    //--------------------------------------------------------------------
    void ControlFeatureInterception::dispose()
    {
        // release all interceptors
        Reference< XDispatchProviderInterceptor > xInterceptor( m_xFirstDispatchInterceptor );
        m_xFirstDispatchInterceptor.clear();
        while ( xInterceptor.is() )
        {
            // tell the interceptor it has a new (means no) predecessor
            xInterceptor->setMasterDispatchProvider( NULL );

            // ask for it's successor
            Reference< XDispatchProvider > xSlave = xInterceptor->getSlaveDispatchProvider();
            // and give it the new (means no) successoert
            xInterceptor->setSlaveDispatchProvider( NULL );

            // start over with the next chain element
            xInterceptor = xInterceptor.query( xSlave );
        }
    }
    //--------------------------------------------------------------------
    Reference< XDispatch > ControlFeatureInterception::queryDispatch( const URL& _rURL, const ::rtl::OUString& _rTargetFrameName, ::sal_Int32 _nSearchFlags ) SAL_THROW((RuntimeException))
    {
        Reference< XDispatch > xDispatcher;
        if ( m_xFirstDispatchInterceptor.is() )
            xDispatcher = m_xFirstDispatchInterceptor->queryDispatch( _rURL, _rTargetFrameName, _nSearchFlags );
        return xDispatcher;
    }

    //--------------------------------------------------------------------
    Reference< XDispatch > ControlFeatureInterception::queryDispatch( const URL& _rURL ) SAL_THROW((RuntimeException))
    {
        return queryDispatch( _rURL, ::rtl::OUString(), 0 );
    }

    //--------------------------------------------------------------------
    Reference< XDispatch > ControlFeatureInterception::queryDispatch( const sal_Char* _pAsciiURL ) SAL_THROW((RuntimeException))
    {
        return queryDispatch( m_pUrlTransformer->getStrictURLFromAscii( _pAsciiURL ) );
    }

//........................................................................
} // namespace frm
//........................................................................

