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

#ifndef __FRAMEWORK_ACCELERATORS_MODULEACCELERATORCONFIGURATION_HXX_
#define __FRAMEWORK_ACCELERATORS_MODULEACCELERATORCONFIGURATION_HXX_

#include <accelerators/acceleratorconfiguration.hxx>
#include <accelerators/presethandler.hxx>

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase2.hxx>

// definition

namespace framework
{

//__________________________________________
/**
    implements a read/write access to a module
    dependend accelerator configuration.
 */
typedef ::cppu::ImplInheritanceHelper2<
            XCUBasedAcceleratorConfiguration,
            css::lang::XServiceInfo,
            css::lang::XInitialization > ModuleAcceleratorConfiguration_BASE;

class ModuleAcceleratorConfiguration : ModuleAcceleratorConfiguration_BASE
{
    //______________________________________
    // member

    private:

        //----------------------------------
        /** identify the application module, where this accelerator
            configuration cache should work on. */
        OUString m_sModule;
        OUString m_sLocale;

    //______________________________________
    // interface

    public:

        //----------------------------------
        /** initialize this instance and fill the internal cache.

            @param  xSMGR
                    reference to an uno service manager, which is used internaly.
         */
        ModuleAcceleratorConfiguration(const css::uno::Reference< css::uno::XComponentContext >& xContext);

        //----------------------------------
        /** TODO */
        virtual ~ModuleAcceleratorConfiguration();

        // XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        // XComponent
        virtual  void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

    //______________________________________
    // helper

    private:
        /** helper to listen for configuration changes without ownership cycle problems */
        css::uno::Reference< css::util::XChangesListener > m_xCfgListener;

        //----------------------------------
        /** read all data into the cache. */
        void impl_ts_fillCache();
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_MODULEACCELERATORCONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
