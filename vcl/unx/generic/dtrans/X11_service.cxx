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

#include "unx/salinst.h"

#include <X11_clipboard.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <uno/dispatcher.h>
#include <uno/mapping.hxx>
#include <cppuhelper/factory.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::awt;
using namespace x11;

Sequence< OUString > SAL_CALL x11::X11Clipboard_getSupportedServiceNames()
{
    Sequence< OUString > aRet { "com.sun.star.datatransfer.clipboard.SystemClipboard" };
    return aRet;
}

Sequence< OUString > SAL_CALL x11::Xdnd_getSupportedServiceNames()
{
    Sequence< OUString > aRet { "com.sun.star.datatransfer.dnd.X11DragSource" };
    return aRet;
}

Sequence< OUString > SAL_CALL x11::Xdnd_dropTarget_getSupportedServiceNames()
{
    Sequence< OUString > aRet { "com.sun.star.datatransfer.dnd.X11DropTarget" };
    return aRet;
}

css::uno::Reference< XInterface > X11SalInstance::CreateClipboard( const Sequence< Any >& arguments )
{
    SelectionManager& rManager = SelectionManager::get();
    css::uno::Sequence<css::uno::Any> mgrArgs(1);
    mgrArgs[0] <<= Application::GetDisplayConnection();
    rManager.initialize(mgrArgs);

    OUString sel;
    if (arguments.getLength() == 0) {
        sel = "CLIPBOARD";
    } else if (arguments.getLength() != 1 || !(arguments[0] >>= sel)) {
        throw css::lang::IllegalArgumentException(
            "bad X11SalInstance::CreateClipboard arguments",
            css::uno::Reference<css::uno::XInterface>(), -1);
    }
    Atom nSelection = rManager.getAtom(sel);

    std::unordered_map< Atom, css::uno::Reference< XClipboard > >::iterator it = m_aInstances.find( nSelection );
    if( it != m_aInstances.end() )
        return it->second;

    css::uno::Reference<css::datatransfer::clipboard::XClipboard> pClipboard = X11Clipboard::create( rManager, nSelection );
    m_aInstances[ nSelection ] = pClipboard;

    return pClipboard;
}

css::uno::Reference< XInterface > X11SalInstance::CreateDragSource()
{
    return css::uno::Reference < XInterface >( static_cast<OWeakObject *>(new SelectionManagerHolder()) );
}

css::uno::Reference< XInterface > X11SalInstance::CreateDropTarget()
{
    return css::uno::Reference < XInterface >( static_cast<OWeakObject *>(new DropTarget()) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
