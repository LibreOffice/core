/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accimplaccess.cxx,v $
 * $Revision: 1.9 $
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
#include "precompiled_comphelper.hxx"
#include <comphelper/accimplaccess.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <cppuhelper/typeprovider.hxx>

#include <set>

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

    //---------------------------------------------------------------------
    sal_Bool OAccessibleImplementationAccess::setForeignControlledState( const Reference< XAccessibleContext >& _rxComponent, const sal_Int16 _nState,
        const sal_Bool  _bSet )
    {
        OAccessibleImplementationAccess* pImplementation = getImplementation( _rxComponent );

        if ( pImplementation )
            pImplementation->setStateBit( _nState, _bSet );

        return ( NULL != pImplementation );
    }

    //---------------------------------------------------------------------
    const Sequence< sal_Int8 >& OAccessibleImplementationAccess::getUnoTunnelImplementationId()
    {
        static Sequence< sal_Int8 > aId;
        if ( !aId.getLength() )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !aId.getLength() )
            {
                static ::cppu::OImplementationId aImplId;
                // unfortunately, the OImplementationId::getImplementationId returns a copy, not a static reference ...
                aId = aImplId.getImplementationId();
            }
        }
        return aId;
    }

    //---------------------------------------------------------------------
    sal_Int64 SAL_CALL OAccessibleImplementationAccess::getSomething( const Sequence< sal_Int8 >& _rIdentifier ) throw (RuntimeException)
    {
        sal_Int64 nReturn( 0 );

        if  (   ( _rIdentifier.getLength() == 16 )
            &&  ( 0 == rtl_compareMemory( getUnoTunnelImplementationId().getConstArray(), _rIdentifier.getConstArray(), 16 ) )
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
            OSL_ENSURE( sal_False, "OAccessibleImplementationAccess::setAccessibleParent: caught an exception while retrieving the implementation!" );
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


