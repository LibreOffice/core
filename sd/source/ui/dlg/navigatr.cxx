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

#include <tools/urlobj.hxx>
#include <unotools/localfilehelper.hxx>
#include <sfx2/imgmgr.hxx>
#include <sfx2/fcontnr.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <vcl/menu.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/sfxresid.hxx>

#include "pres.hxx"
#include "navigatr.hxx"
#include "navigatr.hrc"
#include "pgjump.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdresid.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include "slideshow.hxx"
#include "FrameView.hxx"
#include "helpids.h"

namespace {
static const sal_uInt16 nShowNamedShapesFilter=1;
static const sal_uInt16 nShowAllShapesFilter=2;
}

/*************************************************************************
|*  SdNavigatorWin - FloatingWindow
\************************************************************************/

SdNavigatorWin::SdNavigatorWin(
    ::Window* pParent,
    ::sd::NavigatorChildWindow* pChWinCtxt,
    const SdResId& rSdResId,
    SfxBindings* pInBindings )
:   ::Window( pParent, rSdResId )
,   maToolbox        ( this, SdResId( 1 ) )
,   maTlbObjects( this, SdResId( TLB_OBJECTS ) )
,   maLbDocs         ( this, SdResId( LB_DOCS ) )
,   mpChildWinContext( pChWinCtxt )
,   mbDocImported   ( sal_False )
    // Bei Aenderung des DragTypes: SelectionMode der TLB anpassen!
,   meDragType      ( NAVIGATOR_DRAGTYPE_EMBEDDED )
,   mpBindings      ( pInBindings )
,   maImageList     ( SdResId( IL_NAVIGATR ) )
{
    maTlbObjects.SetViewFrame( mpBindings->GetDispatcher()->GetFrame() );

    FreeResource();

    maTlbObjects.SetAccessibleName(String(SdResId(STR_OBJECTS_TREE)));

    mpNavigatorCtrlItem = new SdNavigatorControllerItem( SID_NAVIGATOR_STATE, this, mpBindings );
    mpPageNameCtrlItem = new SdPageNameControllerItem( SID_NAVIGATOR_PAGENAME, this, mpBindings );

    ApplyImageList(); // load images *before* calculating sizes to get something useful !!!

    Size aTbxSize( maToolbox.CalcWindowSizePixel() );
    maToolbox.SetOutputSizePixel( aTbxSize );
    maToolbox.SetSelectHdl( LINK( this, SdNavigatorWin, SelectToolboxHdl ) );
    maToolbox.SetClickHdl( LINK( this, SdNavigatorWin, ClickToolboxHdl ) );
    maToolbox.SetDropdownClickHdl( LINK(this, SdNavigatorWin, DropdownClickToolBoxHdl) );
    maToolbox.SetItemBits( TBI_DRAGTYPE, maToolbox.GetItemBits( TBI_DRAGTYPE ) | TIB_DROPDOWNONLY );

    // Shape filter drop down menu.
    maToolbox.SetItemBits(TBI_SHAPE_FILTER,
        maToolbox.GetItemBits(TBI_SHAPE_FILTER) | TIB_DROPDOWNONLY);

    // TreeListBox
    // set position below toolbox
    long nListboxYPos = maToolbox.GetPosPixel().Y() + maToolbox.GetSizePixel().Height() + 4;
    maTlbObjects.setPosSizePixel( 0, nListboxYPos, 0, 0, WINDOW_POSSIZE_Y );
    maTlbObjects.SetDoubleClickHdl( LINK( this, SdNavigatorWin, ClickObjectHdl ) );
    maTlbObjects.SetSelectionMode( SINGLE_SELECTION );
    // set focus to listbox, otherwise it is in the toolbox which is only useful
    // for keyboard navigation
    maTlbObjects.GrabFocus();

    // DragTypeListBox
    maLbDocs.SetSelectHdl( LINK( this, SdNavigatorWin, SelectDocumentHdl ) );
    // set position below treelistbox
    nListboxYPos = maTlbObjects.GetPosPixel().Y() + maTlbObjects.GetSizePixel().Height() + 4;
    maLbDocs.setPosSizePixel( 0, nListboxYPos, 0, 0, WINDOW_POSSIZE_Y );

    // set min outputsize after all sizes are known
    long nFullHeight = nListboxYPos + maLbDocs.GetSizePixel().Height() + 4;
    maSize = GetOutputSizePixel();
    if( maSize.Height() < nFullHeight )
    {
        maSize.Height() = nFullHeight;
        SetOutputSizePixel( maSize );
    }
    maMinSize = maSize;
    long nMinWidth = 2*maToolbox.GetPosPixel().X() + aTbxSize.Width(); // never clip the toolbox
    if( nMinWidth > maMinSize.Width() )
        maMinSize.Width() = nMinWidth;
    maMinSize.Height() -= 40;
    ((SfxDockingWindow*)GetParent())->SetMinOutputSizePixel( maMinSize );

    // InitTlb; Wird ueber Slot initiiert
    SfxBoolItem aItem( SID_NAVIGATOR_INIT, sal_True );
    mpBindings->GetDispatcher()->Execute(
        SID_NAVIGATOR_INIT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );

}

// -----------------------------------------------------------------------

SdNavigatorWin::~SdNavigatorWin()
{
    delete mpNavigatorCtrlItem;
    delete mpPageNameCtrlItem;
}

// -----------------------------------------------------------------------

void SdNavigatorWin::InitTreeLB( const SdDrawDocument* pDoc )
{
    SdDrawDocument* pNonConstDoc = (SdDrawDocument*) pDoc; // const as const can...
    ::sd::DrawDocShell* pDocShell = pNonConstDoc->GetDocSh();
    String aDocShName( pDocShell->GetName() );
    ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();

    // Restore the 'ShowAllShapes' flag from the last time (in this session)
    // that the navigator was shown.
    if (pViewShell != NULL)
    {
        ::sd::FrameView* pFrameView = pViewShell->GetFrameView();
        if (pFrameView != NULL)
            maTlbObjects.SetShowAllShapes(pFrameView->IsNavigatorShowingAllShapes(), false);
    }

    // Disable the shape filter drop down menu when there is a running slide
    // show.
    if (pViewShell!=NULL && sd::SlideShow::IsRunning( pViewShell->GetViewShellBase() ))
        maToolbox.EnableItem(TBI_SHAPE_FILTER, sal_False);
    else
        maToolbox.EnableItem(TBI_SHAPE_FILTER);

    if( !maTlbObjects.IsEqualToDoc( pDoc ) )
    {
        String aDocName = pDocShell->GetMedium()->GetName();
        maTlbObjects.Clear();
        maTlbObjects.Fill( pDoc, (sal_Bool) sal_False, aDocName ); // Nur normale Seiten

        RefreshDocumentLB();
        maLbDocs.SelectEntry( aDocShName );
    }
    else
    {
        maLbDocs.SetNoSelection();
        maLbDocs.SelectEntry( aDocShName );

// auskommentiert um 30246 zu fixen
//        if( maLbDocs.GetSelectEntryCount() == 0 )
        {
            RefreshDocumentLB();
            maLbDocs.SelectEntry( aDocShName );
        }
    }

    SfxViewFrame* pViewFrame = ( ( pViewShell && pViewShell->GetViewFrame() ) ? pViewShell->GetViewFrame() : SfxViewFrame::Current() );
    if( pViewFrame )
        pViewFrame->GetBindings().Invalidate(SID_NAVIGATOR_PAGENAME, sal_True, sal_True);
}

/*************************************************************************
|*
|* DragType wird in Abhaengigkeit davon gesetzt, ob ein Drag ueberhaupt
|* moeglich ist. Graphiken duerfen beispielsweise unter gewissen Umstaenden
|* nicht gedragt werden (#31038#).
|*
\************************************************************************/

NavigatorDragType SdNavigatorWin::GetNavigatorDragType()
{
    NavigatorDragType   eDT = meDragType;
    NavDocInfo*         pInfo = GetDocInfo();

    if( ( eDT == NAVIGATOR_DRAGTYPE_LINK ) && ( ( pInfo && !pInfo->HasName() ) || !maTlbObjects.IsLinkableSelected() ) )
        eDT = NAVIGATOR_DRAGTYPE_NONE;

    return( eDT );
}

// -----------------------------------------------------------------------


IMPL_LINK_NOARG(SdNavigatorWin, SelectToolboxHdl)
{
    sal_uInt16 nId = maToolbox.GetCurItemId();
    sal_uInt16 nSId = 0;
    PageJump ePage = PAGE_NONE;

    switch( nId )
    {
        case TBI_PEN:
        {
            if( nId == TBI_PEN )
            {
                nSId = SID_NAVIGATOR_PEN;
            }

            if( nSId > 0 )
            {
                SfxBoolItem aItem( nSId, sal_True );
                mpBindings->GetDispatcher()->Execute(
                    nSId, SFX_CALLMODE_SLOT |SFX_CALLMODE_RECORD, &aItem, 0L );
            }
        }
        break;

        case TBI_FIRST:
        case TBI_PREVIOUS:
        case TBI_NEXT:
        case TBI_LAST:
        {
            if( nId == TBI_FIRST )
                ePage = PAGE_FIRST;
            else if( nId == TBI_PREVIOUS )
                ePage = PAGE_PREVIOUS;
            else if( nId == TBI_NEXT )
                ePage = PAGE_NEXT;
            else if( nId == TBI_LAST )
                ePage = PAGE_LAST;

            if( ePage != PAGE_NONE )
            {
                SfxUInt16Item aItem( SID_NAVIGATOR_PAGE, (sal_uInt16)ePage );
                mpBindings->GetDispatcher()->Execute(
                    SID_NAVIGATOR_PAGE, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aItem, 0L );
            }
        }
        break;
    }
    return 0;
}

// -----------------------------------------------------------------------


IMPL_LINK_NOARG(SdNavigatorWin, ClickToolboxHdl)
{
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SdNavigatorWin, DropdownClickToolBoxHdl, ToolBox*, pBox )
{
    sal_uInt16 nId = maToolbox.GetCurItemId();

    switch( nId )
    {
        case TBI_DRAGTYPE:
        {
            // Popup-Menu wird in Abhaengigkeit davon erzeugt, ob Dokument
            // gespeichert ist oder nicht
            PopupMenu *pMenu = new PopupMenu;

            static const char* aHIDs[] =
            {
                 HID_SD_NAVIGATOR_MENU1,
                 HID_SD_NAVIGATOR_MENU2,
                 HID_SD_NAVIGATOR_MENU3,
                 0
            };

            for( sal_uInt16 nID = NAVIGATOR_DRAGTYPE_URL;
                 nID < NAVIGATOR_DRAGTYPE_COUNT;
                 nID++ )
            {
                sal_uInt16 nRId = GetDragTypeSdResId( (NavigatorDragType)nID );
                if( nRId > 0 )
                {
                    DBG_ASSERT(aHIDs[nID-NAVIGATOR_DRAGTYPE_URL],"HelpId not added!");
                    pMenu->InsertItem( nID, String( SdResId( nRId ) ) );
                    pMenu->SetHelpId( nID, aHIDs[nID - NAVIGATOR_DRAGTYPE_URL] );
                }

            }
            NavDocInfo* pInfo = GetDocInfo();

            if( ( pInfo && !pInfo->HasName() ) || !maTlbObjects.IsLinkableSelected() )
            {
                pMenu->EnableItem( NAVIGATOR_DRAGTYPE_LINK, sal_False );
                pMenu->EnableItem( NAVIGATOR_DRAGTYPE_URL, sal_False );
                meDragType = NAVIGATOR_DRAGTYPE_EMBEDDED;
            }

            pMenu->CheckItem( (sal_uInt16)meDragType );
            pMenu->SetSelectHdl( LINK( this, SdNavigatorWin, MenuSelectHdl ) );

            pMenu->Execute( this, maToolbox.GetItemRect( nId ), POPUPMENU_EXECUTE_DOWN );
            pBox->EndSelection();
            delete pMenu;
        }
        break;

        case TBI_SHAPE_FILTER:
        {
            PopupMenu *pMenu = new PopupMenu;

            pMenu->InsertItem(
                nShowNamedShapesFilter,
                String(SdResId(STR_NAVIGATOR_SHOW_NAMED_SHAPES)));
            pMenu->InsertItem(
                nShowAllShapesFilter,
                String(SdResId(STR_NAVIGATOR_SHOW_ALL_SHAPES)));

            if (maTlbObjects.GetShowAllShapes())
                pMenu->CheckItem(nShowAllShapesFilter);
            else
                pMenu->CheckItem(nShowNamedShapesFilter);
            pMenu->SetSelectHdl( LINK( this, SdNavigatorWin, ShapeFilterCallback ) );

            pMenu->Execute( this, maToolbox.GetItemRect( nId ), POPUPMENU_EXECUTE_DOWN );
            pBox->EndSelection();
            delete pMenu;
        }
        break;
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SdNavigatorWin, ClickObjectHdl)
{
    if( !mbDocImported || maLbDocs.GetSelectEntryPos() != 0 )
    {
        NavDocInfo* pInfo = GetDocInfo();

        // Nur wenn es sich um das aktive Fenster handelt, wird
        // auf die Seite gesprungen
        if( pInfo && pInfo->IsActive() )
        {
            String aStr( maTlbObjects.GetSelectEntry() );

            if( aStr.Len() > 0 )
            {
                SfxStringItem aItem( SID_NAVIGATOR_OBJECT, aStr );
                mpBindings->GetDispatcher()->Execute(
                    SID_NAVIGATOR_OBJECT, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aItem, 0L );

                // moved here from SetGetFocusHdl. Reset the
                // focus only if something has been selected in the
                // document.
                SfxViewShell* pCurSh = SfxViewShell::Current();

                if ( pCurSh )
                {
                    Window* pShellWnd = pCurSh->GetWindow();
                    if ( pShellWnd )
                        pShellWnd->GrabFocus();
                }
            }
        }
    }
    return( 0L );
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SdNavigatorWin, SelectDocumentHdl)
{
    String aStrLb = maLbDocs.GetSelectEntry();
    long   nPos = maLbDocs.GetSelectEntryPos();
    sal_Bool   bFound = sal_False;
    ::sd::DrawDocShell* pDocShell = NULL;
    NavDocInfo* pInfo = GetDocInfo();

    // Handelt es sich um ein gedragtes Objekt?
    if( mbDocImported && nPos == 0 )
    {
        // Dokument in TLB aufbauen
        InsertFile( aStrLb );
    }
    else if (pInfo)
    {
        pDocShell = pInfo->mpDocShell;

        bFound = sal_True;
    }

    if( bFound )
    {
        SdDrawDocument* pDoc = pDocShell->GetDoc();
        if( !maTlbObjects.IsEqualToDoc( pDoc ) )
        {
            SdDrawDocument* pNonConstDoc = (SdDrawDocument*) pDoc; // const as const can...
            ::sd::DrawDocShell* pNCDocShell = pNonConstDoc->GetDocSh();
            String aDocName = pNCDocShell->GetMedium()->GetName();
            maTlbObjects.Clear();
            maTlbObjects.Fill( pDoc, (sal_Bool) sal_False, aDocName ); // Nur normale Seiten
        }
    }

    // Pruefen, ob Link oder URL moeglich ist
    if( ( pInfo && !pInfo->HasName() ) || !maTlbObjects.IsLinkableSelected() || ( meDragType != NAVIGATOR_DRAGTYPE_EMBEDDED ) )
    {
        meDragType = NAVIGATOR_DRAGTYPE_EMBEDDED;
        SetDragImage();
    }

    return( 0L );
}

/*************************************************************************
|*
|* DrageType wird gesetzt und Image wird entspr. gesetzt.
|* Sollte Handler mit NULL gerufen werden, so wird der Default (URL) gesetzt.
|*
\************************************************************************/

IMPL_LINK( SdNavigatorWin, MenuSelectHdl, Menu *, pMenu )
{
    sal_uInt16 nMenuId;
    if( pMenu )
        nMenuId = pMenu->GetCurItemId();
    else
        nMenuId = NAVIGATOR_DRAGTYPE_URL;

    if( nMenuId != USHRT_MAX ) // Notwendig ?
    {
        NavigatorDragType eDT = (NavigatorDragType) nMenuId;
        if( meDragType != eDT )
        {
            meDragType = eDT;
            SetDragImage();

            if( meDragType == NAVIGATOR_DRAGTYPE_URL )
            {
                // Fix, um Endlosschleife zu unterbinden
                if( maTlbObjects.GetSelectionCount() > 1 )
                    maTlbObjects.SelectAll( sal_False );

                maTlbObjects.SetSelectionMode( SINGLE_SELECTION );
            }
            else
                maTlbObjects.SetSelectionMode( MULTIPLE_SELECTION );
        }
    }
    return( 0 );
}




IMPL_LINK( SdNavigatorWin, ShapeFilterCallback, Menu *, pMenu )
{
    if (pMenu != NULL)
    {
        bool bShowAllShapes (maTlbObjects.GetShowAllShapes());
        sal_uInt16 nMenuId (pMenu->GetCurItemId());
        switch (nMenuId)
        {
            case nShowNamedShapesFilter:
                bShowAllShapes = false;
                break;

            case nShowAllShapesFilter:
                bShowAllShapes = true;
                break;

            default:
                OSL_FAIL(
                    "SdNavigatorWin::ShapeFilterCallback called for unknown menu entry");
                break;
        }

        maTlbObjects.SetShowAllShapes(bShowAllShapes, true);

        // Remember the selection in the FrameView.
        NavDocInfo* pInfo = GetDocInfo();
        if (pInfo != NULL)
        {
            ::sd::DrawDocShell* pDocShell = pInfo->mpDocShell;
            if (pDocShell != NULL)
            {
                ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();
                if (pViewShell != NULL)
                {
                    ::sd::FrameView* pFrameView = pViewShell->GetFrameView();
                    if (pFrameView != NULL)
                    {
                        pFrameView->SetIsNavigatorShowingAllShapes(bShowAllShapes);
                    }
                }
            }
        }
    }

    return 0;
}

// -----------------------------------------------------------------------

void SdNavigatorWin::Resize()
{
    Size aWinSize( GetOutputSizePixel() );
    if( aWinSize.Height() >= maMinSize.Height() )
        //aWinSize.Width() >= maMinSize.Width() )
    {
        Size aDiffSize;
        aDiffSize.Width() = aWinSize.Width() - maSize.Width();
        aDiffSize.Height() = aWinSize.Height() - maSize.Height();

        // Umgroessern der Toolbox
        Size aObjSize( maToolbox.GetOutputSizePixel() );
        aObjSize.Width() += aDiffSize.Width();
        maToolbox.SetOutputSizePixel( aObjSize );

        // Umgroessern der TreeLB
        aObjSize = maTlbObjects.GetSizePixel();
        aObjSize.Width() += aDiffSize.Width();
        aObjSize.Height() = maLbDocs.GetPosPixel().Y() + aDiffSize.Height() -
            maTlbObjects.GetPosPixel().Y() - 4;
        maTlbObjects.SetSizePixel( aObjSize );

        Point aPt( 0, aDiffSize.Height() );

        // Verschieben der anderen Controls (DocumentLB)
        maLbDocs.Hide();
        aObjSize = maLbDocs.GetOutputSizePixel();
        aObjSize.Width() += aDiffSize.Width();
        maLbDocs.SetPosPixel( maLbDocs.GetPosPixel() + aPt );
        maLbDocs.SetOutputSizePixel( aObjSize );
        maLbDocs.Show();

        maSize = aWinSize;
    }
    Window::Resize();
}

// -----------------------------------------------------------------------

sal_Bool SdNavigatorWin::InsertFile(const String& rFileName)
{
    INetURLObject   aURL( rFileName );
    sal_Bool            bReturn = sal_True;

    if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        rtl::OUString aURLStr;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFileName, aURLStr );
        aURL = INetURLObject( aURLStr );
    }

    // get adjusted FileName
    String aFileName( aURL.GetMainURL( INetURLObject::NO_DECODE ) );

    if (!aFileName.Len())
    {
        /**********************************************************************
        * Wieder aktuelles Dokument anzeigen
        **********************************************************************/
        maDropFileName = aFileName;
    }
    else
    {
        /**********************************************************************
        * Hineingedraggtes Dokument anzeigen
        **********************************************************************/
        const SfxFilter* pFilter = NULL;
        ErrCode nErr = 0;

        if (aFileName != maDropFileName)
        {
            SfxMedium aMed(aFileName, (STREAM_READ | STREAM_SHARE_DENYNONE));
            SfxFilterMatcher aMatch( rtl::OUString("simpress") );
            aMed.UseInteractionHandler( sal_True );
            nErr = aMatch.GuessFilter(aMed, &pFilter);
        }

        if ((pFilter && !nErr) || aFileName == maDropFileName)
        {
            // Das Medium muss ggf. mit READ/WRITE geoeffnet werden, daher wird
            // ersteinmal nachgeschaut, ob es einen Storage enthaelt
            SfxMedium* pMedium = new SfxMedium( aFileName,
                                                STREAM_READ | STREAM_NOCREATE);

            if (pMedium->IsStorage())
            {
                // Jetzt modusabhaengig:
                // maTlbObjects.SetSelectionMode(MULTIPLE_SELECTION);
                // Eigentuemeruebergabe von pMedium;
                SdDrawDocument* pDropDoc = maTlbObjects.GetBookmarkDoc(pMedium);

                if (pDropDoc)
                {
                    maTlbObjects.Clear();
                    maDropFileName = aFileName;

                    if( !maTlbObjects.IsEqualToDoc( pDropDoc ) )
                    {
                        // Nur normale Seiten
                        maTlbObjects.Fill(pDropDoc, (sal_Bool) sal_False, maDropFileName);
                        RefreshDocumentLB( &maDropFileName );
                    }
                }
            }
            else
            {
                bReturn = sal_False;
                delete pMedium;
            }
        }
        else
        {
            bReturn = sal_False;
        }
    }

    return (bReturn);
}

// -----------------------------------------------------------------------

void SdNavigatorWin::RefreshDocumentLB( const String* pDocName )
{
    sal_uInt16 nPos = 0;

    if( pDocName )
    {
        if( mbDocImported )
            maLbDocs.RemoveEntry( 0 );

        maLbDocs.InsertEntry( *pDocName, 0 );
        mbDocImported = sal_True;
    }
    else
    {
        nPos = maLbDocs.GetSelectEntryPos();
        if( nPos == LISTBOX_ENTRY_NOTFOUND )
            nPos = 0;

        String aStr;
        if( mbDocImported )
            aStr = maLbDocs.GetEntry( 0 );

        maLbDocs.Clear();

        // Liste der DocInfos loeschen
         maDocList.clear();

        if( mbDocImported )
            maLbDocs.InsertEntry( aStr, 0 );

        ::sd::DrawDocShell* pCurrentDocShell =
              PTR_CAST(::sd::DrawDocShell, SfxObjectShell::Current() );
        SfxObjectShell* pSfxDocShell = SfxObjectShell::GetFirst(0, sal_False);
        while( pSfxDocShell )
        {
            ::sd::DrawDocShell* pDocShell = PTR_CAST(::sd::DrawDocShell, pSfxDocShell );
            if( pDocShell  && !pDocShell->IsInDestruction() && ( pDocShell->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED ) )
            {
                NavDocInfo aInfo ;
                aInfo.mpDocShell = pDocShell;

                SfxMedium *pMedium = pDocShell->GetMedium();
                aStr = pMedium ? pMedium->GetName() : OUString();
                if( aStr.Len() )
                    aInfo.SetName();
                else
                    aInfo.SetName( sal_False );
                // z.Z. wird wieder der Name der Shell genommen (also ohne Pfad)
                // da Koose es als Fehler ansieht, wenn er Pfad in URL-Notation
                // angezeigt wird!
                aStr = pDocShell->GetName();

                maLbDocs.InsertEntry( aStr, LISTBOX_APPEND );

                if( pDocShell == pCurrentDocShell )
                    aInfo.SetActive();
                else
                    aInfo.SetActive( sal_False );

                maDocList.push_back( aInfo );
            }
            pSfxDocShell = SfxObjectShell::GetNext( *pSfxDocShell, 0 , sal_False );
        }
    }
    maLbDocs.SelectEntryPos( nPos );
}

//------------------------------------------------------------------------

sal_uInt16 SdNavigatorWin::GetDragTypeSdResId( NavigatorDragType eDT, sal_Bool bImage )
{
    switch( eDT )
    {
        case NAVIGATOR_DRAGTYPE_NONE:
                return( bImage ? 0 : STR_NONE );
        case NAVIGATOR_DRAGTYPE_URL:
                return( bImage ? TBI_HYPERLINK : STR_DRAGTYPE_URL );
        case NAVIGATOR_DRAGTYPE_EMBEDDED:
                return( bImage ? TBI_EMBEDDED : STR_DRAGTYPE_EMBEDDED );
        case NAVIGATOR_DRAGTYPE_LINK:
                return( bImage ? TBI_LINK : STR_DRAGTYPE_LINK );
        default: OSL_FAIL( "Keine Resource fuer DragType vorhanden!" );
    }
    return( 0 );
}

//------------------------------------------------------------------------

NavDocInfo* SdNavigatorWin::GetDocInfo()
{
    sal_uInt32 nPos = maLbDocs.GetSelectEntryPos();

    if( mbDocImported )
    {
        if( nPos == 0 )
        {
            return( NULL );
        }
        nPos--;
    }

    return nPos < maDocList.size() ? &(maDocList[ nPos ]) : NULL;
}

/*************************************************************************
|*
|* PreNotify
|*
\************************************************************************/

long SdNavigatorWin::Notify(NotifyEvent& rNEvt)
{
    const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
    long            nOK = sal_False;

    if( pKEvt )
    {
        if( KEY_ESCAPE == pKEvt->GetKeyCode().GetCode() )
        {
            if( SdPageObjsTLB::IsInDrag() )
            {
                // during drag'n'drop we just stop the drag but do not close the navigator
                nOK = sal_True;
            }
            else
            {
                ::sd::ViewShellBase* pBase = ::sd::ViewShellBase::GetViewShellBase( mpBindings->GetDispatcher()->GetFrame());
                if( pBase )
                {
                    sd::SlideShow::Stop( *pBase );
                    // Stopping the slide show may result in a synchronous
                    // deletion of the navigator window.  Calling the
                    // parents Notify after this is unsafe.  Therefore we
                    // return now.
                    return sal_True;
                }
            }
        }
    }

    if( !nOK )
        nOK = Window::Notify( rNEvt );

    return( nOK );
}


/*************************************************************************
|*
|* KeyInput: ESCAPE abfangen, um Show zu beenden
|*
\************************************************************************/

void SdNavigatorWin::KeyInput( const KeyEvent& rKEvt )
{
    long nOK = sal_False;

    if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
    {
        if( SdPageObjsTLB::IsInDrag() )
        {
            // during drag'n'drop we just stop the drag but do not close the navigator
            nOK = sal_True;
        }
        else
        {
            ::sd::ViewShellBase* pBase = ::sd::ViewShellBase::GetViewShellBase( mpBindings->GetDispatcher()->GetFrame());
            if(pBase)
            {
                ::sd::SlideShow::Stop( *pBase );
            }
        }
    }

    if (!nOK)
    {
        Window::KeyInput(rKEvt);
    }
}

void SdNavigatorWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            ApplyImageList();

    Window::DataChanged( rDCEvt );
}

void SdNavigatorWin::SetDragImage()
{
    maToolbox.SetItemImage( TBI_DRAGTYPE, maToolbox.GetImageList().GetImage( GetDragTypeSdResId( meDragType, sal_True ) ) );
}

void SdNavigatorWin::ApplyImageList()
{

    maToolbox.SetImageList( maImageList );
    maToolbox.SetItemImage( TBI_SHAPE_FILTER, BitmapEx( SdResId( BMP_GRAPHIC ) ) );

    SetDragImage();
}



/*************************************************************************
|*
|* ControllerItem fuer Navigator
|*
\************************************************************************/

SdNavigatorControllerItem::SdNavigatorControllerItem( sal_uInt16 _nId,
                                SdNavigatorWin* pNavWin,
                                SfxBindings*    _pBindings) :
    SfxControllerItem( _nId, *_pBindings ),
    pNavigatorWin( pNavWin )
{
}

// -----------------------------------------------------------------------

void SdNavigatorControllerItem::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    if( eState >= SFX_ITEM_AVAILABLE && nSId == SID_NAVIGATOR_STATE )
    {
        const SfxUInt32Item* pStateItem = PTR_CAST( SfxUInt32Item, pItem );
        DBG_ASSERT( pStateItem, "SfxUInt16Item erwartet");
        sal_uInt32 nState = pStateItem->GetValue();

        // Stift
        if( nState & NAVBTN_PEN_ENABLED &&
            !pNavigatorWin->maToolbox.IsItemEnabled( TBI_PEN ) )
            pNavigatorWin->maToolbox.EnableItem( TBI_PEN );
        if( nState & NAVBTN_PEN_DISABLED &&
            pNavigatorWin->maToolbox.IsItemEnabled( TBI_PEN ) )
            pNavigatorWin->maToolbox.EnableItem( TBI_PEN, sal_False );
        if( nState & NAVBTN_PEN_CHECKED &&
            !pNavigatorWin->maToolbox.IsItemChecked( TBI_PEN ) )
            pNavigatorWin->maToolbox.CheckItem( TBI_PEN );
        if( nState & NAVBTN_PEN_UNCHECKED &&
            pNavigatorWin->maToolbox.IsItemChecked( TBI_PEN ) )
            pNavigatorWin->maToolbox.CheckItem( TBI_PEN, sal_False );

        // Nur wenn Doc in LB das Aktive ist
        NavDocInfo* pInfo = pNavigatorWin->GetDocInfo();
        if( pInfo && pInfo->IsActive() )
        {
            // First
            if( nState & NAVBTN_FIRST_ENABLED &&
                !pNavigatorWin->maToolbox.IsItemEnabled( TBI_FIRST ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_FIRST );
            if( nState & NAVBTN_FIRST_DISABLED &&
                pNavigatorWin->maToolbox.IsItemEnabled( TBI_FIRST ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_FIRST, sal_False );

            // Prev
            if( nState & NAVBTN_PREV_ENABLED &&
                !pNavigatorWin->maToolbox.IsItemEnabled( TBI_PREVIOUS ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_PREVIOUS );
            if( nState & NAVBTN_PREV_DISABLED &&
                pNavigatorWin->maToolbox.IsItemEnabled( TBI_PREVIOUS ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_PREVIOUS, sal_False );

            // Last
            if( nState & NAVBTN_LAST_ENABLED &&
                !pNavigatorWin->maToolbox.IsItemEnabled( TBI_LAST ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_LAST );
            if( nState & NAVBTN_LAST_DISABLED &&
                pNavigatorWin->maToolbox.IsItemEnabled( TBI_LAST ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_LAST, sal_False );

            // Next
            if( nState & NAVBTN_NEXT_ENABLED &&
                !pNavigatorWin->maToolbox.IsItemEnabled( TBI_NEXT ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_NEXT );
            if( nState & NAVBTN_NEXT_DISABLED &&
                pNavigatorWin->maToolbox.IsItemEnabled( TBI_NEXT ) )
                pNavigatorWin->maToolbox.EnableItem( TBI_NEXT, sal_False );

            if( nState & NAVTLB_UPDATE )
            {
                // InitTlb; Wird ueber Slot initiiert
                SfxBoolItem aItem( SID_NAVIGATOR_INIT, sal_True );
                GetBindings().GetDispatcher()->Execute(
                    SID_NAVIGATOR_INIT, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aItem, 0L );
            }
        }
    }
}

/*************************************************************************
|*
|* ControllerItem fuer Navigator, um die Seite in der TreeLB anzuzeigen
|*
\************************************************************************/

SdPageNameControllerItem::SdPageNameControllerItem( sal_uInt16 _nId,
                                SdNavigatorWin* pNavWin,
                                SfxBindings*    _pBindings) :
    SfxControllerItem( _nId, *_pBindings ),
    pNavigatorWin( pNavWin )
{
}

// -----------------------------------------------------------------------

void SdPageNameControllerItem::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    if( eState >= SFX_ITEM_AVAILABLE && nSId == SID_NAVIGATOR_PAGENAME )
    {
        // Nur wenn Doc in LB das Aktive ist
        NavDocInfo* pInfo = pNavigatorWin->GetDocInfo();
        if( pInfo && pInfo->IsActive() )
        {
            const SfxStringItem* pStateItem = PTR_CAST( SfxStringItem, pItem );
            DBG_ASSERT( pStateItem, "SfxStringItem erwartet");
            String aPageName = pStateItem->GetValue();

            if( !pNavigatorWin->maTlbObjects.HasSelectedChildren( aPageName ) )
            {
                if( pNavigatorWin->maTlbObjects.GetSelectionMode() == MULTIPLE_SELECTION )
                {
                    // Weil sonst immer dazuselektiert wird
                    pNavigatorWin->maTlbObjects.SelectAll( sal_False );
                }
                pNavigatorWin->maTlbObjects.SelectEntry( aPageName );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
