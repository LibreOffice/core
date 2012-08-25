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

#include <comphelper/accimplaccess.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <cppuhelper/typeprovider.hxx>

#include <set>
#include <string.h>

//.........................................................................
namespace comphelper
{
//.........................................................................

#define BITFIELDSIZE    ( sizeof( sal_Int64 ) * 8 )
    // maximum number of bits we have in a sal_Int64

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::lang::XUnoTunnel;
    using ::com::sun::star::accessibility::XAccessible;
    using ::com::sun::star::accessibility::XAccessibleContext;

    //=====================================================================
    //= OAccImpl_Impl
    //=====================================================================
    struct OAccImpl_Impl
    {
        Reference< XAccessible >    m_xAccParent;
        sal_Int64                   m_nForeignControlledStates;
    };


    //=====================================================================
    //= OAccessibleImplementationAccess
    //=====================================================================
    //---------------------------------------------------------------------
    OAccessibleImplementationAccess::OAccessibleImplementationAccess( )
        :m_pImpl( new OAccImpl_Impl )
    {
    }

    //---------------------------------------------------------------------
    OAccessibleImplementationAccess::~OAccessibleImplementationAccess( )
    {
        delete m_pImpl;
        m_pImpl = NULL;
    }

    //---------------------------------------------------------------------
    Reference< XAccessible > OAccessibleImplementationAccess::implGetForeignControlledParent( ) const
    {
        return m_pImpl->m_xAccParent;
    }

    //---------------------------------------------------------------------
    void OAccessibleImplementationAccess::setAccessibleParent( const Reference< XAccessible >& _rxAccParent )
    {
        m_pImpl->m_xAccParent = _rxAccParent;
    }

    //---------------------------------------------------------------------
    sal_Int64 OAccessibleImplementationAccess::implGetForeignControlledStates( ) const
    {
        return m_pImpl->m_nForeignControlledStates;
    }

    //---------------------------------------------------------------------
    void OAccessibleImplementationAccess::setStateBit( const sal_Int16 _nState, const sal_Bool _bSet )
    {
        OSL_ENSURE( _nState >= 0 && static_cast< sal_uInt16 >(_nState) < BITFIELDSIZE, "OAccessibleImplementationAccess::setStateBit: no more bits (shutting down the universe now)!" );

        sal_uInt64 nBitMask( 1 );
        nBitMask <<= _nState;
        if ( _bSet )
            m_pImpl->m_nForeignControlledStates |= nBitMask;
        else
            m_pImpl->m_nForeignControlledStates &= ~nBitMask;
    }

    namespace { struct lcl_ImplId : public rtl::Static< ::cppu::OImplementationId, lcl_ImplId > {}; }

    //---------------------------------------------------------------------
    const Sequence< sal_Int8 > OAccessibleImplementationAccess::getUnoTunnelImplementationId()
    {
        ::cppu::OImplementationId &rID = lcl_ImplId::get();
        return rID.getImplementationId();
    }

    //---------------------------------------------------------------------
    sal_Int64 SAL_CALL OAccessibleImplementationAccess::getSomething( const Sequence< sal_Int8 >& _rIdentifier ) throw (RuntimeException)
    {
        sal_Int64 nReturn( 0 );

        if  (   ( _rIdentifier.getLength() == 16 )
            &&  ( 0 == memcmp( getUnoTunnelImplementationId().getConstArray(), _rIdentifier.getConstArray(), 16 ) )
            )
            nReturn = reinterpret_cast< sal_Int64 >( this );

        return nReturn;
    }

    //---------------------------------------------------------------------
    OAccessibleImplementationAccess* OAccessibleImplementationAccess::getImplementation( const Reference< XAccessibleContext >& _rxComponent )
    {
        OAccessibleImplementationAccess* pImplementation = NULL;
        try
        {
            Reference< XUnoTunnel > xTunnel( _rxComponent, UNO_QUERY );
            if ( xTunnel.is() )
            {
                pImplementation = reinterpret_cast< OAccessibleImplementationAccess* >(
                        xTunnel->getSomething( getUnoTunnelImplementationId() ) );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "OAccessibleImplementationAccess::setAccessibleParent: caught an exception while retrieving the implementation!" );
        }
        return pImplementation;
    }

    //---------------------------------------------------------------------
    sal_Bool OAccessibleImplementationAccess::setAccessibleParent(
            const Reference< XAccessibleContext >& _rxComponent, const Reference< XAccessible >& _rxNewParent )
    {
        OAccessibleImplementationAccess* pImplementation = getImplementation( _rxComponent );

        if ( pImplementation )
            pImplementation->setAccessibleParent( _rxNewParent );

        return ( NULL != pImplementation );
    }

//.........................................................................
}   // namespace comphelper
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
