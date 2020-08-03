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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <connectivity/CommonTools.hxx>
#include <file/FDriver.hxx>

namespace connectivity::dbase
    {
        /// @throws css::uno::Exception
        css::uno::Reference< css::uno::XInterface > ODriver_CreateInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxFactory);

        class ODriver : public file::OFileDriver
        {
        public:
            ODriver(const css::uno::Reference< css::uno::XComponentContext >& _rxContext) : file::OFileDriver(_rxContext){}

            OUString SAL_CALL getImplementationName(  ) override;

            // XDriver
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL connect( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) override;
            virtual sal_Bool SAL_CALL acceptsURL( const OUString& url ) override;
            virtual css::uno::Sequence< css::sdbc::DriverPropertyInfo > SAL_CALL getPropertyInfo( const OUString& url, const css::uno::Sequence< css::beans::PropertyValue >& info ) override;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
