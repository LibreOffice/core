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

#include <hintids.hxx>

#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svtools/imap.hxx>
#include <svtools/inetimg.hxx>
#include <svtools/transfer.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/gallery.hxx>
#include <svx/graphichelper.hxx>
#include <editeng/brushitem.hxx>

#include <frmatr.hxx>
#include <fmturl.hxx>
#include <fmtinfmt.hxx>
#include <docsh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <swmodule.hxx>
#include <romenu.hxx>
#include <pagedesc.hxx>
#include <modcfg.hxx>

#include <cmdid.h>
#include <helpid.h>
#include <docvw.hrc>

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::sfx2;

SwReadOnlyPopup::~SwReadOnlyPopup()
{
    delete pImageMap;
    delete pTargetURL;
}

void SwReadOnlyPopup::Check( sal_uInt16 nMID, sal_uInt16 nSID, SfxDispatcher &rDis )
{
    SfxPoolItem *_pItem = 0;
    SfxItemState eState = rDis.GetBindings()->QueryState( nSID, _pItem );
    if (eState >= SFX_ITEM_AVAILABLE)
    {
        EnableItem( nMID, sal_True );
        if (_pItem)
        {
            CheckItem ( nMID, !_pItem->ISA(SfxVoidItem) &&
                            _pItem->ISA(SfxBoolItem) &&
                            ((SfxBoolItem*)_pItem)->GetValue());
            //remove full screen entry when not in full screen mode
            if( SID_WIN_FULLSCREEN == nSID && !IsItemChecked(SID_WIN_FULLSCREEN) )
                EnableItem(nMID, sal_False);
        }
    }
    else
        EnableItem( nMID, sal_False );

    delete _pItem;
}


SwReadOnlyPopup::SwReadOnlyPopup( const Point &rDPos, SwView &rV ) :
    PopupMenu( SW_RES(MN_READONLY_POPUP) ),
    rView  ( rV ),
    rDocPos( rDPos ),
    pImageMap( 0 ),
    pTargetURL( 0 )
{
    bGrfToGalleryAsLnk = SW_MOD()->GetModuleConfig()->IsGrfToGalleryAsLnk();
    SwWrtShell &rSh = rView.GetWrtShell();
    rSh.IsURLGrfAtPos( rDocPos, &sURL, &sTargetFrameName, &sDescription );
    if ( !sURL.Len() )
    {
        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_INETATTR );
        if( rSh.GetContentAtPos( rDocPos, aCntntAtPos, sal_False))
        {
            SwFmtINetFmt &rIItem = *(SwFmtINetFmt*)aCntntAtPos.aFnd.pAttr;
            sURL = rIItem.GetValue();
            sTargetFrameName = rIItem.GetTargetFrame();
            sDescription = aCntntAtPos.sStr;
        }
    }

    sal_Bool bLink = sal_False;
    const Graphic *pGrf;
    if ( 0 == (pGrf = rSh.GetGrfAtPos( rDocPos, sGrfName, bLink )) )
    {
        EnableItem( MN_READONLY_SAVEGRAPHIC, sal_False );
        EnableItem( MN_READONLY_COPYGRAPHIC, sal_False );
    }
    else
    {
        aGraphic = *pGrf;
        const SwFrmFmt* pGrfFmt = rSh.GetFmtFromObj( rDocPos );
        const SfxPoolItem* pURLItem;
        if( pGrfFmt && SFX_ITEM_SET == pGrfFmt->GetItemState(
            RES_URL, sal_True, &pURLItem ))
        {
            const SwFmtURL& rURL = *(SwFmtURL*)pURLItem;
            if( rURL.GetMap() )
                pImageMap = new ImageMap( *rURL.GetMap() );
            else if( !rURL.GetURL().isEmpty() )
                pTargetURL = new INetImage( bLink ? sGrfName : aEmptyStr,
                                            rURL.GetURL(),
                                            rURL.GetTargetFrameName(),
                                            aEmptyStr, Size() );
        }
    }

    bool bEnableGraphicToGallery = bLink;
    if ( bEnableGraphicToGallery )
    {
        if (GalleryExplorer::FillThemeList( aThemeList ))
        {
            PopupMenu *pMenu = GetPopupMenu(MN_READONLY_GRAPHICTOGALLERY);
            pMenu->CheckItem( MN_READONLY_TOGALLERYLINK,  bGrfToGalleryAsLnk );
            pMenu->CheckItem( MN_READONLY_TOGALLERYCOPY, !bGrfToGalleryAsLnk );

            for ( sal_uInt16 i=0; i < aThemeList.size(); ++i )
                pMenu->InsertItem( MN_READONLY_GRAPHICTOGALLERY+i + 3, aThemeList[ i ] );
        }
        else
            bEnableGraphicToGallery = false;
    }

    EnableItem( MN_READONLY_GRAPHICTOGALLERY, bEnableGraphicToGallery );

    SfxViewFrame * pVFrame = rV.GetViewFrame();
    SfxDispatcher &rDis = *pVFrame->GetDispatcher();
    const SwPageDesc &rDesc = rSh.GetPageDesc( rSh.GetCurPageDesc() );
    pItem = &rDesc.GetMaster().GetBackground();
    sal_Bool bEnableBackGallery = sal_False,
         bEnableBack = sal_False;

    if ( GPOS_NONE != pItem->GetGraphicPos() )
    {
        bEnableBack = sal_True;
        if ( pItem->GetGraphicLink() )
        {
            if ( aThemeList.empty() )
                GalleryExplorer::FillThemeList( aThemeList );

            if ( !aThemeList.empty() )
            {
                PopupMenu *pMenu = GetPopupMenu(MN_READONLY_BACKGROUNDTOGALLERY);
                pMenu->CheckItem( MN_READONLY_TOGALLERYLINK,  bGrfToGalleryAsLnk );
                pMenu->CheckItem( MN_READONLY_TOGALLERYCOPY, !bGrfToGalleryAsLnk );
                bEnableBackGallery = sal_True;

                for ( sal_uInt16 i=0; i < aThemeList.size(); ++i )
                    pMenu->InsertItem( MN_READONLY_GRAPHICTOGALLERY+i + 3, aThemeList[ i ] );
            }
        }
    }
    EnableItem( MN_READONLY_SAVEBACKGROUND, bEnableBack );
    EnableItem( MN_READONLY_BACKGROUNDTOGALLERY, bEnableBackGallery );

    if ( !rSh.GetViewOptions()->IsGraphic() )
        CheckItem( MN_READONLY_GRAPHICOFF );
    else
        EnableItem( MN_READONLY_LOADGRAPHIC, sal_False );

    sal_Bool bReloadFrame = 0 != rSh.GetView().GetViewFrame()->GetFrame().GetParentFrame();
    EnableItem( MN_READONLY_RELOAD_FRAME,
            bReloadFrame );
    EnableItem( MN_READONLY_RELOAD, !bReloadFrame);

    Check( MN_READONLY_EDITDOC,         SID_EDITDOC,        rDis );
    Check( MN_READONLY_SELECTION_MODE,  FN_READONLY_SELECTION_MODE,    rDis );
    Check( MN_READONLY_SOURCEVIEW,      SID_SOURCEVIEW,     rDis );
    Check( MN_READONLY_BROWSE_BACKWARD, SID_BROWSE_BACKWARD,rDis );
    Check( MN_READONLY_BROWSE_FORWARD,  SID_BROWSE_FORWARD, rDis );
#ifdef WNT
    Check( MN_READONLY_PLUGINOFF,       SID_PLUGINS_ACTIVE, rDis );
#endif
    Check( MN_READONLY_OPENURL,         SID_OPENDOC,        rDis );
    Check( MN_READONLY_OPENURLNEW,      SID_OPENDOC,        rDis );

    SfxPoolItem* pState;

    SfxItemState eState = pVFrame->GetBindings().QueryState( SID_COPY, pState );
    Check( MN_READONLY_COPY,            SID_COPY,           rDis );
    if(eState < SFX_ITEM_AVAILABLE)
        EnableItem( MN_READONLY_COPY, sal_False );

    eState = pVFrame->GetBindings().QueryState( SID_EDITDOC, pState );
    if (
        eState < SFX_ITEM_DEFAULT ||
        (rSh.IsGlobalDoc() && rView.GetDocShell()->IsReadOnlyUI())
       )
    {
        EnableItem( MN_READONLY_EDITDOC, sal_False );
    }

    if ( !sURL.Len() )
    {
        EnableItem( MN_READONLY_OPENURL, sal_False );
        EnableItem( MN_READONLY_OPENURLNEW, sal_False );
        EnableItem( MN_READONLY_COPYLINK, sal_False );
    }
    Check( SID_WIN_FULLSCREEN,         SID_WIN_FULLSCREEN,        rDis );

    RemoveDisabledEntries( sal_True, sal_True );
}

void SwReadOnlyPopup::Execute( Window* pWin, const Point &rPixPos )
{
    sal_uInt16 nId     = PopupMenu::Execute(
    pWin,
    rPixPos );
    Execute(pWin, nId);
}

// execute the resulting ID only - necessary to support XContextMenuInterception
void SwReadOnlyPopup::Execute( Window* pWin, sal_uInt16 nId )
{
    SwWrtShell &rSh = rView.GetWrtShell();
    SfxDispatcher &rDis = *rView.GetViewFrame()->GetDispatcher();
    if ( nId >= MN_READONLY_GRAPHICTOGALLERY )
    {
        String sTmp;
        sal_uInt16 nSaveId;
        if ( nId >= MN_READONLY_BACKGROUNDTOGALLERY )
        {
            nId -= MN_READONLY_BACKGROUNDTOGALLERY+3;
            nSaveId = MN_READONLY_SAVEBACKGROUND;
            sTmp = *pItem->GetGraphicLink();
        }
        else
        {
            nId -= MN_READONLY_GRAPHICTOGALLERY+3;
            nSaveId = MN_READONLY_SAVEGRAPHIC;
            sTmp = sGrfName;
        }
        if ( !bGrfToGalleryAsLnk )
            sTmp = SaveGraphic( nSaveId );

        if ( sTmp.Len() )
            GalleryExplorer::InsertURL( aThemeList[nId], sTmp );

        return;
    }

    TransferDataContainer* pClipCntnr = 0;

    sal_uInt16 nExecId = USHRT_MAX;
    sal_uInt16 nFilter = USHRT_MAX;
    switch( nId )
    {
        case SID_WIN_FULLSCREEN :           nExecId = SID_WIN_FULLSCREEN; break;
        case MN_READONLY_OPENURL:           nFilter = URLLOAD_NOFILTER;   break;
        case MN_READONLY_OPENURLNEW:        nFilter = URLLOAD_NEWVIEW;    break;
        case MN_READONLY_COPY:              nExecId = SID_COPY;           break;

        case MN_READONLY_EDITDOC:           nExecId = SID_EDITDOC;        break;
        case MN_READONLY_SELECTION_MODE:    nExecId = FN_READONLY_SELECTION_MODE; break;
        case MN_READONLY_RELOAD:
        case MN_READONLY_RELOAD_FRAME:
            rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(SID_RELOAD);
        break;

        case MN_READONLY_BROWSE_BACKWARD:   nExecId = SID_BROWSE_BACKWARD;break;
        case MN_READONLY_BROWSE_FORWARD:    nExecId = SID_BROWSE_FORWARD; break;
        case MN_READONLY_SOURCEVIEW:        nExecId = SID_SOURCEVIEW;     break;
        case MN_READONLY_SAVEGRAPHIC:
        case MN_READONLY_SAVEBACKGROUND:
            {
                SaveGraphic( nId );
                break;
            }
        case MN_READONLY_COPYLINK:
            pClipCntnr = new TransferDataContainer;
            pClipCntnr->CopyString( sURL );
            break;

        case MN_READONLY_COPYGRAPHIC:
            pClipCntnr = new TransferDataContainer;
            pClipCntnr->CopyGraphic( aGraphic );

            if( pImageMap )
                pClipCntnr->CopyImageMap( *pImageMap );
            if( pTargetURL )
                pClipCntnr->CopyINetImage( *pTargetURL );
            break;

        case MN_READONLY_LOADGRAPHIC:
            {
                sal_Bool bModified = rSh.IsModified();
                SwViewOption aOpt( *rSh.GetViewOptions() );
                aOpt.SetGraphic( sal_True );
                rSh.ApplyViewOptions( aOpt );
                if(!bModified)
                    rSh.ResetModified();
                break;
            }
        case MN_READONLY_GRAPHICOFF:        nExecId = FN_VIEW_GRAPHIC;    break;
#ifdef WNT
        case MN_READONLY_PLUGINOFF:         nExecId = SID_PLUGINS_ACTIVE; break;
#endif
        case MN_READONLY_TOGALLERYLINK:
            SW_MOD()->GetModuleConfig()->SetGrfToGalleryAsLnk( sal_True );
            break;
        case MN_READONLY_TOGALLERYCOPY:
            SW_MOD()->GetModuleConfig()->SetGrfToGalleryAsLnk( sal_False );
            break;

        default: //forward the id to the SfxBindings
            nExecId = nId;
    }
    if( USHRT_MAX != nExecId )
        rDis.GetBindings()->Execute( nExecId );
    if( USHRT_MAX != nFilter )
        ::LoadURL(rSh, sURL, nFilter, sTargetFrameName);

    if( pClipCntnr )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > xRef( pClipCntnr );
        if( pClipCntnr->HasAnyData() )
            pClipCntnr->CopyToClipboard( pWin );
    }
}

String SwReadOnlyPopup::SaveGraphic( sal_uInt16 nId )
{
    // fish out the graphic's name
    if ( MN_READONLY_SAVEBACKGROUND == nId )
    {
        if ( pItem->GetGraphicLink() )
            sGrfName = *pItem->GetGraphicLink();
        ((SvxBrushItem*)pItem)->SetDoneLink( Link() );
        const Graphic *pGrf = pItem->GetGraphic();
        if ( pGrf )
        {
            aGraphic = *pGrf;
            if ( pItem->GetGraphicLink() )
                sGrfName = *pItem->GetGraphicLink();
        }
        else
            return aEmptyStr;
    }
    return GraphicHelper::ExportGraphic( aGraphic, sGrfName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
