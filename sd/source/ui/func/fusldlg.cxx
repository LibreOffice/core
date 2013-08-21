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

#include "fusldlg.hxx"
#include <svl/itemset.hxx>
#include <vcl/msgbox.hxx>

#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "sdattr.hxx"
#include "glob.hrc"
#include "sdmod.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "optsitem.hxx"
#include "sdabstdlg.hxx"

namespace sd {

#define ITEMVALUE(ItemSet,Id,Cast) ((const Cast&)(ItemSet).Get(Id)).GetValue()

TYPEINIT1( FuSlideShowDlg, FuPoor );


FuSlideShowDlg::FuSlideShowDlg (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor( pViewSh, pWin, pView, pDoc, rReq )
{
}

FunctionReference FuSlideShowDlg::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuSlideShowDlg( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSlideShowDlg::DoExecute( SfxRequest& )
{
    PresentationSettings& rPresentationSettings = mpDoc->getPresentationSettings();

    SfxItemSet      aDlgSet( mpDoc->GetPool(), ATTR_PRESENT_START, ATTR_PRESENT_END );
    std::vector<OUString> aPageNameList(mpDoc->GetSdPageCount( PK_STANDARD ));
    const String&   rPresPage = rPresentationSettings.maPresPage;
    String          aFirstPage;
    SdPage*         pPage = NULL;
    long            nPage;

    for( nPage = mpDoc->GetSdPageCount( PK_STANDARD ) - 1L; nPage >= 0L; nPage-- )
    {
        pPage = mpDoc->GetSdPage( (sal_uInt16) nPage, PK_STANDARD );
        OUString aStr( pPage->GetName() );

        if ( aStr.isEmpty() )
        {
            aStr = SD_RESSTR( STR_PAGE ) + OUString::number( nPage + 1 );
        }

        aPageNameList[ nPage ] = aStr;

        // is this our (existing) first page?
        if ( rPresPage == aStr )
            aFirstPage = rPresPage;
        else if ( pPage->IsSelected() && !aFirstPage.Len() )
            aFirstPage = aStr;
    }
    SdCustomShowList* pCustomShowList = mpDoc->GetCustomShowList(); // No Create

    sal_Bool bStartWithActualPage = SD_MOD()->GetSdOptions( mpDoc->GetDocumentType() )->IsStartWithActualPage();
    if( !aFirstPage.Len() && pPage )
        aFirstPage = pPage->GetName();

    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_ALL, rPresentationSettings.mbAll ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_CUSTOMSHOW, rPresentationSettings.mbCustomShow ) );
    aDlgSet.Put( SfxStringItem( ATTR_PRESENT_DIANAME, aFirstPage ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_ENDLESS, rPresentationSettings.mbEndless ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_MANUEL, rPresentationSettings.mbManual ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_MOUSE, rPresentationSettings.mbMouseVisible ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_PEN, rPresentationSettings.mbMouseAsPen ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_NAVIGATOR, rPresentationSettings.mbStartWithNavigator ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_ANIMATION_ALLOWED, rPresentationSettings.mbAnimationAllowed ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_CHANGE_PAGE, !rPresentationSettings.mbLockedPages ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_ALWAYS_ON_TOP, rPresentationSettings.mbAlwaysOnTop ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_FULLSCREEN, rPresentationSettings.mbFullScreen ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_START_ACTUAL_PAGE, bStartWithActualPage ) );
    aDlgSet.Put( SfxUInt32Item( ATTR_PRESENT_PAUSE_TIMEOUT, rPresentationSettings.mnPauseTimeout ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_SHOW_PAUSELOGO, rPresentationSettings.mbShowPauseLogo ) );

    SdOptions* pOptions = SD_MOD()->GetSdOptions(DOCUMENT_TYPE_IMPRESS);
    aDlgSet.Put( SfxInt32Item( ATTR_PRESENT_DISPLAY, pOptions->GetDisplay() ) );

    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
    AbstractSdStartPresDlg* pDlg = pFact ? pFact->CreateSdStartPresentationDlg(mpWindow, aDlgSet, aPageNameList, pCustomShowList) : 0;
    if( pDlg && (pDlg->Execute() == RET_OK) )
    {
        OUString aPage;
        long    nValue32;
        sal_Bool bValue;
        bool    bValuesChanged = sal_False;

        pDlg->GetAttr( aDlgSet );

        aPage = ITEMVALUE( aDlgSet, ATTR_PRESENT_DIANAME, SfxStringItem );
        if( aPage != rPresentationSettings.maPresPage )
        {
            bValuesChanged = true;
            rPresentationSettings.maPresPage = aPage;
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_ALL, SfxBoolItem );
        if ( bValue != rPresentationSettings.mbAll )
        {
            bValuesChanged = true;
            rPresentationSettings.mbAll = bValue;
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_CUSTOMSHOW, SfxBoolItem );
        if ( bValue != rPresentationSettings.mbCustomShow )
        {
            bValuesChanged = true;
            rPresentationSettings.mbCustomShow = bValue;
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_ENDLESS, SfxBoolItem );
        if ( bValue != rPresentationSettings.mbEndless )
        {
            bValuesChanged = true;
            rPresentationSettings.mbEndless = bValue;
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_MANUEL, SfxBoolItem );
        if ( bValue != rPresentationSettings.mbManual )
        {
            bValuesChanged = true;
            rPresentationSettings.mbManual = bValue;
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_MOUSE, SfxBoolItem );
        if ( bValue != rPresentationSettings.mbMouseVisible )
        {
            bValuesChanged = true;
            rPresentationSettings.mbMouseVisible = bValue;
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_PEN, SfxBoolItem );
        if ( bValue != rPresentationSettings.mbMouseAsPen )
        {
            bValuesChanged = true;
            rPresentationSettings.mbMouseAsPen = bValue;
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_NAVIGATOR, SfxBoolItem );
        if( bValue != rPresentationSettings.mbStartWithNavigator )
        {
            bValuesChanged = true;
            rPresentationSettings.mbStartWithNavigator = bValue;
        }

        bValue = !ITEMVALUE( aDlgSet, ATTR_PRESENT_CHANGE_PAGE, SfxBoolItem );
        if ( bValue != rPresentationSettings.mbLockedPages )
        {
            bValuesChanged = true;
            rPresentationSettings.mbLockedPages = bValue;
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_ANIMATION_ALLOWED, SfxBoolItem );
        if ( bValue != rPresentationSettings.mbAnimationAllowed )
        {
            bValuesChanged = true;
            rPresentationSettings.mbAnimationAllowed = bValue;
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_ALWAYS_ON_TOP, SfxBoolItem );
        if ( bValue != rPresentationSettings.mbAlwaysOnTop )
        {
            bValuesChanged = true;
            rPresentationSettings.mbAlwaysOnTop = bValue;
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_FULLSCREEN, SfxBoolItem );
        if ( bValue != rPresentationSettings.mbFullScreen )
        {
            bValuesChanged = true;
            rPresentationSettings.mbFullScreen = bValue;
        }

        nValue32 = ITEMVALUE( aDlgSet, ATTR_PRESENT_PAUSE_TIMEOUT, SfxUInt32Item );
        if( nValue32 != rPresentationSettings.mnPauseTimeout )
        {
            bValuesChanged = true;
            rPresentationSettings.mnPauseTimeout = nValue32;
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_SHOW_PAUSELOGO, SfxBoolItem );
        if ( bValue != rPresentationSettings.mbShowPauseLogo )
        {
            bValuesChanged = true;
            rPresentationSettings.mbShowPauseLogo = bValue;
        }

        pOptions->SetDisplay( ITEMVALUE( aDlgSet, ATTR_PRESENT_DISPLAY, SfxInt32Item ) );

        // is something has changed, we set the modified flag
        if ( bValuesChanged )
            mpDoc->SetChanged( sal_True );
    }
    delete pDlg;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
