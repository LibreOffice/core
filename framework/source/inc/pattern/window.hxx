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

#ifndef INCLUDED_FRAMEWORK_SOURCE_INC_PATTERN_WINDOW_HXX
#define INCLUDED_FRAMEWORK_SOURCE_INC_PATTERN_WINDOW_HXX

#include <general.h>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustring.hxx>

// namespaces

namespace framework{

class WindowHelper
{
    public:


static bool isTopWindow(const css::uno::Reference< css::awt::XWindow >& xWindow)
{
    // even child frame containing top level windows (e.g. query designer of database) will be closed
    css::uno::Reference< css::awt::XTopWindow > xTopWindowCheck(xWindow, css::uno::UNO_QUERY);
    if (xTopWindowCheck.is())
    {
        // Note: Toolkit interface XTopWindow sometimes is used by real VCL-child-windows also .-)
        // Be sure that these window is really a "top system window".
        // Attention ! Checking Window->GetParent() is not the right approach here.
        // Because sometimes VCL create "implicit border windows" as parents even we created
        // a simple XWindow using the toolkit only .-(
        SolarMutexGuard aSolarGuard;
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->IsSystemWindow() )
            return true;
    }

    return false;
}

};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_SOURCE_INC_PATTERN_WINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
