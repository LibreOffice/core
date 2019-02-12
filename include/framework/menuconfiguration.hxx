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

#include <framework/fwedllapi.h>

#include <cppuhelper/weakref.hxx>

namespace com::sun::star::container { class XIndexAccess; }
namespace com::sun::star::frame { class XDispatchProvider; }
namespace com::sun::star::io { class XInputStream; }
namespace com::sun::star::io { class XOutputStream; }
namespace com::sun::star::uno { class XComponentContext; }

const sal_uInt16 START_ITEMID_WINDOWLIST    = 4600;
const sal_uInt16 END_ITEMID_WINDOWLIST      = 4699;
const sal_uInt16 ITEMID_ADDONLIST           = 6678; // used to be a SID in sfx2, now just a unique id...

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
    {
    }

    MenuAttributes(const css::uno::WeakReference<css::frame::XDispatchProvider>& rDispatchProvider)
        : refCount(0)
        , xDispatchProvider(rDispatchProvider)
    {
    }

    MenuAttributes(const MenuAttributes&) = delete;

public:
    OUString aTargetFrame;
    OUString aImageId;
    css::uno::WeakReference<css::frame::XDispatchProvider> xDispatchProvider;

    static void* CreateAttribute(const OUString& rFrame, const OUString& rImageIdStr);
    static void* CreateAttribute(const css::uno::WeakReference<css::frame::XDispatchProvider>& rDispatchProvider);
    static void ReleaseAttribute(void* nAttributePtr);

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

class FWE_DLLPUBLIC MenuConfiguration final
{
public:
        MenuConfiguration(
            // use const when giving a uno reference by reference
            const css::uno::Reference< css::uno::XComponentContext >& rxContext );

        ~MenuConfiguration();

        /// @throws css::lang::WrappedTargetException
        /// @throws css::uno::RuntimeException
        css::uno::Reference< css::container::XIndexAccess > CreateMenuBarConfigurationFromXML(
            css::uno::Reference< css::io::XInputStream > const & rInputStream );

        /// @throws css::lang::WrappedTargetException
        /// @throws css::uno::RuntimeException
        void StoreMenuBarConfigurationToXML(
                      css::uno::Reference< css::container::XIndexAccess > const & rMenuBarConfiguration,
                      css::uno::Reference< css::io::XOutputStream > const & rOutputStream,
                      bool bIsMenuBar );

private:
        css::uno::Reference< css::uno::XComponentContext> m_xContext;
};

}

#endif // INCLUDED_FRAMEWORK_MENUCONFIGURATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
