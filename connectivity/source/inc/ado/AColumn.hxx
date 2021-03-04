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

#include <connectivity/sdbcx/VColumn.hxx>
#include <ado/Awrapadox.hxx>

namespace connectivity::ado
{
        class OConnection;
        typedef sdbcx::OColumn OColumn_ADO;
        class OAdoColumn :  public OColumn_ADO
        {
            WpADOColumn     m_aColumn;
            OConnection*    m_pConnection;
            OUString m_ReferencedColumn;
            bool            m_IsAscending;

            void fillPropertyValues();
        protected:
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const css::uno::Any& rValue
                                     ) override;
        public:
            OAdoColumn(bool _bCase,OConnection* _pConnection,_ADOColumn* _pColumn);
            OAdoColumn(bool _bCase,OConnection* _pConnection);
            // ODescriptor
            virtual void construct() override;
            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelId();

            WpADOColumn     getColumnImpl() const;
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
