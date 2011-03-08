/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_ACCELERATORS_DOCUMENTACCELERATORCONFIGURATION_HXX_
#define __FRAMEWORK_ACCELERATORS_DOCUMENTACCELERATORCONFIGURATION_HXX_

//__________________________________________
// own includes

#include <accelerators/acceleratorconfiguration.hxx>
#include <accelerators/istoragelistener.hxx>
#include <accelerators/presethandler.hxx>

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>

//__________________________________________
// interface includes
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>

//__________________________________________
// other includes

//__________________________________________
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
