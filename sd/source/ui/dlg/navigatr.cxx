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

#include <memory>
#include <sal/config.h>

#include <osl/file.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/fcontnr.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>

#include <vcl/menu.hxx>

#include <sfx2/viewfrm.hxx>

#include <pres.hxx>
#include <navigatr.hxx>
#include <pgjump.hxx>
#include <app.hrc>
#include <strings.hrc>

#include <bitmaps.hlst>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <sdresid.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <slideshow.hxx>
#include <FrameView.hxx>
#include <helpids.h>
#include <Window.hxx>

/**
 * SdNavigatorWin - FloatingWindow
 */
SdNavigatorWin::SdNavigatorWin(vcl::Window* pParent, SfxBindings* pInBindings)
    : PanelLayout(pParent, "NavigatorPanel", "modules/simpress/ui/navigatorpanel.ui", nullptr)
    , mxToolbox(m_xBuilder->weld_toolbar("toolbox"))
    , mxTlbObjects(new SdPageObjsTLV(m_xBuilder->weld_tree_view("tree")))
    , mxLbDocs(m_xBuilder->weld_combo_box("documents"))
    , mxDragModeMenu(m_xBuilder->weld_menu("dragmodemenu"))
    , mxShapeMenu(m_xBuilder->weld_menu("shapemenu"))
    , mbDocImported ( false )
      // On changes of the DragType: adjust SelectionMode of TLB!
    , meDragType ( NAVIGATOR_DRAGTYPE_EMBEDDED )
    , mpBindings ( pInBindings )
{
    Size aSize(LogicToPixel(Size(97, 67), MapMode(MapUnit::MapAppFont)));
    mxTlbObjects->set_size_request(aSize.Width(), aSize.Height());
    mxTlbObjects->SetViewFrame( mpBindings->GetDispatcher()->GetFrame() );

    mxTlbObjects->connect_row_activated(LINK(this, SdNavigatorWin, ClickObjectHdl));
    mxTlbObjects->set_selection_mode(SelectionMode::Single);

    mxToolbox->connect_clicked(LINK(this, SdNavigatorWin, SelectToolboxHdl));
    mxToolbox->connect_menu_toggled(LINK(this, SdNavigatorWin, DropdownClickToolBoxHdl));

    mxToolbox->set_item_menu("dragmode", mxDragModeMenu.get());
    mxDragModeMenu->connect_activate(LINK(this, SdNavigatorWin, MenuSelectHdl));

    // Shape filter drop down menu.
    mxToolbox->set_item_menu("shapes", mxShapeMenu.get());
    mxShapeMenu->connect_activate(LINK(this, SdNavigatorWin, ShapeFilterCallback));

    // set focus to listbox, otherwise it is in the toolbox which is only useful
    // for keyboard navigation
    mxTlbObjects->grab_focus();
    mxTlbObjects->SetSdNavigator(this);

    // DragTypeListBox
    mxLbDocs->set_size_request(42, -1); // set a nominal width so it takes width of surroundings
    mxLbDocs->connect_changed(LINK(this, SdNavigatorWin, SelectDocumentHdl));

    SetDragImage();

    m_pInitialFocusWidget = mxToolbox.get();
}

void SdNavigatorWin::SetUpdateRequestFunctor(const UpdateRequestFunctor& rUpdateRequest)
{
    mpNavigatorCtrlItem.reset( new SdNavigatorControllerItem(SID_NAVIGATOR_STATE, this, mpBindings, rUpdateRequest) );
    mpPageNameCtrlItem.reset( new SdPageNameControllerItem(SID_NAVIGATOR_PAGENAME, this, mpBindings) );

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
    mpNavigatorCtrlItem.reset();
    mpPageNameCtrlItem.reset();
    mxDragModeMenu.reset();
    mxShapeMenu.reset();
    mxToolbox.reset();
    mxTlbObjects.reset();
    mxLbDocs.reset();
    PanelLayout::dispose();
}

//when object is marked , fresh the corresponding entry tree .
void SdNavigatorWin::FreshTree( const SdDrawDocument* pDoc )
{
    SdDrawDocument* pNonConstDoc = const_cast<SdDrawDocument*>(pDoc); // const as const can...
    sd::DrawDocShell* pDocShell = pNonConstDoc->GetDocSh();
    const OUString& aDocShName( pDocShell->GetName() );
    OUString aDocName = pDocShell->GetMedium()->GetName();
    mxTlbObjects->Fill( pDoc, false, aDocName ); // Only normal pages
    RefreshDocumentLB();
    mxLbDocs->set_active_text(aDocShName);
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
            mxTlbObjects->SetShowAllShapes(pFrameView->IsNavigatorShowingAllShapes(), false);
    }

    // Disable the shape filter drop down menu when there is a running slide
    // show.
    if (pViewShell!=nullptr && sd::SlideShow::IsRunning( pViewShell->GetViewShellBase() ))
        mxToolbox->set_item_sensitive("shapes", false);
    else
        mxToolbox->set_item_sensitive("shapes", true);

    if( !mxTlbObjects->IsEqualToDoc( pDoc ) )
    {
        OUString aDocName = pDocShell->GetMedium()->GetName();
        mxTlbObjects->clear();
        mxTlbObjects->Fill( pDoc, false, aDocName ); // only normal pages

        RefreshDocumentLB();
        mxLbDocs->set_active_text(aDocShName);
    }
    else
    {
        mxLbDocs->set_active(-1);
        mxLbDocs->set_active_text(aDocShName);

// commented in order to fix 30246
//        if( mxLbDocs->get_active() == -1 )
        {
            RefreshDocumentLB();
            mxLbDocs->set_active_text(aDocShName);
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

    if( ( eDT == NAVIGATOR_DRAGTYPE_LINK ) && ( ( pInfo && !pInfo->HasName() ) || !mxTlbObjects->IsLinkableSelected() ) )
        eDT = NAVIGATOR_DRAGTYPE_NONE;

    return eDT;
}

SdPageObjsTLV& SdNavigatorWin::GetObjects()
{
    return *mxTlbObjects;
}

IMPL_LINK(SdNavigatorWin, SelectToolboxHdl, const OString&, rCommand, void)
{
    PageJump ePage = PAGE_NONE;

    if (rCommand == "first")
        ePage = PAGE_FIRST;
    else if (rCommand == "previous")
        ePage = PAGE_PREVIOUS;
    else if (rCommand == "next")
        ePage = PAGE_NEXT;
    else if (rCommand == "last")
        ePage = PAGE_LAST;
    else if (rCommand == "dragmode")
        mxToolbox->set_menu_item_active("dragmode", !mxToolbox->get_menu_item_active("dragmode"));
    else if (rCommand == "shapes")
        mxToolbox->set_menu_item_active("shapes", !mxToolbox->get_menu_item_active("shapes"));

    if (ePage != PAGE_NONE)
    {
        SfxUInt16Item aItem( SID_NAVIGATOR_PAGE, static_cast<sal_uInt16>(ePage) );
        mpBindings->GetDispatcher()->ExecuteList(SID_NAVIGATOR_PAGE,
                SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });
    }
}

IMPL_LINK(SdNavigatorWin, DropdownClickToolBoxHdl, const OString&, rCommand, void)
{
    if (!mxToolbox->get_menu_item_active(rCommand))
        return;

    if (rCommand == "dragmode")
    {
        NavDocInfo* pInfo = GetDocInfo();
        if( ( pInfo && !pInfo->HasName() ) || !mxTlbObjects->IsLinkableSelected() )
        {
            mxDragModeMenu->set_sensitive(OString::number(NAVIGATOR_DRAGTYPE_LINK), false);
            mxDragModeMenu->set_sensitive(OString::number(NAVIGATOR_DRAGTYPE_URL), false);
            meDragType = NAVIGATOR_DRAGTYPE_EMBEDDED;
        }

        mxDragModeMenu->set_active(OString::number(meDragType), true);
    }
    else if (rCommand == "shapes")
    {
        bool bAll = mxTlbObjects->GetShowAllShapes();
        mxShapeMenu->set_active("named", !bAll);
        mxShapeMenu->set_active("all", bAll);
    }
}

IMPL_LINK_NOARG(SdNavigatorWin, ClickObjectHdl, weld::TreeView&, bool)
{
    if( !mbDocImported || mxLbDocs->get_active() != 0 )
    {
        NavDocInfo* pInfo = GetDocInfo();

        // if it is the active window, we jump to the page
        if( pInfo && pInfo->IsActive() )
        {
            OUString aStr(mxTlbObjects->get_selected_text());

            if( !aStr.isEmpty() )
            {
                SfxStringItem aItem( SID_NAVIGATOR_OBJECT, aStr );
                mpBindings->GetDispatcher()->ExecuteList(
                    SID_NAVIGATOR_OBJECT,
                    SfxCallMode::SLOT | SfxCallMode::RECORD, { &aItem });

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

                // We navigated to an object, but the current shell may be
                // still the slide sorter. Explicitly try to grab the draw
                // shell focus, so follow-up operations work with the object
                // and not with the whole slide.
                sd::DrawDocShell* pDocShell = pInfo->mpDocShell;
                if (pDocShell)
                {
                    sd::ViewShell* pViewShell = pDocShell->GetViewShell();
                    if (pViewShell)
                    {
                        vcl::Window* pWindow = pViewShell->GetActiveWindow();
                        if (pWindow)
                            pWindow->GrabFocus();
                    }
                }

                if (!mxTlbObjects->IsNavigationGrabsFocus())
                    // This is the case when keyboard navigation inside the
                    // navigator should continue to work.
                    mxTlbObjects->grab_focus();
            }
        }
    }
    return false;
}

IMPL_LINK_NOARG(SdNavigatorWin, SelectDocumentHdl, weld::ComboBox&, void)
{
    OUString aStrLb = mxLbDocs->get_active_text();
    tools::Long   nPos = mxLbDocs->get_active();
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
        if( !mxTlbObjects->IsEqualToDoc( pDoc ) )
        {
            SdDrawDocument* pNonConstDoc = pDoc; // const as const can...
            ::sd::DrawDocShell* pNCDocShell = pNonConstDoc->GetDocSh();
            OUString aDocName = pNCDocShell->GetMedium()->GetName();
            mxTlbObjects->clear();
            mxTlbObjects->Fill( pDoc, false, aDocName ); // only normal pages
        }
    }

    // check if link or url is possible
    if( ( pInfo && !pInfo->HasName() ) || !mxTlbObjects->IsLinkableSelected() || ( meDragType != NAVIGATOR_DRAGTYPE_EMBEDDED ) )
    {
        meDragType = NAVIGATOR_DRAGTYPE_EMBEDDED;
        SetDragImage();
    }
}

/**
 * Set DrageType and set image accordingly to it.
 */
IMPL_LINK(SdNavigatorWin, MenuSelectHdl, const OString&, rIdent, void)
{
    sal_uInt32 nMenuId = rIdent.toUInt32();

    NavigatorDragType eDT = static_cast<NavigatorDragType>(nMenuId);
    if( meDragType == eDT )
        return;

    meDragType = eDT;
    SetDragImage();

    if( meDragType == NAVIGATOR_DRAGTYPE_URL )
    {
        // patch, prevents endless loop
        if (mxTlbObjects->count_selected_rows() > 1)
            mxTlbObjects->unselect_all();

        mxTlbObjects->set_selection_mode(SelectionMode::Single);
    }
    else
        mxTlbObjects->set_selection_mode(SelectionMode::Multiple);
}

IMPL_LINK( SdNavigatorWin, ShapeFilterCallback, const OString&, rIdent, void )
{
    bool bShowAllShapes(mxTlbObjects->GetShowAllShapes());
    if (rIdent == "named")
        bShowAllShapes = false;
    else if (rIdent == "all")
        bShowAllShapes = true;
    else
        OSL_FAIL("SdNavigatorWin::ShapeFilterCallback called for unknown menu entry");

    mxTlbObjects->SetShowAllShapes(bShowAllShapes, true);

    // Remember the selection in the FrameView.
    NavDocInfo* pInfo = GetDocInfo();
    if (pInfo == nullptr)
        return;

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
    OUString aFileName( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    if (aFileName.isEmpty())
    {
        // show actual document again
        maDropFileName = aFileName;
    }
    else
    {
        // show dragged-in document
        std::shared_ptr<const SfxFilter> pFilter;
        ErrCode nErr = ERRCODE_NONE;

        if (aFileName != maDropFileName)
        {
            SfxMedium aMed(aFileName, (StreamMode::READ | StreamMode::SHARE_DENYNONE));
            SfxFilterMatcher aMatch( "simpress" );
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
                // mxTlbObjects->set_selection_mode(SelectionMode::Multiple);
                // handover of ownership of xMedium;
                SdDrawDocument* pDropDoc = mxTlbObjects->GetBookmarkDoc(xMedium.release());

                if (pDropDoc)
                {
                    mxTlbObjects->clear();
                    maDropFileName = aFileName;

                    if( !mxTlbObjects->IsEqualToDoc( pDropDoc ) )
                    {
                        // only normal pages
                        mxTlbObjects->Fill(pDropDoc, false, maDropFileName);
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
            mxLbDocs->remove(0);

        mxLbDocs->insert_text(0, *pDocName);
        mbDocImported = true;
    }
    else
    {
        nPos = mxLbDocs->get_active();
        if (nPos == -1)
            nPos = 0;

        OUString aStr;
        if( mbDocImported )
            aStr = mxLbDocs->get_text(0);

        mxLbDocs->clear();

        // delete list of DocInfos
        maDocList.clear();

        if( mbDocImported )
            mxLbDocs->insert_text(0, aStr);

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
                    aInfo.SetName( true );
                else
                    aInfo.SetName( false );
                // at the moment, we use the name of the shell again (i.e.
                // without path) since Koose thinks it is an error if the path
                // is shown in url notation!
                aStr = pDocShell->GetName();

                mxLbDocs->append_text(aStr);

                if( pDocShell == pCurrentDocShell )
                    aInfo.SetActive( true );
                else
                    aInfo.SetActive( false );

                maDocList.push_back( aInfo );
            }
            pSfxDocShell = SfxObjectShell::GetNext( *pSfxDocShell, [](const SfxObjectShell*){return true;}, false );
        }
    }
    mxLbDocs->set_active(nPos);
}

OUString SdNavigatorWin::GetDragTypeSdBmpId(NavigatorDragType eDT)
{
    switch( eDT )
    {
        case NAVIGATOR_DRAGTYPE_NONE:
                return OUString();
        case NAVIGATOR_DRAGTYPE_URL:
                return BMP_HYPERLINK;
        case NAVIGATOR_DRAGTYPE_EMBEDDED:
                return BMP_EMBEDDED;
        case NAVIGATOR_DRAGTYPE_LINK:
                return BMP_LINK;
        default: OSL_FAIL( "No resource for DragType available!" );
    }
    return OUString();
}

NavDocInfo* SdNavigatorWin::GetDocInfo()
{
    sal_uInt32 nPos = mxLbDocs->get_active();

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
bool SdNavigatorWin::EventNotify(NotifyEvent& rNEvt)
{
    const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
    bool            bOK = false;

    if( pKEvt )
    {
        if( KEY_ESCAPE == pKEvt->GetKeyCode().GetCode() )
        {
            if( SdPageObjsTLV::IsInDrag() )
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
                    // parent's EventNotify after this is unsafe.  Therefore we
                    // return now.
                    return true;
                }
            }
        }
    }

    if( !bOK )
        bOK = Window::EventNotify(rNEvt);

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
        if( SdPageObjsTLV::IsInDrag() )
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

void SdNavigatorWin::SetDragImage()
{
    mxToolbox->set_item_icon_name("dragmode", GetDragTypeSdBmpId(meDragType));
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
    if( eState < SfxItemState::DEFAULT || nSId != SID_NAVIGATOR_STATE )
        return;

    const SfxUInt32Item& rStateItem = dynamic_cast<const SfxUInt32Item&>(*pItem);
    NavState nState = static_cast<NavState>(rStateItem.GetValue());

    // only if doc in LB is the active
    NavDocInfo* pInfo = pNavigatorWin->GetDocInfo();
    if( !(pInfo && pInfo->IsActive()) )
        return;

    // First
    if (nState & NavState::BtnFirstEnabled &&
        !pNavigatorWin->mxToolbox->get_item_sensitive("first"))
        pNavigatorWin->mxToolbox->set_item_sensitive("first", true);
    if (nState & NavState::BtnFirstDisabled &&
        pNavigatorWin->mxToolbox->get_item_sensitive("first"))
        pNavigatorWin->mxToolbox->set_item_sensitive("first", false);

    // Prev
    if (nState & NavState::BtnPrevEnabled &&
        !pNavigatorWin->mxToolbox->get_item_sensitive("previous"))
        pNavigatorWin->mxToolbox->set_item_sensitive("previous", true);
    if (nState & NavState::BtnPrevDisabled &&
        pNavigatorWin->mxToolbox->get_item_sensitive("previous"))
        pNavigatorWin->mxToolbox->set_item_sensitive("previous", false);

    // Last
    if (nState & NavState::BtnLastEnabled &&
        !pNavigatorWin->mxToolbox->get_item_sensitive("last"))
        pNavigatorWin->mxToolbox->set_item_sensitive("last", true);
    if (nState & NavState::BtnLastDisabled &&
        pNavigatorWin->mxToolbox->get_item_sensitive("last"))
        pNavigatorWin->mxToolbox->set_item_sensitive("last", false);

    // Next
    if (nState & NavState::BtnNextEnabled &&
        !pNavigatorWin->mxToolbox->get_item_sensitive("next"))
        pNavigatorWin->mxToolbox->set_item_sensitive("next", true);
    if (nState & NavState::BtnNextDisabled &&
        pNavigatorWin->mxToolbox->get_item_sensitive("next"))
        pNavigatorWin->mxToolbox->set_item_sensitive("next", false);

    if (nState & NavState::TableUpdate)
    {
        // InitTlb; is initiated by Slot
        if (maUpdateRequest)
            maUpdateRequest();
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
    if( eState < SfxItemState::DEFAULT || nSId != SID_NAVIGATOR_PAGENAME )
        return;

    // only if doc in LB is the active
    NavDocInfo* pInfo = pNavigatorWin->GetDocInfo();
    if( !(pInfo && pInfo->IsActive()) )
        return;

    const SfxStringItem& rStateItem = dynamic_cast<const SfxStringItem&>(*pItem);
    const OUString& aPageName = rStateItem.GetValue();

    if( !pNavigatorWin->mxTlbObjects->HasSelectedChildren( aPageName ) )
    {
        if (pNavigatorWin->mxTlbObjects->get_selection_mode() == SelectionMode::Multiple)
        {
            // because otherwise it is always additional select
            pNavigatorWin->mxTlbObjects->unselect_all();
        }
        pNavigatorWin->mxTlbObjects->SelectEntry( aPageName );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
