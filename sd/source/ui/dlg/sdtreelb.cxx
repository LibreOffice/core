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

#include <sal/log.hxx>
#include <sal/types.h>
#include <sot/formats.hxx>
#include <utility>
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

#include <vcl/commandevent.hxx>

#include <svx/svdview.hxx>
#include <DrawViewShell.hxx>

#include <svx/svdoashp.hxx>
#include <svx/sdasitm.hxx>
#include <svl/poolitem.hxx>
#include <svl/stritem.hxx>

using namespace com::sun::star;

namespace {

sd::DrawViewShell* lcl_getDrawViewShell(const SdDrawDocument* pDoc)
{
    if (!pDoc || !pDoc->GetDocSh())
        return nullptr;
    return static_cast<sd::DrawViewShell*>(pDoc->GetDocSh()->GetViewShell());
}

}

bool SdPageObjsTLV::bIsInDrag = false;

bool SdPageObjsTLV::IsInDrag()
{
    return bIsInDrag;
}

SotClipboardFormatId SdPageObjsTLV::SdPageObjsTransferable::mnListBoxDropFormatId = static_cast<SotClipboardFormatId>(SAL_MAX_UINT32);

SdPageObjsTLV::SdPageObjsTransferable::SdPageObjsTransferable(
        INetBookmark aBookmark,
    ::sd::DrawDocShell& rDocShell,
    NavigatorDragType eDragType)
    : SdTransferable(rDocShell.GetDoc(), nullptr, true),
      maBookmark(std::move( aBookmark )),
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

SdPageObjsTLV::SdPageObjsTransferable* SdPageObjsTLV::SdPageObjsTransferable::getImplementation( const css::uno::Reference< css::uno::XInterface >& rxData )
    noexcept
{
    return dynamic_cast<SdPageObjsTLV::SdPageObjsTransferable*>(rxData.get());
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
bool SdPageObjsTLV::HasSelectedChildren( std::u16string_view rName )
{
    bool bChildren = false;

    if( !rName.empty() )
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

void SdPageObjsTLV::SetOrderFrontToBack(const bool bOrderFrontToBack)
{
    m_bOrderFrontToBack = bOrderFrontToBack;
}

bool SdPageObjsTLV::IsEqualToShapeList(std::unique_ptr<weld::TreeIter>& rEntry, const SdrObjList& rList,
                                       std::u16string_view rListName)
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

    sd::DrawViewShell* pDrawViewShell = lcl_getDrawViewShell(m_pDoc);
    if (!pDrawViewShell)
        return false;
    PageKind eDrawViewShellPageKind = pDrawViewShell->GetPageKind();
    if (eDrawViewShellPageKind != PageKind::Standard && eDrawViewShellPageKind != PageKind::Notes)
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
        if (pPage->GetPageKind() == eDrawViewShellPageKind)
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

IMPL_LINK(SdPageObjsTLV, CommandHdl, const CommandEvent&, rCEvt, bool)
{
    if (IsEditingActive())
        return false;

    if (rCEvt.GetCommand() == CommandEventId::ContextMenu)
    {
        m_bMouseReleased = false;
        m_xTreeView->grab_focus();

        // select clicked entry
        if (std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator());
                rCEvt.IsMouseEvent() &&  m_xTreeView->get_dest_row_at_pos(
                    rCEvt.GetMousePosPixel(), xEntry.get(), false))
        {
            m_bSelectionHandlerNavigates = true;
            m_bNavigationGrabsFocus = false;
            m_xTreeView->set_cursor(*xEntry);
            Select();
        }

        bool bRet = m_aPopupMenuHdl.Call(rCEvt);
        m_bMouseReleased = true;
        return bRet;
    }

    return false;
}

IMPL_LINK(SdPageObjsTLV, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if (m_xAccel->execute(rKeyCode))
    {
        m_bEditing = false;
        // the accelerator consumed the event
        return true;
    }
    if (rKeyCode.GetCode() == KEY_RETURN)
    {
        m_bEditing = false;
        std::unique_ptr<weld::TreeIter> xCursor(m_xTreeView->make_iterator());
        if (m_xTreeView->get_cursor(xCursor.get()) && m_xTreeView->iter_has_child(*xCursor))
        {
            if (m_xTreeView->get_row_expanded(*xCursor))
                m_xTreeView->collapse_row(*xCursor);
            else
                m_xTreeView->expand_row(*xCursor);
        }
        m_bNavigationGrabsFocus = true;
        m_aRowActivatedHdl.Call(*m_xTreeView);
        m_bNavigationGrabsFocus = false;
        return true;
    }
    bool bRet = m_aKeyPressHdl.Call(rKEvt);
    // m_bEditing needs to be set after key press handler call back or x11 won't end editing on
    // Esc key press. See SdNavigatorWin::KeyInputHdl.
    m_bEditing = false;
    return bRet;
}

IMPL_LINK(SdPageObjsTLV, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    m_bMouseReleased = false;
    m_bEditing = false;
    m_bSelectionHandlerNavigates = rMEvt.GetClicks() == 1;
    m_bNavigationGrabsFocus = rMEvt.GetClicks() != 1;
    return false;
}

IMPL_LINK_NOARG(SdPageObjsTLV, MouseReleaseHdl, const MouseEvent&, bool)
{
    m_bMouseReleased = true;
    if (m_aMouseReleaseHdl.IsSet() && m_aMouseReleaseHdl.Call(MouseEvent()))
        return false;

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
        // disallow root drag
        if (!bSourceHasParent)
            return false;

        SdrObject* pSourceObject = weld::fromId<SdrObject*>(rTreeView.get_id(*xSource));
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

    m_xDropTargetHelper->SetDrawView(pViewShell->GetDrawView());
    m_xDropTargetHelper->SetOrderFrontToBack(m_bOrderFrontToBack);
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
    , m_pSdrView(nullptr)
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

    // disallow when root is drop target
    if (m_rTreeView.get_iter_depth(*xTarget) == 0)
        return DND_ACTION_NONE;

    // disallow if there is no source entry selected
    std::unique_ptr<weld::TreeIter> xSource(m_rTreeView.make_iterator());
    if (!m_rTreeView.get_selected(xSource.get()))
        return DND_ACTION_NONE;

    // disallow when root is source
    if (m_rTreeView.get_iter_depth(*xSource) == 0)
        return DND_ACTION_NONE;

    // disallow when the source is the parent or ancestral parent of the target
    std::unique_ptr<weld::TreeIter> xTargetParent(m_rTreeView.make_iterator(xTarget.get()));
    while (m_rTreeView.get_iter_depth(*xTargetParent) > 1)
    {
        if (!m_rTreeView.iter_parent(*xTargetParent) ||
                m_rTreeView.iter_compare(*xSource, *xTargetParent) == 0)
            return DND_ACTION_NONE;
    }

    // disallow drop when source and target are not within the same page
    std::unique_ptr<weld::TreeIter> xSourcePage(m_rTreeView.make_iterator(xSource.get()));
    std::unique_ptr<weld::TreeIter> xTargetPage(m_rTreeView.make_iterator(xTarget.get()));
    while (m_rTreeView.get_iter_depth(*xTargetPage))
        m_rTreeView.iter_parent(*xTargetPage);
    while (m_rTreeView.get_iter_depth(*xSourcePage))
        m_rTreeView.iter_parent(*xSourcePage);
    if (m_rTreeView.iter_compare(*xTargetPage, *xSourcePage) != 0)
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
    if (!m_rTreeView.get_dest_row_at_pos(rEvt.maPosPixel, xTarget.get(), false))
        return DND_ACTION_NONE;

    auto nIterCompare = m_rTreeView.iter_compare(*xSource, *xTarget);
    if (nIterCompare == 0)
    {
        // drop position is the same as source position
        return DND_ACTION_NONE;
    }

    SdrObject* pTargetObject = weld::fromId<SdrObject*>(m_rTreeView.get_id(*xTarget));
    SdrObject* pSourceObject = weld::fromId<SdrObject*>(m_rTreeView.get_id(*xSource));
    if (pSourceObject == reinterpret_cast<SdrObject*>(1))
        pSourceObject = nullptr;
    if (pTargetObject == reinterpret_cast<SdrObject*>(1))
        pTargetObject = nullptr;

    if (pTargetObject != nullptr && pSourceObject != nullptr && m_pSdrView)
    {
        SdrPage* pObjectList = pSourceObject->getSdrPageFromSdrObject();

        std::unique_ptr<weld::TreeIter> xSourceParent(m_rTreeView.make_iterator(xSource.get()));
        m_rTreeView.iter_parent(*xSourceParent);
        std::unique_ptr<weld::TreeIter> xTargetParent(m_rTreeView.make_iterator(xTarget.get()));
        m_rTreeView.iter_parent(*xTargetParent);

        int nTargetPos = m_rTreeView.get_iter_index_in_parent(*xTarget);

        // Make the tree view what the model will be when it is changed below.
        m_rTreeView.move_subtree(*xSource, xTargetParent.get(), nTargetPos);
        m_rTreeView.iter_previous_sibling(*xTarget);
        m_rTreeView.set_cursor(*xTarget);

        // Remove and insert are required for moving objects into and out of groups.
        // PutMarked... by itself would suffice if this wasn't allowed.

        // Remove the source object from source parent list and insert it in the target parent list.
        SdrObject* pSourceParentObject = weld::fromId<SdrObject*>(m_rTreeView.get_id(*xSourceParent));
        SdrObject* pTargetParentObject = weld::fromId<SdrObject*>(m_rTreeView.get_id(*xTargetParent));

        // Presumably there is need for a hard reference to hold on to the removed object so it is
        // guaranteed to be valid for insert back into an object list.
        rtl::Reference<SdrObject> rSourceObject;

        // remove object
        if (pSourceParentObject == reinterpret_cast<SdrObject*>(1))
        {
            rSourceObject = pObjectList->NbcRemoveObject(pSourceObject->GetOrdNum());
        }
        else
        {
            SdrObjList* pList = pSourceParentObject->GetSubList();
            rSourceObject = pList->NbcRemoveObject(pSourceObject->GetOrdNum());
        }

        // insert object
        if (pTargetParentObject == reinterpret_cast<SdrObject*>(1))
        {
            pObjectList->NbcInsertObject(rSourceObject.get());
        }
        else
        {
            SdrObjList* pList = pTargetParentObject->GetSubList();
            pList->NbcInsertObject(rSourceObject.get());
        }

        m_bOrderFrontToBack ? m_pSdrView->PutMarkedInFrontOfObj(pTargetObject) :
                              m_pSdrView->PutMarkedBehindObj(pTargetObject);
    }

    return DND_ACTION_NONE;
}

void SdPageObjsTLV::AddShapeToTransferable (
    SdTransferable& rTransferable,
    const SdrObject& rObject) const
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

            auto xController = xFrame->getController();
            ::sd::DrawController* pController = dynamic_cast<sd::DrawController*>(xController.get());
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
    , m_bOrderFrontToBack(false)
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
    m_xTreeView->connect_editing(LINK(this, SdPageObjsTLV, EditingEntryHdl),
                                 LINK(this, SdPageObjsTLV, EditedEntryHdl));
    m_xTreeView->connect_popup_menu(LINK(this, SdPageObjsTLV, CommandHdl));

    m_xTreeView->set_size_request(m_xTreeView->get_approximate_digit_width() * 28,
                                  m_xTreeView->get_text_height() * 8);
    m_xTreeView->set_column_editables({true});
}

IMPL_LINK(SdPageObjsTLV, EditEntryAgain, void*, p, void)
{
    m_xTreeView->grab_focus();
    std::unique_ptr<weld::TreeIter> xEntry(static_cast<weld::TreeIter*>(p));
    m_xTreeView->start_editing(*xEntry);
    m_bEditing = true;
}

IMPL_LINK_NOARG(SdPageObjsTLV, EditingEntryHdl, const weld::TreeIter&, bool)
{
    m_bEditing = true;
    return true;
}

IMPL_LINK(SdPageObjsTLV, EditedEntryHdl, const IterString&, rIterString, bool)
{
    m_bEditing = false;

    // Did the name change?
    if (m_xTreeView->get_text(rIterString.first) == rIterString.second)
        return true;

    // If the new name is empty or not unique, start editing again.
    if (rIterString.second.isEmpty() || m_pDoc->GetObj(rIterString.second))
    {
        std::unique_ptr<weld::TreeIter> xEntry(m_xTreeView->make_iterator(&rIterString.first));
        Application::PostUserEvent(LINK(this, SdPageObjsTLV, EditEntryAgain), xEntry.release());
        return false;
    }

    // set the new name
    const auto& rEntryId = m_xTreeView->get_id(rIterString.first);
    if (rEntryId.toInt64() == 1)
    {
        // page name
        if (::sd::DrawDocShell* pDocShell = m_pDoc->GetDocSh())
        {
            if (::sd::ViewShell* pViewShell = GetViewShellForDocShell(*pDocShell))
            {
                SdPage* pPage = pViewShell->GetActualPage();
                pPage->SetName(rIterString.second);
            }
        }
    }
    else if (SdrObject* pCursorEntryObject = weld::fromId<SdrObject*>(rEntryId))
    {
        // object name
        pCursorEntryObject->SetName(rIterString.second);
    }

    return true;
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
    return false;
}

IMPL_LINK_NOARG(SdPageObjsTLV, AsyncSelectHdl, void*, void)
{
    Select();
}

void SdPageObjsTLV::Select()
{
    m_nSelectEventId = nullptr;

    // m_bMouseReleased is a hack to make inplace editing work for X11
    if (m_bMouseReleased)
        return;

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

    OUString sSelectedEntry = get_cursor_text(); // what about multiple selections?
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
    m_xHelper.set(new SdPageObjsTLV::SdPageObjsTransferable(std::move(aBookmark), *pDocShell, eDragType));
    rtl::Reference<TransferDataContainer> xHelper(m_xHelper);
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

        if (aRet.isEmpty())
            if (auto pOleObj = dynamic_cast<const SdrOle2Obj* >(pObject))
                aRet = pOleObj->GetPersistName();
    }

    if (bCreate
        && m_bShowAllShapes
        && aRet.isEmpty()
        && pObject!=nullptr)
    {
        OUString sObjName;
        if (pObject->GetObjIdentifier() == SdrObjKind::CustomShape)
        {
            // taken from SdrObjCustomShape::GetCustomShapeName
            OUString aEngine(pObject->GetMergedItem(SDRATTR_CUSTOMSHAPE_ENGINE).GetValue());
            if (aEngine.isEmpty() || aEngine == "com.sun.star.drawing.EnhancedCustomShapeEngine")
            {
                OUString sShapeType;
                const SdrCustomShapeGeometryItem& rGeometryItem
                    = pObject->GetMergedItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
                const uno::Any* pAny = rGeometryItem.GetPropertyValueByName("Type");
                if (pAny && (*pAny >>= sShapeType))
                    sObjName = SdResId(STR_NAVIGATOR_CUSTOMSHAPE) + u": " + sShapeType;
            }
        }
        else
            sObjName = pObject->TakeObjNameSingul();

        aRet = SdResId(STR_NAVIGATOR_SHAPE_BASE_NAME) + " (" + sObjName +")";
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

std::vector<OUString> SdPageObjsTLV::GetSelectedEntryIds() const
{
    std::vector<OUString> vEntryIds;

    m_xTreeView->selected_foreach([this, &vEntryIds](weld::TreeIter& rEntry){
        vEntryIds.push_back(m_xTreeView->get_id(rEntry));
        return false;
    });

    return vEntryIds;
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
                            if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == SdrObjKind::OLE2 )
                            {
                                m_xTreeView->insert(xPageEntry.get(), -1, &aStr, nullptr,
                                                    nullptr, nullptr, false, m_xScratchIter.get());
                                m_xTreeView->set_image(*m_xScratchIter, sImgOle);
                            }
                            else if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == SdrObjKind::Graphic )
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
    const SdrObject* pShape,
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
        aUserData = weld::toId(pShape);

    std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
    InsertEntry(pParentEntry, aUserData, rsName, aIcon, xEntry.get());

    SdrObjListIter aIter(
        &rList,
        !rList.HasObjectNavigationOrder() /* use navigation order, if available */,
        SdrIterMode::Flat);

    while( aIter.IsMore() )
    {
        SdrObject* pObj = aIter.Next();
        assert(pObj!=nullptr);

        // Get the shape name.
        OUString aStr (GetObjectName( pObj ) );
        OUString sId(weld::toId(pObj));

        if( !aStr.isEmpty() )
        {
            if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == SdrObjKind::OLE2 )
            {
                InsertEntry(xEntry.get(), sId, aStr, BMP_OLE);
            }
            else if( pObj->GetObjInventor() == SdrInventor::Default && pObj->GetObjIdentifier() == SdrObjKind::Graphic )
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
                InsertEntry(xEntry.get(), sId, aStr, BMP_OBJECTS); // BMP_OBJECTS
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

    PageKind eDrawViewShellPageKind = PageKind::Standard;
    if (sd::DrawViewShell* pDrawViewShell = lcl_getDrawViewShell(m_pDoc))
        eDrawViewShellPageKind = pDrawViewShell->GetPageKind();

    while( nPage < nMaxPages )
    {
        const SdPage* pPage = static_cast<const SdPage*>( m_pDoc->GetPage( nPage ) );
        PageKind ePagePageKind = pPage->GetPageKind();
        if ((m_bShowAllPages ||
             (ePagePageKind == PageKind::Standard &&
              eDrawViewShellPageKind == PageKind::Standard) ||
             (ePagePageKind == PageKind::Notes &&
              eDrawViewShellPageKind == PageKind::Notes)) &&
                ePagePageKind != PageKind::Handout) //#94954# never list the normal handout page ( handout-masterpage is used instead )
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
bool SdPageObjsTLV::SelectEntry( std::u16string_view rName )
{
    bool bFound = false;

    if (!rName.empty())
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

void SdPageObjsTLV::SelectEntry(const SdrObject *pObj)
{
    if (pObj)
    {
        m_xTreeView->all_foreach([this, &pObj](weld::TreeIter& rEntry){
            if (weld::fromId<SdrObject*>(m_xTreeView->get_id(rEntry)) == pObj)
            {
                // Only scroll to the row of the first selected. And only when the treeview
                // doesn't have the focus.
                if (!m_xTreeView->has_focus() && m_xTreeView->get_selected_rows().empty())
                    m_xTreeView->set_cursor(rEntry);
                m_xTreeView->select(rEntry);
                return true;
            }
            return false;
        });
    }
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
