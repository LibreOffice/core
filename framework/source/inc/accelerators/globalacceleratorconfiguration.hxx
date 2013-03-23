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
#if 1

#include <accelerators/acceleratorconfiguration.hxx>
#include <accelerators/presethandler.hxx>

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

#include <com/sun/star/lang/XInitialization.hpp>

// definition

namespace framework
{

//__________________________________________
/**
    implements a read/write access to the global
    accelerator configuration.
 */
class GlobalAcceleratorConfiguration : public XCUBasedAcceleratorConfiguration
                                     , public css::lang::XServiceInfo
                                     , public css::lang::XInitialization
{
    //______________________________________
    // interface

    public:

        //----------------------------------
        /** initialize this instance and fill the internal cache.

            @param  xSMGR
                    reference to an uno service manager, which is used internaly.
         */
        GlobalAcceleratorConfiguration(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR);

        //----------------------------------
        /** TODO */
        virtual ~GlobalAcceleratorConfiguration();

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw (css::uno::Exception       ,
            css::uno::RuntimeException);

    //______________________________________
    // helper

    private:

        ::rtl::OUString m_sLocale;

        /** helper to listen for configuration changes without ownership cycle problems */
        css::uno::Reference< css::util::XChangesListener > m_xCfgListener;

        //----------------------------------
        /** read all data into the cache. */
        void impl_ts_fillCache();
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_GLOBALACCELERATORCONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
