/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VDescriptor.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:10:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_
#include <connectivity/sdbcx/VDescriptor.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

#include <functional>
#include <algorithm>

namespace connectivity
{
    namespace sdbcx
    {
        using namespace ::com::sun::star::uno;
        using namespace ::com::sun::star::lang;
        using namespace ::com::sun::star::beans;

        // =========================================================================
        // = ODescriptor
        // =========================================================================
        // -------------------------------------------------------------------------
        ODescriptor::ODescriptor(::cppu::OBroadcastHelper& _rBHelper,sal_Bool _bCase, sal_Bool _bNew)
            :ODescriptor_PBASE(_rBHelper)
            ,m_aCase(_bCase)
            ,m_bNew(_bNew)
        {
        }

        // -------------------------------------------------------------------------
        // com::sun::star::lang::XUnoTunnel
        sal_Int64 SAL_CALL ODescriptor::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
        {
            return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ? reinterpret_cast< sal_Int64 >( this )
                : 0;
        }

        // -----------------------------------------------------------------------------
        ODescriptor* ODescriptor::getImplementation( const Reference< XInterface >& _rxSomeComp )
        {
            Reference< XUnoTunnel > xTunnel( _rxSomeComp, UNO_QUERY );
            if ( xTunnel.is() )
                return reinterpret_cast< ODescriptor* >( xTunnel->getSomething( getUnoTunnelImplementationId() ) );
            return NULL;
        }

        // -----------------------------------------------------------------------------
        namespace
        {
            struct ResetROAttribute : public ::std::unary_function< Property, void >
            {
                void operator ()( Property& _rProperty ) const
                {
                    _rProperty.Attributes &= ~PropertyAttribute::READONLY;
                }
            };
            struct SetROAttribute : public ::std::unary_function< Property, void >
            {
                void operator ()( Property& _rProperty ) const
                {
                    _rProperty.Attributes |= PropertyAttribute::READONLY;
                }
            };
        }

        // -----------------------------------------------------------------------------
        ::cppu::IPropertyArrayHelper* ODescriptor::doCreateArrayHelper() const
        {
            Sequence< Property > aProperties;
            describeProperties( aProperties );

            if ( isNew() )
                ::std::for_each( aProperties.getArray(), aProperties.getArray() + aProperties.getLength(), ResetROAttribute() );
            else
                ::std::for_each( aProperties.getArray(), aProperties.getArray() + aProperties.getLength(), SetROAttribute() );

            return new ::cppu::OPropertyArrayHelper( aProperties );
        }

        // -----------------------------------------------------------------------------
        sal_Bool ODescriptor::isNew( const Reference< XInterface >& _rxDescriptor )
        {
            ODescriptor* pImplementation = getImplementation( _rxDescriptor );
            return pImplementation != NULL ? pImplementation->isNew() : sal_False;
        }

        // -----------------------------------------------------------------------------
        Sequence< sal_Int8 > ODescriptor::getUnoTunnelImplementationId()
        {
            static ::cppu::OImplementationId * pId = 0;
            if (! pId)
            {
                ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
                if (! pId)
                {
                    static ::cppu::OImplementationId aId;
                    pId = &aId;
                }
            }
            return pId->getImplementationId();
        }

        // -----------------------------------------------------------------------------
        Any SAL_CALL ODescriptor::queryInterface( const Type & rType ) throw(RuntimeException)
        {
            Any aRet = ::cppu::queryInterface(rType,static_cast< XUnoTunnel*> (this));
            return aRet.hasValue() ? aRet : ODescriptor_PBASE::queryInterface(rType);
        }

        // -----------------------------------------------------------------------------
        void ODescriptor::setNew(sal_Bool _bNew)
        {
            m_bNew = _bNew;
        }

        // -----------------------------------------------------------------------------
        Sequence< Type > SAL_CALL ODescriptor::getTypes(  ) throw(RuntimeException)
        {
            ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                                            ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                                            ::getCppuType( (const Reference< XPropertySet > *)0 ),
                                            ::getCppuType( (const Reference< XUnoTunnel > *)0 ));
            return aTypes.getTypes();
        }

    }
}


