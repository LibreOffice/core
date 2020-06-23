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
#include <svx/dialmgr.hxx>
#include <svx/fmshell.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svditer.hxx>

#include <helpids.h>
#include <fmexpl.hxx>
#include <fmshimp.hxx>
#include <fmservs.hxx>
#include <fmundo.hxx>
#include <fmpgeimp.hxx>
#include <fmobj.hxx>
#include <fmprop.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/types.hxx>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <svx/sdrpaintwindow.hxx>

#include <svx/strings.hrc>
#include <tools/diagnose_ex.h>
#include <svx/svxids.hrc>
#include <bitmaps.hlst>
#include <vcl/commandevent.hxx>

namespace svxform
{


    #define DROP_ACTION_TIMER_INITIAL_TICKS     10
        // Time until scroll starts
    #define DROP_ACTION_TIMER_SCROLL_TICKS      3
#if 0
        // Time to scroll one line
    #define DROP_ACTION_TIMER_TICK_BASE         10
#endif
        // factor for both declarations (in ms)

    #define EXPLORER_SYNC_DELAY                 200
        // Time (in ms) until explorer synchronizes the view after select or deselect

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::datatransfer;
    using namespace ::com::sun::star::datatransfer::clipboard;
    using namespace ::com::sun::star::sdb;


    // helper


    typedef ::std::map< Reference< XInterface >, SdrObject* > MapModelToShape;


    static void    collectShapeModelMapping( SdrPage const * _pPage, MapModelToShape& _rMapping )
    {
        OSL_ENSURE( _pPage, "collectShapeModelMapping: invalid arg!" );

        _rMapping.clear();

        SdrObjListIter aIter( _pPage );
        while ( aIter.IsMore() )
        {
            SdrObject* pSdrObject = aIter.Next();
            FmFormObj* pFormObject = FmFormObj::GetFormObject( pSdrObject );
            if ( !pFormObject )
                continue;

            Reference< XInterface > xNormalizedModel( pFormObject->GetUnoControlModel(), UNO_QUERY );
                // note that this is normalized (i.e. queried for XInterface explicitly)

            ::std::pair< MapModelToShape::iterator, bool > aPos =
                  _rMapping.emplace( xNormalizedModel, pSdrObject );
            DBG_ASSERT( aPos.second, "collectShapeModelMapping: model was already existent!" );
                // if this asserts, this would mean we have 2 shapes pointing to the same model
        }
    }

    NavigatorTree::NavigatorTree(std::unique_ptr<weld::TreeView> xTreeView)
        :m_xTreeView(std::move(xTreeView))
//TODO        ,m_aControlExchange(this)
        ,nEditEvent(nullptr)
        ,m_sdiState(SDI_DIRTY)
        ,m_aTimerTriggered(-1,-1)
        ,m_aDropActionType( DA_SCROLLUP )
        ,m_nSelectLock(0)
        ,m_nFormsSelected(0)
        ,m_nControlsSelected(0)
        ,m_nHiddenControls(0)
        ,m_aTimerCounter( DROP_ACTION_TIMER_INITIAL_TICKS )
        ,m_bDragDataDirty(false)
        ,m_bPrevSelectionMixed(false)
        ,m_bRootSelected(false)
        ,m_bInitialUpdate(true)
        ,m_bKeyboardCut( false )
        ,m_bEditing( false )
    {
        m_xTreeView->set_help_id(HID_FORM_NAVIGATOR);

#if 0
        SetDragDropMode(DragDropMode::ALL);
#endif
        m_xTreeView->set_selection_mode(SelectionMode::Multiple);

        m_pNavModel.reset(new NavigatorTreeModel());
        Clear();

        StartListening( *m_pNavModel );

        m_aDropActionTimer.SetInvokeHandler(LINK(this, NavigatorTree, OnDropActionTimer));

        m_aSynchronizeTimer.SetInvokeHandler(LINK(this, NavigatorTree, OnSynchronizeTimer));
        m_xTreeView->connect_changed(LINK(this, NavigatorTree, OnEntrySelDesel));
        m_xTreeView->connect_key_press(LINK(this, NavigatorTree, KeyInputHdl));
        m_xTreeView->connect_popup_menu(LINK(this, NavigatorTree, PopupMenuHdl));
        m_xTreeView->connect_editing(LINK(this, NavigatorTree, EditingEntryHdl),
                                     LINK(this, NavigatorTree, EditedEntryHdl));
    }

    NavigatorTree::~NavigatorTree()
    {
        if( nEditEvent )
            Application::RemoveUserEvent( nEditEvent );

        if (m_aSynchronizeTimer.IsActive())
            m_aSynchronizeTimer.Stop();

        DBG_ASSERT(GetNavModel() != nullptr, "NavigatorTree::~NavigatorTree : unexpected : no ExplorerModel");
        EndListening( *m_pNavModel );
        Clear();
        m_pNavModel.reset();
    }

    void NavigatorTree::Clear()
    {
        m_pNavModel->Clear();
    }

    void NavigatorTree::UpdateContent( FmFormShell* pFormShell )
    {
        if (m_bInitialUpdate)
        {
            GrabFocus();
            m_bInitialUpdate = false;
        }

        FmFormShell* pOldShell = GetNavModel()->GetFormShell();
        FmFormPage* pOldPage = GetNavModel()->GetFormPage();
        FmFormPage* pNewPage = pFormShell ? pFormShell->GetCurPage() : nullptr;

        if ((pOldShell != pFormShell) || (pOldPage != pNewPage))
        {
            // new shell during editing
            if (IsEditingActive())
            {
                m_xTreeView->end_editing();
                m_bEditing = false;
            }

            m_bDragDataDirty = true;    // as a precaution, although I don't drag
        }
        GetNavModel()->UpdateContent( pFormShell );

        // if there is a form, expand root
        if (m_xRootEntry && !m_xTreeView->get_row_expanded(*m_xRootEntry))
            m_xTreeView->expand_row(*m_xRootEntry);
        // if there is EXACTLY ONE form, expand it too
        if (m_xRootEntry)
        {
            std::unique_ptr<weld::TreeIter> xFirst(m_xTreeView->make_iterator(m_xRootEntry.get()));
            bool bFirst = m_xTreeView->iter_children(*xFirst);
            if (bFirst)
            {
                std::unique_ptr<weld::TreeIter> xSibling(m_xTreeView->make_iterator(xFirst.get()));
                if (!m_xTreeView->iter_next_sibling(*xSibling))
                    m_xTreeView->expand_row(*xFirst);
            }
        }
    }

    bool NavigatorTree::implAllowExchange( sal_Int8 _nAction, bool* _pHasNonHidden )
    {
        bool bCurEntry = m_xTreeView->get_cursor(nullptr);
        if (!bCurEntry)
            return false;

        // Information for AcceptDrop and Execute Drop
        CollectSelectionData(SDI_ALL);
        if (m_arrCurrentSelection.empty())
            // nothing to do
            return false;

        // check whether there are only hidden controls
        // I may add a format to pCtrlExch
        bool bHasNonHidden = std::any_of(m_arrCurrentSelection.begin(), m_arrCurrentSelection.end(),
            [this](const auto& rEntry) {
                FmEntryData* pCurrent = reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(*rEntry).toInt64());
                return !IsHiddenControl( pCurrent );
            });

        if ( bHasNonHidden && ( 0 == ( _nAction & DND_ACTION_MOVE ) ) )
            // non-hidden controls need to be moved
            return false;

        if ( _pHasNonHidden )
            *_pHasNonHidden = bHasNonHidden;

        return true;
    }

#if 0
    bool NavigatorTree::implPrepareExchange( sal_Int8 _nAction )
    {
        EndSelection();

        bool bHasNonHidden = false;
        if ( !implAllowExchange( _nAction, &bHasNonHidden ) )
            return false;

        m_aControlExchange.prepareDrag();
        m_aControlExchange->setFocusEntry( GetCurEntry() );

        for (const auto& rpEntry : m_arrCurrentSelection)
            m_aControlExchange->addSelectedEntry(rpEntry);

        m_aControlExchange->setFormsRoot( GetNavModel()->GetFormPage()->GetForms() );
        m_aControlExchange->buildPathFormat( this, m_xRootEntry );

        if (!bHasNonHidden)
        {
            // create a sequence
            Sequence< Reference< XInterface > > seqIFaces(m_arrCurrentSelection.size());
            Reference< XInterface >* pArray = seqIFaces.getArray();
            for (const auto& rpEntry : m_arrCurrentSelection)
            {
                *pArray = static_cast< FmEntryData* >( rpEntry->GetUserData() )->GetElement();
                ++pArray;
            }
            // and the new format
            m_aControlExchange->addHiddenControlsFormat(seqIFaces);
        }

        m_bDragDataDirty = false;
        return true;
    }
#endif

#if 0
    void NavigatorTree::StartDrag( sal_Int8 /*nAction*/, const ::Point& /*rPosPixel*/ )
    {
        EndSelection();

        if ( !implPrepareExchange( DND_ACTION_COPYMOVE ) )
            // nothing to do or something went wrong
            return;

        // collected all possible formats for current situation, we can start now
        m_aControlExchange.startDrag( DND_ACTION_COPYMOVE );
    }
#endif

    IMPL_LINK(NavigatorTree, PopupMenuHdl, const CommandEvent&, rEvt, bool)
    {
        bool bHandled = false;
        switch( rEvt.GetCommand() )
        {
            case CommandEventId::ContextMenu:
            {
                // Position of click
                ::Point ptWhere;
                if (rEvt.IsMouseEvent())
                {
                    ptWhere = rEvt.GetMousePosPixel();
                    std::unique_ptr<weld::TreeIter> xClickedOn(m_xTreeView->make_iterator());
                    if (!m_xTreeView->get_dest_row_at_pos(ptWhere, xClickedOn.get(), false))
                        break;
                    if (!m_xTreeView->is_selected(*xClickedOn))
                    {
                        m_xTreeView->unselect_all();
                        m_xTreeView->select(*xClickedOn);
                        m_xTreeView->set_cursor(*xClickedOn);
                    }
                }
                else
                {
                    if (m_arrCurrentSelection.empty()) // only happens with context menu via keyboard
                        break;

                    std::unique_ptr<weld::TreeIter> xCurrent(m_xTreeView->make_iterator());
                    if (!m_xTreeView->get_cursor(xCurrent.get()))
                        break;
                    ptWhere = m_xTreeView->get_row_area(*xCurrent).Center();
                }

                // update my selection data
                CollectSelectionData(SDI_ALL);

                // if there is at least one no-root-entry and the root selected, I deselect root
                if ( (m_arrCurrentSelection.size() > 1) && m_bRootSelected )
                {
                    const std::unique_ptr<weld::TreeIter>& rIter = *m_arrCurrentSelection.begin();
                    m_xTreeView->set_cursor(*rIter);
                    m_xTreeView->unselect(*m_xRootEntry);
                }
                bool bSingleSelection = (m_arrCurrentSelection.size() == 1);


                DBG_ASSERT( (!m_arrCurrentSelection.empty()) || m_bRootSelected, "no entries selected" );
                    // shouldn't happen, because I would have selected one during call to IsSelected,
                    // if there was none before


                // create menu
                FmFormShell* pFormShell = GetNavModel()->GetFormShell();
                FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : nullptr;
                if( pFormShell && pFormModel )
                {
                    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xTreeView.get(), "svx/ui/formnavimenu.ui"));
                    std::unique_ptr<weld::Menu> xContextMenu(xBuilder->weld_menu("menu"));
                    std::unique_ptr<weld::Menu> xSubMenuNew(xBuilder->weld_menu("submenu"));

                    // menu 'New' only exists, if only the root or only one form is selected
                    bool bShowNew = bSingleSelection && (m_nFormsSelected || m_bRootSelected);
                    if (!bShowNew)
                        xContextMenu->remove("new");

                    // 'New'\'Form' under the same terms
                    bool bShowForm = bSingleSelection && (m_nFormsSelected || m_bRootSelected);
                    if (bShowForm)
                        xSubMenuNew->append("form", SvxResId(RID_STR_FORM), RID_SVXBMP_FORM);

                    // 'New'\'hidden...', if exactly one form is selected
                    bool bShowHidden = bSingleSelection && m_nFormsSelected;
                    if (bShowHidden)
                        xSubMenuNew->append("hidden", SvxResId(RID_STR_HIDDEN), RID_SVXBMP_HIDDEN);

                    // 'Delete': everything which is not root can be removed
                    if (m_bRootSelected)
                        xContextMenu->remove("delete");

                    // 'Cut', 'Copy' and 'Paste'
                    bool bShowCut = !m_bRootSelected && implAllowExchange(DND_ACTION_MOVE);
                    if (!bShowCut)
                        xContextMenu->remove("cut");
                    bool bShowCopy = !m_bRootSelected && implAllowExchange(DND_ACTION_COPY);
                    if (!bShowCopy)
                        xContextMenu->remove("copy");
//TODO                    if (!implAcceptPaste())
                    xContextMenu->remove("paste");

                    // TabDialog, if exactly one form
                    bool bShowTabOrder = bSingleSelection && m_nFormsSelected;
                    if (!bShowTabOrder)
                        xContextMenu->remove("taborder");

                    bool bShowProps = true;
                    // in XML forms, we don't allow for the properties of a form
                    // #i36484#
                    if (pFormShell->GetImpl()->isEnhancedForm_Lock() && !m_nControlsSelected)
                        bShowProps = false;
                    // if the property browser is already open, we don't allow for the properties, too
                    if (pFormShell->GetImpl()->IsPropBrwOpen_Lock())
                        bShowProps = false;

                    // and finally, if there's a mixed selection of forms and controls, disable the entry, too
                    if (bShowProps && !pFormShell->GetImpl()->IsPropBrwOpen_Lock())
                        bShowProps =
                            (m_nControlsSelected && !m_nFormsSelected) || (!m_nControlsSelected && m_nFormsSelected);

                    if (!bShowProps)
                        xContextMenu->remove("props");

                    // rename, if one element and no root
                    bool bShowRename = bSingleSelection && !m_bRootSelected;
                    if (!bShowRename)
                        xContextMenu->remove("rename");

                    if (!m_bRootSelected)
                    {
                        // Readonly-entry is only for root
                        xContextMenu->remove("designmode");
                        // the same for automatic control focus
                        xContextMenu->remove("controlfocus");
                    }
#if 0
                    std::unique_ptr<VclBuilder> xBuilder;
                    VclPtr<PopupMenu> xConversionMenu;
                    // ConvertTo-Slots are enabled, if one control is selected
                    // the corresponding slot is disabled
                    const sal_Int16 nChangeId = aContextMenu->GetItemId("change");
#endif
                    if (!m_bRootSelected && !m_nFormsSelected && (m_nControlsSelected == 1))
                    {
#if 0
                        xBuilder = FmXFormShell::GetConversionMenu_Lock();
                        xConversionMenu = xBuilder->get_menu("menu");
                        aContextMenu->SetPopupMenu(nChangeId, xConversionMenu);
#if OSL_DEBUG_LEVEL > 0
                        const std::unique_ptr<weld::TreeIter>& rIter = *m_arrCurrentSelection.begin();
                        FmControlData* pCurrent = reinterpret_cast<FmControlData*>(m_xTreeView->get_id(*rIter).toInt64());
                        OSL_ENSURE( pFormShell->GetImpl()->isSolelySelected_Lock( pCurrent->GetFormComponent() ),
                            "NavigatorTree::Command: inconsistency between the navigator selection, and the selection as the shell knows it!" );
#endif

                        pFormShell->GetImpl()->checkControlConversionSlotsForCurrentSelection_Lock(*aContextMenu->GetPopupMenu(nChangeId));
#endif
                    }
                    else
                        xContextMenu->remove("change");

                    if (m_bRootSelected)
                    {
                        // set OpenReadOnly
                        xContextMenu->set_active("designmode", pFormModel->GetOpenInDesignMode());
                        xContextMenu->set_active("controlfocus", pFormModel->GetAutoControlFocus());
                    }

                    OString sIdent = xContextMenu->popup_at_rect(m_xTreeView.get(), tools::Rectangle(ptWhere, ::Size(1, 1)));
#if 0
                    if (xConversionMenu)
                        sIdent = xConversionMenu->GetCurItemIdent();
                    if (sIdent.isEmpty())
                        sIdent = pSubMenuNew->GetCurItemIdent();
                    if (sIdent.isEmpty())
                        sIdent = aContextMenu->GetCurItemIdent();
#endif
                    if (sIdent == "form")
                    {
                        OUString aStr(SvxResId(RID_STR_FORM));
                        OUString aUndoStr = SvxResId(RID_STR_UNDO_CONTAINER_INSERT).replaceAll("#", aStr);

                        pFormModel->BegUndo(aUndoStr);
                        // slot was only available, if there is only one selected entry,
                        // which is a root or a form
                        const std::unique_ptr<weld::TreeIter>& rIter = *m_arrCurrentSelection.begin();
                        NewForm(*rIter);
                        pFormModel->EndUndo();
                    }
                    else if (sIdent == "hidden")
                    {
                        OUString aStr(SvxResId(RID_STR_CONTROL));
                        OUString aUndoStr = SvxResId(RID_STR_UNDO_CONTAINER_INSERT).replaceAll("#", aStr);

                        pFormModel->BegUndo(aUndoStr);
                        // slot was valid for (exactly) one selected form
                        const std::unique_ptr<weld::TreeIter>& rIter = *m_arrCurrentSelection.begin();
                        NewControl(FM_COMPONENT_HIDDEN, *rIter, true);
                        pFormModel->EndUndo();
                    }
#if 0
                    else if (sIdent == "cut")
                        doCut();
                    else if (sIdent == "copy")
                        doCopy();
                    else if (sIdent == "paste")
                        doPaste();
#endif
                    else if (sIdent == "delete")
                        DeleteSelection();
                    else if (sIdent == "taborder")
                    {
                        // this slot was effective for exactly one selected form
                        const std::unique_ptr<weld::TreeIter>& rSelectedForm = *m_arrCurrentSelection.begin();
                        DBG_ASSERT( IsFormEntry(*rSelectedForm), "NavigatorTree::Command: This entry must be a FormEntry." );

                        FmFormData* pFormData = reinterpret_cast<FmFormData*>(m_xTreeView->get_id(*rSelectedForm).toInt64());
                        const Reference< XForm >&  xForm(  pFormData->GetFormIface());

                        Reference< XTabControllerModel >  xTabController(xForm, UNO_QUERY);
                        if( !xTabController.is() )
                            break;
                        GetNavModel()->GetFormShell()->GetImpl()->ExecuteTabOrderDialog_Lock(xTabController);
                    }
                    else if (sIdent == "props")
                        ShowSelectionProperties(true);
                    else if (sIdent == "rename")
                    {
                        // only allowed for one no-root-entry
                        const std::unique_ptr<weld::TreeIter>& rIter = *m_arrCurrentSelection.begin();
                        m_xTreeView->start_editing(*rIter);
                        m_bEditing = true;
                    }
                    else if (sIdent == "designmode")
                    {
                        pFormModel->SetOpenInDesignMode( !pFormModel->GetOpenInDesignMode() );
                        pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_OPEN_READONLY);
                    }
                    else if (sIdent == "controlfocus")
                    {
                        pFormModel->SetAutoControlFocus( !pFormModel->GetAutoControlFocus() );
                        pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_AUTOCONTROLFOCUS);
                    }
                    else if (FmXFormShell::isControlConversionSlot(sIdent))
                    {
                        const std::unique_ptr<weld::TreeIter>& rIter = *m_arrCurrentSelection.begin();
                        FmControlData* pCurrent = reinterpret_cast<FmControlData*>(m_xTreeView->get_id(*rIter).toInt64());
                        if (pFormShell->GetImpl()->executeControlConversionSlot_Lock(pCurrent->GetFormComponent(), sIdent))
                            ShowSelectionProperties();
                    }
                }
                bHandled = true;
            }
            break;
            default: break;
        }

        return bHandled;
    }

    std::unique_ptr<weld::TreeIter> NavigatorTree::FindEntry(FmEntryData* pEntryData)
    {
        std::unique_ptr<weld::TreeIter> xRet;
        if(!pEntryData)
            return xRet;

        m_xTreeView->all_foreach([this, pEntryData, &xRet](weld::TreeIter& rEntry){
            FmEntryData* pCurEntryData = reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(rEntry).toInt64());
            if (pCurEntryData && pCurEntryData->IsEqualWithoutChildren(pEntryData))
            {
                xRet = m_xTreeView->make_iterator(&rEntry);
                return true;
            }
            return false;
        });

        return xRet;
    }

    void NavigatorTree::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
    {
        if( dynamic_cast<const FmNavRemovedHint*>(&rHint) )
        {
            const FmNavRemovedHint* pRemovedHint = static_cast<const FmNavRemovedHint*>(&rHint);
            FmEntryData* pEntryData = pRemovedHint->GetEntryData();
            Remove( pEntryData );
        }

        else if( dynamic_cast<const FmNavInsertedHint*>(&rHint) )
        {
            const FmNavInsertedHint* pInsertedHint = static_cast<const FmNavInsertedHint*>(&rHint);
            FmEntryData* pEntryData = pInsertedHint->GetEntryData();
            sal_uInt32 nRelPos = pInsertedHint->GetRelPos();
            Insert( pEntryData, nRelPos );
        }

        else if( dynamic_cast<const FmNavModelReplacedHint*>(&rHint) )
        {
            FmEntryData* pData = static_cast<const FmNavModelReplacedHint*>(&rHint)->GetEntryData();
            std::unique_ptr<weld::TreeIter> xEntry = FindEntry(pData);
            if (xEntry)
            {
                // reset image
                m_xTreeView->set_image(*xEntry, pData->GetNormalImage());
            }
        }

        else if( dynamic_cast<const FmNavNameChangedHint*>(&rHint) )
        {
            const FmNavNameChangedHint* pNameChangedHint = static_cast<const FmNavNameChangedHint*>(&rHint);
            std::unique_ptr<weld::TreeIter> xEntry = FindEntry(pNameChangedHint->GetEntryData());
            m_xTreeView->set_text(*xEntry, pNameChangedHint->GetNewName());
        }

        else if( dynamic_cast<const FmNavClearedHint*>(&rHint) )
        {
//TODO            SvTreeListBox::Clear();
            m_xTreeView->clear();

            // default-entry "Forms"
            OUString sText(SvxResId(RID_STR_FORMS));
            m_xRootEntry = m_xTreeView->make_iterator();
            m_xTreeView->insert(nullptr, -1, &sText, nullptr, nullptr, nullptr,
                                false, m_xRootEntry.get());
            m_xTreeView->set_image(*m_xRootEntry, RID_SVXBMP_FORMS);
        }
        else if (dynamic_cast<const FmNavRequestSelectHint*>(&rHint))
        {
            FmNavRequestSelectHint* pershHint = const_cast<FmNavRequestSelectHint*>(static_cast<const FmNavRequestSelectHint*>(&rHint));
            FmEntryDataArray& arredToSelect = pershHint->GetItems();
            SynchronizeSelection(arredToSelect);

            if (pershHint->IsMixedSelection())
                // in this case I deselect all, although the view had a mixed selection
                // during next selection, I must adapt the navigator to the view
                m_bPrevSelectionMixed = true;
        }
    }

    std::unique_ptr<weld::TreeIter> NavigatorTree::Insert(FmEntryData* pEntryData, int nRelPos)
    {
        // insert current entry
        std::unique_ptr<weld::TreeIter> xParentEntry = FindEntry( pEntryData->GetParent() );
        std::unique_ptr<weld::TreeIter> xNewEntry(m_xTreeView->make_iterator());
        OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pEntryData)));

        if(!xParentEntry)
        {
            m_xTreeView->insert(m_xRootEntry.get(), nRelPos, &pEntryData->GetText(), &sId,
                                nullptr, nullptr, false, xNewEntry.get());
        }
        else
        {
            m_xTreeView->insert(xParentEntry.get(), nRelPos, &pEntryData->GetText(), &sId,
                                nullptr, nullptr, false, xNewEntry.get());
        }

        m_xTreeView->set_image(*xNewEntry, pEntryData->GetNormalImage());

        // If root-entry, expand root
        if (!xParentEntry)
            m_xTreeView->expand_row(*m_xRootEntry);

        // insert children
        FmEntryDataList* pChildList = pEntryData->GetChildList();
        size_t nChildCount = pChildList->size();
        for( size_t i = 0; i < nChildCount; i++ )
        {
            FmEntryData* pChildData = pChildList->at( i );
            Insert(pChildData, -1);
        }

        return xNewEntry;
    }

    void NavigatorTree::Remove( FmEntryData* pEntryData )
    {
        if( !pEntryData )
            return;

        // entry for the data
        std::unique_ptr<weld::TreeIter> xEntry = FindEntry(pEntryData);
        if (!xEntry)
            return;

        // delete entry from TreeListBox
        // I'm not allowed, to treat the selection, which I trigger:
        // select changes the MarkList of the view, if somebody else does this at the same time
        // and removes a selection, we get a problem
        // e.g. Group controls with open navigator
        LockSelectionHandling();

        // little problem: I remember the selected data, but if somebody deletes one of these entries,
        // I get inconsistent... this would be bad
        m_xTreeView->unselect(*xEntry);

        // selection can be modified during deletion,
        // but because I disabled SelectionHandling, I have to do it later
        auto nExpectedSelectionCount = m_xTreeView->count_selected_rows();

//TODO        GetModel()->Remove(pEntry);
        m_xTreeView->remove(*xEntry);

        if (nExpectedSelectionCount != m_xTreeView->count_selected_rows())
            SynchronizeSelection();

        // by default I treat the selection of course
        UnlockSelectionHandling();
    }

    bool NavigatorTree::IsFormEntry(const weld::TreeIter& rEntry)
    {
        FmEntryData* pEntryData = reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(rEntry).toInt64());
        return !pEntryData || dynamic_cast<const FmFormData*>( pEntryData) !=  nullptr;
    }

    bool NavigatorTree::IsFormComponentEntry(const weld::TreeIter& rEntry)
    {
        FmEntryData* pEntryData = reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(rEntry).toInt64());
        return dynamic_cast<const FmControlData*>( pEntryData) != nullptr;
    }

#if 0
    bool NavigatorTree::implAcceptPaste( )
    {
        SvTreeListEntry* pFirstSelected = FirstSelected();
        if ( !pFirstSelected || NextSelected( pFirstSelected ) )
            // no selected entry, or at least two selected entries
            return false;

        // get the clipboard
        TransferableDataHelper aClipboardContent( TransferableDataHelper::CreateFromSystemClipboard( this ) );

        sal_Int8 nAction = m_aControlExchange.isClipboardOwner() && doingKeyboardCut( ) ? DND_ACTION_MOVE : DND_ACTION_COPY;
        return ( nAction == implAcceptDataTransfer( aClipboardContent.GetDataFlavorExVector(), nAction, pFirstSelected, false ) );
    }
#endif

#if 0
    sal_Int8 NavigatorTree::implAcceptDataTransfer( const DataFlavorExVector& _rFlavors, sal_Int8 _nAction, SvTreeListEntry* _pTargetEntry, bool _bDnD )
    {
        // no target -> no drop
        if (!_pTargetEntry)
            return DND_ACTION_NONE;

        // format check
        bool bHasDefControlFormat = OControlExchange::hasFieldExchangeFormat( _rFlavors );
        bool bHasControlPathFormat = OControlExchange::hasControlPathFormat( _rFlavors );
        bool bHasHiddenControlsFormat = OControlExchange::hasHiddenControlModelsFormat( _rFlavors );
        if (!bHasDefControlFormat && !bHasControlPathFormat && !bHasHiddenControlsFormat)
            return DND_ACTION_NONE;

        bool bSelfSource = _bDnD ? m_aControlExchange.isDragSource() : m_aControlExchange.isClipboardOwner();

        if ( bHasHiddenControlsFormat )
        {   // bHasHiddenControlsFormat means that only hidden controls are part of the data

            // hidden controls can be copied to a form only
            if ((_pTargetEntry == m_xRootEntry) || !IsFormEntry(_pTargetEntry))
                return DND_ACTION_NONE;

            return bSelfSource ? ( DND_ACTION_COPYMOVE & _nAction ) : DND_ACTION_COPY;
        }

        if  ( !bSelfSource )
        {
            // DnD or CnP crossing navigator boundaries
            // The main problem here is that the current API does not allow us to sneak into the content which
            // is to be inserted. So we have to allow it for the moment, but maybe reject later on (in the real drop).

            // TODO: this smart behaviour later on ... at the moment, we disallow data transfer crossing navigator
            // boundaries.

            return DND_ACTION_NONE;
        }

        DBG_ASSERT( _bDnD ? m_aControlExchange.isDragSource() : m_aControlExchange.isClipboardOwner(),
            "NavigatorTree::implAcceptDataTransfer: here only with source=dest!" );
            // somebody changed the logic of this method ...

        // from here on, I can work with m_aControlExchange instead of _rData!

        bool bForeignCollection = m_aControlExchange->getFormsRoot().get() != GetNavModel()->GetFormPage()->GetForms().get();
        if ( bForeignCollection )
        {
            // crossing shell/page boundaries, we can exchange hidden controls only
            // But if we survived the checks above, we do not have hidden controls.
            // -> no data transfer
            DBG_ASSERT( !bHasHiddenControlsFormat, "NavigatorTree::implAcceptDataTransfer: still hidden controls format!" );
                // somebody changed the logic of this method ...

            return DND_ACTION_COPY;
        }

        if (DND_ACTION_MOVE != _nAction) // 'normal' controls within a shell are moved only (never copied)
            return DND_ACTION_NONE;

        if ( m_bDragDataDirty || !bHasDefControlFormat )
        {
            if (!bHasControlPathFormat)
                // I am in the shell/page, which has the controls, but I have no format,
                // which survived the shell change (SVX_FM_CONTROLS_AS_PATH)
                return DND_ACTION_NONE;

            // I must recreate the list of the ExchangeObjects, because the shell was changed during dragging
            // (there are SvLBoxEntries in it, and we lost them during change)
            m_aControlExchange->buildListFromPath(this, m_xRootEntry);
            m_bDragDataDirty = false;
        }

        // List of dropped entries from DragServer
        const ListBoxEntrySet& aDropped = m_aControlExchange->selected();
        DBG_ASSERT(!aDropped.empty(), "NavigatorTree::implAcceptDataTransfer: no entries !");

        bool bDropTargetIsComponent = IsFormComponentEntry( _pTargetEntry );
        //SvTreeListEntry* pDropTargetParent = GetParent( _pTargetEntry );

        // conditions to disallow the drop
        // 0) the root entry is part of the list (can't DnD the root!)
        // 1) one of the dragged entries is to be dropped onto its own parent
        // 2) -               "       - is to be dropped onto itself
        // 3) -               "       - is a Form and to be dropped onto one of its descendants
        // 4) one of the entries is a control and to be dropped onto the root
        // 5) a control or form will be dropped onto a control which is _not_ a sibling (dropping onto a sibling
        //      means moving the control)

        // collect the ancestors of the drop target (speeds up 3)
        SvLBoxEntrySortedArray arrDropAnchestors;
        SvTreeListEntry* pLoop = _pTargetEntry;
        while (pLoop)
        {
            arrDropAnchestors.insert(pLoop);
            pLoop = GetParent(pLoop);
        }

        for (SvTreeListEntry* pCurrent : aDropped)
        {
            SvTreeListEntry* pCurrentParent = GetParent(pCurrent);

            // test for 0)
            if (pCurrent == m_xRootEntry)
                return DND_ACTION_NONE;

            // test for 1)
            if ( _pTargetEntry == pCurrentParent )
                return DND_ACTION_NONE;

            // test for 2)
            if (pCurrent == _pTargetEntry)
                return DND_ACTION_NONE;

            // test for 5)
    //      if ( bDropTargetIsComponent && (pDropTargetParent != pCurrentParent) )
            if ( bDropTargetIsComponent )   // TODO : the line above can be inserted, if ExecuteDrop can handle inversion
                return DND_ACTION_NONE;

            // test for 3)
            if ( IsFormEntry(pCurrent) )
            {
                if ( arrDropAnchestors.find(pCurrent) != arrDropAnchestors.end() )
                    return DND_ACTION_NONE;
            } else if ( IsFormComponentEntry(pCurrent) )
            {
                // test for 4)
                if (_pTargetEntry == m_xRootEntry)
                    return DND_ACTION_NONE;
            }
        }
        return DND_ACTION_MOVE;
    }
#endif

#if 0
    sal_Int8 NavigatorTree::AcceptDrop( const AcceptDropEvent& rEvt )
    {
        ::Point aDropPos = rEvt.maPosPixel;

        // first handle possible DropActions (Scroll and swing open)
        if (rEvt.mbLeaving)
        {
            if (m_aDropActionTimer.IsActive())
                m_aDropActionTimer.Stop();
        } else
        {
            bool bNeedTrigger = false;
            // on the first entry ?
            if ((aDropPos.Y() >= 0) && (aDropPos.Y() < GetEntryHeight()))
            {
                m_aDropActionType = DA_SCROLLUP;
                bNeedTrigger = true;
            } else
                // on the last one (respectively the area, an entry would tale, if it flush with the bottom ?
                if ((aDropPos.Y() < GetSizePixel().Height()) && (aDropPos.Y() >= GetSizePixel().Height() - GetEntryHeight()))
                {
                    m_aDropActionType = DA_SCROLLDOWN;
                    bNeedTrigger = true;
                } else
                {   // on an entry with children, not swang open
                    SvTreeListEntry* pDroppedOn = GetEntry(aDropPos);
                    if (pDroppedOn && (GetChildCount(pDroppedOn) > 0) && !IsExpanded(pDroppedOn))
                    {
                        // -> swing open
                        m_aDropActionType = DA_EXPANDNODE;
                        bNeedTrigger = true;
                    }
                }

            if (bNeedTrigger && (m_aTimerTriggered != aDropPos))
            {
                // restart counting
                m_aTimerCounter = DROP_ACTION_TIMER_INITIAL_TICKS;
                // remember pos, because I get AcceptDrops, although mouse hasn't moved
                m_aTimerTriggered = aDropPos;
                // start Timer
                if (!m_aDropActionTimer.IsActive()) // exist Timer?
                {
                    m_aDropActionTimer.SetTimeout(DROP_ACTION_TIMER_TICK_BASE);
                    m_aDropActionTimer.Start();
                }
            } else if (!bNeedTrigger)
                m_aDropActionTimer.Stop();
        }

        return implAcceptDataTransfer( GetDataFlavorExVector(), rEvt.mnAction, GetEntry( aDropPos ), true );
    }
#endif

#if 0
    sal_Int8 NavigatorTree::implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, const ::Point& _rDropPos, bool _bDnD )
    {
        std::unique_ptr<weld::TreeIter> xDrop(m_xTreeView->make_iterator());
        if (!m_xTreeView->get_dest_row_at_pos(_rDropPos, xDrop.get(), false))
            xDrop.reset();
        return implExecuteDataTransfer( _rData, _nAction, xDrop.get(), _bDnD );
    }
#endif

#if 0
    sal_Int8 NavigatorTree::implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, weld::TreeIter* _pTargetEntry, bool _bDnD )
    {
        const DataFlavorExVector& rDataFlavors = _rData.GetDataFlavorExVector();

        if ( DND_ACTION_NONE == implAcceptDataTransfer( rDataFlavors, _nAction, _pTargetEntry, _bDnD ) )
            // under some platforms, it may happen that ExecuteDrop is called though AcceptDrop returned DND_ACTION_NONE
            return DND_ACTION_NONE;

        // would be bad, if we scroll after drop
        if (m_aDropActionTimer.IsActive())
            m_aDropActionTimer.Stop();

        if (!_pTargetEntry)
            // no target -> no drop
            return DND_ACTION_NONE;

        // format checks
#ifdef DBG_UTIL
        bool bHasHiddenControlsFormat = OControlExchange::hasHiddenControlModelsFormat( rDataFlavors );
        bool bForeignCollection = _rData.getFormsRoot().get() != GetNavModel()->GetFormPage()->GetForms().get();
        DBG_ASSERT(!bForeignCollection || bHasHiddenControlsFormat, "NavigatorTree::implExecuteDataTransfer: invalid format (AcceptDrop shouldn't have let this pass) !");
        DBG_ASSERT(bForeignCollection || !m_bDragDataDirty, "NavigatorTree::implExecuteDataTransfer: invalid state (shell changed since last exchange resync) !");
            // this should be done in AcceptDrop: the list of controls is created in _rData
            // and m_bDragDataDirty is reset
#endif

        if ( DND_ACTION_COPY == _nAction )
        {   // bHasHiddenControlsFormat means that only hidden controls are part of the data
#ifdef DBG_UTIL
            DBG_ASSERT( bHasHiddenControlsFormat, "NavigatorTree::implExecuteDataTransfer: copy allowed for hidden controls only!" );
#endif
            DBG_ASSERT( _pTargetEntry && ( _pTargetEntry != m_xRootEntry ) && IsFormEntry( _pTargetEntry ),
                "NavigatorTree::implExecuteDataTransfer: should not be here!" );
                // implAcceptDataTransfer should have caught both cases

#ifdef DBG_UTIL
            DBG_ASSERT(bHasHiddenControlsFormat, "NavigatorTree::implExecuteDataTransfer: only copying of hidden controls is supported !");
                // should be caught by AcceptDrop
#endif

            // because i want to select all targets (and only them)
            SelectAll(false);

            const Sequence< Reference< XInterface > >& aControls = _rData.hiddenControls();
            sal_Int32 nCount = aControls.getLength();
            const Reference< XInterface >* pControls = aControls.getConstArray();

            FmFormShell* pFormShell = GetNavModel()->GetFormShell();
            FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : nullptr;

            // within undo
            if (pFormModel)
            {
                OUString aStr(SvxResId(RID_STR_CONTROL));
                OUString aUndoStr = SvxResId(RID_STR_UNDO_CONTAINER_INSERT).replaceAll("#", aStr);
                pFormModel->BegUndo(aUndoStr);
            }

            // copy controls
            for (sal_Int32 i=0; i<nCount; ++i)
            {
                // create new control
                FmControlData* pNewControlData = NewControl( FM_COMPONENT_HIDDEN, _pTargetEntry, false);
                Reference< XPropertySet >  xNewPropSet( pNewControlData->GetPropertySet() );

                // copy properties form old control to new one
                Reference< XPropertySet >  xCurrent(pControls[i], UNO_QUERY);
#if (OSL_DEBUG_LEVEL > 0)
                // check whether it is a hidden control
                sal_Int16 nClassId = ::comphelper::getINT16(xCurrent->getPropertyValue(FM_PROP_CLASSID));
                OSL_ENSURE(nClassId == FormComponentType::HIDDENCONTROL, "NavigatorTree::implExecuteDataTransfer: invalid control in drop list !");
                    // if SVX_FM_HIDDEN_CONTROLS-format exists, the sequence
                    // should only contain hidden controls
#endif // (OSL_DEBUG_LEVEL > 0)
                Reference< XPropertySetInfo >  xPropInfo( xCurrent->getPropertySetInfo());
                const Sequence< Property> seqAllCurrentProps = xPropInfo->getProperties();
                for (Property const & currentProp : seqAllCurrentProps)
                {
                    if (((currentProp.Attributes & PropertyAttribute::READONLY) == 0) && (currentProp.Name != FM_PROP_NAME))
                    {   // (read-only attribs aren't set, ditto name,
                        // NewControl defined it uniquely
                        xNewPropSet->setPropertyValue(currentProp.Name, xCurrent->getPropertyValue(currentProp.Name));
                    }
                }

                SvTreeListEntry* pToSelect = FindEntry(pNewControlData);
                Select(pToSelect);
                if (i == 0)
                    SetCurEntry(pToSelect);
            }

            if (pFormModel)
                pFormModel->EndUndo();

            return _nAction;
        }

        if ( !OControlExchange::hasFieldExchangeFormat( _rData.GetDataFlavorExVector() ) )
        {
            // can't do anything without the internal format here ... usually happens when doing DnD or CnP
            // over navigator boundaries
            return DND_ACTION_NONE;
        }

        // some data for the target
        bool bDropTargetIsForm = IsFormEntry(_pTargetEntry);
        FmFormData* pTargetData = bDropTargetIsForm ? static_cast<FmFormData*>(_pTargetEntry->GetUserData()) : nullptr;

        DBG_ASSERT( DND_ACTION_COPY != _nAction, "NavigatorTree::implExecuteDataTransfer: somebody changed the logics!" );

        // list of dragged entries
        const ListBoxEntrySet aDropped = _rData.selected();
        DBG_ASSERT(!aDropped.empty(), "NavigatorTree::implExecuteDataTransfer: no entries!");

        // shell and model
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : nullptr;
        if (!pFormModel)
            return DND_ACTION_NONE;

        // for Undo
        const bool bUndo = pFormModel->IsUndoEnabled();

        if( bUndo )
        {
            OUString strUndoDescription(SvxResId(RID_STR_UNDO_CONTAINER_REPLACE));
            pFormModel->BegUndo(strUndoDescription);
        }

        // remove selection before adding an entry, so the mark doesn't flicker
        // -> lock action of selection
        LockSelectionHandling();

        // go through all dropped entries
        for (   ListBoxEntrySet::const_iterator dropped = aDropped.begin();
                dropped != aDropped.end();
                ++dropped
            )
        {
            // some data of the current element
            SvTreeListEntry* pCurrent = *dropped;
            DBG_ASSERT(pCurrent != nullptr, "NavigatorTree::implExecuteDataTransfer: invalid entry");
            DBG_ASSERT(GetParent(pCurrent) != nullptr, "NavigatorTree::implExecuteDataTransfer: invalid entry");
                // don't drag root

            FmEntryData* pCurrentUserData = static_cast<FmEntryData*>(pCurrent->GetUserData());

            Reference< XChild >  xCurrentChild = pCurrentUserData->GetChildIFace();
            Reference< XIndexContainer >  xContainer(xCurrentChild->getParent(), UNO_QUERY);

            FmFormData* pCurrentParentUserData = static_cast<FmFormData*>(pCurrentUserData->GetParent());
            DBG_ASSERT(pCurrentParentUserData == nullptr || dynamic_cast<const FmFormData*>(pCurrentUserData->GetParent()) !=  nullptr, "NavigatorTree::implExecuteDataTransfer: invalid parent");

            // remove from parent
            if (pCurrentParentUserData)
                pCurrentParentUserData->GetChildList()->removeNoDelete( pCurrentUserData );
            else
                GetNavModel()->GetRootList()->removeNoDelete( pCurrentUserData );

            // remove from container
            sal_Int32 nIndex = getElementPos(xContainer, xCurrentChild);
            GetNavModel()->m_pPropChangeList->Lock();
            // UndoAction for removal
            if ( bUndo && GetNavModel()->m_pPropChangeList->CanUndo())
            {
                pFormModel->AddUndo(std::make_unique<FmUndoContainerAction>(*pFormModel, FmUndoContainerAction::Removed,
                                                            xContainer, xCurrentChild, nIndex));
            }
            else if( !GetNavModel()->m_pPropChangeList->CanUndo() )
            {
                FmUndoContainerAction::DisposeElement( xCurrentChild );
            }

            // copy events
            Reference< XEventAttacherManager >  xManager(xContainer, UNO_QUERY);
            Sequence< ScriptEventDescriptor > aEvts;

            if (xManager.is() && nIndex >= 0)
                aEvts = xManager->getScriptEvents(nIndex);
            xContainer->removeByIndex(nIndex);

            // remove selection
            Select(pCurrent, false);
            // and delete it
            Remove(pCurrentUserData);

            // position in DropParents, where to insert dropped entries
            if (pTargetData)
                xContainer.set(pTargetData->GetElement(), UNO_QUERY);
            else
                xContainer = GetNavModel()->GetForms();

            // always insert at the end
            nIndex = xContainer->getCount();

            // UndoAction for insertion
            if ( bUndo && GetNavModel()->m_pPropChangeList->CanUndo())
                pFormModel->AddUndo(std::make_unique<FmUndoContainerAction>(*pFormModel, FmUndoContainerAction::Inserted,
                                                         xContainer, xCurrentChild, nIndex));

            // insert in new container
            if (pTargetData)
            {
                 // insert in a form needs a FormComponent
                xContainer->insertByIndex( nIndex,
                    makeAny( Reference< XFormComponent >( xCurrentChild, UNO_QUERY ) ) );
            }
            else
            {
                xContainer->insertByIndex( nIndex,
                    makeAny( Reference< XForm >( xCurrentChild, UNO_QUERY ) ) );
            }

            if (aEvts.hasElements())
            {
                xManager.set(xContainer, UNO_QUERY);
                if (xManager.is())
                    xManager->registerScriptEvents(nIndex, aEvts);
            }

            GetNavModel()->m_pPropChangeList->UnLock();

            // give an entry the new parent
            pCurrentUserData->SetParent(pTargetData);

            // give parent the new child
            if (pTargetData)
                pTargetData->GetChildList()->insert( std::unique_ptr<FmEntryData>(pCurrentUserData), nIndex );
            else
                GetNavModel()->GetRootList()->insert( std::unique_ptr<FmEntryData>(pCurrentUserData), nIndex );

            // announce to myself and reselect
            SvTreeListEntry* pNew = Insert( pCurrentUserData, nIndex );
            if ( ( aDropped.begin() == dropped ) && pNew )
            {
                SvTreeListEntry* pParent = GetParent( pNew );
                if ( pParent )
                    Expand( pParent );
            }
        }

        UnlockSelectionHandling();

        if( bUndo )
            pFormModel->EndUndo();

        // During the move, the markings of the underlying view did not change (because the view is not affected by the logical
        // hierarchy of the form/control models. But my selection changed - which means I have to adjust it according to the
        // view marks, again.
        SynchronizeSelection();

        // in addition, with the move of controls such things as "the current form" may have changed - force the shell
        // to update itself accordingly
        if( pFormShell && pFormShell->GetImpl() && pFormShell->GetFormView() )
            pFormShell->GetImpl()->DetermineSelection_Lock( pFormShell->GetFormView()->GetMarkedObjectList() );

        if ( m_aControlExchange.isClipboardOwner() && ( DND_ACTION_MOVE == _nAction ) )
            m_aControlExchange->clear();

        return _nAction;
    }
#endif

#if 0
    sal_Int8 NavigatorTree::ExecuteDrop( const ExecuteDropEvent& rEvt )
    {
        sal_Int8 nResult( DND_ACTION_NONE );
        if ( m_aControlExchange.isDragSource() )
            nResult = implExecuteDataTransfer( *m_aControlExchange, rEvt.mnAction, rEvt.maPosPixel, true );
        else
        {
            OControlTransferData aDroppedData( rEvt.maDropEvent.Transferable );
            nResult = implExecuteDataTransfer( aDroppedData, rEvt.mnAction, rEvt.maPosPixel, true );
        }
        return nResult;
    }
#endif

#if 0
    void NavigatorTree::doPaste()
    {
        try
        {
            if ( m_aControlExchange.isClipboardOwner() )
            {
                implExecuteDataTransfer( *m_aControlExchange, doingKeyboardCut( ) ? DND_ACTION_MOVE : DND_ACTION_COPY, FirstSelected(), false );
            }
            else
            {
                // the clipboard content
                Reference< XClipboard > xClipboard( GetClipboard() );
                Reference< XTransferable > xTransferable;
                if ( xClipboard.is() )
                    xTransferable = xClipboard->getContents();

                OControlTransferData aClipboardContent( xTransferable );
                implExecuteDataTransfer( aClipboardContent, DND_ACTION_COPY, FirstSelected(), false );
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "svx", "NavigatorTree::doPaste" );
        }
    }
#endif

#if 0
    void NavigatorTree::doCopy()
    {
        if ( implPrepareExchange( DND_ACTION_COPY ) )
        {
            m_aControlExchange.setClipboardListener( LINK( this, NavigatorTree, OnClipboardAction ) );
            m_aControlExchange.copyToClipboard( );
        }
    }
#endif

#if 0
    void NavigatorTree::ModelHasRemoved( SvTreeListEntry* _pEntry )
    {
        SvTreeListEntry* pTypedEntry = _pEntry;
        if ( doingKeyboardCut() )
            m_aCutEntries.erase( pTypedEntry );

        if ( m_aControlExchange.isDataExchangeActive() )
        {
            if ( 0 == m_aControlExchange->onEntryRemoved( pTypedEntry ) )
            {
                // last of the entries which we put into the clipboard has been deleted from the tree.
                // Give up the clipboard ownership.
                m_aControlExchange.clear();
            }
        }
    }
#endif

#if 0
    void NavigatorTree::doCut()
    {
        if ( implPrepareExchange( DND_ACTION_MOVE ) )
        {
            m_aControlExchange.setClipboardListener( LINK( this, NavigatorTree, OnClipboardAction ) );
            m_aControlExchange.copyToClipboard( );
            m_bKeyboardCut = true;

            // mark all the entries we just "cut" into the clipboard as "nearly moved"
            for ( SvTreeListEntry* pEntry : m_arrCurrentSelection )
            {
                if ( pEntry )
                {
                    m_aCutEntries.insert( pEntry );
                    pEntry->SetFlags( pEntry->GetFlags() | SvTLEntryFlags::SEMITRANSPARENT );
                    InvalidateEntry( pEntry );
                }
            }
        }
    }
#endif

    IMPL_LINK(NavigatorTree, KeyInputHdl, const ::KeyEvent&, rKEvt, bool)
    {
        const vcl::KeyCode& rCode = rKEvt.GetKeyCode();

        // delete?
        if (rCode.GetCode() == KEY_DELETE && !rCode.GetModifier())
        {
            DeleteSelection();
            return true;
        }

#if 0
        // copy'n'paste?
        switch ( rCode.GetFunction() )
        {
            case KeyFuncType::CUT:
                doCut();
                break;

            case KeyFuncType::PASTE:
                if ( implAcceptPaste() )
                    doPaste();
                break;

            case KeyFuncType::COPY:
                doCopy();
                break;

            default:
                break;
        }
#endif

        return false;
    }

    IMPL_LINK(NavigatorTree, EditingEntryHdl, const weld::TreeIter&, rIter, bool)
    {
        // root, which isn't allowed to be renamed, has UserData=NULL
        m_bEditing = !m_xTreeView->get_id(rIter).isEmpty();
        return m_bEditing;
    }

    void NavigatorTree::NewForm(const weld::TreeIter& rParentEntry)
    {
        // get ParentFormData
        if (!IsFormEntry(rParentEntry))
            return;

        FmFormData* pParentFormData = reinterpret_cast<FmFormData*>(m_xTreeView->get_id(rParentEntry).toInt64());


        // create new form
        Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
        Reference< XForm >  xNewForm(xContext->getServiceManager()->createInstanceWithContext(FM_SUN_COMPONENT_FORM, xContext), UNO_QUERY);
        if (!xNewForm.is())
            return;

        Reference< XPropertySet >  xPropertySet(xNewForm, UNO_QUERY);
        if (!xPropertySet.is())
            return;

        FmFormData* pNewFormData = new FmFormData(xNewForm, pParentFormData);


        // set name
        OUString aName = GenerateName(pNewFormData);
        pNewFormData->SetText(aName);

        try
        {
            xPropertySet->setPropertyValue( FM_PROP_NAME, makeAny(aName) );
            // a form should always have the command type table as default
            xPropertySet->setPropertyValue( FM_PROP_COMMANDTYPE, makeAny(sal_Int32(CommandType::TABLE)));
        }
        catch ( const Exception& )
        {
            OSL_FAIL("NavigatorTree::NewForm : could not set essential properties!");
        }


        // insert form
        GetNavModel()->Insert(pNewFormData, SAL_MAX_UINT32, true);


        // set new form as active
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( pFormShell )
        {
            InterfaceBag aSelection;
            aSelection.insert( Reference<XInterface>( xNewForm, UNO_QUERY ) );
            pFormShell->GetImpl()->setCurrentSelection_Lock(aSelection);

            pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_PROPERTIES, true, true);
        }
        GetNavModel()->SetModified();

        // switch to EditMode
        std::unique_ptr<weld::TreeIter> xNewEntry = FindEntry(pNewFormData);
        m_xTreeView->start_editing(*xNewEntry);
        m_bEditing = true;
    }

    FmControlData* NavigatorTree::NewControl(const OUString& rServiceName, const weld::TreeIter& rParentEntry, bool bEditName)
    {
        // get ParentForm
        if (!GetNavModel()->GetFormShell())
            return nullptr;
        if (!IsFormEntry(rParentEntry))
            return nullptr;

        FmFormData* pParentFormData = reinterpret_cast<FmFormData*>(m_xTreeView->get_id(rParentEntry).toInt64());
        Reference<XForm>  xParentForm(pParentFormData->GetFormIface());

        // create new component
        Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
        Reference<XFormComponent> xNewComponent( xContext->getServiceManager()->createInstanceWithContext(rServiceName, xContext), UNO_QUERY);
        if (!xNewComponent.is())
            return nullptr;

        FmControlData* pNewFormControlData = new FmControlData(xNewComponent, pParentFormData);

        // set name
        OUString sName = FmFormPageImpl::setUniqueName( xNewComponent, xParentForm );

        pNewFormControlData->SetText( sName );

        // insert FormComponent
        GetNavModel()->Insert(pNewFormControlData, SAL_MAX_UINT32, true);
        GetNavModel()->SetModified();

        if (bEditName)
        {
            // switch to EditMode
            std::unique_ptr<weld::TreeIter> xNewEntry = FindEntry( pNewFormControlData );
            m_xTreeView->select(*xNewEntry);

            m_xTreeView->start_editing(*xNewEntry);
            m_bEditing = true;
        }

        return pNewFormControlData;
    }

    OUString NavigatorTree::GenerateName( FmEntryData const * pEntryData )
    {
        const sal_uInt16 nMaxCount = 99;
        OUString aNewName;

        // create base name
        OUString aBaseName;
        if( dynamic_cast<const FmFormData*>( pEntryData) !=  nullptr )
            aBaseName = SvxResId( RID_STR_STDFORMNAME );
        else if( dynamic_cast<const FmControlData*>( pEntryData) !=  nullptr )
            aBaseName = SvxResId( RID_STR_CONTROL );


        // create new name
        FmFormData* pFormParentData = static_cast<FmFormData*>(pEntryData->GetParent());

        for( sal_Int32 i=0; i<nMaxCount; i++ )
        {
            aNewName = aBaseName;
            if( i>0 )
            {
                aNewName += " " + OUString::number(i);
            }

            if( GetNavModel()->FindData(aNewName, pFormParentData,false) == nullptr )
                break;
        }

        return aNewName;
    }

    IMPL_LINK(NavigatorTree, EditedEntryHdl, const IterString&, rIterString, bool)
    {
        m_bEditing = false;

        const weld::TreeIter& rIter = rIterString.first;

        FmEntryData* pEntryData = reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(rIter).toInt64());
        bool bRes = NavigatorTreeModel::Rename(pEntryData, rIterString.second);
        if (!bRes)
        {
            m_xEditEntry = m_xTreeView->make_iterator(&rIter);
            nEditEvent = Application::PostUserEvent(LINK(this, NavigatorTree, OnEdit));
        }

        return bRes;
    }

    IMPL_LINK_NOARG(NavigatorTree, OnEdit, void*, void)
    {
        nEditEvent = nullptr;
        m_xTreeView->start_editing(*m_xEditEntry);
        m_bEditing = true;
        m_xEditEntry.reset();
    }

    IMPL_LINK_NOARG(NavigatorTree, OnDropActionTimer, Timer *, void)
    {
        if (--m_aTimerCounter > 0)
            return;

        switch ( m_aDropActionType )
        {
        case DA_EXPANDNODE:
        {
            std::unique_ptr<weld::TreeIter> xToExpand(m_xTreeView->make_iterator());
            if (!m_xTreeView->get_dest_row_at_pos(m_aTimerTriggered, xToExpand.get(), false))
                xToExpand.reset();
            if (xToExpand && m_xTreeView->iter_has_child(*xToExpand) && !m_xTreeView->get_row_expanded(*xToExpand))
            {
                // normally, we have to test, if the node is expanded,
                // but there is no method for this either in base class nor the model
                // the base class should tolerate it anyway
                m_xTreeView->expand_row(*xToExpand);
            }

            // After expansion there is nothing to do like after scrolling
            m_aDropActionTimer.Stop();
        }
        break;

        case DA_SCROLLUP :
//TODO            ScrollOutputArea( 1 );
            m_aTimerCounter = DROP_ACTION_TIMER_SCROLL_TICKS;
            break;

        case DA_SCROLLDOWN :
//TODO            ScrollOutputArea( -1 );
            m_aTimerCounter = DROP_ACTION_TIMER_SCROLL_TICKS;
            break;

        }
    }

    IMPL_LINK_NOARG(NavigatorTree, OnEntrySelDesel, weld::TreeView&, void)
    {
        m_sdiState = SDI_DIRTY;

        if (IsSelectionHandlingLocked())
            return;

        if (m_aSynchronizeTimer.IsActive())
            m_aSynchronizeTimer.Stop();

        m_aSynchronizeTimer.SetTimeout(EXPLORER_SYNC_DELAY);
        m_aSynchronizeTimer.Start();
    }

    IMPL_LINK_NOARG(NavigatorTree, OnSynchronizeTimer, Timer *, void)
    {
        SynchronizeMarkList();
    }


#if 0
    IMPL_LINK_NOARG(NavigatorTree, OnClipboardAction, OLocalExchange&, void)
    {
        if ( !m_aControlExchange.isClipboardOwner() )
        {
            if ( doingKeyboardCut() )
            {
                for (SvTreeListEntry* pEntry : m_aCutEntries)
                {
                    if ( !pEntry )
                        continue;

                    pEntry->SetFlags( pEntry->GetFlags() & ~SvTLEntryFlags::SEMITRANSPARENT );
                    InvalidateEntry( pEntry );
                }
                ListBoxEntrySet aEmpty;
                m_aCutEntries.swap( aEmpty );

                m_bKeyboardCut = false;
            }
        }
    }
#endif

    void NavigatorTree::ShowSelectionProperties(bool bForce)
    {
        // at first i need the FormShell
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell)
            // no shell -> impossible to set curObject -> leave
            return;

        CollectSelectionData(SDI_ALL);
        SAL_WARN_IF(static_cast<size_t>(m_nFormsSelected + m_nControlsSelected
                + (m_bRootSelected ? 1 : 0)) != m_arrCurrentSelection.size(),
            "svx.form",
            "NavigatorTree::ShowSelectionProperties : selection meta data invalid !");


        InterfaceBag aSelection;
        bool bSetSelectionAsMarkList = false;

        if (m_bRootSelected)
            ;                                   // no properties for the root, neither for single nor for multi selection
        else if ( m_nFormsSelected + m_nControlsSelected == 0 )   // none of the two should be less 0
            ;                                   // no selection -> no properties
        else if ( m_nFormsSelected * m_nControlsSelected != 0 )
            ;                                   // mixed selection -> no properties
        else
        {   // either only forms, or only controls are selected
            if (m_arrCurrentSelection.size() == 1)
            {
                const std::unique_ptr<weld::TreeIter>& rIter = *m_arrCurrentSelection.begin();
                if (m_nFormsSelected > 0)
                {   // exactly one form is selected
                    FmFormData* pFormData = reinterpret_cast<FmFormData*>(m_xTreeView->get_id(*rIter).toInt64());
                    aSelection.insert( Reference< XInterface >( pFormData->GetFormIface(), UNO_QUERY ) );
                }
                else
                {   // exactly one control is selected (whatever hidden or normal)
                    FmEntryData* pEntryData = reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(*rIter).toInt64());

                    aSelection.insert( Reference< XInterface >( pEntryData->GetElement(), UNO_QUERY ) );
                }
            }
            else
            {   // it's a MultiSelection, so we must build a MultiSet
                if (m_nFormsSelected > 0)
                {   // ... only forms
                    // first of all collect PropertySet-Interfaces of the forms
                    SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
                    for ( sal_Int32 i = 0; i < m_nFormsSelected; ++i )
                    {
                        const std::unique_ptr<weld::TreeIter>& rIter = *it;
                        FmFormData* pFormData = reinterpret_cast<FmFormData*>(m_xTreeView->get_id(*rIter).toInt64());
                        aSelection.insert( pFormData->GetPropertySet().get() );
                        ++it;
                    }
                }
                else
                {   // ... only controls
                    if (m_nHiddenControls == m_nControlsSelected)
                    {   // a MultiSet for properties of hidden controls
                        SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
                        for ( sal_Int32 i = 0; i < m_nHiddenControls; ++i )
                        {
                            const std::unique_ptr<weld::TreeIter>& rIter = *it;
                            FmEntryData* pEntryData = reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(*rIter).toInt64());
                            aSelection.insert( pEntryData->GetPropertySet().get() );
                            ++it;
                        }
                    }
                    else if (m_nHiddenControls == 0)
                    {   // only normal controls
                        bSetSelectionAsMarkList = true;
                    }
                }
            }

        }

        // and now my form and my SelObject
        if ( bSetSelectionAsMarkList )
            pFormShell->GetImpl()->setCurrentSelectionFromMark_Lock(pFormShell->GetFormView()->GetMarkedObjectList());
        else
            pFormShell->GetImpl()->setCurrentSelection_Lock(aSelection);

        if (pFormShell->GetImpl()->IsPropBrwOpen_Lock() || bForce)
        {
            // and now deliver all to the PropertyBrowser
            pFormShell->GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_SHOW_PROPERTY_BROWSER, SfxCallMode::ASYNCHRON );
        }
    }


    void NavigatorTree::DeleteSelection()
    {
        // of course, i can't delete root
        bool bRootSelected = m_xTreeView->is_selected(*m_xRootEntry);
        auto nSelectedEntries = m_xTreeView->count_selected_rows();
        if (bRootSelected && (nSelectedEntries > 1))     // root and other elements ?
            m_xTreeView->unselect(*m_xRootEntry);                // yes -> remove root from selection

        if ((nSelectedEntries == 0) || bRootSelected)    // still root ?
            return;                                     // -> only selected element -> leave

        DBG_ASSERT(!m_bPrevSelectionMixed, "NavigatorTree::DeleteSelection() : delete permitted if mark and selection are inconsistent");

        // i need the FormModel later
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell)
            return;
        FmFormModel* pFormModel = pFormShell->GetFormModel();
        if (!pFormModel)
            return;

        // now I have to safeguard the DeleteList: if you delete a form and a dependent element
        // - in this order - than the SvLBoxEntryPtr of the dependent element is already invalid,
        // when it should be deleted... you have to prohibit this GPF, that of course would happen,
        // so I take the 'normalized' list
        CollectSelectionData( SDI_NORMALIZED );

        // see below for why we need this mapping from models to shapes
        FmFormView*     pFormView       = pFormShell->GetFormView();
        SdrPageView*    pPageView       = pFormView ? pFormView->GetSdrPageView() : nullptr;
        SdrPage*        pPage           = pPageView ? pPageView->GetPage() : nullptr;
        DBG_ASSERT( pPage, "NavigatorTree::DeleteSelection: invalid form page!" );

        MapModelToShape aModelShapes;
        if ( pPage )
            collectShapeModelMapping( pPage, aModelShapes );

        // problem: we have to use ExplorerModel::Remove, since only this one properly deletes Form objects.
        // But, the controls themself must be deleted via DeleteMarked (else, the Writer has some problems
        // somewhere). In case I'd first delete the structure, then the controls, the UNDO would not work
        // (since UNDO then would mean to first restore the controls, then the structure, means their parent
        // form). The other way round, the EntryDatas would be invalid, if I'd first delete the controls and
        // then go on to the structure. This means I have to delete the forms *after* the normal controls, so
        // that during UNDO, they're restored in the proper order.
        pFormShell->GetImpl()->EnableTrackProperties_Lock(false);
        for (SvLBoxEntrySortedArray::reverse_iterator it = m_arrCurrentSelection.rbegin();
             it != m_arrCurrentSelection.rend(); )
        {
            const std::unique_ptr<weld::TreeIter>& rIter = *it;
            FmEntryData* pCurrent = reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(*rIter).toInt64());

            // a form ?
            bool bIsForm = dynamic_cast<const FmFormData*>( pCurrent) !=  nullptr;

            // because deletion is done by the view, and i build on its MarkList,
            // but normally only direct controls, no indirect ones, are marked in a marked form,
            // I have to do it later
            if (bIsForm)
                MarkViewObj(static_cast<FmFormData*>(pCurrent), true/*deep*/);

            // a hidden control ?
            bool bIsHidden = IsHiddenControl(pCurrent);

            // keep forms and hidden controls, the rest not
            if (!bIsForm && !bIsHidden)
            {
                // well, no form and no hidden control -> we can remove it from m_arrCurrentSelection, as it will
                // be deleted automatically. This is because for every model (except forms and hidden control models)
                // there exist a shape, which is marked _if_and_only_if_ the model is selected in our tree.
                if ( aModelShapes.find( pCurrent->GetElement() ) != aModelShapes.end() )
                {
                    // if there's a shape for the current entry, then either it is marked or it is in a
                    // hidden layer (#i28502#), or something like this.
                    // In the first case, it will be deleted below, in the second case, we currently don't
                    // delete it, as there's no real (working!) API for this, neither in UNO nor in non-UNO.
                    m_arrCurrentSelection.erase( --(it.base()) );
                }
                else
                   ++it;
                // In case there is no shape for the current entry, we keep the entry in m_arrCurrentSelection,
                // since then we can definitely remove it.
            }
            else
                ++it;
        }
        pFormShell->GetImpl()->EnableTrackProperties_Lock(true);

        // let the view delete the marked controls
        pFormShell->GetFormView()->DeleteMarked();

        // start UNDO at this point. Unfortunately, this results in 2 UNDO actions, since DeleteMarked is
        // creating an own one. However, if we'd move it before DeleteMarked, Writer does not really like
        // this ... :(
        // #i31038#
        {

            // initialize UNDO
            OUString aUndoStr;
            if ( m_arrCurrentSelection.size() == 1 )
            {
                aUndoStr = SvxResId(RID_STR_UNDO_CONTAINER_REMOVE);
                if (m_nFormsSelected)
                    aUndoStr = aUndoStr.replaceFirst( "#", SvxResId( RID_STR_FORM ) );
                else
                    // it must be a control (else the root would be selected, but it cannot be deleted)
                    aUndoStr = aUndoStr.replaceFirst( "#", SvxResId( RID_STR_CONTROL ) );
            }
            else
            {
                aUndoStr = SvxResId(RID_STR_UNDO_CONTAINER_REMOVE_MULTIPLE);
                aUndoStr = aUndoStr.replaceFirst( "#", OUString::number( m_arrCurrentSelection.size() ) );
            }
            pFormModel->BegUndo(aUndoStr);
        }

        // remove remaining structure
        for (const auto& rpSelection : m_arrCurrentSelection)
        {
            FmEntryData* pCurrent = reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(*rpSelection).toInt64());

            // if the entry still has children, we skipped deletion of one of those children.
            // This may for instance be because the shape is in a hidden layer, where we're unable
            // to remove it
            if ( pCurrent->GetChildList()->size() )
                continue;

            // one remaining subtile problem, before deleting it : if it's a form and the shell
            // knows it as CurrentObject, I have to tell it something else
            if (dynamic_cast<const FmFormData*>( pCurrent) !=  nullptr)
            {
                Reference< XForm >  xCurrentForm( static_cast< FmFormData* >( pCurrent )->GetFormIface() );
                if (pFormShell->GetImpl()->getCurrentForm_Lock() == xCurrentForm)  // shell knows form to be deleted ?
                    pFormShell->GetImpl()->forgetCurrentForm_Lock();                 // -> take away ...
            }
            GetNavModel()->Remove(pCurrent, true);
        }
        pFormModel->EndUndo();
    }


    void NavigatorTree::CollectSelectionData(SELDATA_ITEMS sdiHow)
    {
        DBG_ASSERT(sdiHow != SDI_DIRTY, "NavigatorTree::CollectSelectionData : ever thought about your parameter ? DIRTY ?");
        if (sdiHow == m_sdiState)
            return;

        m_arrCurrentSelection.clear();
        m_nFormsSelected = m_nControlsSelected = m_nHiddenControls = 0;
        m_bRootSelected = false;

        m_xTreeView->selected_foreach([this, sdiHow](weld::TreeIter& rSelectionLoop){
            // count different elements
            if (m_xTreeView->iter_compare(rSelectionLoop, *m_xRootEntry) == 0)
                m_bRootSelected = true;
            else
            {
                if (IsFormEntry(rSelectionLoop))
                    ++m_nFormsSelected;
                else
                {
                    ++m_nControlsSelected;
                    if (IsHiddenControl(reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(rSelectionLoop).toInt64())))
                        ++m_nHiddenControls;
                }
            }

            if (sdiHow == SDI_NORMALIZED)
            {
                // don't take something with a selected ancestor
                if (m_xTreeView->iter_compare(rSelectionLoop, *m_xRootEntry) == 0)
                    m_arrCurrentSelection.emplace(m_xTreeView->make_iterator(&rSelectionLoop));
                else
                {
                    std::unique_ptr<weld::TreeIter> xParentLoop(m_xTreeView->make_iterator(&rSelectionLoop));
                    bool bParentLoop = m_xTreeView->iter_parent(*xParentLoop);
                    while (bParentLoop)
                    {
                        // actually i would have to test, if parent is part of m_arr_CurrentSelection ...
                        // but if it's selected, then it's in m_arrCurrentSelection
                        // or one of its ancestors, which was selected earlier.
                        // In both cases IsSelected is enough
                        if (m_xTreeView->is_selected(*xParentLoop))
                            break;
                        else
                        {
                            if (m_xTreeView->iter_compare(*xParentLoop, *m_xRootEntry) == 0)
                            {
                                // until root (exclusive), there was no selected parent -> entry belongs to normalized list
                                m_arrCurrentSelection.emplace(m_xTreeView->make_iterator(&rSelectionLoop));
                                break;
                            }
                            else
                                bParentLoop = m_xTreeView->iter_parent(*xParentLoop);
                        }
                    }
                }
            }
            else if (sdiHow == SDI_NORMALIZED_FORMARK)
            {
                std::unique_ptr<weld::TreeIter> xParent(m_xTreeView->make_iterator(&rSelectionLoop));
                bool bParent = m_xTreeView->iter_parent(*xParent);
                if (!bParent || !m_xTreeView->is_selected(*xParent) || IsFormEntry(rSelectionLoop))
                    m_arrCurrentSelection.emplace(m_xTreeView->make_iterator(&rSelectionLoop));
            }
            else
                m_arrCurrentSelection.emplace(m_xTreeView->make_iterator(&rSelectionLoop));

            return false;
        });

        m_sdiState = sdiHow;
    }

    void NavigatorTree::SynchronizeSelection(FmEntryDataArray& arredToSelect)
    {
        LockSelectionHandling();
        if (arredToSelect.empty())
        {
            m_xTreeView->unselect_all();
        }
        else
        {
            // compare current selection with requested SelectList
            m_xTreeView->selected_foreach([this, &arredToSelect](weld::TreeIter& rSelection) {
                FmEntryData* pCurrent = reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(rSelection).toInt64());
                if (pCurrent != nullptr)
                {
                    FmEntryDataArray::iterator it = arredToSelect.find(pCurrent);
                    if ( it != arredToSelect.end() )
                    {   // entry already selected, but also in SelectList
                        // remove it from there
                        arredToSelect.erase(it);
                    } else
                    {   // entry selected, but not in SelectList -> remove selection
                        m_xTreeView->unselect(rSelection);
                        // make it visible (maybe it's the only modification i do in this handler
                        // so you should see it
                        m_xTreeView->scroll_to_row(rSelection);
                    }
                }
                else
                    m_xTreeView->unselect(rSelection);

                return false;
            });

            // now SelectList contains only entries, which have to be selected
            // two possibilities : 1) run through SelectList, get SvTreeListEntry for every entry and select it (is more intuitive)
            // 2) run through my SvLBoxEntries and select those, i can find in the SelectList
            // 1) needs =(k*n) (k=length of SelectList, n=number of entries),
            // plus the fact, that FindEntry uses extensive IsEqualWithoutChilden instead of comparing pointer to UserData
            // 2) needs =(n*log k), duplicates some code from FindEntry
            // This may be a frequently used code ( at every change in mark of the view!),
            // so i use latter one
            m_xTreeView->all_foreach([this, &arredToSelect](weld::TreeIter& rLoop){
                FmEntryData* pCurEntryData = reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(rLoop).toInt64());
                FmEntryDataArray::iterator it = arredToSelect.find(pCurEntryData);
                if (it != arredToSelect.end())
                {
                    m_xTreeView->select(rLoop);
                    m_xTreeView->scroll_to_row(rLoop);
                    // TODO SetCursor(pLoop, true);
                }

                return false;
            });
        }
        UnlockSelectionHandling();
    }


    void NavigatorTree::SynchronizeSelection()
    {
        // shell and view
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if(!pFormShell) return;

        FmFormView* pFormView = pFormShell->GetFormView();
        if (!pFormView) return;

        GetNavModel()->BroadcastMarkedObjects(pFormView->GetMarkedObjectList());
    }


    void NavigatorTree::SynchronizeMarkList()
    {
        // i'll need this shell
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell) return;

        CollectSelectionData(SDI_NORMALIZED_FORMARK);

        // the view shouldn't notify now if MarkList changed
        pFormShell->GetImpl()->EnableTrackProperties_Lock(false);

        UnmarkAllViewObj();

        for (auto& rSelectionLoop : m_arrCurrentSelection)
        {
            // When form selection, mark all controls of form
            if (IsFormEntry(*rSelectionLoop) && m_xTreeView->iter_compare(*rSelectionLoop, *m_xRootEntry) != 0)
                MarkViewObj(reinterpret_cast<FmFormData*>(m_xTreeView->get_id(*rSelectionLoop).toInt64()), false/*deep*/);

            // When control selection, mark Control-SdrObjects
            else if (IsFormComponentEntry(*rSelectionLoop))
            {
                FmControlData* pControlData = reinterpret_cast<FmControlData*>(m_xTreeView->get_id(*rSelectionLoop).toInt64());
                if (pControlData)
                {

                    // When HiddenControl no object can be selected
                    Reference< XFormComponent >  xFormComponent( pControlData->GetFormComponent());
                    if (!xFormComponent.is())
                        continue;
                    Reference< XPropertySet >  xSet(xFormComponent, UNO_QUERY);
                    if (!xSet.is())
                        continue;

                    sal_uInt16 nClassId = ::comphelper::getINT16(xSet->getPropertyValue(FM_PROP_CLASSID));
                    if (nClassId != FormComponentType::HIDDENCONTROL)
                        MarkViewObj(pControlData);
                }
            }
        }

        // if PropertyBrowser is open, I have to adopt it according to my selection
        // (Not as MarkList of view : if a form is selected, all belonging controls are selected in the view
        // but of course i want to see the form-properties
        ShowSelectionProperties();

        // reset flag at view
        pFormShell->GetImpl()->EnableTrackProperties_Lock(true);

        // if exactly one form is selected now, shell should notice it as CurrentForm
        // (if selection handling isn't locked, view cares about it in MarkListHasChanged
        // but mechanism doesn't work, if form is empty for example
        if ((m_arrCurrentSelection.size() == 1) && (m_nFormsSelected == 1))
        {
            std::unique_ptr<weld::TreeIter> xSelected(m_xTreeView->make_iterator());
            m_xTreeView->get_selected(xSelected.get());
            FmFormData* pSingleSelectionData = dynamic_cast<FmFormData*>(reinterpret_cast<FmEntryData*>(m_xTreeView->get_id(*xSelected).toInt64()));
            DBG_ASSERT( pSingleSelectionData, "NavigatorTree::SynchronizeMarkList: invalid selected form!" );
            if ( pSingleSelectionData )
            {
                InterfaceBag aSelection;
                aSelection.insert( Reference< XInterface >( pSingleSelectionData->GetFormIface(), UNO_QUERY ) );
                pFormShell->GetImpl()->setCurrentSelection_Lock(aSelection);
            }
        }
    }

    bool NavigatorTree::IsHiddenControl(FmEntryData const * pEntryData)
    {
        if (pEntryData == nullptr) return false;

        Reference< XPropertySet > xProperties( pEntryData->GetPropertySet() );
        if (::comphelper::hasProperty(FM_PROP_CLASSID, xProperties))
        {
            Any aClassID = xProperties->getPropertyValue( FM_PROP_CLASSID );
            return (::comphelper::getINT16(aClassID) == FormComponentType::HIDDENCONTROL);
        }
        return false;
    }

#if 0
    bool NavigatorTree::Select( SvTreeListEntry* pEntry, bool bSelect )
    {
        if (bSelect == IsSelected(pEntry))  // this happens sometimes, maybe base class is to exact ;)
            return true;

        return SvTreeListBox::Select(pEntry, bSelect );
    }
#endif

    void NavigatorTree::UnmarkAllViewObj()
    {
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( !pFormShell )
            return;
        FmFormView* pFormView = pFormShell->GetFormView();
        pFormView->UnMarkAll();
    }

    void NavigatorTree::MarkViewObj(FmFormData const * pFormData, bool bDeep )
    {
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( !pFormShell )
            return;

        // first collect all sdrobjects
        ::std::set< Reference< XFormComponent > > aObjects;
        CollectObjects(pFormData,bDeep,aObjects);


        // find and select appropriate SdrObj in page
        FmFormView*     pFormView       = pFormShell->GetFormView();
        SdrPageView*    pPageView       = pFormView->GetSdrPageView();
        SdrPage*        pPage           = pPageView->GetPage();
        //FmFormPage*     pFormPage       = dynamic_cast< FmFormPage* >( pPage );

        SdrObjListIter aIter( pPage );
        while ( aIter.IsMore() )
        {
            SdrObject* pSdrObject = aIter.Next();
            FmFormObj* pFormObject = FmFormObj::GetFormObject( pSdrObject );
            if ( !pFormObject )
                continue;

            Reference< XFormComponent > xControlModel( pFormObject->GetUnoControlModel(),UNO_QUERY );
            if ( xControlModel.is() && aObjects.find(xControlModel) != aObjects.end() && !pFormView->IsObjMarked( pSdrObject ) )
            {
                // unfortunately, the writer doesn't like marking an already-marked object, again, so reset the mark first
                pFormView->MarkObj( pSdrObject, pPageView );
            }
        } // while ( aIter.IsMore() )
        // make the mark visible
        ::tools::Rectangle aMarkRect( pFormView->GetAllMarkedRect());
        for ( sal_uInt32 i = 0; i < pFormView->PaintWindowCount(); ++i )
        {
            SdrPaintWindow* pPaintWindow = pFormView->GetPaintWindow( i );
            OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
            if ( ( OUTDEV_WINDOW == rOutDev.GetOutDevType() ) && !aMarkRect.IsEmpty() )
            {
                pFormView->MakeVisible( aMarkRect, static_cast<vcl::Window&>(rOutDev) );
            }
        } // for ( sal_uInt32 i = 0; i < pFormView->PaintWindowCount(); ++i )
    }

    void NavigatorTree::CollectObjects(FmFormData const * pFormData, bool bDeep, ::std::set< Reference< XFormComponent > >& _rObjects)
    {
        FmEntryDataList* pChildList = pFormData->GetChildList();
        FmControlData* pControlData;
        for( size_t i = 0; i < pChildList->size(); ++i )
        {
            FmEntryData* pEntryData = pChildList->at( i );
            if( dynamic_cast<const FmControlData*>( pEntryData) !=  nullptr )
            {
                pControlData = static_cast<FmControlData*>(pEntryData);
                _rObjects.insert(pControlData->GetFormComponent());
            } // if( dynamic_cast<const FmControlData*>( pEntryData) !=  nullptr )
            else if (bDeep && (dynamic_cast<const FmFormData*>( pEntryData) !=  nullptr))
                CollectObjects(static_cast<FmFormData*>(pEntryData), bDeep, _rObjects);
        } // for( sal_uInt32 i=0; i<pChildList->Count(); i++ )
    }

    void NavigatorTree::MarkViewObj( FmControlData const * pControlData)
    {
        if( !pControlData )
            return;
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( !pFormShell )
            return;


        // find and select appropriate SdrObj
        FmFormView*     pFormView       = pFormShell->GetFormView();
        Reference< XFormComponent >  xFormComponent( pControlData->GetFormComponent());
        SdrPageView*    pPageView       = pFormView->GetSdrPageView();
        SdrPage*        pPage           = pPageView->GetPage();

        bool bPaint = false;
        SdrObjListIter aIter( pPage );
        while ( aIter.IsMore() )
        {
            SdrObject* pSdrObject = aIter.Next();
            FmFormObj* pFormObject = FmFormObj::GetFormObject( pSdrObject );
            if ( !pFormObject )
                continue;

            Reference< XInterface > xControlModel( pFormObject->GetUnoControlModel() );
            if ( xControlModel != xFormComponent )
                continue;

            // mark the object
            if ( !pFormView->IsObjMarked( pSdrObject ) )
                // unfortunately, the writer doesn't like marking an already-marked object, again, so reset the mark first
                pFormView->MarkObj( pSdrObject, pPageView );

            bPaint = true;

        } // while ( aIter.IsMore() )
        if ( bPaint )
        {
            // make the mark visible
            ::tools::Rectangle aMarkRect( pFormView->GetAllMarkedRect());
            for ( sal_uInt32 i = 0; i < pFormView->PaintWindowCount(); ++i )
            {
                SdrPaintWindow* pPaintWindow = pFormView->GetPaintWindow( i );
                OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
                if ( OUTDEV_WINDOW == rOutDev.GetOutDevType() )
                {
                    pFormView->MakeVisible( aMarkRect, static_cast<vcl::Window&>(rOutDev) );
                }
            } // for ( sal_uInt32 i = 0; i < pFormView->PaintWindowCount(); ++i )
        }
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
