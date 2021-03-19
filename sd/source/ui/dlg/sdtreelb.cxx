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

#include <sal/types.h>
#include <sal/log.hxx>
#include <sot/formats.hxx>
#include <vcl/weld.hxx>
#include <svx/svditer.hxx>
#include <sfx2/docfile.hxx>
#include <svx/svdoole2.hxx>
#include <vcl/svapp.hxx>
#include <cusshow.hxx>

#include <sfx2/viewfrm.hxx>

#include <sdtreelb.hxx>
#include <DrawDocShell.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdmod.hxx>
#include <sdresid.hxx>
#include <navigatr.hxx>
#include <strings.hrc>

#include <bitmaps.hlst>
#include <customshowlist.hxx>
#include <ViewShell.hxx>
#include <DrawController.hxx>
#include <ViewShellBase.hxx>

#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <svtools/acceleratorexecute.hxx>
#include <svtools/embedtransfer.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/scopeguard.hxx>


using namespace com::sun::star;

bool SdPageObjsTLV::bIsInDrag = false;

bool SdPageObjsTLV::IsInDrag()
{
    return bIsInDrag;
}

SotClipboardFormatId SdPageObjsTLV::SdPageObjsTransferable::mnListBoxDropFormatId = static_cast<SotClipboardFormatId>(SAL_MAX_UINT32);

SdPageObjsTLV::SdPageObjsTransferable::SdPageObjsTransferable(
        const INetBookmark& rBookmark,
    ::sd::DrawDocShell& rDocShell,
    NavigatorDragType eDragType)
    : SdTransferable(rDocShell.GetDoc(), nullptr, true),
      maBookmark( rBookmark ),
      mrDocShell( rDocShell ),
      meDragType( eDragType )
{
}

SdPageObjsTLV::SdPageObjsTransferable::~SdPageObjsTransferable()
{
}

void SdPageObjsTLV::SdPageObjsTransferable::AddSupportedFormats()
{
    AddFormat(SotClipboardFormatId::NETSCAPE_BOOKMARK);
    AddFormat(SotClipboardFormatId::TREELISTBOX);
    AddFormat(GetListBoxDropFormatId());
}

bool SdPageObjsTLV::SdPageObjsTransferable::GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& /*rDestDoc*/ )
{
    SotClipboardFormatId nFormatId = SotExchange::GetFormat( rFlavor );
    switch (nFormatId)
    {
        case SotClipboardFormatId::NETSCAPE_BOOKMARK:
            SetINetBookmark( maBookmark, rFlavor );
            return true;

        case SotClipboardFormatId::TREELISTBOX:
        {
            css::uno::Any aTreeListBoxData; // empty for now
            SetAny(aTreeListBoxData);
            return true;
        }

        default:
            return false;
    }
}

void SdPageObjsTLV::SdPageObjsTransferable::DragFinished( sal_Int8 nDropAction )
{
    SdPageObjsTLV::OnDragFinished();
    SdTransferable::DragFinished(nDropAction);
}

sal_Int64 SAL_CALL SdPageObjsTLV::SdPageObjsTransferable::getSomething( const css::uno::Sequence< sal_Int8 >& rId )
{
    sal_Int64 nRet;

    if (isUnoTunnelId<SdPageObjsTLV::SdPageObjsTransferable>(rId))
    {
        nRet = static_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    else
        nRet = SdTransferable::getSomething(rId);

    return nRet;
}

namespace
{
    class theSdPageObjsTLBUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSdPageObjsTLBUnoTunnelId > {};
}

const css::uno::Sequence<sal_Int8>& SdPageObjsTLV::SdPageObjsTransferable::getUnoTunnelId()
{
    return theSdPageObjsTLBUnoTunnelId::get().getSeq();
}

SdPageObjsTLV::SdPageObjsTransferable* SdPageObjsTLV::SdPageObjsTransferable::getImplementation( const css::uno::Reference< css::uno::XInterface >& rxData )
    throw()
{
    try
    {
        css::uno::Reference< css::lang::XUnoTunnel > xUnoTunnel( rxData, css::uno::UNO_QUERY_THROW );

        return reinterpret_cast<SdPageObjsTLV::SdPageObjsTransferable*>(
                sal::static_int_cast<sal_uIntPtr>(
                    xUnoTunnel->getSomething( SdPageObjsTLV::SdPageObjsTransferable::getUnoTunnelId()) ) );
    }
    catch( const css::uno::Exception& )
    {
    }
    return nullptr;
}

SotClipboardFormatId SdPageObjsTLV::SdPageObjsTransferable::GetListBoxDropFormatId()
{
    if (mnListBoxDropFormatId == static_cast<SotClipboardFormatId>(SAL_MAX_UINT32))
        mnListBoxDropFormatId = SotExchange::RegisterFormatMimeType("application/x-openoffice-treelistbox-moveonly;windows_formatname=\"SV_LBOX_DD_FORMAT_MOVE\"");
    return mnListBoxDropFormatId;
}

/**
 * @return true if children of the specified string are selected
 */
bool SdPageObjsTLV::HasSelectedChildren( const OUString& rName )
{
    bool bChildren = false;

    if( !rName.isEmpty() )
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        OUString aTmp;

        if (m_xTreeView->get_iter_first(*xEntry))
        {
            do
            {
                aTmp = m_xTreeView->get_text(*xEntry);
                if (aTmp == rName)
                {

                    // see if any of the selected nodes are subchildren of this node
                    m_xTreeView->selected_foreach([this, &bChildren, &xEntry](weld::TreeIter& rEntry){
                        std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(&rEntry));
                        while (!bChildren && m_xTreeView->iter_parent(*xParent))
                            bChildren = m_xTreeView->iter_compare(*xParent, *xEntry) == 0;
                        return bChildren;
                    });

                    break;
                }
            }
            while (m_xTreeView->iter_next(*xEntry));
        }
    }

    return bChildren;
}

void SdPageObjsTLV::SetShowAllShapes (
    const bool bShowAllShapes,
    const bool bFillList)
{
    m_bShowAllShapes = bShowAllShapes;
    if (bFillList)
    {
        if (m_pMedium == nullptr)
            Fill(m_pDoc, m_bShowAllPages, m_aDocName);
        else
            Fill(m_pDoc, m_pMedium, m_aDocName);
    }
}

bool SdPageObjsTLV::IsEqualToShapeList(std::unique_ptr<weld::TreeIter>& rEntry, const SdrObjList& rList,
                                       const OUString& rListName)
{
    if (!rEntry)
        return false;
    OUString aName = m_xTreeView->get_text(*rEntry);

    if (rListName != aName)
        return false;

    if (!m_xTreeView->iter_next(*rEntry))
        rEntry.reset();

    SdrObjListIter aIter(&rList,
                         !rList.HasObjectNavigationOrder() /* use navigation order, if available */,
                         SdrIterMode::Flat);

    while (aIter.IsMore())
    {
        SdrObject* pObj = aIter.Next();

        const OUString aObjectName(GetObjectName(pObj));

        if (!aObjectName.isEmpty())
        {
            if (!rEntry)
                return false;

            aName = m_xTreeView->get_text(*rEntry);

            if (aObjectName != aName)
                return false;

            if (pObj->IsGroupObject())
            {
                bool bRet = IsEqualToShapeList(rEntry, *pObj->GetSubList(), aObjectName);
                if (!bRet)
                    return false;
            }
            else
            {
                if (!m_xTreeView->iter_next(*rEntry))
                    rEntry.reset();
            }
        }
    }

    return true;
}

/**
 * Checks if the pages (PageKind::Standard) of a doc and the objects on the pages
 * are identical to the TreeLB.
 * If a doc is provided, this will be the used doc (important by more than
 * one document).
 */
bool SdPageObjsTLV::IsEqualToDoc( const SdDrawDocument* pInDoc )
{
    if( pInDoc )
        m_pDoc = pInDoc;

    if( !m_pDoc )
        return false;

    std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
    if (!m_xTreeView->get_iter_first(*xEntry))
        xEntry.reset();

    // compare all pages including the objects
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = m_pDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        const SdPage* pPage = static_cast<const SdPage*>( m_pDoc->GetPage( nPage ) );
        if( pPage->GetPageKind() == PageKind::Standard )
        {
            bool bRet = IsEqualToShapeList(xEntry, *pPage, pPage->GetName());
            if (!bRet)
                return false;
        }
        nPage++;
    }
    // If there are still entries in the listbox,
    // then objects (with names) or pages were deleted
    return !xEntry;
}

IMPL_LINK(SdPageObjsTLV, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if (m_xAccel->execute(rKeyCode))
    {
        // the accelerator consumed the event
        return true;
    }
    if (rKeyCode.GetCode() == KEY_RETURN)
    {
        std::unique_ptr<weld::TreeIter> xCursor(m_xTreeView->make_iterator());
        if (m_xTreeView->get_cursor(xCursor.get()))
        {
            if (m_xTreeView->get_row_expanded(*xCursor))
                m_xTreeView->collapse_row(*xCursor);
            else
                m_xTreeView->expand_row(*xCursor);
        }
        m_aRowActivatedHdl.Call(*m_xTreeView);
        return true;
    }
    return m_aKeyPressHdl.Call(rKEvt);
}

IMPL_LINK(SdPageObjsTLV, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    m_bSelectionHandlerNavigates = rMEvt.GetClicks() == 1;
    m_bNavigationGrabsFocus = rMEvt.GetClicks() != 1;
    return false;
}

IMPL_LINK_NOARG(SdPageObjsTLV, MouseReleaseHdl, const MouseEvent&, bool)
{
    m_bSelectionHandlerNavigates = false;
    m_bNavigationGrabsFocus = true;
    return false;
}

IMPL_LINK(SdPageObjsTLV, DragBeginHdl, bool&, rUnsetDragIcon, bool)
{
    rUnsetDragIcon = false;
    return StartDrag();
}

namespace
{
    bool CanDragSource(const weld::TreeView& rTreeView)
    {
        std::unique_ptr<weld::TreeIter> xSource(rTreeView.make_iterator());
        if (!rTreeView.get_selected(xSource.get()))
            return false;

        std::unique_ptr<weld::TreeIter> xSourceParent(rTreeView.make_iterator(xSource.get()));
        bool bSourceHasParent = rTreeView.iter_parent(*xSourceParent);
        // level 1 objects only
        if (!bSourceHasParent || rTreeView.get_iter_depth(*xSourceParent))
            return false;

        SdrObject* pSourceObject = reinterpret_cast<SdrObject*>(rTreeView.get_id(*xSource).toInt64());
        if (pSourceObject == reinterpret_cast<SdrObject*>(1))
            pSourceObject = nullptr;

        if (!pSourceObject)
            return false;

        SdrPage* pObjectList = pSourceObject->getSdrPageFromSdrObject();
        if (!pObjectList)
            return false;

        return true;
    }
}

/**
 * StartDrag-Request
 */
bool SdPageObjsTLV::StartDrag()
{
    return !CanDragSource(*m_xTreeView) || DoDrag();
}

/**
 * Begin drag
 */
bool SdPageObjsTLV::DoDrag()
{
    if (!m_pNavigator)
        return true;

    if (!m_xHelper)
        return true;

    // Get the view.
    ::sd::DrawDocShell* pDocShell = m_pDoc->GetDocSh();
    ::sd::ViewShell* pViewShell = GetViewShellForDocShell(*pDocShell);
    if (pViewShell == nullptr)
    {
        OSL_ASSERT(pViewShell!=nullptr);
        return true;
    }
    sd::View* pView = pViewShell->GetView();
    if (pView == nullptr)
    {
        OSL_ASSERT(pView!=nullptr);
        return true;
    }

    bIsInDrag = true;

    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
    bool bUserData = m_xTreeView->get_cursor(xEntry.get());

    SdrObject* pObject = nullptr;
    sal_Int64 nUserData = bUserData ? m_xTreeView->get_id(*xEntry).toInt64() : 0;
    if (nUserData != 1)
        pObject = reinterpret_cast<SdrObject*>(nUserData);
    if (pObject != nullptr)
    {
        // For shapes without a user supplied name (the automatically
        // created name does not count), a different drag and drop technique
        // is used.
        if (GetObjectName(pObject, false).isEmpty())
        {
            AddShapeToTransferable(*m_xHelper, *pObject);
            m_xHelper->SetView(pView);
            SD_MOD()->pTransferDrag = m_xHelper.get();
        }

        // Unnamed shapes have to be selected to be recognized by the
        // current drop implementation.  In order to have a consistent
        // behaviour for all shapes, every shape that is to be dragged is
        // selected first.
        SdrPageView* pPageView = pView->GetSdrPageView();
        pView->UnmarkAllObj(pPageView);
        pView->MarkObj(pObject, pPageView);
    }
    else
    {
        m_xHelper->SetView(pView);
        SD_MOD()->pTransferDrag = m_xHelper.get();
    }

    return false;
}

void SdPageObjsTLV::OnDragFinished()
{
    bIsInDrag = false;
}

SdPageObjsTLVDropTarget::SdPageObjsTLVDropTarget(weld::TreeView& rTreeView)
    : DropTargetHelper(rTreeView.get_drop_target())
    , m_rTreeView(rTreeView)
{
}

/**
 * AcceptDrop-Event
 */
sal_Int8 SdPageObjsTLVDropTarget::AcceptDrop(const AcceptDropEvent& rEvt)
{
    weld::TreeView* pSource = m_rTreeView.get_drag_source();
    // only dragging within the same widget allowed
    if (!pSource || pSource != &m_rTreeView)
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xTarget(m_rTreeView.make_iterator());
    if (!m_rTreeView.get_dest_row_at_pos(rEvt.maPosPixel, xTarget.get(), true))
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xSource(m_rTreeView.make_iterator());
    if (!m_rTreeView.get_selected(xSource.get()))
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xTargetParent(m_rTreeView.make_iterator(xTarget.get()));
    while (m_rTreeView.get_iter_depth(*xTargetParent))
        m_rTreeView.iter_parent(*xTargetParent);

    std::unique_ptr<weld::TreeIter> xSourceParent(m_rTreeView.make_iterator(xSource.get()));
    while (m_rTreeView.get_iter_depth(*xSourceParent))
        m_rTreeView.iter_parent(*xSourceParent);

    // can only drop within the same page
    if (m_rTreeView.iter_compare(*xTargetParent, *xSourceParent) != 0)
        return DND_ACTION_NONE;

    return DND_ACTION_MOVE;
}

/**
 * ExecuteDrop-Event
 */
sal_Int8 SdPageObjsTLVDropTarget::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    weld::TreeView* pSource = m_rTreeView.get_drag_source();
    // only dragging within the same widget allowed
    if (!pSource || pSource != &m_rTreeView)
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xSource(m_rTreeView.make_iterator());
    if (!m_rTreeView.get_selected(xSource.get()))
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xTarget(m_rTreeView.make_iterator());
    if (!m_rTreeView.get_dest_row_at_pos(rEvt.maPosPixel, xTarget.get(), true))
        return DND_ACTION_NONE;
    int nTargetPos = m_rTreeView.get_iter_index_in_parent(*xTarget) + 1;

    SdrObject* pTargetObject = reinterpret_cast<SdrObject*>(m_rTreeView.get_id(*xTarget).toInt64());
    SdrObject* pSourceObject = reinterpret_cast<SdrObject*>(m_rTreeView.get_id(*xSource).toInt64());
    if (pSourceObject == reinterpret_cast<SdrObject*>(1))
        pSourceObject = nullptr;

    if (pTargetObject != nullptr && pSourceObject != nullptr)
    {
        SdrPage* pObjectList = pSourceObject->getSdrPageFromSdrObject();
        if (pObjectList != nullptr)
        {
            sal_uInt32 nNewPosition;
            if (pTargetObject == reinterpret_cast<SdrObject*>(1))
            {
                nNewPosition = 0;
                nTargetPos = 0;
            }
            else
                nNewPosition = pTargetObject->GetNavigationPosition() + 1;
            pObjectList->SetObjectNavigationPosition(*pSourceObject, nNewPosition);
        }

        std::unique_ptr<weld::TreeIter> xSourceParent(m_rTreeView.make_iterator(xSource.get()));
        m_rTreeView.iter_parent(*xSourceParent);

        m_rTreeView.move_subtree(*xSource, xSourceParent.get(), nTargetPos);
    }

    return DND_ACTION_NONE;
}

void SdPageObjsTLV::AddShapeToTransferable (
    SdTransferable& rTransferable,
    SdrObject& rObject) const
{
    std::unique_ptr<TransferableObjectDescriptor> pObjectDescriptor(new TransferableObjectDescriptor);
    bool bIsDescriptorFillingPending (true);

    const SdrOle2Obj* pOleObject = dynamic_cast<const SdrOle2Obj*>(&rObject);
    if (pOleObject != nullptr && pOleObject->GetObjRef().is())
    {
        // If object has no persistence it must be copied as part of the document
        try
        {
            uno::Reference< embed::XEmbedPersist > xPersObj (pOleObject->GetObjRef(), uno::UNO_QUERY );
            if (xPersObj.is() && xPersObj->hasEntry())
            {
                SvEmbedTransferHelper::FillTransferableObjectDescriptor(
                    *pObjectDescriptor,
                    pOleObject->GetObjRef(),
                    pOleObject->GetGraphic(),
                    pOleObject->GetAspect());
                bIsDescriptorFillingPending = false;
            }
        }
        catch( uno::Exception& )
        {
        }
    }

    ::sd::DrawDocShell* pDocShell = m_pDoc->GetDocSh();
    if (bIsDescriptorFillingPending && pDocShell!=nullptr)
    {
        pDocShell->FillTransferableObjectDescriptor(*pObjectDescriptor);
    }

    Point aDragPos (rObject.GetCurrentBoundRect().Center());
    pObjectDescriptor->maDragStartPos = aDragPos;
    if (pDocShell != nullptr)
        pObjectDescriptor->maDisplayName = pDocShell->GetMedium()->GetURLObject().GetURLNoPass();
    else
        pObjectDescriptor->maDisplayName.clear();

    rTransferable.SetStartPos(aDragPos);
    rTransferable.SetObjectDescriptor( std::move(pObjectDescriptor) );
}

::sd::ViewShell* SdPageObjsTLV::GetViewShellForDocShell (::sd::DrawDocShell& rDocShell)
{
    {
        ::sd::ViewShell* pViewShell = rDocShell.GetViewShell();
        if (pViewShell != nullptr)
            return pViewShell;
    }

    try
    {
        // Get a component enumeration from the desktop and search it for documents.
        uno::Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext());

        uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(xContext);

        if ( ! xDesktop.is())
            return nullptr;

        uno::Reference<container::XIndexAccess> xFrameAccess = xDesktop->getFrames();
        if ( ! xFrameAccess.is())
            return nullptr;

        for (sal_Int32 nIndex=0,nCount=xFrameAccess->getCount(); nIndex<nCount; ++nIndex)
        {
            uno::Reference<frame::XFrame> xFrame;
            if ( ! (xFrameAccess->getByIndex(nIndex) >>= xFrame))
                continue;

            ::sd::DrawController* pController = dynamic_cast<sd::DrawController*>(xFrame->getController().get());
            if (pController == nullptr)
                continue;
            ::sd::ViewShellBase* pBase = pController->GetViewShellBase();
            if (pBase == nullptr)
                continue;
            if (pBase->GetDocShell() != &rDocShell)
                continue;

            const std::shared_ptr<sd::ViewShell> pViewShell (pBase->GetMainViewShell());
            if (pViewShell)
                return pViewShell.get();
        }
    }
    catch (uno::Exception &)
    {
        // When there is an exception then simply use the default value of
        // bIsEnabled and disable the controls.
    }
    return nullptr;
}

SdPageObjsTLV::SdPageObjsTLV(std::unique_ptr<weld::TreeView> xTreeView)
    : m_xTreeView(std::move(xTreeView))
    , m_xScratchIter(m_xTreeView->make_iterator())
    , m_xDropTargetHelper(new SdPageObjsTLVDropTarget(*m_xTreeView))
    , m_xAccel(::svt::AcceleratorExecute::createAcceleratorHelper())
    , m_pNavigator(nullptr)
    , m_pDoc(nullptr)
    , m_pBookmarkDoc(nullptr)
    , m_pMedium(nullptr)
    , m_pOwnMedium(nullptr)
    , m_bLinkableSelected(false)
    , m_bShowAllShapes(false)
    , m_bShowAllPages(false)
    , m_bSelectionHandlerNavigates(false)
    , m_bNavigationGrabsFocus(true)
    , m_eSelectionMode(SelectionMode::Single)
    , m_nSelectEventId(nullptr)
    , m_nRowActivateEventId(nullptr)
{
    m_xTreeView->connect_expanding(LINK(this, SdPageObjsTLV, RequestingChildrenHdl));
    m_xTreeView->connect_changed(LINK(this, SdPageObjsTLV, SelectHdl));
    m_xTreeView->connect_row_activated(LINK(this, SdPageObjsTLV, RowActivatedHdl));
    m_xTreeView->connect_drag_begin(LINK(this, SdPageObjsTLV, DragBeginHdl));
    m_xTreeView->connect_key_press(LINK(this, SdPageObjsTLV, KeyInputHdl));
    m_xTreeView->connect_mouse_press(LINK(this, SdPageObjsTLV, MousePressHdl));
    m_xTreeView->connect_mouse_release(LINK(this, SdPageObjsTLV, MouseReleaseHdl));

    m_xTreeView->set_size_request(m_xTreeView->get_approximate_digit_width() * 28,
                                  m_xTreeView->get_text_height() * 8);
}

IMPL_LINK_NOARG(SdPageObjsTLV, SelectHdl, weld::TreeView&, void)
{
    if (m_nSelectEventId)
        Application::RemoveUserEvent(m_nSelectEventId);
    // post the event to process select event after mouse press event
    m_nSelectEventId = Application::PostUserEvent(LINK(this, SdPageObjsTLV, AsyncSelectHdl));
}

IMPL_LINK_NOARG(SdPageObjsTLV, RowActivatedHdl, weld::TreeView&, bool)
{
    if (m_nRowActivateEventId)
        Application::RemoveUserEvent(m_nRowActivateEventId);
    // post the event to process row activate after mouse press event
    m_nRowActivateEventId = Application::PostUserEvent(LINK(this, SdPageObjsTLV, AsyncRowActivatedHdl));
    return true;
}

IMPL_LINK_NOARG(SdPageObjsTLV, AsyncSelectHdl, void*, void)
{
    Select();
}

void SdPageObjsTLV::Select()
{
    m_nSelectEventId = nullptr;

    m_bLinkableSelected = true;

    m_xTreeView->selected_foreach([this](weld::TreeIter& rEntry){
        if (m_xTreeView->get_id(rEntry).toInt64() == 0)
            m_bLinkableSelected = false;
        return false;
    });

    m_aChangeHdl.Call(*m_xTreeView);

    if (m_bSelectionHandlerNavigates)
        m_aRowActivatedHdl.Call(*m_xTreeView);

    if (!m_pNavigator)
    {
        m_xHelper.clear();
        return;
    }

    ::sd::DrawDocShell* pDocShell = m_pDoc->GetDocSh();
    OUString aURL = INetURLObject(pDocShell->GetMedium()->GetPhysicalName(), INetProtocol::File).GetMainURL(INetURLObject::DecodeMechanism::NONE);
    NavigatorDragType eDragType = m_pNavigator->GetNavigatorDragType();

    OUString sSelectedEntry = m_xTreeView->get_selected_text();
    aURL += "#" + sSelectedEntry;

    INetBookmark aBookmark(aURL, sSelectedEntry);
    sal_Int8 nDNDActions = DND_ACTION_COPYMOVE;

    if( eDragType == NAVIGATOR_DRAGTYPE_LINK )
        nDNDActions = DND_ACTION_LINK;  // Either COPY *or* LINK, never both!
    else if (m_pDoc->GetSdPageCount(PageKind::Standard) == 1)
    {
        // Can not move away the last slide in a document.
        nDNDActions = DND_ACTION_COPY;
    }

    // object is destroyed by internal reference mechanism
    m_xHelper.set(new SdPageObjsTLV::SdPageObjsTransferable(aBookmark, *pDocShell, eDragType));
    rtl::Reference<TransferDataContainer> xHelper(m_xHelper.get());
    m_xTreeView->enable_drag_source(xHelper, nDNDActions);
}

IMPL_LINK_NOARG(SdPageObjsTLV, AsyncRowActivatedHdl, void*, void)
{
    m_nRowActivateEventId = nullptr;
    m_aRowActivatedHdl.Call(*m_xTreeView);
}

OUString SdPageObjsTLV::GetObjectName(
    const SdrObject* pObject,
    const bool bCreate) const
{
    OUString aRet;

    if ( pObject )
    {
        aRet = pObject->GetName();

        if (aRet.isEmpty() && dynamic_cast<const SdrOle2Obj* >(pObject) !=  nullptr)
            aRet = static_cast< const SdrOle2Obj* >( pObject )->GetPersistName();
    }

    if (bCreate
        && m_bShowAllShapes
        && aRet.isEmpty()
        && pObject!=nullptr)
    {
        aRet = SdResId(STR_NAVIGATOR_SHAPE_BASE_NAME);
        aRet = aRet.replaceFirst("%1", OUString::number(pObject->GetOrdNum() + 1));
    }

    return aRet;
}

std::vector<OUString> SdPageObjsTLV::GetSelectEntryList(const int nDepth) const
{
    std::vector<OUString> aEntries;

    m_xTreeView->selected_foreach([this, nDepth, &aEntries](weld::TreeIter& rEntry){
        int nListDepth = m_xTreeView->get_iter_depth(rEntry);
        if (nListDepth == nDepth)
            aEntries.push_back(m_xTreeView->get_text(rEntry));
        return false;
    });

    return aEntries;
}

/**
 * Checks if it is a draw file and opens the BookmarkDoc depending of
 * the provided Docs
 */
SdDrawDocument* SdPageObjsTLV::GetBookmarkDoc(SfxMedium* pMed)
{
    if (
       !m_pBookmarkDoc ||
         (pMed && (!m_pOwnMedium || m_pOwnMedium->GetName() != pMed->GetName()))
      )
    {
        // create a new BookmarkDoc if now one exists or if a new Medium is provided
        if (m_pOwnMedium != pMed)
        {
            CloseBookmarkDoc();
        }

        if (pMed)
        {
            // it looks that it is undefined if a Medium was set by Fill() already
            DBG_ASSERT( !m_pMedium, "SfxMedium confusion!" );
            delete m_pMedium;
            m_pMedium = nullptr;

            // take over this Medium (currently used only be Navigator)
            m_pOwnMedium = pMed;
        }

        DBG_ASSERT( m_pMedium || pMed, "No SfxMedium provided!" );

        if( pMed )
        {
            // in this mode the document is also owned and controlled by this instance
            m_xBookmarkDocShRef = new ::sd::DrawDocShell(SfxObjectCreateMode::STANDARD, true, DocumentType::Impress);
            if (m_xBookmarkDocShRef->DoLoad(pMed))
                m_pBookmarkDoc = m_xBookmarkDocShRef->GetDoc();
            else
                m_pBookmarkDoc = nullptr;
        }
        else if ( m_pMedium )
            // in this mode the document is owned and controlled by the SdDrawDocument
            // it can be released by calling the corresponding CloseBookmarkDoc method
            // successful creation of a document makes this the owner of the medium
            m_pBookmarkDoc = const_cast<SdDrawDocument*>(m_pDoc)->OpenBookmarkDoc(m_pMedium);

        if ( !m_pBookmarkDoc )
        {
            std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xTreeView.get(),
                                                           VclMessageType::Warning, VclButtonsType::Ok, SdResId(STR_READ_DATA_ERROR)));
            xErrorBox->run();
            m_pMedium = nullptr; //On failure the SfxMedium is invalid
        }
    }

    return m_pBookmarkDoc;
}

/**
 * Entries are inserted only by request (double click)
 */
IMPL_LINK(SdPageObjsTLV, RequestingChildrenHdl, const weld::TreeIter&, rFileEntry, bool)
{
    if (!m_xTreeView->iter_has_child(rFileEntry))
    {
        if (GetBookmarkDoc())
        {
            SdrObject*   pObj = nullptr;

            OUString sImgPage(BMP_PAGE);
            OUString sImgPageObjs(BMP_PAGEOBJS);
            OUString sImgObjects(BMP_OBJECTS);
            OUString sImgOle(BMP_OLE);
            OUString sImgGraphic(BMP_GRAPHIC);

            // document name already inserted

            // only insert all "normal" ? slides with objects
            sal_uInt16 nPage = 0;
            const sal_uInt16 nMaxPages = m_pBookmarkDoc->GetPageCount();

            std::unique_ptr<weld::TreeIter> xPageEntry;
            while (nPage < nMaxPages)
            {
                SdPage* pPage = static_cast<SdPage*>(m_pBookmarkDoc->GetPage(nPage));
                if (pPage->GetPageKind() == PageKind::Standard)
                {
                    OUString sId(OUString::number(1));
                    m_xTreeView->insert(&rFileEntry, -1, &pPage->GetName(), &sId,
                                        nullptr, nullptr, false, m_xScratchIter.get());
                    m_xTreeView->set_image(*m_xScratchIter, sImgPage);

                    if (!xPageEntry)
                    {
                        xPageEntry = m_xTreeView->make_iterator(&rFileEntry);
                        (void)m_xTreeView->iter_children(*xPageEntry);
                    }
                    else
                        (void)m_xTreeView->iter_next_sibling(*xPageEntry);

                    SdrObjListIter aIter( pPage, SdrIterMode::DeepWithGroups );

                    while( aIter.IsMore() )
                    {
                        pObj = aIter.Next();
                        OUString aStr( GetObjectName( pObj ) );
                        if( !aStr.isEmpty() )
                        {
                            if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_OLE2 )
                            {
                                m_xTreeView->insert(xPageEntry.get(), -1, &aStr, nullptr,
                                                    nullptr, nullptr, false, m_xScratchIter.get());
                                m_xTreeView->set_image(*m_xScratchIter, sImgOle);
                            }
                            else if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_GRAF )
                            {
                                m_xTreeView->insert(xPageEntry.get(), -1, &aStr, nullptr,
                                                    nullptr, nullptr, false, m_xScratchIter.get());
                                m_xTreeView->set_image(*m_xScratchIter, sImgGraphic);
                            }
                            else
                            {
                                m_xTreeView->insert(xPageEntry.get(), -1, &aStr, nullptr,
                                                    nullptr, nullptr, false, m_xScratchIter.get());
                                m_xTreeView->set_image(*m_xScratchIter, sImgObjects);
                            }
                        }
                    }
                    if (m_xTreeView->iter_has_child(*xPageEntry))
                    {
                        m_xTreeView->set_image(*xPageEntry, sImgPageObjs);
                    }
                }
                nPage++;
            }
        }
    }
    return true;
}

void SdPageObjsTLV::SetSdNavigator(SdNavigatorWin* pNavigator)
{
    m_pNavigator = pNavigator;
}

void SdPageObjsTLV::SetViewFrame(const SfxViewFrame* pViewFrame)
{
    sd::ViewShellBase* pBase = sd::ViewShellBase::GetViewShellBase(pViewFrame);
    std::shared_ptr<sd::ViewShell> xViewShell = pBase->GetMainViewShell();
    SAL_WARN_IF(!xViewShell, "sd", "null pBaseViewFrame");
    const css::uno::Reference< css::frame::XFrame > xFrame = xViewShell ? xViewShell->GetViewFrame()->GetFrame().GetFrameInterface() : nullptr;
    m_xAccel->init(::comphelper::getProcessComponentContext(), xFrame);
}

/**
 * Close and delete bookmark document
 */
void SdPageObjsTLV::CloseBookmarkDoc()
{
    if (m_xBookmarkDocShRef.is())
    {
        m_xBookmarkDocShRef->DoClose();
        m_xBookmarkDocShRef.clear();

        // Medium is owned by document, so it's destroyed already
        m_pOwnMedium = nullptr;
    }
    else if (m_pBookmarkDoc)
    {
        DBG_ASSERT(!m_pOwnMedium, "SfxMedium confusion!");
        if (m_pDoc)
        {
            // The document owns the Medium, so the Medium will be invalid after closing the document
            const_cast<SdDrawDocument*>(m_pDoc)->CloseBookmarkDoc();
            m_pMedium = nullptr;
        }
    }
    else
    {
        // perhaps mpOwnMedium provided, but no successful creation of BookmarkDoc
        delete m_pOwnMedium;
        m_pOwnMedium = nullptr;
    }

    m_pBookmarkDoc = nullptr;
}

bool SdPageObjsTLV::PageBelongsToCurrentShow(const SdPage* pPage) const
{
    // Return <TRUE/> as default when there is no custom show or when none
    // is used.  The page does then belong to the standard show.
    bool bBelongsToShow = true;

    if (m_pDoc->getPresentationSettings().mbCustomShow)
    {
        // Get the current custom show.
        SdCustomShow* pCustomShow = nullptr;
        SdCustomShowList* pShowList = const_cast<SdDrawDocument*>(m_pDoc)->GetCustomShowList();
        if (pShowList != nullptr)
        {
            sal_uLong nCurrentShowIndex = pShowList->GetCurPos();
            pCustomShow = (*pShowList)[nCurrentShowIndex].get();
        }

        // Check whether the given page is part of that custom show.
        if (pCustomShow != nullptr)
        {
            bBelongsToShow = false;
            size_t nPageCount = pCustomShow->PagesVector().size();
            for (size_t i=0; i<nPageCount && !bBelongsToShow; i++)
                if (pPage == pCustomShow->PagesVector()[i])
                    bBelongsToShow = true;
        }
    }

    return bBelongsToShow;
}

void SdPageObjsTLV::AddShapeList (
    const SdrObjList& rList,
    SdrObject* pShape,
    const OUString& rsName,
    const bool bIsExcluded,
    const weld::TreeIter* pParentEntry)
{
    OUString aIcon(BMP_PAGE);
    if (bIsExcluded)
        aIcon = BMP_PAGE_EXCLUDED;
    else if (pShape != nullptr)
        aIcon = BMP_GROUP;

    OUString aUserData("1");
    if (pShape != nullptr)
        aUserData = OUString::number(reinterpret_cast<sal_Int64>(pShape));

    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
    InsertEntry(pParentEntry, aUserData, rsName, aIcon, xEntry.get());

    SdrObjListIter aIter(
        &rList,
        !rList.HasObjectNavigationOrder() /* use navigation order, if available */,
        SdrIterMode::Flat);

    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        OSL_ASSERT(pObj!=nullptr);

        // Get the shape name.
        OUString aStr (GetObjectName( pObj ) );
        OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pObj)));

        if( !aStr.isEmpty() )
        {
            if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_OLE2 )
            {
                InsertEntry(xEntry.get(), sId, aStr, BMP_OLE);
            }
            else if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == OBJ_GRAF )
            {
                InsertEntry(xEntry.get(), sId, aStr, BMP_GRAPHIC);
            }
            else if (pObj->IsGroupObject())
            {
                AddShapeList(
                    *pObj->GetSubList(),
                    pObj,
                    aStr,
                    false,
                    xEntry.get());
            }
            else
            {
                InsertEntry(xEntry.get(), sId, aStr, BMP_OBJECTS);
            }
        }
    }

    if (!m_xTreeView->iter_has_child(*xEntry))
        return;

    if (bIsExcluded)
        m_xTreeView->set_image(*xEntry, BMP_PAGEOBJS_EXCLUDED);
    else
        m_xTreeView->set_image(*xEntry, BMP_PAGEOBJS);
    m_xTreeView->expand_row(*xEntry);
}

/**
 * Fill TreeLB with pages and objects
 */
void SdPageObjsTLV::Fill(const SdDrawDocument* pInDoc, bool bAllPages, const OUString& rDocName)
{
    OUString aSelection = m_xTreeView->get_selected_text();
    clear();

    m_pDoc = pInDoc;
    m_aDocName = rDocName;
    m_bShowAllPages = bAllPages;
    m_pMedium = nullptr;

    // first insert all pages including objects
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = m_pDoc->GetPageCount();

    while( nPage < nMaxPages )
    {
        const SdPage* pPage = static_cast<const SdPage*>( m_pDoc->GetPage( nPage ) );
        if(  (m_bShowAllPages || pPage->GetPageKind() == PageKind::Standard)
             && (pPage->GetPageKind() != PageKind::Handout)   ) //#94954# never list the normal handout page ( handout-masterpage is used instead )
        {
            bool bPageExcluded = pPage->IsExcluded();

            bool bPageBelongsToShow = PageBelongsToCurrentShow (pPage);
            bPageExcluded |= !bPageBelongsToShow;

            AddShapeList(*pPage, nullptr, pPage->GetName(), bPageExcluded, nullptr);
        }
        nPage++;
    }

    // then insert all master pages including objects
    if( m_bShowAllPages )
    {
        nPage = 0;
        const sal_uInt16 nMaxMasterPages = m_pDoc->GetMasterPageCount();

        while( nPage < nMaxMasterPages )
        {
            const SdPage* pPage = static_cast<const SdPage*>( m_pDoc->GetMasterPage( nPage ) );
            AddShapeList(*pPage, nullptr, pPage->GetName(), false, nullptr);
            nPage++;
        }
    }
    if (!aSelection.isEmpty())
    {
        m_xTreeView->all_foreach([this, &aSelection](weld::TreeIter& rEntry){
            if (m_xTreeView->get_text(rEntry) == aSelection)
            {
                m_xTreeView->select(rEntry);
                return true;
            }
            return false;
        });
    }
}

/**
 * We insert only the first entry. Children are created on demand.
 */
void SdPageObjsTLV::Fill( const SdDrawDocument* pInDoc, SfxMedium* pInMedium,
                          const OUString& rDocName )
{
    m_pDoc = pInDoc;

    // this object now owns the Medium
    m_pMedium = pInMedium;
    m_aDocName = rDocName;

    OUString sId(OUString::number(1));
    // insert document name
    m_xTreeView->insert(nullptr, -1, &m_aDocName, &sId, nullptr, nullptr, true, m_xScratchIter.get());
    m_xTreeView->set_image(*m_xScratchIter, BMP_DOC_OPEN);
}

/**
 * select an entry in TreeLB
 */
bool SdPageObjsTLV::SelectEntry( const OUString& rName )
{
    bool bFound = false;

    if (!rName.isEmpty())
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
        OUString aTmp;

        if (m_xTreeView->get_iter_first(*xEntry))
        {
            do
            {
                aTmp = m_xTreeView->get_text(*xEntry);
                if (aTmp == rName)
                {
                    m_xTreeView->set_cursor(*xEntry);
                    m_xTreeView->select(*xEntry);
                    bFound = true;
                    break;
                }
            }
            while (m_xTreeView->iter_next(*xEntry));
        }
    }

    return bFound;
}

SdPageObjsTLV::~SdPageObjsTLV()
{
    if (m_nSelectEventId)
        Application::RemoveUserEvent(m_nSelectEventId);
    if (m_nRowActivateEventId)
        Application::RemoveUserEvent(m_nRowActivateEventId);

    if (m_pBookmarkDoc)
        CloseBookmarkDoc();
    else
    {
        // no document was created from m_pMedium, so this object is still the owner of it
        delete m_pMedium;
    }
    m_xAccel.reset();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
