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

#include <sal/config.h>

#include <osl/file.hxx>
#include <tools/urlobj.hxx>
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
#include <vcl/settings.hxx>

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
    vcl::Window* pParent,
    const SdResId& rSdResId,
    SfxBindings* pInBindings)
    : vcl::Window( pParent, rSdResId )
    , maToolbox ( VclPtr<ToolBox>::Create( this, SdResId( 1 ) ) )
    , maTlbObjects( VclPtr<SdPageObjsTLB>::Create( this, SdResId( TLB_OBJECTS ) ) )
    , maLbDocs ( VclPtr<ListBox>::Create( this, SdResId( LB_DOCS ) ) )
    , mbDocImported ( false )
      // On changes of the DragType: adjust SelectionMode of TLB!
    , meDragType ( NAVIGATOR_DRAGTYPE_EMBEDDED )
    , mpBindings ( pInBindings )
    , mpNavigatorCtrlItem( nullptr )
    , mpPageNameCtrlItem( nullptr )
    , maImageList ( SdResId( IL_NAVIGATR ) )
{
    maTlbObjects->SetViewFrame( mpBindings->GetDispatcher()->GetFrame() );

    FreeResource();

    maTlbObjects->SetAccessibleName(SD_RESSTR(STR_OBJECTS_TREE));

    ApplyImageList(); // load images *before* calculating sizes to get something useful !!!

    Size aTbxSize( maToolbox->CalcWindowSizePixel() );
    maToolbox->SetOutputSizePixel( aTbxSize );
    maToolbox->SetSelectHdl( LINK( this, SdNavigatorWin, SelectToolboxHdl ) );
    maToolbox->SetDropdownClickHdl( LINK(this, SdNavigatorWin, DropdownClickToolBoxHdl) );
    maToolbox->SetItemBits( TBI_DRAGTYPE, maToolbox->GetItemBits( TBI_DRAGTYPE ) | ToolBoxItemBits::DROPDOWNONLY );

    // Shape filter drop down menu.
    maToolbox->SetItemBits(
        TBI_SHAPE_FILTER,
        maToolbox->GetItemBits(TBI_SHAPE_FILTER) | ToolBoxItemBits::DROPDOWNONLY);

    // TreeListBox
    // set position below toolbox
    long nListboxYPos = maToolbox->GetPosPixel().Y() + maToolbox->GetSizePixel().Height() + 4;
    maTlbObjects->setPosSizePixel( 0, nListboxYPos, 0, 0, PosSizeFlags::Y );
    maTlbObjects->SetDoubleClickHdl( LINK( this, SdNavigatorWin, ClickObjectHdl ) );
    maTlbObjects->SetSelectionMode( SINGLE_SELECTION );
    // set focus to listbox, otherwise it is in the toolbox which is only useful
    // for keyboard navigation
    maTlbObjects->GrabFocus();
    maTlbObjects->SetSdNavigatorWinFlag(true);

    // DragTypeListBox
    maLbDocs->SetSelectHdl( LINK( this, SdNavigatorWin, SelectDocumentHdl ) );
    // set position below treelistbox
    nListboxYPos = maTlbObjects->GetPosPixel().Y() + maTlbObjects->GetSizePixel().Height() + 4;
    maLbDocs->setPosSizePixel( 0, nListboxYPos, 0, 0, PosSizeFlags::Y );

    // assure that tool box is at least as wide as the tree list box
    {
        const Size aTlbSize( maTlbObjects->GetOutputSizePixel() );
        if ( aTlbSize.Width() > aTbxSize.Width() )
        {
            maToolbox->setPosSizePixel( 0, 0, aTlbSize.Width(), 0, PosSizeFlags::Width );
            aTbxSize = maToolbox->GetOutputSizePixel();
        }
    }

    // set min outputsize after all sizes are known
    const long nFullHeight = nListboxYPos + maLbDocs->GetSizePixel().Height() + 4;
    maSize = GetOutputSizePixel();
    if( maSize.Height() < nFullHeight )
    {
        maSize.Height() = nFullHeight;
        SetOutputSizePixel( maSize );
    }
    maMinSize = maSize;
    const long nMinWidth = 2*maToolbox->GetPosPixel().X() + aTbxSize.Width(); // never clip the toolbox
    if( nMinWidth > maMinSize.Width() )
        maMinSize.Width() = nMinWidth;
    maMinSize.Height() -= 40;
    SfxDockingWindow* pDockingParent = dynamic_cast<SfxDockingWindow*>(GetParent());
    if (pDockingParent != nullptr)
        pDockingParent->SetMinOutputSizePixel( maMinSize );

}

void SdNavigatorWin::SetUpdateRequestFunctor(const UpdateRequestFunctor& rUpdateRequest)
{
    mpNavigatorCtrlItem = new SdNavigatorControllerItem( SID_NAVIGATOR_STATE, this, mpBindings, rUpdateRequest);
    mpPageNameCtrlItem = new SdPageNameControllerItem( SID_NAVIGATOR_PAGENAME, this, mpBindings);

    // InitTlb; is initiated over Slot
    if (rUpdateRequest)
        rUpdateRequest();
}

SdNavigatorWin::~SdNavigatorWin()
{
    disposeOnce();
}

void SdNavigatorWin::dispose()
{
    DELETEZ( mpNavigatorCtrlItem );
    DELETEZ( mpPageNameCtrlItem );
    maToolbox.disposeAndClear();
    maTlbObjects.disposeAndClear();
    maLbDocs.disposeAndClear();
    vcl::Window::dispose();
}

//when object is marked , fresh the corresponding entry tree .
void SdNavigatorWin::FreshTree( const SdDrawDocument* pDoc )
{
    SdDrawDocument* pNonConstDoc = const_cast<SdDrawDocument*>(pDoc); // const as const can...
    sd::DrawDocShell* pDocShell = pNonConstDoc->GetDocSh();
    OUString aDocShName( pDocShell->GetName() );
    OUString aDocName = pDocShell->GetMedium()->GetName();
    maTlbObjects->SetSaveTreeItemStateFlag(true); //Added by yanjun for sym2_6385
    maTlbObjects->Clear();
    maTlbObjects->Fill( pDoc, false, aDocName ); // Nur normale Seiten
    maTlbObjects->SetSaveTreeItemStateFlag(false); //Added by yanjun for sym2_6385
    RefreshDocumentLB();
    maLbDocs->SelectEntry( aDocShName );
}

void SdNavigatorWin::FreshEntry( )
{
    maTlbObjects->FreshCurEntry();
}

void SdNavigatorWin::InitTreeLB( const SdDrawDocument* pDoc )
{
    SdDrawDocument* pNonConstDoc = const_cast<SdDrawDocument*>(pDoc); // const as const can...
    ::sd::DrawDocShell* pDocShell = pNonConstDoc->GetDocSh();
    OUString aDocShName( pDocShell->GetName() );
    ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();

    // Restore the 'ShowAllShapes' flag from the last time (in this session)
    // that the navigator was shown.
    if (pViewShell != nullptr)
    {
        ::sd::FrameView* pFrameView = pViewShell->GetFrameView();
        if (pFrameView != nullptr)
            maTlbObjects->SetShowAllShapes(pFrameView->IsNavigatorShowingAllShapes(), false);
    }

    // Disable the shape filter drop down menu when there is a running slide
    // show.
    if (pViewShell!=nullptr && sd::SlideShow::IsRunning( pViewShell->GetViewShellBase() ))
        maToolbox->EnableItem(TBI_SHAPE_FILTER, false);
    else
        maToolbox->EnableItem(TBI_SHAPE_FILTER);

    if( !maTlbObjects->IsEqualToDoc( pDoc ) )
    {
        OUString aDocName = pDocShell->GetMedium()->GetName();
        maTlbObjects->Clear();
        maTlbObjects->Fill( pDoc, false, aDocName ); // only normal pages

        RefreshDocumentLB();
        maLbDocs->SelectEntry( aDocShName );
    }
    else
    {
        maLbDocs->SetNoSelection();
        maLbDocs->SelectEntry( aDocShName );

// commented in order to fix 30246
//        if( maLbDocs->GetSelectEntryCount() == 0 )
        {
            RefreshDocumentLB();
            maLbDocs->SelectEntry( aDocShName );
        }
    }

    SfxViewFrame* pViewFrame = ( ( pViewShell && pViewShell->GetViewFrame() ) ? pViewShell->GetViewFrame() : SfxViewFrame::Current() );
    if( pViewFrame )
        pViewFrame->GetBindings().Invalidate(SID_NAVIGATOR_PAGENAME, true, true);
}

/**
 * DragType is set on dependence if a Drag is even possible. For example,
 * under certain circumstances, it is not allowed to drag graphics (#31038#).
 */
NavigatorDragType SdNavigatorWin::GetNavigatorDragType()
{
    NavigatorDragType   eDT = meDragType;
    NavDocInfo*         pInfo = GetDocInfo();

    if( ( eDT == NAVIGATOR_DRAGTYPE_LINK ) && ( ( pInfo && !pInfo->HasName() ) || !maTlbObjects->IsLinkableSelected() ) )
        eDT = NAVIGATOR_DRAGTYPE_NONE;

    return eDT;
}

//Get SdDrawDocShell
sd::DrawDocShell* SdNavigatorWin::GetDrawDocShell( const SdDrawDocument* pDoc )
{
    if( !pDoc )
        return nullptr; // const as const can...
    sd::DrawDocShell* pDocShell = pDoc->GetDocSh();
    return pDocShell;
}

IMPL_LINK_NOARG_TYPED(SdNavigatorWin, SelectToolboxHdl, ToolBox *, void)
{
    sal_uInt16 nId = maToolbox->GetCurItemId();
    PageJump ePage = PAGE_NONE;

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
        mpBindings->GetDispatcher()->ExecuteList(SID_NAVIGATOR_PAGE,
                SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
    }
}

IMPL_LINK_TYPED( SdNavigatorWin, DropdownClickToolBoxHdl, ToolBox*, pBox, void )
{
    sal_uInt16 nId = maToolbox->GetCurItemId();

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
                 nullptr
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

            if( ( pInfo && !pInfo->HasName() ) || !maTlbObjects->IsLinkableSelected() )
            {
                pMenu->EnableItem( NAVIGATOR_DRAGTYPE_LINK, false );
                pMenu->EnableItem( NAVIGATOR_DRAGTYPE_URL, false );
                meDragType = NAVIGATOR_DRAGTYPE_EMBEDDED;
            }

            pMenu->CheckItem( (sal_uInt16)meDragType );
            pMenu->SetSelectHdl( LINK( this, SdNavigatorWin, MenuSelectHdl ) );

            pMenu->Execute( this, maToolbox->GetItemRect( nId ), PopupMenuFlags::ExecuteDown );
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

            if (maTlbObjects->GetShowAllShapes())
                pMenu->CheckItem(nShowAllShapesFilter);
            else
                pMenu->CheckItem(nShowNamedShapesFilter);
            pMenu->SetSelectHdl( LINK( this, SdNavigatorWin, ShapeFilterCallback ) );

            pMenu->Execute( this, maToolbox->GetItemRect( nId ), PopupMenuFlags::ExecuteDown );
            pBox->EndSelection();
            delete pMenu;
        }
        break;
    }
}

IMPL_LINK_NOARG_TYPED(SdNavigatorWin, ClickObjectHdl, SvTreeListBox*, bool)
{
    if( !mbDocImported || maLbDocs->GetSelectEntryPos() != 0 )
    {
        NavDocInfo* pInfo = GetDocInfo();

        // if it is the active window, we jump to the page
        if( pInfo && pInfo->IsActive() )
        {
            OUString aStr( maTlbObjects->GetSelectEntry() );

            if( !aStr.isEmpty() )
            {
                SfxStringItem aItem( SID_NAVIGATOR_OBJECT, aStr );
                mpBindings->GetDispatcher()->ExecuteList(
                    SID_NAVIGATOR_OBJECT,
                    SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
                //set sign variable
                maTlbObjects->MarkCurEntry(aStr);

                // moved here from SetGetFocusHdl. Reset the
                // focus only if something has been selected in the
                // document.
                SfxViewShell* pCurSh = SfxViewShell::Current();

                if ( pCurSh )
                {
                    vcl::Window* pShellWnd = pCurSh->GetWindow();
                    if ( pShellWnd )
                        pShellWnd->GrabFocus();
                }
            }
        }
    }
    return false;
}

IMPL_LINK_NOARG_TYPED(SdNavigatorWin, SelectDocumentHdl, ListBox&, void)
{
    OUString aStrLb = maLbDocs->GetSelectEntry();
    long   nPos = maLbDocs->GetSelectEntryPos();
    bool   bFound = false;
    ::sd::DrawDocShell* pDocShell = nullptr;
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

        bFound = true;
    }

    if( bFound )
    {
        SdDrawDocument* pDoc = pDocShell->GetDoc();
        if( !maTlbObjects->IsEqualToDoc( pDoc ) )
        {
            SdDrawDocument* pNonConstDoc = pDoc; // const as const can...
            ::sd::DrawDocShell* pNCDocShell = pNonConstDoc->GetDocSh();
            OUString aDocName = pNCDocShell->GetMedium()->GetName();
            maTlbObjects->Clear();
            maTlbObjects->Fill( pDoc, false, aDocName ); // only normal pages
        }
    }

    // check if link or url is possible
    if( ( pInfo && !pInfo->HasName() ) || !maTlbObjects->IsLinkableSelected() || ( meDragType != NAVIGATOR_DRAGTYPE_EMBEDDED ) )
    {
        meDragType = NAVIGATOR_DRAGTYPE_EMBEDDED;
        SetDragImage();
    }
}

/**
 * Set DrageType and set image accordingly to it.
 * If the handler is called with NULL, the default (URL) is set.
 */
IMPL_LINK_TYPED( SdNavigatorWin, MenuSelectHdl, Menu *, pMenu, bool )
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
                if( maTlbObjects->GetSelectionCount() > 1 )
                    maTlbObjects->SelectAll( false );

                maTlbObjects->SetSelectionMode( SINGLE_SELECTION );
            }
            else
                maTlbObjects->SetSelectionMode( MULTIPLE_SELECTION );
        }
    }
    return false;
}

IMPL_LINK_TYPED( SdNavigatorWin, ShapeFilterCallback, Menu *, pMenu, bool )
{
    if (pMenu != nullptr)
    {
        bool bShowAllShapes (maTlbObjects->GetShowAllShapes());
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

        maTlbObjects->SetShowAllShapes(bShowAllShapes, true);

        // Remember the selection in the FrameView.
        NavDocInfo* pInfo = GetDocInfo();
        if (pInfo != nullptr)
        {
            ::sd::DrawDocShell* pDocShell = pInfo->mpDocShell;
            if (pDocShell != nullptr)
            {
                ::sd::ViewShell* pViewShell = pDocShell->GetViewShell();
                if (pViewShell != nullptr)
                {
                    ::sd::FrameView* pFrameView = pViewShell->GetFrameView();
                    if (pFrameView != nullptr)
                    {
                        pFrameView->SetIsNavigatorShowingAllShapes(bShowAllShapes);
                    }
                }
            }
        }
    }

    return false;
}

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
        Size aObjSize( maToolbox->GetOutputSizePixel() );
        aObjSize.Width() += aDiffSize.Width();
        maToolbox->SetOutputSizePixel( aObjSize );

        // change size of TreeLB
        aObjSize = maTlbObjects->GetSizePixel();
        aObjSize.Width() += aDiffSize.Width();
        aObjSize.Height() = maLbDocs->GetPosPixel().Y() + aDiffSize.Height() -
            maTlbObjects->GetPosPixel().Y() - 4;
        maTlbObjects->SetSizePixel( aObjSize );

        Point aPt( 0, aDiffSize.Height() );

        // move other controls (DocumentLB)
        maLbDocs->Hide();
        aObjSize = maLbDocs->GetOutputSizePixel();
        aObjSize.Width() += aDiffSize.Width();
        maLbDocs->SetPosPixel( maLbDocs->GetPosPixel() + aPt );
        maLbDocs->SetOutputSizePixel( aObjSize );
        maLbDocs->Show();

        maSize = aWinSize;
    }
    Window::Resize();
}

bool SdNavigatorWin::InsertFile(const OUString& rFileName)
{
    INetURLObject   aURL( rFileName );

    if( aURL.GetProtocol() == INetProtocol::NotValid )
    {
        OUString aURLStr;
        osl::FileBase::getFileURLFromSystemPath( rFileName, aURLStr );
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
        std::shared_ptr<const SfxFilter> pFilter;
        ErrCode nErr = 0;

        if (aFileName != maDropFileName)
        {
            SfxMedium aMed(aFileName, (StreamMode::READ | StreamMode::SHARE_DENYNONE));
            SfxFilterMatcher aMatch( OUString("simpress") );
            aMed.UseInteractionHandler( true );
            nErr = aMatch.GuessFilter(aMed, pFilter);
        }

        if ((pFilter && !nErr) || aFileName == maDropFileName)
        {
            // The medium may be opened with READ/WRITE. Therefore, we first
            // check if it contains a Storage.
            std::unique_ptr<SfxMedium> xMedium(new SfxMedium(aFileName,
                                                StreamMode::READ | StreamMode::NOCREATE));

            if (xMedium->IsStorage())
            {
                // Now depending on mode:
                // maTlbObjects->SetSelectionMode(MULTIPLE_SELECTION);
                // handover of ownership of xMedium;
                SdDrawDocument* pDropDoc = maTlbObjects->GetBookmarkDoc(xMedium.release());

                if (pDropDoc)
                {
                    maTlbObjects->Clear();
                    maDropFileName = aFileName;

                    if( !maTlbObjects->IsEqualToDoc( pDropDoc ) )
                    {
                        // only normal pages
                        maTlbObjects->Fill(pDropDoc, false, maDropFileName);
                        RefreshDocumentLB( &maDropFileName );
                    }
                }
            }
            else
            {
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

void SdNavigatorWin::RefreshDocumentLB( const OUString* pDocName )
{
    sal_Int32 nPos = 0;

    if( pDocName )
    {
        if( mbDocImported )
            maLbDocs->RemoveEntry( 0 );

        maLbDocs->InsertEntry( *pDocName, 0 );
        mbDocImported = true;
    }
    else
    {
        nPos = maLbDocs->GetSelectEntryPos();
        if( nPos == LISTBOX_ENTRY_NOTFOUND )
            nPos = 0;

        OUString aStr;
        if( mbDocImported )
            aStr = maLbDocs->GetEntry( 0 );

        maLbDocs->Clear();

        // delete list of DocInfos
         maDocList.clear();

        if( mbDocImported )
            maLbDocs->InsertEntry( aStr, 0 );

        ::sd::DrawDocShell* pCurrentDocShell =
              dynamic_cast< ::sd::DrawDocShell *>( SfxObjectShell::Current() );
        SfxObjectShell* pSfxDocShell = SfxObjectShell::GetFirst([](const SfxObjectShell*){return true;}, false);
        while( pSfxDocShell )
        {
            ::sd::DrawDocShell* pDocShell = dynamic_cast< ::sd::DrawDocShell *>( pSfxDocShell );
            if( pDocShell  && !pDocShell->IsInDestruction() && ( pDocShell->GetCreateMode() != SfxObjectCreateMode::EMBEDDED ) )
            {
                NavDocInfo aInfo ;
                aInfo.mpDocShell = pDocShell;

                SfxMedium *pMedium = pDocShell->GetMedium();
                aStr = pMedium ? pMedium->GetName() : OUString();
                if( !aStr.isEmpty() )
                    aInfo.SetName();
                else
                    aInfo.SetName( false );
                // at the moment, we use the name of the shell again (i.e.
                // without path) since Koose thinks it is an error if the path
                // is shown in url notation!
                aStr = pDocShell->GetName();

                maLbDocs->InsertEntry( aStr );

                if( pDocShell == pCurrentDocShell )
                    aInfo.SetActive();
                else
                    aInfo.SetActive( false );

                maDocList.push_back( aInfo );
            }
            pSfxDocShell = SfxObjectShell::GetNext( *pSfxDocShell, [](const SfxObjectShell*){return true;}, false );
        }
    }
    maLbDocs->SelectEntryPos( nPos );
}

sal_uInt16 SdNavigatorWin::GetDragTypeSdResId( NavigatorDragType eDT, bool bImage )
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
    return 0;
}

NavDocInfo* SdNavigatorWin::GetDocInfo()
{
    sal_uInt32 nPos = maLbDocs->GetSelectEntryPos();

    if( mbDocImported )
    {
        if( nPos == 0 )
        {
            return nullptr;
        }
        nPos--;
    }

    return nPos < maDocList.size() ? &(maDocList[ nPos ]) : nullptr;
}

/**
 * PreNotify
 */
bool SdNavigatorWin::Notify(NotifyEvent& rNEvt)
{
    const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
    bool            bOK = false;

    if( pKEvt )
    {
        if( KEY_ESCAPE == pKEvt->GetKeyCode().GetCode() )
        {
            if( SdPageObjsTLB::IsInDrag() )
            {
                // during drag'n'drop we just stop the drag but do not close the navigator
                bOK = true;
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
                    return true;
                }
            }
        }
    }

    if( !bOK )
        bOK = Window::Notify( rNEvt );

    return bOK;
}

/**
 * catch ESCAPE in order to end show
 */
void SdNavigatorWin::KeyInput( const KeyEvent& rKEvt )
{
    bool bOK = false;

    if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
    {
        if( SdPageObjsTLB::IsInDrag() )
        {
            // during drag'n'drop we just stop the drag but do not close the navigator
            bOK = true;
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

    if (!bOK)
    {
        Window::KeyInput(rKEvt);
    }
}

void SdNavigatorWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
            ApplyImageList();

    Window::DataChanged( rDCEvt );
}

void SdNavigatorWin::SetDragImage()
{
    maToolbox->SetItemImage( TBI_DRAGTYPE, maToolbox->GetImageList().GetImage( GetDragTypeSdResId( meDragType, true ) ) );
}

void SdNavigatorWin::ApplyImageList()
{

    maToolbox->SetImageList( maImageList );
    maToolbox->SetItemImage(TBI_SHAPE_FILTER, Image(BitmapEx(SdResId(BMP_GRAPHIC))));

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

void SdNavigatorControllerItem::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    if( eState >= SfxItemState::DEFAULT && nSId == SID_NAVIGATOR_STATE )
    {
        const SfxUInt32Item& rStateItem = dynamic_cast<const SfxUInt32Item&>(*pItem);
        sal_uInt32 nState = rStateItem.GetValue();

        // only if doc in LB is the active
        NavDocInfo* pInfo = pNavigatorWin->GetDocInfo();
        if( pInfo && pInfo->IsActive() )
        {
            // First
            if( nState & NAVBTN_FIRST_ENABLED &&
                !pNavigatorWin->maToolbox->IsItemEnabled( TBI_FIRST ) )
                pNavigatorWin->maToolbox->EnableItem( TBI_FIRST );
            if( nState & NAVBTN_FIRST_DISABLED &&
                pNavigatorWin->maToolbox->IsItemEnabled( TBI_FIRST ) )
                pNavigatorWin->maToolbox->EnableItem( TBI_FIRST, false );

            // Prev
            if( nState & NAVBTN_PREV_ENABLED &&
                !pNavigatorWin->maToolbox->IsItemEnabled( TBI_PREVIOUS ) )
                pNavigatorWin->maToolbox->EnableItem( TBI_PREVIOUS );
            if( nState & NAVBTN_PREV_DISABLED &&
                pNavigatorWin->maToolbox->IsItemEnabled( TBI_PREVIOUS ) )
                pNavigatorWin->maToolbox->EnableItem( TBI_PREVIOUS, false );

            // Last
            if( nState & NAVBTN_LAST_ENABLED &&
                !pNavigatorWin->maToolbox->IsItemEnabled( TBI_LAST ) )
                pNavigatorWin->maToolbox->EnableItem( TBI_LAST );
            if( nState & NAVBTN_LAST_DISABLED &&
                pNavigatorWin->maToolbox->IsItemEnabled( TBI_LAST ) )
                pNavigatorWin->maToolbox->EnableItem( TBI_LAST, false );

            // Next
            if( nState & NAVBTN_NEXT_ENABLED &&
                !pNavigatorWin->maToolbox->IsItemEnabled( TBI_NEXT ) )
                pNavigatorWin->maToolbox->EnableItem( TBI_NEXT );
            if( nState & NAVBTN_NEXT_DISABLED &&
                pNavigatorWin->maToolbox->IsItemEnabled( TBI_NEXT ) )
                pNavigatorWin->maToolbox->EnableItem( TBI_NEXT, false );

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
    SfxBindings*    _pBindings)
    : SfxControllerItem( _nId, *_pBindings ),
      pNavigatorWin( pNavWin )
{
}

void SdPageNameControllerItem::StateChanged( sal_uInt16 nSId,
                        SfxItemState eState, const SfxPoolItem* pItem )
{
    if( eState >= SfxItemState::DEFAULT && nSId == SID_NAVIGATOR_PAGENAME )
    {
        // only if doc in LB is the active
        NavDocInfo* pInfo = pNavigatorWin->GetDocInfo();
        if( pInfo && pInfo->IsActive() )
        {
            const SfxStringItem& rStateItem = dynamic_cast<const SfxStringItem&>(*pItem);
            const OUString& aPageName = rStateItem.GetValue();

            if( !pNavigatorWin->maTlbObjects->HasSelectedChildren( aPageName ) )
            {
                if( pNavigatorWin->maTlbObjects->GetSelectionMode() == MULTIPLE_SELECTION )
                {
                    // because otherwise it is always additional select
                    pNavigatorWin->maTlbObjects->SelectAll( false );
                }
                pNavigatorWin->maTlbObjects->SelectEntry( aPageName );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
