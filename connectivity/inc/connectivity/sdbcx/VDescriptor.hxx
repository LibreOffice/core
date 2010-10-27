/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
            ::rtl::OUString         m_Name;

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
            inline sal_Bool operator == ( const ::rtl::OUString & _rRH )
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
