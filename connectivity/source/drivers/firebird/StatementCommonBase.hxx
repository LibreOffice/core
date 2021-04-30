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

#pragma once

#include <sal/config.h>

#include <string_view>

#include "Connection.hxx"
#include "SubComponent.hxx"

#include <ibase.h>

#include <cppuhelper/compbase.hxx>
#include <rtl/ref.hxx>

#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/util/XCancellable.hpp>

namespace connectivity::firebird
    {

        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XWarningsSupplier,
                                                   css::util::XCancellable,
                                                   css::sdbc::XCloseable,
                                                   css::sdbc::XMultipleResults> OStatementCommonBase_Base;

        class OStatementCommonBase  :   public  OStatementCommonBase_Base,
                                        public  ::cppu::OPropertySetHelper,
                                        public  OPropertyArrayUsageHelper<OStatementCommonBase>

        {
        protected:
            ::osl::Mutex        m_aMutex;

            css::uno::Reference< css::sdbc::XResultSet> m_xResultSet;   // The last ResultSet created
            //  for this Statement

            ::rtl::Reference<Connection>                m_pConnection;

            ISC_STATUS_ARRAY                            m_statusVector;
            isc_stmt_handle                             m_aStatementHandle;

        protected:
            virtual void disposeResultSet();
            /// @throws css::sdbc::SQLException
            void freeStatementHandle();

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            using OPropertySetHelper::getFastPropertyValue;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                                                css::uno::Any & rConvertedValue,
                                                                css::uno::Any & rOldValue,
                                                                sal_Int32 nHandle,
                                                                const css::uno::Any& rValue ) override;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                                                sal_Int32 nHandle,
                                                                const css::uno::Any& rValue) override;
            virtual void SAL_CALL getFastPropertyValue(
                                                                css::uno::Any& rValue,
                                                                sal_Int32 nHandle) const override;
            virtual ~OStatementCommonBase() override;

            /// @throws css::sdbc::SQLException
            void prepareAndDescribeStatement(std::u16string_view sqlIn,
                                             XSQLDA*& pOutSqlda,
                                             XSQLDA* pInSqlda=nullptr);

            /// @throws css::sdbc::SQLException
            short getSqlInfoItem(char aInfoItem);
            /// @throws css::sdbc::SQLException
            bool isDDLStatement();
            /// @throws css::sdbc::SQLException
            sal_Int32 getStatementChangeCount();

        public:

            explicit OStatementCommonBase(Connection* _pConnection);
            using OStatementCommonBase_Base::operator css::uno::Reference< css::uno::XInterface >;

            // OComponentHelper
            virtual void SAL_CALL disposing() override {
                disposeResultSet();
                OStatementCommonBase_Base::disposing();
            }
            // XInterface
            virtual void SAL_CALL release() noexcept override;
            virtual void SAL_CALL acquire() noexcept override;
            // XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

            // XWarningsSupplier - UNSUPPORTED
            virtual css::uno::Any SAL_CALL getWarnings(  ) override;
            virtual void SAL_CALL clearWarnings(  ) override;
            // XMultipleResults - UNSUPPORTED
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getResultSet(  ) override;
            virtual sal_Int32 SAL_CALL getUpdateCount(  ) override;
            virtual sal_Bool SAL_CALL getMoreResults(  ) override;

            // XCancellable
            virtual void SAL_CALL cancel(  ) override;
            // XCloseable
            virtual void SAL_CALL close(  ) override;

        };

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
