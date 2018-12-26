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

#include <vcl/window.hxx>
#include <vcl/outdev.hxx>
#include <tools/debug.hxx>

#include <window.h>

#ifdef DBG_UTIL
const char* ImplDbgCheckWindow( const void* pObj )
{
    DBG_TESTSOLARMUTEX();

    const vcl::Window* pWindow = static_cast<vcl::Window const *>(pObj);

    if ( (pWindow->GetType() < WindowType::FIRST) || (pWindow->GetType() > WindowType::LAST) )
        return "Window data overwrite";

    // check window-chain
    vcl::Window* pChild = pWindow->mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        if ( pChild->mpWindowImpl->mpParent != pWindow )
            return "Child-Window-Parent wrong";
        pChild = pChild->mpWindowImpl->mpNext;
    }

    return nullptr;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
