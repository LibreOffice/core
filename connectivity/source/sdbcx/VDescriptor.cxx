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
        using namespace ::com::sun::star::beans;


        // = ODescriptor


        ODescriptor::ODescriptor(::cppu::OBroadcastHelper& _rBHelper, bool _bCase, bool _bNew)
            :ODescriptor_PBASE(_rBHelper)
            ,m_aCase(_bCase)
            ,m_bNew(_bNew)
        {
        }


        // css::lang::XUnoTunnel
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

            auto [begin, end] = asNonConstRange(aProperties);
            if ( isNew() )
                std::for_each( begin, end, ResetROAttribute() );
            else
                std::for_each( begin, end, SetROAttribute() );

            return new ::cppu::OPropertyArrayHelper( aProperties );
        }


        bool ODescriptor::isNew( const Reference< XInterface >& _rxDescriptor )
        {
            ODescriptor* pImplementation = dynamic_cast<ODescriptor*>( _rxDescriptor.get() );
            return pImplementation && pImplementation->isNew();
        }


        void ODescriptor::setNew(bool _bNew)
        {
            m_bNew = _bNew;
        }


        Sequence< Type > SAL_CALL ODescriptor::getTypes(  )
        {
            ::cppu::OTypeCollection aTypes( cppu::UnoType<XMultiPropertySet>::get(),
                                            cppu::UnoType<XFastPropertySet>::get(),
                                            cppu::UnoType<XPropertySet>::get());
            return aTypes.getTypes();
        }

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
