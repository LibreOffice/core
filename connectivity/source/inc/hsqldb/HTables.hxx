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

#include <connectivity/sdbcx/VCollection.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <utility>
namespace connectivity::hsqldb
{
    class OTables final : public sdbcx::OCollection
    {
        css::uno::Reference< css::sdbc::XDatabaseMetaData >       m_xMetaData;

        virtual css::uno::Reference< css::beans::XPropertySet > createObject(const OUString& _rName) override;
        virtual void impl_refresh() override;
        virtual css::uno::Reference< css::beans::XPropertySet > createDescriptor() override;
        virtual css::uno::Reference< css::beans::XPropertySet > appendObject( const OUString& _rForName, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;
        virtual void dropObject(sal_Int32 _nPos, const OUString& _sElementName) override;

        void createTable( const css::uno::Reference< css::beans::XPropertySet >& descriptor );
        virtual OUString getNameForObject(const css::uno::Reference< css::beans::XPropertySet >& _xObject) override;
    public:
        OTables(css::uno::Reference< css::sdbc::XDatabaseMetaData > _xMetaData, ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
            const ::std::vector< OUString> &_rVector) : sdbcx::OCollection(_rParent, true, _rMutex, _rVector)
            ,m_xMetaData(std::move(_xMetaData))
        {}

        // only the name is identical to ::cppu::OComponentHelper
        virtual void disposing() override;

        // XDrop
        void appendNew(const OUString& _rsNewTable);
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
