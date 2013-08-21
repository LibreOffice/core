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

/**
 * SdNavigatorWin - FloatingWindow
 */
SdNavigatorWin::SdNavigatorWin(
    ::Window* pParent,
    ::sd::NavigatorChildWindow* pChWinCtxt,
    const SdResId& rSdResId,
    SfxBindings* pInBindings,
    const UpdateRequestFunctor& rUpdateRequest)
    : ::Window( pParent, rSdResId )
    , maToolbox ( this, SdResId( 1 ) )
    , maTlbObjects( this, SdResId( TLB_OBJECTS ) )
    , maLbDocs ( this, SdResId( LB_DOCS ) )
    , mpChildWinContext( pChWinCtxt )
    , mbDocImported ( sal_False )
      // On changes of the DragType: adjust SelectionMode of TLB!
    , meDragType ( NAVIGATOR_DRAGTYPE_EMBEDDED )
    , mpBindings ( pInBindings )
    , maImageList ( SdResId( IL_NAVIGATR ) )
{
    maTlbObjects.SetViewFrame( mpBindings->GetDispatcher()->GetFrame() );

    FreeResource();

    maTlbObjects.SetAccessibleName(SD_RESSTR(STR_OBJECTS_TREE));

    mpNavigatorCtrlItem = new SdNavigatorControllerItem( SID_NAVIGATOR_STATE, this, mpBindings, rUpdateRequest);
    mpPageNameCtrlItem = new SdPageNameControllerItem( SID_NAVIGATOR_PAGENAME, this, mpBindings, rUpdateRequest);

    ApplyImageList(); // load images *before* calculating sizes to get something useful !!!

    Size aTbxSize( maToolbox.CalcWindowSizePixel() );
    maToolbox.SetOutputSizePixel( aTbxSize );
    maToolbox.SetSelectHdl( LINK( this, SdNavigatorWin, SelectToolboxHdl ) );
    maToolbox.SetClickHdl( LINK( this, SdNavigatorWin, ClickToolboxHdl ) );
    maToolbox.SetDropdownClickHdl( LINK(this, SdNavigatorWin, DropdownClickToolBoxHdl) );
    maToolbox.SetItemBits( TBI_DRAGTYPE, maToolbox.GetItemBits( TBI_DRAGTYPE ) | TIB_DROPDOWNONLY );

    // Shape filter drop down menu.
    maToolbox.SetItemBits(
        TBI_SHAPE_FILTER,
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

    // assure that tool box is at least as wide as the tree list box
    {
        const Size aTlbSize( maTlbObjects.GetOutputSizePixel() );
        if ( aTlbSize.Width() > aTbxSize.Width() )
        {
            maToolbox.setPosSizePixel( 0, 0, aTlbSize.Width(), 0, WINDOW_POSSIZE_WIDTH );
            aTbxSize = maToolbox.GetOutputSizePixel();
        }
    }

    // set min outputsize after all sizes are known
    const long nFullHeight = nListboxYPos + maLbDocs.GetSizePixel().Height() + 4;
    maSize = GetOutputSizePixel();
    if( maSize.Height() < nFullHeight )
    {
        maSize.Height() = nFullHeight;
        SetOutputSizePixel( maSize );
    }
    maMinSize = maSize;
    const long nMinWidth = 2*maToolbox.GetPosPixel().X() + aTbxSize.Width(); // never clip the toolbox
    if( nMinWidth > maMinSize.Width() )
        maMinSize.Width() = nMinWidth;
    maMinSize.Height() -= 40;
    SfxDockingWindow* pDockingParent = dynamic_cast<SfxDockingWindow*>(GetParent());
    if (pDockingParent != NULL)
        pDockingParent->SetMinOutputSizePixel( maMinSize );

    // InitTlb; is initiated over Slot
    if (rUpdateRequest)
        rUpdateRequest();
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
    OUString aDocShName( pDocShell->GetName() );
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
        OUString aDocName = pDocShell->GetMedium()->GetName();
        maTlbObjects.Clear();
        maTlbObjects.Fill( pDoc, (sal_Bool) sal_False, aDocName ); // only normal pages

        RefreshDocumentLB();
        maLbDocs.SelectEntry( aDocShName );
    }
    else
    {
        maLbDocs.SetNoSelection();
        maLbDocs.SelectEntry( aDocShName );

// commented in order to fix 30246
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

/**
 * DragType is set on dependance if a Drag is even possible. For example,
 * under certain circumstances, it is not allowed to drag graphics (#31038#).
 */
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
            // Popup menu is created depending if the document is saved or not
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
                    pMenu->InsertItem( nID, SD_RESSTR( nRId ) );
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
                SD_RESSTR(STR_NAVIGATOR_SHOW_NAMED_SHAPES));
            pMenu->InsertItem(
                nShowAllShapesFilter,
                SD_RESSTR(STR_NAVIGATOR_SHOW_ALL_SHAPES));

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

        // if it is the active window, we jump to the page
        if( pInfo && pInfo->IsActive() )
        {
            OUString aStr( maTlbObjects.GetSelectEntry() );

            if( !aStr.isEmpty() )
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
    OUString aStrLb = maLbDocs.GetSelectEntry();
    long   nPos = maLbDocs.GetSelectEntryPos();
    sal_Bool   bFound = sal_False;
    ::sd::DrawDocShell* pDocShell = NULL;
    NavDocInfo* pInfo = GetDocInfo();

    // is it a dragged object?
    if( mbDocImported && nPos == 0 )
    {
        // construct document in TLB
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
            OUString aDocName = pNCDocShell->GetMedium()->GetName();
            maTlbObjects.Clear();
            maTlbObjects.Fill( pDoc, (sal_Bool) sal_False, aDocName ); // only normal pages
        }
    }

    // check if link or url is possible
    if( ( pInfo && !pInfo->HasName() ) || !maTlbObjects.IsLinkableSelected() || ( meDragType != NAVIGATOR_DRAGTYPE_EMBEDDED ) )
    {
        meDragType = NAVIGATOR_DRAGTYPE_EMBEDDED;
        SetDragImage();
    }

    return( 0L );
}

/**
 * Set DrageType and set image accordingly to it.
 * If the handler is called with NULL, the default (URL) is set.
 */
IMPL_LINK( SdNavigatorWin, MenuSelectHdl, Menu *, pMenu )
{
    sal_uInt16 nMenuId;
    if( pMenu )
        nMenuId = pMenu->GetCurItemId();
    else
        nMenuId = NAVIGATOR_DRAGTYPE_URL;

    if( nMenuId != USHRT_MAX ) // Necessary ?
    {
        NavigatorDragType eDT = (NavigatorDragType) nMenuId;
        if( meDragType != eDT )
        {
            meDragType = eDT;
            SetDragImage();

            if( meDragType == NAVIGATOR_DRAGTYPE_URL )
            {
                // patch, prevents endless loop
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

        // change size of Toolbox
        Size aObjSize( maToolbox.GetOutputSizePixel() );
        aObjSize.Width() += aDiffSize.Width();
        maToolbox.SetOutputSizePixel( aObjSize );

        // change size of TreeLB
        aObjSize = maTlbObjects.GetSizePixel();
        aObjSize.Width() += aDiffSize.Width();
        aObjSize.Height() = maLbDocs.GetPosPixel().Y() + aDiffSize.Height() -
            maTlbObjects.GetPosPixel().Y() - 4;
        maTlbObjects.SetSizePixel( aObjSize );

        Point aPt( 0, aDiffSize.Height() );

        // move other controls (DocumentLB)
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

bool SdNavigatorWin::InsertFile(const OUString& rFileName)
{
    INetURLObject   aURL( rFileName );

    if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        OUString aURLStr;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFileName, aURLStr );
        aURL = INetURLObject( aURLStr );
    }

    // get adjusted FileName
    OUString aFileName( aURL.GetMainURL( INetURLObject::NO_DECODE ) );

    if (aFileName.isEmpty())
    {
        // show actual document again
        maDropFileName = aFileName;
    }
    else
    {
        // show dragged-in document
        const SfxFilter* pFilter = NULL;
        ErrCode nErr = 0;

        if (aFileName != maDropFileName)
        {
            SfxMedium aMed(aFileName, (STREAM_READ | STREAM_SHARE_DENYNONE));
            SfxFilterMatcher aMatch( OUString("simpress") );
            aMed.UseInteractionHandler( sal_True );
            nErr = aMatch.GuessFilter(aMed, &pFilter);
        }

        if ((pFilter && !nErr) || aFileName == maDropFileName)
        {
            // The medium may be opened with READ/WRITE. Therefore, we first
            // check if it contains a Storage.
            SfxMedium* pMedium = new SfxMedium( aFileName,
                                                STREAM_READ | STREAM_NOCREATE);

            if (pMedium->IsStorage())
            {
                // Now depending on mode:
                // maTlbObjects.SetSelectionMode(MULTIPLE_SELECTION);
                // handover of ownership of pMedium;
                SdDrawDocument* pDropDoc = maTlbObjects.GetBookmarkDoc(pMedium);

                if (pDropDoc)
                {
                    maTlbObjects.Clear();
                    maDropFileName = aFileName;

                    if( !maTlbObjects.IsEqualToDoc( pDropDoc ) )
                    {
                        // only normal pages
                        maTlbObjects.Fill(pDropDoc, (sal_Bool) sal_False, maDropFileName);
                        RefreshDocumentLB( &maDropFileName );
                    }
                }
            }
            else
            {
                delete pMedium;
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------

void SdNavigatorWin::RefreshDocumentLB( const OUString* pDocName )
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

        OUString aStr;
        if( mbDocImported )
            aStr = maLbDocs.GetEntry( 0 );

        maLbDocs.Clear();

        // delete list of DocInfos
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
                if( !aStr.isEmpty() )
                    aInfo.SetName();
                else
                    aInfo.SetName( sal_False );
                // at the moment, we use the name of the shell again (i.e.
                // without path) since Koose thinks it is an error if the path
                // is shown in url notation!
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
        default: OSL_FAIL( "No resource for DragType available!" );
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

/**
 * PreNotify
 */
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


/**
 * catch ESCAPE in order to end show
 */
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



/**
 * ControllerItem for Navigator
 */
SdNavigatorControllerItem::SdNavigatorControllerItem(
    sal_uInt16 _nId,
    SdNavigatorWin* pNavWin,
    SfxBindings*    _pBindings,
    const SdNavigatorWin::UpdateRequestFunctor& rUpdateRequest)
    : SfxControllerItem( _nId, *_pBindings ),
      pNavigatorWin( pNavWin ),
      maUpdateRequest(rUpdateRequest)
{
}

// -----------------------------------------------------------------------

void SdNavigatorControllerItem::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    if( eState >= SFX_ITEM_AVAILABLE && nSId == SID_NAVIGATOR_STATE )
    {
        const SfxUInt32Item* pStateItem = PTR_CAST( SfxUInt32Item, pItem );
        DBG_ASSERT( pStateItem, "SfxUInt16Item expected");
        sal_uInt32 nState = pStateItem->GetValue();

        // pen
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

        // only if doc in LB is the active
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
                // InitTlb; is initiated by Slot
                if (maUpdateRequest)
                    maUpdateRequest();
            }
        }
    }
}

/**
 * ControllerItem for Navigator to show page in TreeLB
 */
SdPageNameControllerItem::SdPageNameControllerItem(
    sal_uInt16 _nId,
    SdNavigatorWin* pNavWin,
    SfxBindings*    _pBindings,
    const SdNavigatorWin::UpdateRequestFunctor& rUpdateRequest)
    : SfxControllerItem( _nId, *_pBindings ),
      pNavigatorWin( pNavWin ),
      maUpdateRequest(rUpdateRequest)
{
}

// -----------------------------------------------------------------------

void SdPageNameControllerItem::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    if( eState >= SFX_ITEM_AVAILABLE && nSId == SID_NAVIGATOR_PAGENAME )
    {
        // only if doc in LB is the active
        NavDocInfo* pInfo = pNavigatorWin->GetDocInfo();
        if( pInfo && pInfo->IsActive() )
        {
            const SfxStringItem* pStateItem = PTR_CAST( SfxStringItem, pItem );
            DBG_ASSERT( pStateItem, "SfxStringItem expected");
            OUString aPageName = pStateItem->GetValue();

            if( !pNavigatorWin->maTlbObjects.HasSelectedChildren( aPageName ) )
            {
                if( pNavigatorWin->maTlbObjects.GetSelectionMode() == MULTIPLE_SELECTION )
                {
                    // because otherwise it is always additional select
                    pNavigatorWin->maTlbObjects.SelectAll( sal_False );
                }
                pNavigatorWin->maTlbObjects.SelectEntry( aPageName );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
