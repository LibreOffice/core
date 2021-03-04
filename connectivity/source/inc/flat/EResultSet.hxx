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

#include <file/FResultSet.hxx>
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#include <cppuhelper/implbase1.hxx>

namespace connectivity::flat
    {
        class OFlatResultSet;
        // these typedef's are only necessary for the compiler
        typedef ::cppu::ImplHelper1<  css::sdbcx::XRowLocate> OFlatResultSet_BASE;
        typedef file::OResultSet                                            OFlatResultSet_BASE2;
        typedef ::comphelper::OPropertyArrayUsageHelper<OFlatResultSet>     OFlatResultSet_BASE3;


        class OFlatResultSet :  public OFlatResultSet_BASE2,
                                public OFlatResultSet_BASE,
                                public OFlatResultSet_BASE3
        {
            bool m_bBookmarkable;
        protected:
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
        public:
            DECLARE_SERVICE_INFO();

            OFlatResultSet( file::OStatement_Base* pStmt,connectivity::OSQLParseTreeIterator&   _aSQLIterator);

            // XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

            // XRowLocate
            virtual css::uno::Any SAL_CALL getBookmark(  ) override;
            virtual sal_Bool SAL_CALL moveToBookmark( const css::uno::Any& bookmark ) override;
            virtual sal_Bool SAL_CALL moveRelativeToBookmark( const css::uno::Any& bookmark, sal_Int32 rows ) override;
            virtual sal_Int32 SAL_CALL compareBookmarks( const css::uno::Any& first, const css::uno::Any& second ) override;
            virtual sal_Bool SAL_CALL hasOrderedBookmarks(  ) override;
            virtual sal_Int32 SAL_CALL hashBookmark( const css::uno::Any& bookmark ) override;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
