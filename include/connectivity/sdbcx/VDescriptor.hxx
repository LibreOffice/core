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

#ifndef _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_
#define _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_

#include <comphelper/propertycontainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/stl_types.hxx>
#include "connectivity/dbtoolsdllapi.hxx"

namespace connectivity
{
    namespace sdbcx
    {
        // =========================================================================
        // = ODescriptor
        // =========================================================================
        typedef ::comphelper::OPropertyContainer ODescriptor_PBASE;
        class OOO_DLLPUBLIC_DBTOOLS ODescriptor
                    :public ODescriptor_PBASE
                    ,public ::com::sun::star::lang::XUnoTunnel
        {
        protected:
            OUString         m_Name;

            /** helper for derived classes to implement OPropertyArrayUsageHelper::createArrayHelper

                This method just calls describeProperties, and flags all properties as READONLY if and
                only if we do *not* act as descriptor, but as final object.

                @seealso    isNew
            */
            ::cppu::IPropertyArrayHelper*   doCreateArrayHelper() const;

        private:
            comphelper::UStringMixEqual m_aCase;
            sal_Bool                    m_bNew;

        public:
            ODescriptor(::cppu::OBroadcastHelper& _rBHelper,sal_Bool _bCase, sal_Bool _bNew = sal_False);

            virtual ~ODescriptor();

            sal_Bool isNew()  const         { return m_bNew;    }
            sal_Bool getNew() const         { return m_bNew;    }
            virtual void     setNew(sal_Bool _bNew);

            sal_Bool isCaseSensitive() const { return m_aCase.isCaseSensitive(); }

            virtual void construct();

            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);

            // compare
            inline sal_Bool operator == ( const OUString & _rRH )
            {
                return m_aCase(m_Name,_rRH);
            }

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();

            static ODescriptor* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxSomeComp );
            // retrieves the ODescriptor implementation of a given UNO component, and returns its ->isNew flag
            static sal_Bool isNew( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxDescriptor );
        };
    }

}
#endif // _CONNECTIVITY_SDBCX_DESCRIPTOR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
