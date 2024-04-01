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

#include <config_features.h>
#include <wrtsh.hxx>
#include <view.hxx>
#include <textsh.hxx>
#include <drawdoc.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <docsh.hxx>
#include <avmedia/mediawindow.hxx>
#include <editeng/sizeitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <svx/svdomedia.hxx>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/media/XPlayer.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

bool SwTextShell::InsertMediaDlg( SfxRequest const & rReq )
{
    bool bRet = false;

#if !HAVE_FEATURE_AVMEDIA
    (void) rReq;
#else
    OUString     aURL;
    const SfxItemSet*   pReqArgs = rReq.GetArgs();
    vcl::Window&        rWindow = GetView().GetViewFrame().GetWindow();
    bool bAPI = false;

    const SvxSizeItem* pSizeItem = rReq.GetArg<SvxSizeItem>(FN_PARAM_1);
    const SfxBoolItem* pLinkItem = rReq.GetArg<SfxBoolItem>(FN_PARAM_2);
    const bool bSizeUnknown = !pSizeItem;

    if( pReqArgs )
    {
        const SfxStringItem* pStringItem = dynamic_cast<const SfxStringItem*>( &pReqArgs->Get( rReq.GetSlot() )  );
        if( pStringItem )
        {
            aURL = pStringItem->GetValue();
            bAPI = !aURL.isEmpty();
        }
    }

    bool bLink(pLinkItem ? pLinkItem->GetValue() : true);

    if (bAPI || ::avmedia::MediaWindow::executeMediaURLDialog(rWindow.GetFrameWeld(), aURL, & bLink))
    {
        Size aPrefSize;

        if (!bSizeUnknown)
            aPrefSize = pSizeItem->GetSize();
        else
        {
            rWindow.EnterWait();

            css::uno::Reference<css::frame::XDispatchProvider> xDispatchProvider(GetView().GetViewFrame().GetFrame().GetFrameInterface(), css::uno::UNO_QUERY);

            rtl::Reference<avmedia::PlayerListener> xPlayerListener(new avmedia::PlayerListener(
                [xDispatchProvider=std::move(xDispatchProvider), aURL, bLink](const css::uno::Reference<css::media::XPlayer>& rPlayer){
                    css::awt::Size aSize = rPlayer->getPreferredPlayerWindowSize();
                    avmedia::MediaWindow::dispatchInsertAVMedia(xDispatchProvider, aSize, aURL, bLink);
                }));

            const bool bIsMediaURL = ::avmedia::MediaWindow::isMediaURL(aURL, "", true, xPlayerListener);

            rWindow.LeaveWait();

            if (!bIsMediaURL)
            {
                if( !bAPI )
                    ::avmedia::MediaWindow::executeFormatErrorBox(rWindow.GetFrameWeld());

                return bRet;
            }

            return true;
        }

        rWindow.EnterWait();

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

        rtl::Reference<SdrMediaObj> pObj = new SdrMediaObj(
            *rSh.GetDoc()->getIDocumentDrawModelAccess().GetDrawModel(),
            tools::Rectangle(aPos, aSize));

        pObj->setURL( realURL, "" );
        rSh.EnterStdMode();
        rSh.SwFEShell::InsertDrawObj( *pObj, aPos );
        bRet = true;

        rWindow.LeaveWait();
    }
#endif

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
