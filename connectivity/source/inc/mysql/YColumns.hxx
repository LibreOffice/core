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
#include <connectivity/TColumnsHelper.hxx>
#include <connectivity/sdbcx/VColumn.hxx>

namespace connectivity::mysql
    {
        class OMySQLColumns : public OColumnsHelper
        {
        protected:
            virtual css::uno::Reference< css::beans::XPropertySet > createDescriptor() override;
        public:
            OMySQLColumns(  ::cppu::OWeakObject& _rParent
                            ,::osl::Mutex& _rMutex
                            ,const ::std::vector< OUString> &_rVector
                        );
        };

        class OMySQLColumn;
        typedef ::comphelper::OIdPropertyArrayUsageHelper<OMySQLColumn> OMySQLColumn_PROP;

        class OMySQLColumn :    public sdbcx::OColumn,
                                public OMySQLColumn_PROP
        {
            OUString m_sAutoIncrement;
        protected:
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( sal_Int32 _nId) const override;
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

        public:
            OMySQLColumn();
            virtual void construct() override;

            virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
