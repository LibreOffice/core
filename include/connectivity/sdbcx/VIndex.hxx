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

#ifndef _CONNECTIVITY_SDBCX_INDEX_HXX_
#define _CONNECTIVITY_SDBCX_INDEX_HXX_

#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <comphelper/IdPropArrayHelper.hxx>
#include <cppuhelper/compbase2.hxx>
#include "connectivity/CommonTools.hxx"
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/sdbcx/VTypeDef.hxx"
#include "connectivity/sdbcx/IRefreshable.hxx"
#include "connectivity/sdbcx/VDescriptor.hxx"
#include "connectivity/dbtoolsdllapi.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase1.hxx>

namespace connectivity
{
    namespace sdbcx
    {
        class OCollection;
        class OIndex;
        typedef ::cppu::ImplHelper1< ::com::sun::star::sdbcx::XDataDescriptorFactory > OIndex_BASE;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OIndex> OIndex_PROP;

        class OOO_DLLPUBLIC_DBTOOLS OIndex :
                                    public comphelper::OBaseMutex,
                                    public ODescriptor_BASE,
                                    public IRefreshableColumns,
                                    public OIndex_PROP,
                                    public ODescriptor,
                                    public OIndex_BASE
        {
        protected:
            OUString m_Catalog;
            sal_Bool        m_IsUnique;
            sal_Bool        m_IsPrimaryKeyIndex;
            sal_Bool        m_IsClustered;

            OCollection*    m_pColumns;

            using ODescriptor_BASE::rBHelper;
            virtual void refreshColumns();
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const;
            virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        public:
            OIndex(sal_Bool _bCase);
            OIndex( const OUString& _Name,
                    const OUString& _Catalog,
                    sal_Bool _isUnique,
                    sal_Bool _isPrimaryKeyIndex,
                    sal_Bool _isClustered,
                    sal_Bool _bCase);

            virtual ~OIndex( );

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
            virtual void SAL_CALL disposing(void);
            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
            // XColumnsSupplier
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);

            // XNamed
            virtual OUString SAL_CALL getName(  ) throw(::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL setName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
            // XDataDescriptorFactory
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL createDataDescriptor(  ) throw(::com::sun::star::uno::RuntimeException);
        };
    }
}

#endif // _CONNECTIVITY_SDBCX_INDEX_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
