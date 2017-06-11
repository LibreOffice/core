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
#include <strings.hrc>
#include <caption.hxx>
#include <vcl/graphicfilter.hxx>
#include <sfx2/htmlmode.hxx>
#include <drawdoc.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <docsh.hxx>
#include <frmfmt.hxx>
#include <frmmgr.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdomedia.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <SwStyleNameMapper.hxx>
#include <sfx2/filedlghelper.hxx>
#include <strings.hrc>

#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <avmedia/mediawindow.hxx>
#include <vcl/svapp.hxx>

// -> #111827#
#include <SwRewriter.hxx>
#include <strings.hrc>
// <- #111827#

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::sfx2;

bool SwTextShell::InsertMediaDlg( SfxRequest& rReq )
{
    OUString     aURL;
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    vcl::Window*             pWindow = &GetView().GetViewFrame()->GetWindow();
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
    if (bAPI ||
        ::avmedia::MediaWindow::executeMediaURLDialog(aURL, & bLink))
    {
        Size aPrefSize;

        if( pWindow )
            pWindow->EnterWait();

        if( !::avmedia::MediaWindow::isMediaURL( aURL, "", true, &aPrefSize ) )
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
                    aSize = pWindow->PixelToLogic( aPrefSize, MapUnit::MapTwip );
                else
                    aSize = Application::GetDefaultDevice()->PixelToLogic( aPrefSize, MapUnit::MapTwip );
            }
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

            SdrMediaObj* pObj = new SdrMediaObj( tools::Rectangle( aPos, aSize ) );

            pObj->SetModel(rSh.GetDoc()->getIDocumentDrawModelAccess().GetDrawModel()); // set before setURL
            pObj->setURL( realURL, "" );
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
