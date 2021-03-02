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

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <config_options.h>
#include <com/sun/star/uno/Reference.h>
#include <vcl/dllapi.h>
#include <vcl/vclptr.hxx>

namespace vcl { class Window; }
class Menu;
class OutputDevice;
class PopupMenu;
namespace com::sun::star::awt {
    class XGraphics;
    class XPopupMenu;
    class XToolkit;
    class XWindow;
    class XWindowPeer;
}
namespace com::sun::star::accessibility {
    class XAccessible;
}

class UNLESS_MERGELIBS(VCL_DLLPUBLIC) UnoWrapperBase
{
public:
    virtual void                Destroy() = 0;

    // Toolkit
    virtual css::uno::Reference< css::awt::XToolkit > GetVCLToolkit() = 0;

    // Graphics
    virtual css::uno::Reference< css::awt::XGraphics >    CreateGraphics( OutputDevice* pOutDev ) = 0;
    virtual void                ReleaseAllGraphics( OutputDevice* pOutDev ) = 0;

    // Window
    virtual css::uno::Reference<css::awt::XWindowPeer> GetWindowInterface( vcl::Window* pWindow ) = 0;
    virtual void                SetWindowInterface( vcl::Window* pWindow, css::uno::Reference< css::awt::XWindowPeer > xIFace ) = 0;
    virtual VclPtr<vcl::Window> GetWindow(const css::uno::Reference<css::awt::XWindow>& rxWindow) = 0;

    // PopupMenu
    virtual css::uno::Reference<css::awt::XPopupMenu> CreateMenuInterface( PopupMenu* pPopupMenu ) = 0;

    virtual void                WindowDestroyed( vcl::Window* pWindow ) = 0;

    // Accessibility
    virtual css::uno::Reference< css::accessibility::XAccessible >
                                CreateAccessible( Menu* pMenu, bool bIsMenuBar ) = 0;

    /** Get the application's UNO wrapper object.

     Note that this static function will only ever try to create UNO wrapper object once, and
     if it fails then it will not ever try again, even if the function is called multiple times.

     @param     bCreateIfNotExists  Create the UNO wrapper object if it doesn't exist when true.

     @return UNO wrapper object.
    */
    static UnoWrapperBase*      GetUnoWrapper( bool bCreateIfNotExists = true );

    /** Sets the application's UNO Wrapper object.

     @param     pWrapper        Pointer to UNO wrapper object.
    */
    static void                 SetUnoWrapper( UnoWrapperBase* pWrapper );

protected:
    ~UnoWrapperBase() {}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
