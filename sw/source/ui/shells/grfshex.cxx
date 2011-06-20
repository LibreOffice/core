/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sw.hxx"


#include <docary.hxx>
#include <grfsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <textsh.hxx>
#include <viewopt.hxx>
#include <swundo.hxx>
#include <shells.hrc>
#include <caption.hxx>
#define _SVSTDARR_STRINGSSORTDTOR
#include <svl/svstdarr.hxx>
#include <svtools/filter.hxx>
#include <svx/htmlmode.hxx>
#include <docsh.hxx>
#include <frmfmt.hxx>
#include <frmmgr.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <SwStyleNameMapper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <poolfmt.hrc>

#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <avmedia/mediawindow.hxx>
#include <vcl/svapp.hxx>

// -> #111827#
#include <SwRewriter.hxx>
#include <comcore.hrc>
// <- #111827#

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::sfx2;
using ::rtl::OUString;

bool SwTextShell::InsertMediaDlg( SfxRequest& rReq )
{
    ::rtl::OUString     aURL;
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    Window*             pWindow = &GetView().GetViewFrame()->GetWindow();
    bool                bAPI = false, bRet = false;

    if( pReqArgs )
    {
        const SfxStringItem* pStringItem = PTR_CAST( SfxStringItem, &pReqArgs->Get( rReq.GetSlot() ) );

        if( pStringItem )
        {
            aURL = pStringItem->GetValue();
            bAPI = aURL.getLength();
        }
    }

    if( bAPI || ::avmedia::MediaWindow::executeMediaURLDialog( pWindow, aURL ) )
    {
        Size aPrefSize;

        if( pWindow )
            pWindow->EnterWait();

        if( !::avmedia::MediaWindow::isMediaURL( aURL, true, &aPrefSize ) )
        {
            if( pWindow )
                pWindow->LeaveWait();

            if( !bAPI )
                ::avmedia::MediaWindow::executeFormatErrorBox( pWindow );
        }
        else
        {
            SwWrtShell& rSh = GetShell();

            if( !rSh.HasDrawView() )
                rSh.MakeDrawView();

            Size            aDocSz( rSh.GetDocSize() );
               const SwRect&    rVisArea = rSh.VisArea();
            Point           aPos( rVisArea.Center() );
            Size            aSize;

            if( rVisArea.Width() > aDocSz.Width())
                aPos.X() = aDocSz.Width() / 2 + rVisArea.Left();

            if(rVisArea.Height() > aDocSz.Height())
                aPos.Y() = aDocSz.Height() / 2 + rVisArea.Top();

            if( aPrefSize.Width() && aPrefSize.Height() )
            {
                if( pWindow )
                    aSize = pWindow->PixelToLogic( aPrefSize, MAP_TWIP );
                else
                    aSize = Application::GetDefaultDevice()->PixelToLogic( aPrefSize, MAP_TWIP );
            }
            else
                aSize = Size( 2835, 2835 );

            SdrMediaObj* pObj = new SdrMediaObj( Rectangle( aPos, aSize ) );

            pObj->setURL( aURL );
            rSh.EnterStdMode();
            rSh.SwFEShell::InsertDrawObj( *pObj, aPos );
            bRet = true;

            if( pWindow )
                pWindow->LeaveWait();
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
