/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "fusldlg.hxx"
#include <svl/itemset.hxx>
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

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

    SfxItemSet      aDlgSet( mpDoc->GetItemPool(), ATTR_PRESENT_START, ATTR_PRESENT_END );
    List            aPageNameList;
    const String&   rPresPage = rPresentationSettings.maPresPage;
    String          aFirstPage;
    String          aStandardName( SdResId( STR_PAGE ) );
    SdPage*         pPage = NULL;
    sal_uInt32 nPage(0);

    for( nPage = mpDoc->GetSdPageCount( PK_STANDARD ); nPage > 0; nPage-- )
    {
        pPage = mpDoc->GetSdPage( nPage - 1, PK_STANDARD );
        String* pStr = new String( pPage->GetName() );

        if ( !pStr->Len() )
        {
            *pStr = String( SdResId( STR_PAGE ) );
            (*pStr).Append( UniString::CreateFromInt32( nPage ) );
        }

        aPageNameList.Insert( pStr, (sal_uLong) 0 );

        // ist dies unsere (vorhandene) erste Seite?
        if ( rPresPage == *pStr )
            aFirstPage = rPresPage;
        else if ( pPage->IsSelected() && !aFirstPage.Len() )
            aFirstPage = *pStr;
    }
    List* pCustomShowList = mpDoc->GetCustomShowList(); // No Create

    bool bStartWithActualPage = SD_MOD()->GetSdOptions( mpDoc->GetDocumentType() )->IsStartWithActualPage();
/* #109180# change in behaviour, even when always start with current page is enabled, range settings are
            still used
    if( bStartWithActualPage )
    {
        aFirstPage = pViewSh->GetActualPage()->GetName();
        pCustomShowList = NULL;
    }
*/
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
    AbstractSdStartPresDlg* pDlg = pFact ? pFact->CreateSdStartPresentationDlg(mpWindow, aDlgSet, aPageNameList, pCustomShowList ) : 0;
    if( pDlg && (pDlg->Execute() == RET_OK) )
    {
        rtl::OUString aPage;
        long    nValue32;
        sal_Bool bValue;
        bool    bValuesChanged = false;

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
            mpDoc->SetChanged( true );
    }
    delete pDlg;
    // Strings aus Liste loeschen
    for( void* pStr = aPageNameList.First(); pStr; pStr = aPageNameList.Next() )
        delete (String*) pStr;
}

} // end of namespace sd
