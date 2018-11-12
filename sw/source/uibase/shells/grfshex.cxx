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

#include <docary.hxx>
#include <grfsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <textsh.hxx>
#include <viewopt.hxx>
#include <swundo.hxx>
#include <caption.hxx>
#include <vcl/graphicfilter.hxx>
#include <sfx2/htmlmode.hxx>
#include <drawdoc.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <docsh.hxx>
#include <frmfmt.hxx>
#include <frmmgr.hxx>
#include <vcl/weld.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <SwStyleNameMapper.hxx>
#include <sfx2/filedlghelper.hxx>

#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <avmedia/mediawindow.hxx>
#include <vcl/svapp.hxx>

// -> #111827#
#include <SwRewriter.hxx>
// <- #111827#

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::sfx2;

bool SwTextShell::InsertMediaDlg( SfxRequest const & rReq )
{
    OUString     aURL;
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    vcl::Window&        rWindow = GetView().GetViewFrame()->GetWindow();
    bool                bAPI = false, bRet = false;

    if( pReqArgs )
    {
        const SfxStringItem* pStringItem = dynamic_cast<const SfxStringItem*>( &pReqArgs->Get( rReq.GetSlot() )  );
        if( pStringItem )
        {
            aURL = pStringItem->GetValue();
            bAPI = !aURL.isEmpty();
        }
    }

    bool bLink(true);
    if (bAPI || ::avmedia::MediaWindow::executeMediaURLDialog(rWindow.GetFrameWeld(), aURL, & bLink))
    {
        Size aPrefSize;

        rWindow.EnterWait();

        if( !::avmedia::MediaWindow::isMediaURL( aURL, "", true, &aPrefSize ) )
        {
            rWindow.LeaveWait();

            if( !bAPI )
                ::avmedia::MediaWindow::executeFormatErrorBox(rWindow.GetFrameWeld());
        }
        else
        {
            SwWrtShell& rSh = GetShell();

            if( !rSh.HasDrawView() )
                rSh.MakeDrawView();

            Size            aDocSz( rSh.GetDocSize() );
            const SwRect&   rVisArea = rSh.VisArea();
            Point           aPos( rVisArea.Center() );
            Size            aSize;

            if( rVisArea.Width() > aDocSz.Width())
                aPos.setX( aDocSz.Width() / 2 + rVisArea.Left() );

            if(rVisArea.Height() > aDocSz.Height())
                aPos.setY( aDocSz.Height() / 2 + rVisArea.Top() );

            if( aPrefSize.Width() && aPrefSize.Height() )
                aSize = rWindow.PixelToLogic(aPrefSize, MapMode(MapUnit::MapTwip));
            else
                aSize = Size( 2835, 2835 );

            OUString realURL;
            if (bLink)
            {
                realURL = aURL;
            }
            else
            {
                uno::Reference<frame::XModel> const xModel(
                        rSh.GetDoc()->GetDocShell()->GetModel());
                bRet = ::avmedia::EmbedMedia(xModel, aURL, realURL);
                if (!bRet) { return bRet; }
            }

            SdrMediaObj* pObj = new SdrMediaObj(
                *rSh.GetDoc()->getIDocumentDrawModelAccess().GetDrawModel(),
                tools::Rectangle(aPos, aSize));

            pObj->setURL( realURL, "" );
            rSh.EnterStdMode();
            rSh.SwFEShell::InsertDrawObj( *pObj, aPos );
            bRet = true;

            rWindow.LeaveWait();
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
