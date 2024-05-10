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

#include <unx/salinst.h>
#include <dndhelper.hxx>
#include <vcl/sysdata.hxx>

#include "X11_clipboard.hxx"
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::datatransfer::clipboard;
using namespace x11;

Sequence< OUString > x11::X11Clipboard_getSupportedServiceNames()
{
    return { u"com.sun.star.datatransfer.clipboard.SystemClipboard"_ustr };
}

Sequence< OUString > x11::Xdnd_getSupportedServiceNames()
{
    return { u"com.sun.star.datatransfer.dnd.X11DragSource"_ustr };
}

Sequence< OUString > x11::Xdnd_dropTarget_getSupportedServiceNames()
{
    return { u"com.sun.star.datatransfer.dnd.X11DropTarget"_ustr };
}

css::uno::Reference< XInterface > X11SalInstance::CreateClipboard( const Sequence< Any >& arguments )
{
    if ( IsRunningUnitTest() )
        return SalInstance::CreateClipboard( arguments );

    SelectionManager& rManager = SelectionManager::get();
    css::uno::Sequence<css::uno::Any> mgrArgs{ css::uno::Any(Application::GetDisplayConnection()) };
    rManager.initialize(mgrArgs);

    OUString sel;
    if (!arguments.hasElements()) {
        sel = "CLIPBOARD";
    } else if (arguments.getLength() != 1 || !(arguments[0] >>= sel)) {
        throw css::lang::IllegalArgumentException(
            u"bad X11SalInstance::CreateClipboard arguments"_ustr,
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

css::uno::Reference<XInterface> X11SalInstance::ImplCreateDragSource(const SystemEnvData* pSysEnv)
{
    return vcl::X11DnDHelper(new SelectionManagerHolder(), pSysEnv->aShellWindow);
}

css::uno::Reference<XInterface> X11SalInstance::ImplCreateDropTarget(const SystemEnvData* pSysEnv)
{
    return vcl::X11DnDHelper(new DropTarget(), pSysEnv->aShellWindow);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
