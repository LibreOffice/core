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

#include <sal/config.h>

#include <com/sun/star/lang/NoSupportException.hpp>
#include <toolkit/helper/vclunohelper.hxx>

#include "windowoutdevholder.hxx"

using namespace ::com::sun::star;

namespace vclcanvas
{
    namespace
    {
        vcl::Window& windowFromXWin( const uno::Reference<awt::XWindow>& xWin )
        {
            VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xWin);
            if( !pWindow )
                throw lang::NoSupportException(
                    u"Parent window not VCL window, or canvas out-of-process!"_ustr,
                    nullptr);
            return *pWindow;
        }
    }

    WindowOutDevHolder::WindowOutDevHolder( const uno::Reference<awt::XWindow>& xWin ) :
        mrOutputWindow( windowFromXWin(xWin) )
    {}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
