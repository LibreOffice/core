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
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#define _CONNECTIVITY_SDBCX_COLUMN_HXX_

#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <comphelper/IdPropArrayHelper.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/implbase1.hxx>
#include "connectivity/CommonTools.hxx"
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/sdbcx/VDescriptor.hxx"
#include "connectivity/dbtoolsdllapi.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity
{
    namespace sdbcx
    {
        class OColumn;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OColumn> OColumn_PROP;

        typedef ::cppu::WeakComponentImplHelper2< ::com::sun::star::container::XNamed,
                                                  ::com::sun::star::lang::XServiceInfo> OColumnDescriptor_BASE;
        typedef ::cppu::ImplHelper1< ::com::sun::star::sdbcx::XDataDescriptorFactory > OColumn_BASE;


        class OOO_DLLPUBLIC_DBTOOLS OColumn :
                                    public comphelper::OBaseMutex,
                                    public OColumn_BASE,
                                    public OColumnDescriptor_BASE,
                                    public OColumn_PROP,
                                    public ODescriptor
        {
        protected:
            ::rtl::OUString m_TypeName;
            ::rtl::OUString m_Description;
            ::rtl::OUString m_DefaultValue;

            sal_Int32       m_IsNullable;
            sal_Int32       m_Precision;
            sal_Int32       m_Scale;
            sal_Int32       m_Type;

            sal_Bool        m_IsAutoIncrement;
            sal_Bool        m_IsRowVersion;
            sal_Bool        m_IsCurrency;

            using OColumnDescriptor_BASE::rBHelper;
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const;
            virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

            virtual ~OColumn();
        public:
            virtual void    SAL_CALL acquire() throw();
            virtual void    SAL_CALL release() throw();

            OColumn(    sal_Bool _bCase);
            OColumn(    const ::rtl::OUString& _Name,
                                const ::rtl::OUString& _TypeName,
                                const ::rtl::OUString& _DefaultValue,
                                const ::rtl::OUString& _Description,
                                sal_Int32       _IsNullable,
                                sal_Int32       _Precision,
                                sal_Int32       _Scale,
                                sal_Int32       _Type,
                                sal_Bool        _IsAutoIncrement,
                                sal_Bool        _IsRowVersion,
                                sal_Bool        _IsCurrency,
                                sal_Bool        _bCase);

            DECLARE_SERVICE_INFO();
            //XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            // ODescriptor
            virtual void construct();
            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XNamed
            virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
            // XDataDescriptorFactory
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // _CONNECTIVITY_SDBCX_COLUMN_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
