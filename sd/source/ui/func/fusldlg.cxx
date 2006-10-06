/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fusldlg.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-06 09:52:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#include "fusldlg.hxx"

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#include "drawdoc.hxx"
#include "sdpage.hxx"
//CHINA001 #include "present.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "sdattr.hxx"
#include "glob.hrc"
#include "sdmod.hxx"
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#include "optsitem.hxx"
#include "sdabstdlg.hxx" //CHINA001
#include "present.hrc" //CHINA001

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

void FuSlideShowDlg::DoExecute( SfxRequest& rReq )
{
    PresentationSettings& rPresentationSettings = pDoc->getPresentationSettings();

    SfxItemSet      aDlgSet( pDoc->GetPool(), ATTR_PRESENT_START, ATTR_PRESENT_END );
    List            aPageNameList;
    const String&   rPresPage = rPresentationSettings.maPresPage;
    String          aFirstPage;
    String          aStandardName( SdResId( STR_PAGE ) );
    SdPage*         pPage = NULL;
    long            nPage;

    for( nPage = pDoc->GetSdPageCount( PK_STANDARD ) - 1L; nPage >= 0L; nPage-- )
    {
        pPage = pDoc->GetSdPage( (USHORT) nPage, PK_STANDARD );
        String* pStr = new String( pPage->GetName() );

        if ( !pStr->Len() )
        {
            *pStr = String( SdResId( STR_PAGE ) );
            (*pStr).Append( UniString::CreateFromInt32( nPage + 1 ) );
        }

        aPageNameList.Insert( pStr, (ULONG) 0 );

        // ist dies unsere (vorhandene) erste Seite?
        if ( rPresPage == *pStr )
            aFirstPage = rPresPage;
        else if ( pPage->IsSelected() && !aFirstPage.Len() )
            aFirstPage = *pStr;
    }
    List* pCustomShowList = pDoc->GetCustomShowList(); // No Create

    BOOL bStartWithActualPage = SD_MOD()->GetSdOptions( pDoc->GetDocumentType() )->IsStartWithActualPage();
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

    //CHINA001 SdStartPresentationDlg aDlg (pWindow, aDlgSet, aPageNameList, pCustomShowList );
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
    DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
    AbstractSdStartPresDlg* pDlg = pFact->CreateSdStartPresentationDlg(ResId( DLG_START_PRESENTATION ), pWindow, aDlgSet, aPageNameList, pCustomShowList );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
    if( pDlg->Execute() == RET_OK ) //CHINA001 if( aDlg.Execute() == RET_OK )
    {
        rtl::OUString aPage;
        long    nValue32;
        sal_Bool bValue;
        bool    bValuesChanged = FALSE;

        pDlg->GetAttr( aDlgSet ); //CHINA001 aDlg.GetAttr( aDlgSet );

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
            pDoc->SetChanged( TRUE );
    }
    delete pDlg; //add by CHINA001
    // Strings aus Liste loeschen
    for( void* pStr = aPageNameList.First(); pStr; pStr = aPageNameList.Next() )
        delete (String*) pStr;
}

} // end of namespace sd
