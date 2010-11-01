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

#ifndef _CONNECTIVITY_SDBCX_KEY_HXX_
#define _CONNECTIVITY_SDBCX_KEY_HXX_


#include <comphelper/IdPropArrayHelper.hxx>
#include "connectivity/CommonTools.hxx"
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/sdbcx/VTypeDef.hxx"
#include <com/sun/star/container/XNamed.hpp>
#include "connectivity/sdbcx/IRefreshable.hxx"
#include "connectivity/sdbcx/VDescriptor.hxx"
#include "connectivity/dbtoolsdllapi.hxx"
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <boost/shared_ptr.hpp>

namespace connectivity
{
    namespace sdbcx
    {

        struct OOO_DLLPUBLIC_DBTOOLS KeyProperties
        {
            ::std::vector< ::rtl::OUString> m_aKeyColumnNames;
            ::rtl::OUString m_ReferencedTable;
            sal_Int32       m_Type;
            sal_Int32       m_UpdateRule;
            sal_Int32       m_DeleteRule;
            KeyProperties(const ::rtl::OUString& _ReferencedTable,
                          sal_Int32     _Type,
                          sal_Int32     _UpdateRule,
                          sal_Int32     _DeleteRule)
                          :m_ReferencedTable(_ReferencedTable),
                          m_Type(_Type),
                          m_UpdateRule(_UpdateRule),
                          m_DeleteRule(_DeleteRule)
            {}
            KeyProperties():m_Type(0),m_UpdateRule(0),m_DeleteRule(0){}
        };
        typedef ::boost::shared_ptr< KeyProperties > TKeyProperties;
        typedef ::cppu::ImplHelper1< ::com::sun::star::sdbcx::XDataDescriptorFactory > OKey_BASE;
        class OCollection;

        class OOO_DLLPUBLIC_DBTOOLS OKey :
                                public comphelper::OBaseMutex,
                                public ODescriptor_BASE,
                                public IRefreshableColumns,
                                public ::comphelper::OIdPropertyArrayUsageHelper<OKey>,
                                public ODescriptor,
                                public OKey_BASE
        {
        protected:
            TKeyProperties   m_aProps;
            OCollection*    m_pColumns;

            using ODescriptor_BASE::rBHelper;
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
        public:
            OKey(sal_Bool _bCase);
            OKey(const ::rtl::OUString& _Name,const TKeyProperties& _rProps,sal_Bool _bCase);
            /*OKey( const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _ReferencedTable,
                    sal_Int32       _Type,
                    sal_Int32       _UpdateRule,
                    sal_Int32       _DeleteRule,
                    sal_Bool _bCase);*/

            virtual ~OKey( );

            DECLARE_SERVICE_INFO();
            //XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
            // ODescriptor
            virtual void construct();

            // ::cppu::OComponentHelper
            virtual void SAL_CALL disposing();
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XColumnsSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);

            // XNamed
            virtual ::rtl::OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
            // XDataDescriptorFactory
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // _CONNECTIVITY_SDBCX_KEY_HXX_


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
