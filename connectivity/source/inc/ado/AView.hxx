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

#include <connectivity/sdbcx/VView.hxx>
#include <ado/Awrapadox.hxx>

namespace connectivity::ado
{

        typedef sdbcx::OView OView_ADO;

        class OAdoView :     public OView_ADO
        {
            WpADOView       m_aView;

        protected:
            // OPropertySetHelper
            virtual void SAL_CALL getFastPropertyValue(
                                css::uno::Any& rValue,
                                sal_Int32 nHandle
                                         ) const override;
        public:
            OAdoView(bool _bCase, ADOView* _pView=nullptr);

            // css::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;
            static css::uno::Sequence< sal_Int8 > getUnoTunnelId();

            WpADOView getImpl() const { return m_aView;}
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
