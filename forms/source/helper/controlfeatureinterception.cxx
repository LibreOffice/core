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

#include <controlfeatureinterception.hxx>
#include <urltransformer.hxx>
#include <osl/diagnose.h>


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;

    ControlFeatureInterception::ControlFeatureInterception( const Reference< XComponentContext >& _rxORB )
        :m_pUrlTransformer( new UrlTransformer( _rxORB ) )
    {
    }


    void ControlFeatureInterception::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor )
    {
        if ( !_rxInterceptor.is() )
        {
            OSL_FAIL( "ControlFeatureInterception::registerDispatchProviderInterceptor: invalid interceptor!" );
            return;
        }

        if ( m_xFirstDispatchInterceptor.is() )
        {
            // there is already an interceptor; the new one will become its master
            _rxInterceptor->setSlaveDispatchProvider( m_xFirstDispatchInterceptor );
            m_xFirstDispatchInterceptor->setMasterDispatchProvider( m_xFirstDispatchInterceptor );
        }

        // we are the master of the chain's first interceptor
        m_xFirstDispatchInterceptor = _rxInterceptor;
        m_xFirstDispatchInterceptor->setMasterDispatchProvider( nullptr );
            // it's the first of the interceptor chain
    }


    void ControlFeatureInterception::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor )
    {
        if ( !_rxInterceptor.is() )
        {
            OSL_FAIL( "ControlFeatureInterception::releaseDispatchProviderInterceptor: invalid interceptor!" );
            return;
        }

        Reference< XDispatchProviderInterceptor >  xChainWalk( m_xFirstDispatchInterceptor );

        if ( m_xFirstDispatchInterceptor == _rxInterceptor )
        {   // our chain will have a new first element
            m_xFirstDispatchInterceptor.set(m_xFirstDispatchInterceptor->getSlaveDispatchProvider(), UNO_QUERY);
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
                xChainWalk->setSlaveDispatchProvider( nullptr );
                xChainWalk->setMasterDispatchProvider( nullptr );

                // reconnect the chain
                if ( xMaster.is() )
                {
                    xMaster->setSlaveDispatchProvider( xSlave );
                }

                // if somebody has registered the same interceptor twice, then we will remove
                // it once per call ...
                break;
            }

            xChainWalk = xSlave;
        }
    }


    void ControlFeatureInterception::dispose()
    {
        // release all interceptors
        Reference< XDispatchProviderInterceptor > xInterceptor( m_xFirstDispatchInterceptor );
        m_xFirstDispatchInterceptor.clear();
        while ( xInterceptor.is() )
        {
            // tell the interceptor it has a new (means no) predecessor
            xInterceptor->setMasterDispatchProvider( nullptr );

            // ask for its successor
            Reference< XDispatchProvider > xSlave = xInterceptor->getSlaveDispatchProvider();
            // and give it the new (means no) successoert
            xInterceptor->setSlaveDispatchProvider( nullptr );

            // start over with the next chain element
            xInterceptor.set(xSlave, css::uno::UNO_QUERY);
        }
    }

    Reference< XDispatch > ControlFeatureInterception::queryDispatch( const URL& _rURL )
    {
        Reference< XDispatch > xDispatcher;
        if ( m_xFirstDispatchInterceptor.is() )
            xDispatcher = m_xFirstDispatchInterceptor->queryDispatch( _rURL, OUString(), 0 );
        return xDispatcher;
    }


    Reference< XDispatch > ControlFeatureInterception::queryDispatch( const OUString& _rURL )
    {
        return queryDispatch( m_pUrlTransformer->getStrictURL( _rURL ) );
    }


} // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
