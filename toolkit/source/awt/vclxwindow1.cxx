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

#include <toolkit/awt/vclxwindow.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <vcl/wrkwin.hxx>
#include <vcl/window.hxx>

#ifdef _WIN32
#include <prewin.h>
#include <postwin.h>
#elif defined(MACOSX)
#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>
#endif
#include <vcl/sysdata.hxx>

/// helper method to set a window handle into a SystemParentData struct
void VCLXWindow::SetSystemParent_Impl(const css::uno::Any& rHandle)
{
    // does only work for WorkWindows
    VclPtr<vcl::Window> pWindow = GetWindow();
    if (pWindow->GetType() != WindowType::WORKWINDOW)
    {
        throw css::uno::RuntimeException(u"not a work window"_ustr);
    }

    // use sal_Int64 here to accommodate all int types
    // uno::Any shift operator will upcast if necessary
    sal_Int64 nHandle = 0;
    bool bXEmbed = false;
    bool bThrow = false;
    if (!(rHandle >>= nHandle))
    {
        css::uno::Sequence<css::beans::NamedValue> aProps;
        if (rHandle >>= aProps)
        {
            for (const css::beans::NamedValue& rProp : aProps)
            {
                if (rProp.Name == "WINDOW")
                    rProp.Value >>= nHandle;
                else if (rProp.Name == "XEMBED")
                    rProp.Value >>= bXEmbed;
            }
        }
        else
            bThrow = true;
    }
    if (bThrow)
    {
        throw css::uno::RuntimeException(u"incorrect window handle type"_ustr);
    }
    // create system parent data
    SystemParentData aSysParentData;
    aSysParentData.nSize = sizeof(SystemParentData);
#if defined(_WIN32)
    aSysParentData.hWnd = reinterpret_cast<HWND>(nHandle);
#elif defined(MACOSX)
    aSysParentData.pView = reinterpret_cast<NSView*>(nHandle);
#elif defined(ANDROID)
    // Nothing
#elif defined(IOS)
    // Nothing
#elif defined(UNX)
    aSysParentData.aWindow = nHandle;
    aSysParentData.bXEmbedSupport = bXEmbed;
#endif

    // set system parent
    static_cast<WorkWindow*>(pWindow.get())->SetPluginParent(&aSysParentData);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
