/*************************************************************************
 *
 *  $RCSfile: fusldlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:48:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

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
    SfxItemSet      aDlgSet( pDoc->GetPool(), ATTR_PRESENT_START, ATTR_PRESENT_END );
    List            aPageNameList;
    const String&   rPresPage = pDoc->GetPresPage();
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

    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_ALL, pDoc->GetPresAll() ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_CUSTOMSHOW, pDoc->IsCustomShow() ) );
    aDlgSet.Put( SfxStringItem( ATTR_PRESENT_DIANAME, aFirstPage ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_ENDLESS, pDoc->GetPresEndless() ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_MANUEL, pDoc->GetPresManual() ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_MOUSE, pDoc->GetPresMouseVisible() ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_PEN, pDoc->GetPresMouseAsPen() ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_NAVIGATOR, pDoc->GetStartPresWithNavigator() ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_ANIMATION_ALLOWED, pDoc->IsAnimationAllowed() ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_CHANGE_PAGE, !pDoc->GetPresLockedPages() ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_ALWAYS_ON_TOP, pDoc->GetPresAlwaysOnTop() ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_FULLSCREEN, pDoc->GetPresFullScreen() ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_START_ACTUAL_PAGE, bStartWithActualPage ) );
    aDlgSet.Put( SfxUInt32Item( ATTR_PRESENT_PAUSE_TIMEOUT, pDoc->GetPresPause() ) );
    aDlgSet.Put( SfxBoolItem( ATTR_PRESENT_SHOW_PAUSELOGO, pDoc->IsPresShowLogo() ) );

    //CHINA001 SdStartPresentationDlg aDlg (pWindow, aDlgSet, aPageNameList, pCustomShowList );
    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();//CHINA001
    DBG_ASSERT(pFact, "SdAbstractDialogFactory fail!");//CHINA001
    AbstractSdStartPresDlg* pDlg = pFact->CreateSdStartPresentationDlg(ResId( DLG_START_PRESENTATION ), pWindow, aDlgSet, aPageNameList, pCustomShowList );
    DBG_ASSERT(pDlg, "Dialogdiet fail!");//CHINA001
    if( pDlg->Execute() == RET_OK ) //CHINA001 if( aDlg.Execute() == RET_OK )
    {
        String  aPage;
        ULONG   nValue32;
        BOOL    bValue;
        BOOL    bValuesChanged = FALSE;

        pDlg->GetAttr( aDlgSet ); //CHINA001 aDlg.GetAttr( aDlgSet );

        aPage = ITEMVALUE( aDlgSet, ATTR_PRESENT_DIANAME, SfxStringItem );
        if( aPage != pDoc->GetPresPage() )
        {
            bValuesChanged = TRUE;
            pDoc->SetPresPage( aPage );
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_ALL, SfxBoolItem );
        if ( bValue != pDoc->GetPresAll() )
        {
            bValuesChanged = TRUE;
            pDoc->SetPresAll( bValue );
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_CUSTOMSHOW, SfxBoolItem );
        if ( bValue != pDoc->IsCustomShow() )
        {
            bValuesChanged = TRUE;
            pDoc->SetCustomShow( bValue );
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_ENDLESS, SfxBoolItem );
        if ( bValue != pDoc->GetPresEndless() )
        {
            bValuesChanged = TRUE;
            pDoc->SetPresEndless( bValue );
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_MANUEL, SfxBoolItem );
        if ( bValue != pDoc->GetPresManual() )
        {
            bValuesChanged = TRUE;
            pDoc->SetPresManual( bValue );
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_MOUSE, SfxBoolItem );
        if ( bValue != pDoc->GetPresMouseVisible() )
        {
            bValuesChanged = TRUE;
            pDoc->SetPresMouseVisible( bValue );
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_PEN, SfxBoolItem );
        if ( bValue != pDoc->GetPresMouseAsPen() )
        {
            bValuesChanged = TRUE;
            pDoc->SetPresMouseAsPen( bValue );
        }

        bValue = !ITEMVALUE( aDlgSet, ATTR_PRESENT_CHANGE_PAGE, SfxBoolItem );
        if ( bValue != pDoc->GetPresLockedPages() )
        {
            bValuesChanged = TRUE;
            pDoc->SetPresLockedPages( bValue );
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_NAVIGATOR, SfxBoolItem );
        if ( bValue != pDoc->GetStartPresWithNavigator() )
        {
            bValuesChanged = TRUE;
            pDoc->SetStartPresWithNavigator( bValue );
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_ANIMATION_ALLOWED, SfxBoolItem );
        if ( bValue != pDoc->IsAnimationAllowed() )
        {
            bValuesChanged = TRUE;
            pDoc->SetAnimationAllowed( bValue );
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_ALWAYS_ON_TOP, SfxBoolItem );
        if ( bValue != pDoc->GetPresAlwaysOnTop() )
        {
            bValuesChanged = TRUE;
            pDoc->SetPresAlwaysOnTop( bValue );
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_FULLSCREEN, SfxBoolItem );
        if ( bValue != pDoc->GetPresFullScreen() )
        {
            bValuesChanged = TRUE;
            pDoc->SetPresFullScreen( bValue );
        }

        nValue32 = ITEMVALUE( aDlgSet, ATTR_PRESENT_PAUSE_TIMEOUT, SfxUInt32Item );
        if( nValue32 != pDoc->GetPresPause() )
        {
            bValuesChanged = TRUE;
            pDoc->SetPresPause( nValue32 );
        }

        bValue = ITEMVALUE( aDlgSet, ATTR_PRESENT_SHOW_PAUSELOGO, SfxBoolItem );
        if ( bValue != pDoc->IsPresShowLogo() )
        {
            bValuesChanged = TRUE;
            pDoc->SetPresShowLogo( bValue );
        }

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
