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

#ifndef INCLUDED_FRAMEWORK_MENUCONFIGURATION_HXX
#define INCLUDED_FRAMEWORK_MENUCONFIGURATION_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <framework/fwedllapi.h>

#include <cppuhelper/weak.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>

#define BOOKMARK_NEWMENU        "private:menu_bookmark_new"
#define BOOKMARK_WIZARDMENU     "private:menu_bookmark_wizard"

// Prepare for inclusion by framework and sfx
// Please consider that there is a corresponding define also in sfxsids.hrc!! (SID_SFX_START)/(SID_ADDONS)
#define FWK_SID_SFX_START 5000
#define FWK_SID_ADDONS (FWK_SID_SFX_START+1678)
#define FWK_SID_ADDONHELP (FWK_SID_SFX_START+1684)

const sal_uInt16 START_ITEMID_PICKLIST      = 4500;
const sal_uInt16 END_ITEMID_PICKLIST        = 4599;
const sal_uInt16 MAX_ITEMCOUNT_PICKLIST     =   99; // difference between START_... & END_... for picklist / must be changed too, if these values are changed!
const sal_uInt16 START_ITEMID_WINDOWLIST    = 4600;
const sal_uInt16 END_ITEMID_WINDOWLIST      = 4699;
const sal_uInt16 ITEMID_ADDONLIST           = FWK_SID_ADDONS;
const sal_uInt16 ITEMID_ADDONHELP           = FWK_SID_ADDONHELP;

namespace framework
{

struct FWE_DLLPUBLIC MenuAttributes
{
private:
    oslInterlockedCount refCount;

    MenuAttributes(const OUString& rFrame, const OUString& rImageIdStr)
        : refCount(0)
        , aTargetFrame(rFrame)
        , aImageId(rImageIdStr)
        , nStyle(0)
    {
    }

    MenuAttributes(const css::uno::WeakReference<css::frame::XDispatchProvider>& rDispatchProvider)
        : refCount(0)
        , xDispatchProvider(rDispatchProvider)
        , nStyle(0)
    {
    }

    MenuAttributes(const MenuAttributes&) = delete;

public:
    OUString aTargetFrame;
    OUString aImageId;
    css::uno::WeakReference<css::frame::XDispatchProvider> xDispatchProvider;
    sal_Int16 nStyle;

    static sal_uIntPtr CreateAttribute(const OUString& rFrame, const OUString& rImageIdStr);
    static sal_uIntPtr CreateAttribute(const css::uno::WeakReference<css::frame::XDispatchProvider>& rDispatchProvider);
    static void ReleaseAttribute(sal_uIntPtr nAttributePtr);

    void acquire()
    {
        osl_atomic_increment(&refCount);
    }

    void release()
    {
        if (!osl_atomic_decrement(&refCount))
            delete this;
    }
};

class FWE_DLLPUBLIC MenuConfiguration
{
public:
        MenuConfiguration(
            // use const when giving a uno reference by reference
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );

        virtual ~MenuConfiguration();

        css::uno::Reference< css::container::XIndexAccess > CreateMenuBarConfigurationFromXML(
            css::uno::Reference< css::io::XInputStream >& rInputStream )
            throw (css::lang::WrappedTargetException, css::uno::RuntimeException);

        PopupMenu* CreateBookmarkMenu(css::uno::Reference<css::frame::XFrame >& rFrame, const OUString& aURL)
            throw (css::lang::WrappedTargetException, css::uno::RuntimeException);

        void StoreMenuBarConfigurationToXML(
                      css::uno::Reference< css::container::XIndexAccess >& rMenuBarConfiguration,
                      css::uno::Reference< css::io::XOutputStream >& rOutputStream )
            throw (css::lang::WrappedTargetException, css::uno::RuntimeException);

private:
        css::uno::Reference< css::uno::XComponentContext> m_xContext;
};

}

#endif // INCLUDED_FRAMEWORK_MENUCONFIGURATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
