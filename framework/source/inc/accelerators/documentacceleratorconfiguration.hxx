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

#ifndef __FRAMEWORK_ACCELERATORS_DOCUMENTACCELERATORCONFIGURATION_HXX_
#define __FRAMEWORK_ACCELERATORS_DOCUMENTACCELERATORCONFIGURATION_HXX_

#include <accelerators/acceleratorconfiguration.hxx>
#include <accelerators/istoragelistener.hxx>
#include <accelerators/presethandler.hxx>

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>

// definition

namespace framework
{

//__________________________________________
/**
    implements a read/write access to a document
    based accelerator configuration.
 */
class DocumentAcceleratorConfiguration : public XMLBasedAcceleratorConfiguration
                                       , public css::lang::XServiceInfo
                                       , public css::lang::XInitialization
//                                       , public css::ui::XUIConfigurationStorage
{
    //______________________________________
    // member

    private:

        //----------------------------------
        /** points to the root storage of the outside document,
            where we can read/save our configuration data. */
        css::uno::Reference< css::embed::XStorage > m_xDocumentRoot;

    //______________________________________
    // interface

    public:

        //----------------------------------
        /** initialize this instance and fill the internal cache.

            @param  xSMGR
                    reference to an uno service manager, which is used internaly.
         */
        DocumentAcceleratorConfiguration(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR);
        virtual ~DocumentAcceleratorConfiguration();

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        // XUIConfigurationStorage
        virtual void SAL_CALL setStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
            throw(css::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasStorage()
            throw(css::uno::RuntimeException);

    //______________________________________
    // helper

    private:

        //----------------------------------
        /** read all data into the cache. */
        void impl_ts_fillCache();

        //----------------------------------
        /** forget all currently cached data AND(!)
            forget all currently used storages. */
        void impl_ts_clearCache();
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_DOCUMENTACCELERATORCONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
