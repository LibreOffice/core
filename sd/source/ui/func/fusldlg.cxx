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


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

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
    std::vector<String> aPageNameList(mpDoc->GetSdPageCount( PK_STANDARD ));
    const String&   rPresPage = rPresentationSettings.maPresPage;
    String          aFirstPage;
    String          aStandardName( SdResId( STR_PAGE ) );
    SdPage*         pPage = NULL;
    long            nPage;

    for( nPage = mpDoc->GetSdPageCount( PK_STANDARD ) - 1L; nPage >= 0L; nPage-- )
    {
        pPage = mpDoc->GetSdPage( (sal_uInt16) nPage, PK_STANDARD );
        String aStr( pPage->GetName() );

        if ( !aStr.Len() )
        {
            aStr = String( SdResId( STR_PAGE ) );
            aStr.Append( UniString::CreateFromInt32( nPage + 1 ) );
        }

        aPageNameList[ nPage ] = aStr;

        // ist dies unsere (vorhandene) erste Seite?
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
        rtl::OUString aPage;
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

        // wenn sich etwas geaendert hat, setzen wir das Modified-Flag,
        if ( bValuesChanged )
            mpDoc->SetChanged( sal_True );
    }
    delete pDlg;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
