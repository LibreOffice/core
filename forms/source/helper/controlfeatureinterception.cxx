/*************************************************************************
 *
 *  $RCSfile: controlfeatureinterception.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:43:30 $
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

#ifndef FORMS_SOURCE_INC_CONTROLFEATUREINTERCEPTION_HXX
#include "controlfeatureinterception.hxx"
#endif
#ifndef FORMS_SOURCE_INC_URLTRANSFORMER_HXX
#include "urltransformer.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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

