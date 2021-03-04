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

#include <sdbcx/VKey.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <ado/Awrapadox.hxx>

namespace connectivity::ado
{
        typedef sdbcx::OKey OKey_ADO;

        class OConnection;
        class OAdoKey : public OKey_ADO
        {
            WpADOKey        m_aKey;
            OConnection*    m_pConnection;
        protected:
            void fillPropertyValues();
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const css::uno::Any& rValue) override;
        public:
            virtual void refreshColumns() override;
        public:
            OAdoKey(bool _bCase,OConnection* _pConnection,ADOKey* _pKey);
            OAdoKey(bool _bCase,OConnection* _pConnection);

            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelId();

            WpADOKey        getImpl() const { return m_aKey;}
            // map the update/delete rules
            static RuleEnum Map2Rule(sal_Int32 _eNum);
            static sal_Int32 MapRule(const RuleEnum& _eNum);

            // map the keytypes
            static sal_Int32 MapKeyRule(const KeyTypeEnum& _eNum);
            static KeyTypeEnum Map2KeyRule(sal_Int32 _eNum);
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
