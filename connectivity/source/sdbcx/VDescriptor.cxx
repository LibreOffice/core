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

#include <connectivity/sdbcx/VDescriptor.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <functional>
#include <algorithm>
#include <string.h>

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
            return (rId.getLength() == 16 && 0 == memcmp(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
