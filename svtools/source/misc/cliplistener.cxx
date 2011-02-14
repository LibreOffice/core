/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vos/mutex.hxx>

#include <svtools/cliplistener.hxx>
#include <svtools/transfer.hxx>

using namespace ::com::sun::star;

// -----------------------------------------------------------------------------

TransferableClipboardListener::TransferableClipboardListener( const Link& rCallback ) :
    aLink( rCallback )
{
}

TransferableClipboardListener::~TransferableClipboardListener()
{
}

void SAL_CALL TransferableClipboardListener::disposing( const lang::EventObject& )
                                                        throw(uno::RuntimeException)
{
}

void SAL_CALL TransferableClipboardListener::changedContents(
                            const datatransfer::clipboard::ClipboardEvent& rEventObject )
                                                        throw(uno::RuntimeException)
{
    if ( aLink.IsSet() )
    {
        const ::vos::OGuard aGuard( Application::GetSolarMutex() );

        TransferableDataHelper aDataHelper( rEventObject.Contents );
        aLink.Call( &aDataHelper );
    }
}

void TransferableClipboardListener::AddRemoveListener( Window* pWin, sal_Bool bAdd )
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
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

void TransferableClipboardListener::ClearCallbackLink()
{
    aLink = Link();
}

