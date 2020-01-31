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
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <algorithm>

namespace connectivity::sdbcx
{
        using namespace ::com::sun::star::uno;
        using namespace ::com::sun::star::lang;
        using namespace ::com::sun::star::beans;


        // = ODescriptor


        ODescriptor::ODescriptor(::cppu::OBroadcastHelper& _rBHelper, bool _bCase, bool _bNew)
            :ODescriptor_PBASE(_rBHelper)
            ,m_aCase(_bCase)
            ,m_bNew(_bNew)
        {
        }


        // css::lang::XUnoTunnel
        sal_Int64 SAL_CALL ODescriptor::getSomething( const Sequence< sal_Int8 >& rId )
        {
            return (isUnoTunnelId<ODescriptor>(rId))
                ? reinterpret_cast< sal_Int64 >( this )
                : 0;
        }


        namespace
        {
            struct ResetROAttribute
            {
                void operator ()( Property& _rProperty ) const
                {
                    _rProperty.Attributes &= ~PropertyAttribute::READONLY;
                }
            };
            struct SetROAttribute
            {
                void operator ()( Property& _rProperty ) const
                {
                    _rProperty.Attributes |= PropertyAttribute::READONLY;
                }
            };
        }


        ::cppu::IPropertyArrayHelper* ODescriptor::doCreateArrayHelper() const
        {
            Sequence< Property > aProperties;
            describeProperties( aProperties );

            if ( isNew() )
                std::for_each( aProperties.begin(), aProperties.end(), ResetROAttribute() );
            else
                std::for_each( aProperties.begin(), aProperties.end(), SetROAttribute() );

            return new ::cppu::OPropertyArrayHelper( aProperties );
        }


        bool ODescriptor::isNew( const Reference< XInterface >& _rxDescriptor )
        {
            ODescriptor* pImplementation = comphelper::getUnoTunnelImplementation<ODescriptor>( _rxDescriptor );
            return pImplementation && pImplementation->isNew();
        }


        Sequence< sal_Int8 > ODescriptor::getUnoTunnelId()
        {
            static ::cppu::OImplementationId implId;

            return implId.getImplementationId();
        }


        Any SAL_CALL ODescriptor::queryInterface( const Type & rType )
        {
            Any aRet = ::cppu::queryInterface(rType,static_cast< XUnoTunnel*> (this));
            return aRet.hasValue() ? aRet : ODescriptor_PBASE::queryInterface(rType);
        }


        void ODescriptor::setNew(bool _bNew)
        {
            m_bNew = _bNew;
        }


        Sequence< Type > SAL_CALL ODescriptor::getTypes(  )
        {
            ::cppu::OTypeCollection aTypes( cppu::UnoType<XMultiPropertySet>::get(),
                                            cppu::UnoType<XFastPropertySet>::get(),
                                            cppu::UnoType<XPropertySet>::get(),
                                            cppu::UnoType<XUnoTunnel>::get());
            return aTypes.getTypes();
        }

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
