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


#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <osl/mutex.hxx>

#include <svtools/cliplistener.hxx>
#include <svtools/transfer.hxx>

using namespace ::com::sun::star;



TransferableClipboardListener::TransferableClipboardListener( const Link<TransferableDataHelper*,void>& rCallback ) :
    aLink( rCallback )
{
}

TransferableClipboardListener::~TransferableClipboardListener()
{
}

void SAL_CALL TransferableClipboardListener::disposing( const lang::EventObject& )
                                                        throw(uno::RuntimeException, std::exception)
{
}

void SAL_CALL TransferableClipboardListener::changedContents(
                            const datatransfer::clipboard::ClipboardEvent& rEventObject )
                                                        throw(uno::RuntimeException, std::exception)
{
    if ( aLink.IsSet() )
    {
        const SolarMutexGuard aGuard;

        TransferableDataHelper aDataHelper( rEventObject.Contents );
        aLink.Call( &aDataHelper );
    }
}

void TransferableClipboardListener::AddRemoveListener( vcl::Window* pWin, bool bAdd )
{
    try
    {
        if ( pWin )
        {
            uno::Reference<datatransfer::clipboard::XClipboard> xClipboard = pWin->GetClipboard();
            uno::Reference<datatransfer::clipboard::XClipboardNotifier> xClpbrdNtfr( xClipboard, uno::UNO_QUERY );
            if( xClpbrdNtfr.is() )
            {
                uno::Reference<datatransfer::clipboard::XClipboardListener> xClipEvtLstnr( this );
                if( bAdd )
                    xClpbrdNtfr->addClipboardListener( xClipEvtLstnr );
                else
                    xClpbrdNtfr->removeClipboardListener( xClipEvtLstnr );
            }
        }
    }
    catch( const css::uno::Exception& )
    {
    }
}

void TransferableClipboardListener::ClearCallbackLink()
{
    aLink = Link<TransferableDataHelper*,void>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
