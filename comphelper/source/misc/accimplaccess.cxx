/*************************************************************************
 *
 *  $RCSfile: accimplaccess.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:28:13 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef COMPHELPER_ACCIMPLACCESS_HXX
#include <comphelper/accimplaccess.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLECONTEXT_HPP_
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

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
        OSL_ENSURE( _nState < BITFIELDSIZE, "OAccessibleImplementationAccess::setStateBit: no more bits (shutting down the universe now)!" );

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


