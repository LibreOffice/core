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

#ifndef INCLUDED_SVTOOLS_CLIPLISTENER_HXX
#define INCLUDED_SVTOOLS_CLIPLISTENER_HXX

#include <svtools/svtdllapi.h>
#include <tools/link.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>

namespace vcl { class Window; }

class TransferableDataHelper;

class SVT_DLLPUBLIC TransferableClipboardListener : public ::cppu::WeakImplHelper1<
                            css::datatransfer::clipboard::XClipboardListener >
{
    Link<TransferableDataHelper*,void>  aLink;

public:
            // Link is called with a TransferableDataHelper pointer
            TransferableClipboardListener( const Link<TransferableDataHelper*,void>& rCallback );
            virtual ~TransferableClipboardListener();

    void    AddRemoveListener( vcl::Window* pWin, bool bAdd );
    void    ClearCallbackLink();

            // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
                                            throw(css::uno::RuntimeException, std::exception) override;
            // XClipboardListener
    virtual void SAL_CALL changedContents( const css::datatransfer::clipboard::ClipboardEvent& event )
                                            throw(css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
