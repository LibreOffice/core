/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "controlfeatureinterception.hxx"
#include "urltransformer.hxx"


namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;

    
    
    
    
    ControlFeatureInterception::ControlFeatureInterception( const Reference< XComponentContext >& _rxORB )
        :m_pUrlTransformer( new UrlTransformer( _rxORB ) )
    {
    }

    
    void SAL_CALL ControlFeatureInterception::registerDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException )
    {
        if ( !_rxInterceptor.is() )
        {
            OSL_FAIL( "ControlFeatureInterception::registerDispatchProviderInterceptor: invalid interceptor!" );
            return;
        }

        if ( m_xFirstDispatchInterceptor.is() )
        {
            
            Reference< XDispatchProvider > xFirstProvider( m_xFirstDispatchInterceptor, UNO_QUERY );
            _rxInterceptor->setSlaveDispatchProvider( xFirstProvider );
            m_xFirstDispatchInterceptor->setMasterDispatchProvider( xFirstProvider );
        }

        
        m_xFirstDispatchInterceptor = _rxInterceptor;
        m_xFirstDispatchInterceptor->setMasterDispatchProvider( NULL );
            
    }

    
    void SAL_CALL ControlFeatureInterception::releaseDispatchProviderInterceptor( const Reference< XDispatchProviderInterceptor >& _rxInterceptor ) throw (RuntimeException )
    {
        if ( !_rxInterceptor.is() )
        {
            OSL_FAIL( "ControlFeatureInterception::releaseDispatchProviderInterceptor: invalid interceptor!" );
            return;
        }

        Reference< XDispatchProviderInterceptor >  xChainWalk( m_xFirstDispatchInterceptor );

        if ( m_xFirstDispatchInterceptor == _rxInterceptor )
        {   
            Reference< XDispatchProviderInterceptor >  xSlave( m_xFirstDispatchInterceptor->getSlaveDispatchProvider(), UNO_QUERY );
            m_xFirstDispatchInterceptor = xSlave;
        }
        

        while ( xChainWalk.is() )
        {
            
            Reference< XDispatchProviderInterceptor >  xSlave( xChainWalk->getSlaveDispatchProvider(), UNO_QUERY );

            if ( xChainWalk == _rxInterceptor )
            {
                
                Reference< XDispatchProviderInterceptor >  xMaster( xChainWalk->getMasterDispatchProvider(), UNO_QUERY );

                
                xChainWalk->setSlaveDispatchProvider( NULL );
                xChainWalk->setMasterDispatchProvider( NULL );

                
                if ( xMaster.is() )
                {
                    xMaster->setSlaveDispatchProvider( Reference< XDispatchProvider >::query( xSlave ) );
                }

                
                
                break;
            }

            xChainWalk = xSlave;
        }
    }

    
    void ControlFeatureInterception::dispose()
    {
        
        Reference< XDispatchProviderInterceptor > xInterceptor( m_xFirstDispatchInterceptor );
        m_xFirstDispatchInterceptor.clear();
        while ( xInterceptor.is() )
        {
            
            xInterceptor->setMasterDispatchProvider( NULL );

            
            Reference< XDispatchProvider > xSlave = xInterceptor->getSlaveDispatchProvider();
            
            xInterceptor->setSlaveDispatchProvider( NULL );

            
            xInterceptor = xInterceptor.query( xSlave );
        }
    }
    
    Reference< XDispatch > ControlFeatureInterception::queryDispatch( const URL& _rURL, const OUString& _rTargetFrameName, ::sal_Int32 _nSearchFlags ) SAL_THROW((RuntimeException))
    {
        Reference< XDispatch > xDispatcher;
        if ( m_xFirstDispatchInterceptor.is() )
            xDispatcher = m_xFirstDispatchInterceptor->queryDispatch( _rURL, _rTargetFrameName, _nSearchFlags );
        return xDispatcher;
    }

    
    Reference< XDispatch > ControlFeatureInterception::queryDispatch( const URL& _rURL ) SAL_THROW((RuntimeException))
    {
        return queryDispatch( _rURL, OUString(), 0 );
    }

    
    Reference< XDispatch > ControlFeatureInterception::queryDispatch( const sal_Char* _pAsciiURL ) SAL_THROW((RuntimeException))
    {
        return queryDispatch( m_pUrlTransformer->getStrictURLFromAscii( _pAsciiURL ) );
    }


} 


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
