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

#include <svx/dialmgr.hxx>
#include <svx/fmshell.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svx/svdpagv.hxx>
#include "svx/svditer.hxx"

#include "fmhelp.hrc"
#include "fmexpl.hxx"
#include "svx/fmresids.hrc"
#include "fmshimp.hxx"
#include "fmservs.hxx"
#include "fmundo.hxx"
#include "fmpgeimp.hxx"
#include "fmitems.hxx"
#include "fmobj.hxx"
#include "fmprop.hrc"
#include <vcl/wrkwin.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/script/XEventAttacherManager.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <svx/sdrpaintwindow.hxx>

#include <svx/svxdlg.hxx>
#include <svx/dialogs.hrc>
#include "svtools/treelistentry.hxx"

namespace svxform
{


    #define DROP_ACTION_TIMER_INITIAL_TICKS     10
        // Time until scroll starts
    #define DROP_ACTION_TIMER_SCROLL_TICKS      3
        // Time to scroll one line
    #define DROP_ACTION_TIMER_TICK_BASE         10
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


    typedef ::std::map< Reference< XInterface >, SdrObject*, ::comphelper::OInterfaceCompare< XInterface > >
            MapModelToShape;
    typedef MapModelToShape::value_type ModelShapePair;


    void    collectShapeModelMapping( SdrPage* _pPage, MapModelToShape& _rMapping )
    {
        OSL_ENSURE( _pPage, "collectShapeModelMapping: invalid arg!" );

        _rMapping.clear();

        SdrObjListIter aIter( *_pPage );
        while ( aIter.IsMore() )
        {
            SdrObject* pSdrObject = aIter.Next();
            FmFormObj* pFormObject = FmFormObj::GetFormObject( pSdrObject );
            if ( !pFormObject )
                continue;

            Reference< XInterface > xNormalizedModel( pFormObject->GetUnoControlModel(), UNO_QUERY );
                // note that this is normalized (i.e. queried for XInterface explicitly)

            ::std::pair< MapModelToShape::iterator, bool > aPos =
                  _rMapping.insert( ModelShapePair( xNormalizedModel, pSdrObject ) );
            DBG_ASSERT( aPos.second, "collectShapeModelMapping: model was already existent!" );
                // if this asserts, this would mean we have 2 shapes pointing to the same model
        }
    }

    NavigatorTree::NavigatorTree( vcl::Window* pParent )
        :SvTreeListBox( pParent, WB_HASBUTTONS|WB_HASLINES|WB_BORDER|WB_HSCROLL ) // #100258# OJ WB_HSCROLL added
        ,m_aControlExchange(this)
        ,m_pNavModel( nullptr )
        ,m_pRootEntry(nullptr)
        ,m_pEditEntry(nullptr)
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
        ,m_bMarkingObjects(false)
        ,m_bRootSelected(false)
        ,m_bInitialUpdate(true)
        ,m_bKeyboardCut( false )
    {
        SetHelpId( HID_FORM_NAVIGATOR );

        m_aNavigatorImages = ImageList( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );

        SetNodeBitmaps(
            m_aNavigatorImages.GetImage( RID_SVXIMG_COLLAPSEDNODE ),
            m_aNavigatorImages.GetImage( RID_SVXIMG_EXPANDEDNODE )
        );

        SetDragDropMode(DragDropMode::ALL);
        EnableInplaceEditing( true );
        SetSelectionMode(SelectionMode::Multiple);

        m_pNavModel = new NavigatorTreeModel( m_aNavigatorImages );
        Clear();

        StartListening( *m_pNavModel );

        m_aDropActionTimer.SetTimeoutHdl(LINK(this, NavigatorTree, OnDropActionTimer));

        m_aSynchronizeTimer.SetTimeoutHdl(LINK(this, NavigatorTree, OnSynchronizeTimer));
        SetSelectHdl(LINK(this, NavigatorTree, OnEntrySelDesel));
        SetDeselectHdl(LINK(this, NavigatorTree, OnEntrySelDesel));
    }


    NavigatorTree::~NavigatorTree()
    {
        disposeOnce();
    }

    void NavigatorTree::dispose()
    {
        if( nEditEvent )
            Application::RemoveUserEvent( nEditEvent );

        if (m_aSynchronizeTimer.IsActive())
            m_aSynchronizeTimer.Stop();

        DBG_ASSERT(GetNavModel() != nullptr, "NavigatorTree::~NavigatorTree : unexpected : no ExplorerModel");
        EndListening( *m_pNavModel );
        Clear();
        delete m_pNavModel;
        SvTreeListBox::dispose();
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
                CancelTextEditing();

            m_bDragDataDirty = true;    // as a precaution, although I don't drag
        }
        GetNavModel()->UpdateContent( pFormShell );

        // if there is a form, expand root
        if (m_pRootEntry && !IsExpanded(m_pRootEntry))
            Expand(m_pRootEntry);
        // if there is EXACTLY ONE form, expand it too
        if (m_pRootEntry)
        {
            SvTreeListEntry* pFirst = FirstChild(m_pRootEntry);
            if (pFirst && !NextSibling(pFirst))
                Expand(pFirst);
        }
    }


    bool NavigatorTree::implAllowExchange( sal_Int8 _nAction, bool* _pHasNonHidden )
    {
        SvTreeListEntry* pCurEntry = GetCurEntry();
        if (!pCurEntry)
            return false;

        // Information for AcceptDrop and Execute Drop
        CollectSelectionData(SDI_ALL);
        if (m_arrCurrentSelection.empty())
            // nothing to do
            return false;

        // check whether there are only hidden controls
        // I may add a format to pCtrlExch
        bool bHasNonHidden = false;
        for ( SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
              it != m_arrCurrentSelection.end(); ++it )
        {
            FmEntryData* pCurrent = static_cast< FmEntryData* >( (*it)->GetUserData() );
            if ( IsHiddenControl( pCurrent ) )
                continue;
            bHasNonHidden = true;
            break;
        }

        if ( bHasNonHidden && ( 0 == ( _nAction & DND_ACTION_MOVE ) ) )
            // non-hidden controls need to be moved
            return false;

        if ( _pHasNonHidden )
            *_pHasNonHidden = bHasNonHidden;

        return true;
    }


    bool NavigatorTree::implPrepareExchange( sal_Int8 _nAction )
    {
        EndSelection();

        bool bHasNonHidden = false;
        if ( !implAllowExchange( _nAction, &bHasNonHidden ) )
            return false;

        m_aControlExchange.prepareDrag();
        m_aControlExchange->setFocusEntry( GetCurEntry() );

        for ( SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
              it != m_arrCurrentSelection.end(); ++it )
            m_aControlExchange->addSelectedEntry(*it);

        m_aControlExchange->setFormsRoot( GetNavModel()->GetFormPage()->GetForms() );
        m_aControlExchange->buildPathFormat( this, m_pRootEntry );

        if (!bHasNonHidden)
        {
            // create a sequence
            Sequence< Reference< XInterface > > seqIFaces(m_arrCurrentSelection.size());
            Reference< XInterface >* pArray = seqIFaces.getArray();
            for ( SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
                  it != m_arrCurrentSelection.end(); ++it, ++pArray )
                *pArray = static_cast< FmEntryData* >( (*it)->GetUserData() )->GetElement();

            // and the new format
            m_aControlExchange->addHiddenControlsFormat(seqIFaces);
        }

        m_bDragDataDirty = false;
        return true;
    }


    void NavigatorTree::StartDrag( sal_Int8 /*nAction*/, const ::Point& /*rPosPixel*/ )
    {
        EndSelection();

        if ( !implPrepareExchange( DND_ACTION_COPYMOVE ) )
            // nothing to do or something went wrong
            return;

        // collected all possible formats for current situation, we can start now
        m_aControlExchange.startDrag( DND_ACTION_COPYMOVE );
    }


    void NavigatorTree::Command( const CommandEvent& rEvt )
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
                    SvTreeListEntry* ptClickedOn = GetEntry(ptWhere);
                    if (ptClickedOn == nullptr)
                        break;
                    if ( !IsSelected(ptClickedOn) )
                    {
                        SelectAll(false);
                        Select(ptClickedOn);
                        SetCurEntry(ptClickedOn);
                    }
                }
                else
                {
                    if (m_arrCurrentSelection.empty()) // only happens with context menu via keyboard
                        break;

                    SvTreeListEntry* pCurrent = GetCurEntry();
                    if (!pCurrent)
                        break;
                    ptWhere = GetEntryPosition(pCurrent);
                }

                // update my selection data
                CollectSelectionData(SDI_ALL);

                // if there is at least one no-root-entry and the root selected, I deselect root
                if ( (m_arrCurrentSelection.size() > 1) && m_bRootSelected )
                {
                    Select( m_pRootEntry, false );
                    SetCursor( *m_arrCurrentSelection.begin(), true);
                }
                bool bSingleSelection = (m_arrCurrentSelection.size() == 1);


                DBG_ASSERT( (m_arrCurrentSelection.size() > 0) || m_bRootSelected, "no entries selected" );
                    // shouldn't happen, because I would have selected one during call to IsSelected,
                    // if there was none before


                // create menu
                FmFormShell* pFormShell = GetNavModel()->GetFormShell();
                FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : nullptr;
                if( pFormShell && pFormModel )
                {
                    ScopedVclPtrInstance<PopupMenu> aContextMenu(SVX_RES(RID_FMEXPLORER_POPUPMENU));
                    PopupMenu* pSubMenuNew = aContextMenu->GetPopupMenu( SID_FM_NEW );

                    // menu 'New' only exists, if only the root or only one form is selected
                    aContextMenu->EnableItem( SID_FM_NEW, bSingleSelection && (m_nFormsSelected || m_bRootSelected) );

                    // 'New'\'Form' under the same terms
                    pSubMenuNew->EnableItem( SID_FM_NEW_FORM, bSingleSelection && (m_nFormsSelected || m_bRootSelected) );
                    pSubMenuNew->SetItemImage(SID_FM_NEW_FORM, m_aNavigatorImages.GetImage(RID_SVXIMG_FORM));
                    pSubMenuNew->SetItemImage(SID_FM_NEW_HIDDEN, m_aNavigatorImages.GetImage(RID_SVXIMG_HIDDEN));

                    // 'New'\'hidden...', if exactly one form is selected
                    pSubMenuNew->EnableItem( SID_FM_NEW_HIDDEN, bSingleSelection && m_nFormsSelected );

                    // 'Delete': everything which is not root can be removed
                    aContextMenu->EnableItem( SID_FM_DELETE, !m_bRootSelected );

                    // 'Cut', 'Copy' and 'Paste'
                    aContextMenu->EnableItem( SID_CUT, !m_bRootSelected && implAllowExchange( DND_ACTION_MOVE ) );
                    aContextMenu->EnableItem( SID_COPY, !m_bRootSelected && implAllowExchange( DND_ACTION_COPY ) );
                    aContextMenu->EnableItem( SID_PASTE, implAcceptPaste( ) );

                    // TabDialog, if exactly one form
                    aContextMenu->EnableItem( SID_FM_TAB_DIALOG, bSingleSelection && m_nFormsSelected );

                    // in XML forms, we don't allow for the properties of a form
                    // #i36484#
                    if ( pFormShell->GetImpl()->isEnhancedForm() && !m_nControlsSelected )
                        aContextMenu->RemoveItem( aContextMenu->GetItemPos( SID_FM_SHOW_PROPERTY_BROWSER ) );

                    // if the property browser is already open, we don't allow for the properties, too
                    if( pFormShell->GetImpl()->IsPropBrwOpen() )
                        aContextMenu->RemoveItem( aContextMenu->GetItemPos( SID_FM_SHOW_PROPERTY_BROWSER ) );
                    // and finally, if there's a mixed selection of forms and controls, disable the entry, too
                    else
                        aContextMenu->EnableItem( SID_FM_SHOW_PROPERTY_BROWSER,
                            (m_nControlsSelected && !m_nFormsSelected) || (!m_nControlsSelected && m_nFormsSelected) );

                    // rename, if one element and no root
                    aContextMenu->EnableItem( SID_FM_RENAME_OBJECT, bSingleSelection && !m_bRootSelected );

                    // Readonly-entry is only for root
                    aContextMenu->EnableItem( SID_FM_OPEN_READONLY, m_bRootSelected );
                    // the same for automatic control focus
                    aContextMenu->EnableItem( SID_FM_AUTOCONTROLFOCUS, m_bRootSelected );

                    // ConvertTo-Slots are enabled, if one control is selected
                    // the corresponding slot is disabled
                    if (!m_bRootSelected && !m_nFormsSelected && (m_nControlsSelected == 1))
                    {
                        aContextMenu->SetPopupMenu( SID_FM_CHANGECONTROLTYPE, FmXFormShell::GetConversionMenu() );
#if OSL_DEBUG_LEVEL > 0
                        FmControlData* pCurrent = static_cast<FmControlData*>((*m_arrCurrentSelection.begin())->GetUserData());
                        OSL_ENSURE( pFormShell->GetImpl()->isSolelySelected( pCurrent->GetFormComponent() ),
                            "NavigatorTree::Command: inconsistency between the navigator selection, and the selection as the shell knows it!" );
#endif

                        pFormShell->GetImpl()->checkControlConversionSlotsForCurrentSelection( *aContextMenu->GetPopupMenu( SID_FM_CHANGECONTROLTYPE ) );
                    }
                    else
                        aContextMenu->EnableItem( SID_FM_CHANGECONTROLTYPE, false );

                    // remove all disabled entries
                    aContextMenu->RemoveDisabledEntries(true, true);

                    // set OpenReadOnly

                    aContextMenu->CheckItem( SID_FM_OPEN_READONLY, pFormModel->GetOpenInDesignMode() );
                    aContextMenu->CheckItem( SID_FM_AUTOCONTROLFOCUS, pFormModel->GetAutoControlFocus() );

                    sal_uInt16 nSlotId = aContextMenu->Execute( this, ptWhere );
                    switch( nSlotId )
                    {
                        case SID_FM_NEW_FORM:
                        {
                            OUString aStr(SVX_RESSTR(RID_STR_FORM));
                            OUString aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_INSERT).replaceAll("#", aStr);

                            pFormModel->BegUndo(aUndoStr);
                            // slot was only available, if there is only one selected entry,
                            // which is a root or a form
                            NewForm( *m_arrCurrentSelection.begin() );
                            pFormModel->EndUndo();

                        }   break;
                        case SID_FM_NEW_HIDDEN:
                        {
                            OUString aStr(SVX_RESSTR(RID_STR_CONTROL));
                            OUString aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_INSERT).replaceAll("#", aStr);

                            pFormModel->BegUndo(aUndoStr);
                            // slot was valid for (exactly) one selected form
                            OUString fControlName = FM_COMPONENT_HIDDEN;
                            NewControl( fControlName, *m_arrCurrentSelection.begin(), true );
                            pFormModel->EndUndo();

                        }   break;

                        case SID_CUT:
                            doCut();
                            break;

                        case SID_COPY:
                            doCopy();
                            break;

                        case SID_PASTE:
                            doPaste();
                            break;

                        case SID_FM_DELETE:
                        {
                            DeleteSelection();
                        }
                        break;
                        case SID_FM_TAB_DIALOG:
                        {
                            // this slot was effective for exactly one selected form
                            SvTreeListEntry* pSelectedForm = *m_arrCurrentSelection.begin();
                            DBG_ASSERT( IsFormEntry(pSelectedForm), "NavigatorTree::Command: This entry must be a FormEntry." );

                            FmFormData* pFormData = static_cast<FmFormData*>(pSelectedForm->GetUserData());
                            Reference< XForm >  xForm(  pFormData->GetFormIface());

                            Reference< XTabControllerModel >  xTabController(xForm, UNO_QUERY);
                            if( !xTabController.is() )
                                break;
                            GetNavModel()->GetFormShell()->GetImpl()->ExecuteTabOrderDialog( xTabController );
                        }
                        break;

                        case SID_FM_SHOW_PROPERTY_BROWSER:
                        {
                            ShowSelectionProperties(true);
                        }
                        break;
                        case SID_FM_RENAME_OBJECT:
                        {
                            // only allowed for one no-root-entry
                            EditEntry( *m_arrCurrentSelection.begin() );
                        }
                        break;
                        case SID_FM_OPEN_READONLY:
                        {
                            pFormModel->SetOpenInDesignMode( !pFormModel->GetOpenInDesignMode() );
                            pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_OPEN_READONLY);
                        }
                        break;
                        case SID_FM_AUTOCONTROLFOCUS:
                        {
                            pFormModel->SetAutoControlFocus( !pFormModel->GetAutoControlFocus() );
                            pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_AUTOCONTROLFOCUS);
                        }
                        break;
                        default:
                            if (FmXFormShell::isControlConversionSlot(nSlotId))
                            {
                                FmControlData* pCurrent = static_cast<FmControlData*>((*m_arrCurrentSelection.begin())->GetUserData());
                                if ( pFormShell->GetImpl()->executeControlConversionSlot( pCurrent->GetFormComponent(), nSlotId ) )
                                    ShowSelectionProperties();
                            }
                    }
                }
                bHandled = true;
            }
            break;
            default: break;
        }

        if (!bHandled)
            SvTreeListBox::Command( rEvt );
    }


    SvTreeListEntry* NavigatorTree::FindEntry( FmEntryData* pEntryData )
    {
        if( !pEntryData ) return nullptr;
        SvTreeListEntry* pCurEntry = First();
        while( pCurEntry )
        {
            FmEntryData* pCurEntryData = static_cast<FmEntryData*>(pCurEntry->GetUserData());
            if( pCurEntryData && pCurEntryData->IsEqualWithoutChildren(pEntryData) )
                return pCurEntry;

            pCurEntry = Next( pCurEntry );
        }

        return nullptr;
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
            SvTreeListEntry* pEntry = FindEntry( pData );
            if (pEntry)
            {   // reset image
                SetCollapsedEntryBmp( pEntry, pData->GetNormalImage() );
                SetExpandedEntryBmp( pEntry, pData->GetNormalImage() );
            }
        }

        else if( dynamic_cast<const FmNavNameChangedHint*>(&rHint) )
        {
            const FmNavNameChangedHint* pNameChangedHint = static_cast<const FmNavNameChangedHint*>(&rHint);
            SvTreeListEntry* pEntry = FindEntry( pNameChangedHint->GetEntryData() );
            SetEntryText( pEntry, pNameChangedHint->GetNewName() );
        }

        else if( dynamic_cast<const FmNavClearedHint*>(&rHint) )
        {
            SvTreeListBox::Clear();


            // default-entry "Forms"
            Image aRootImage( m_aNavigatorImages.GetImage( RID_SVXIMG_FORMS ) );
            m_pRootEntry = InsertEntry( SVX_RESSTR(RID_STR_FORMS), aRootImage, aRootImage,
                nullptr, false, 0 );
        }
        else if (!m_bMarkingObjects && dynamic_cast<const FmNavRequestSelectHint*>(&rHint))
        {   // if m_bMarkingObjects is sal_True, I mark objects myself
            // and because of the synchronous mechanism, its exactly the hint,
            // which was triggered by myself, and thus can be ignored
            FmNavRequestSelectHint* pershHint = const_cast<FmNavRequestSelectHint*>(static_cast<const FmNavRequestSelectHint*>(&rHint));
            FmEntryDataArray& arredToSelect = pershHint->GetItems();
            SynchronizeSelection(arredToSelect);

            if (pershHint->IsMixedSelection())
                // in this case I deselect all, although the view had a mixed selection
                // during next selection, I must adapt the navigator to the view
                m_bPrevSelectionMixed = true;
        }
    }


    SvTreeListEntry* NavigatorTree::Insert( FmEntryData* pEntryData, sal_uIntPtr nRelPos )
    {

        // insert current entry
        SvTreeListEntry* pParentEntry = FindEntry( pEntryData->GetParent() );
        SvTreeListEntry* pNewEntry;

        if( !pParentEntry )
            pNewEntry = InsertEntry( pEntryData->GetText(),
                pEntryData->GetNormalImage(), pEntryData->GetNormalImage(),
                m_pRootEntry, false, nRelPos, pEntryData );

        else
            pNewEntry = InsertEntry( pEntryData->GetText(),
                pEntryData->GetNormalImage(), pEntryData->GetNormalImage(),
                pParentEntry, false, nRelPos, pEntryData );


        // If root-entry, expand root
        if( !pParentEntry )
            Expand( m_pRootEntry );


        // insert children
        FmEntryDataList* pChildList = pEntryData->GetChildList();
        size_t nChildCount = pChildList->size();
        for( size_t i = 0; i < nChildCount; i++ )
        {
            FmEntryData* pChildData = pChildList->at( i );
            Insert( pChildData, TREELIST_APPEND );
        }

        return pNewEntry;
    }


    void NavigatorTree::Remove( FmEntryData* pEntryData )
    {
        if( !pEntryData )
            return;

        // entry for the data
        SvTreeListEntry* pEntry = FindEntry( pEntryData );
        if (!pEntry)
            return;

        // delete entry from TreeListBox
        // I'm not allowed, to treat the selection, which I trigger:
        // select changes the MarkList of the view, if somebody else does this at the same time
        // and removes a selection, we get a problem
        // e.g. Group controls with open navigator
        LockSelectionHandling();

        // little problem: I remember the selected data, but if somebody deletes one of these entries,
        // I get inconsistent... this would be bad
        Select(pEntry, false);

        // selection can be modified during deletion,
        // but because I disabled SelectionHandling, I have to do it later
        sal_uIntPtr nExpectedSelectionCount = GetSelectionCount();

        if( pEntry )
            GetModel()->Remove( pEntry );

        if (nExpectedSelectionCount != GetSelectionCount())
            SynchronizeSelection();

        // by default I treat the selection of course
        UnlockSelectionHandling();
    }


    bool NavigatorTree::IsFormEntry( SvTreeListEntry* pEntry )
    {
        FmEntryData* pEntryData = static_cast<FmEntryData*>(pEntry->GetUserData());
        return !pEntryData || dynamic_cast<const FmFormData*>( pEntryData) !=  nullptr;
    }


    bool NavigatorTree::IsFormComponentEntry( SvTreeListEntry* pEntry )
    {
        FmEntryData* pEntryData = static_cast<FmEntryData*>(pEntry->GetUserData());
        return pEntryData && dynamic_cast<const FmControlData*>( pEntryData) !=  nullptr;
    }


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


    sal_Int8 NavigatorTree::implAcceptDataTransfer( const DataFlavorExVector& _rFlavors, sal_Int8 _nAction, const ::Point& _rDropPos, bool _bDnD )
    {
        return implAcceptDataTransfer( _rFlavors, _nAction, GetEntry( _rDropPos ), _bDnD );
    }


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
            if ( !_pTargetEntry || ( _pTargetEntry == m_pRootEntry ) || !IsFormEntry( _pTargetEntry ) )
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
            m_aControlExchange->buildListFromPath(this, m_pRootEntry);
            m_bDragDataDirty = false;
        }

        // List of dropped entries from DragServer
        const ListBoxEntrySet& aDropped = m_aControlExchange->selected();
        DBG_ASSERT(aDropped.size() >= 1, "NavigatorTree::implAcceptDataTransfer: no entries !");

        bool bDropTargetIsComponent = IsFormComponentEntry( _pTargetEntry );
        //SvTreeListEntry* pDropTargetParent = GetParent( _pTargetEntry );

        // conditions to disallow the drop
        // 0) the root entry is part of the list (can't DnD the root!)
        // 1) one of the draged entries is to be dropped onto it's own parent
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

        for (   ListBoxEntrySet::const_iterator dropped = aDropped.begin();
                dropped != aDropped.end();
                ++dropped
            )
        {
            SvTreeListEntry* pCurrent = *dropped;
            SvTreeListEntry* pCurrentParent = GetParent(pCurrent);

            // test for 0)
            if (pCurrent == m_pRootEntry)
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
                if (_pTargetEntry == m_pRootEntry)
                    return DND_ACTION_NONE;
            }
        }

        return DND_ACTION_MOVE;
    }


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
                    SvTreeListEntry* pDropppedOn = GetEntry(aDropPos);
                    if (pDropppedOn && (GetChildCount(pDropppedOn) > 0) && !IsExpanded(pDropppedOn))
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

        return implAcceptDataTransfer( GetDataFlavorExVector(), rEvt.mnAction, aDropPos, true );
    }


    sal_Int8 NavigatorTree::implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, const ::Point& _rDropPos, bool _bDnD )
    {
        return implExecuteDataTransfer( _rData, _nAction, GetEntry( _rDropPos ), _bDnD );
    }


    sal_Int8 NavigatorTree::implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, SvTreeListEntry* _pTargetEntry, bool _bDnD )
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
            // and m_bDragDataDirty is resetted
#endif

        if ( DND_ACTION_COPY == _nAction )
        {   // bHasHiddenControlsFormat means that only hidden controls are part of the data
#ifdef DBG_UTIL
            DBG_ASSERT( bHasHiddenControlsFormat, "NavigatorTree::implExecuteDataTransfer: copy allowed for hidden controls only!" );
#endif
            DBG_ASSERT( _pTargetEntry && ( _pTargetEntry != m_pRootEntry ) && IsFormEntry( _pTargetEntry ),
                "NavigatorTree::implExecuteDataTransfer: should not be here!" );
                // implAcceptDataTransfer should have caught both cases

#ifdef DBG_UTIL
            DBG_ASSERT(bHasHiddenControlsFormat, "NavigatorTree::implExecuteDataTransfer: only copying of hidden controls is supported !");
                // should be catched by AcceptDrop
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
                OUString aStr(SVX_RESSTR(RID_STR_CONTROL));
                OUString aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_INSERT).replaceAll("#", aStr);
                pFormModel->BegUndo(aUndoStr);
            }

            // copy controls
            for (sal_Int32 i=0; i<nCount; ++i)
            {
                // create new control
                OUString fControlName = FM_COMPONENT_HIDDEN;
                FmControlData* pNewControlData = NewControl( fControlName, _pTargetEntry, false);
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
                Sequence< Property> seqAllCurrentProps = xPropInfo->getProperties();
                Property* pAllCurrentProps = seqAllCurrentProps.getArray();
                for (sal_Int32 j=0; j<seqAllCurrentProps.getLength(); ++j)
                {
                    OUString sCurrentProp = pAllCurrentProps[j].Name;
                    if (((pAllCurrentProps[j].Attributes & PropertyAttribute::READONLY) == 0) && (sCurrentProp != FM_PROP_NAME))
                    {   // (read-only attribs aren't set, ditto name,
                        // NewControl defined it uniquely
                        xNewPropSet->setPropertyValue(sCurrentProp, xCurrent->getPropertyValue(sCurrentProp));
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
        ListBoxEntrySet aDropped = _rData.selected();
        DBG_ASSERT(aDropped.size() >= 1, "NavigatorTree::implExecuteDataTransfer: no entries!");

        // shell and model
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : nullptr;
        if (!pFormModel)
            return DND_ACTION_NONE;

        // for Undo
        const bool bUndo = pFormModel->IsUndoEnabled();

        if( bUndo )
        {
            OUString strUndoDescription(SVX_RESSTR(RID_STR_UNDO_CONTAINER_REPLACE));
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

            Reference< XChild >  xCurrentChild(pCurrentUserData->GetChildIFace(), UNO_QUERY);
            Reference< XIndexContainer >  xContainer(xCurrentChild->getParent(), UNO_QUERY);

            FmFormData* pCurrentParentUserData = static_cast<FmFormData*>(pCurrentUserData->GetParent());
            DBG_ASSERT(pCurrentParentUserData == nullptr || dynamic_cast<const FmFormData*>(pCurrentUserData->GetParent()) !=  nullptr, "NavigatorTree::implExecuteDataTransfer: invalid parent");

            // remove from parent
            if (pCurrentParentUserData)
                pCurrentParentUserData->GetChildList()->remove( pCurrentUserData );
            else
                GetNavModel()->GetRootList()->remove( pCurrentUserData );

            // remove from container
            sal_Int32 nIndex = getElementPos(xContainer, xCurrentChild);
            GetNavModel()->m_pPropChangeList->Lock();
            // UndoAction for removal
            if ( bUndo && GetNavModel()->m_pPropChangeList->CanUndo())
            {
                pFormModel->AddUndo(new FmUndoContainerAction(*pFormModel, FmUndoContainerAction::Removed,
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
                xContainer.set(GetNavModel()->GetForms(), UNO_QUERY);

            // always insert at the end
            nIndex = xContainer->getCount();

            // UndoAction for insertion
            if ( bUndo && GetNavModel()->m_pPropChangeList->CanUndo())
                pFormModel->AddUndo(new FmUndoContainerAction(*pFormModel, FmUndoContainerAction::Inserted,
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

            if (aEvts.getLength())
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
                pTargetData->GetChildList()->insert( pCurrentUserData, nIndex );
            else
                GetNavModel()->GetRootList()->insert( pCurrentUserData, nIndex );

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
            pFormShell->GetImpl()->DetermineSelection( pFormShell->GetFormView()->GetMarkedObjectList() );

        if ( m_aControlExchange.isClipboardOwner() && ( DND_ACTION_MOVE == _nAction ) )
            m_aControlExchange->clear();

        return _nAction;
    }


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
            OSL_FAIL( "NavigatorTree::doPaste: caught an exception!" );
        }
    }


    void NavigatorTree::doCopy()
    {
        if ( implPrepareExchange( DND_ACTION_COPY ) )
        {
            m_aControlExchange.setClipboardListener( LINK( this, NavigatorTree, OnClipboardAction ) );
            m_aControlExchange.copyToClipboard( );
        }
    }


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


    void NavigatorTree::doCut()
    {
        if ( implPrepareExchange( DND_ACTION_MOVE ) )
        {
            m_aControlExchange.setClipboardListener( LINK( this, NavigatorTree, OnClipboardAction ) );
            m_aControlExchange.copyToClipboard( );
            m_bKeyboardCut = true;

            // mark all the entries we just "cut" into the clipboard as "nearly moved"
            for ( SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
                  it != m_arrCurrentSelection.end(); ++it )
            {
                SvTreeListEntry* pEntry = *it;
                if ( pEntry )
                {
                    m_aCutEntries.insert( pEntry );
                    pEntry->SetFlags( pEntry->GetFlags() | SvTLEntryFlags::SEMITRANSPARENT );
                    InvalidateEntry( pEntry );
                }
            }
        }
    }


    void NavigatorTree::KeyInput(const ::KeyEvent& rKEvt)
    {
        const vcl::KeyCode& rCode = rKEvt.GetKeyCode();

        // delete?
        if (rKEvt.GetKeyCode().GetCode() == KEY_DELETE && !rKEvt.GetKeyCode().GetModifier())
        {
            DeleteSelection();
            return;
        }

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

        SvTreeListBox::KeyInput(rKEvt);
    }


    bool NavigatorTree::EditingEntry( SvTreeListEntry* pEntry, ::Selection& rSelection )
    {
        if (!SvTreeListBox::EditingEntry( pEntry, rSelection ))
            return false;

        return (pEntry && (pEntry->GetUserData() != nullptr));
            // root, which isn't allowed to be renamed, has UserData=NULL
    }


    void NavigatorTree::NewForm( SvTreeListEntry* pParentEntry )
    {

        // get ParentFormData
        if( !IsFormEntry(pParentEntry) )
            return;

        FmFormData* pParentFormData = static_cast<FmFormData*>(pParentEntry->GetUserData());


        // create new form
        Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
        Reference< XForm >  xNewForm(xContext->getServiceManager()->createInstanceWithContext(FM_SUN_COMPONENT_FORM, xContext), UNO_QUERY);
        if (!xNewForm.is())
            return;

        Reference< XPropertySet >  xPropertySet(xNewForm, UNO_QUERY);
        if (!xPropertySet.is())
            return;

        FmFormData* pNewFormData = new FmFormData( xNewForm, m_aNavigatorImages, pParentFormData );


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
        GetNavModel()->Insert( pNewFormData, TREELIST_APPEND, true );


        // set new form as active
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( pFormShell )
        {
            InterfaceBag aSelection;
            aSelection.insert( Reference<XInterface>( xNewForm, UNO_QUERY ) );
            pFormShell->GetImpl()->setCurrentSelection( aSelection );

            pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_PROPERTIES, true, true);
        }
        GetNavModel()->SetModified();


        // switch to EditMode
        SvTreeListEntry* pNewEntry = FindEntry( pNewFormData );
        EditEntry( pNewEntry );
    }


    FmControlData* NavigatorTree::NewControl( const OUString& rServiceName, SvTreeListEntry* pParentEntry, bool bEditName )
    {

        // get ParentForm
        if (!GetNavModel()->GetFormShell())
            return nullptr;
        if (!IsFormEntry(pParentEntry))
            return nullptr;

        FmFormData* pParentFormData = static_cast<FmFormData*>(pParentEntry->GetUserData());
        Reference< XForm >  xParentForm( pParentFormData->GetFormIface());


        // create new component
        Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
        Reference<XFormComponent> xNewComponent( xContext->getServiceManager()->createInstanceWithContext(rServiceName, xContext), UNO_QUERY);
        if (!xNewComponent.is())
            return nullptr;

        FmControlData* pNewFormControlData = new FmControlData( xNewComponent, m_aNavigatorImages, pParentFormData );


        // set name
        OUString sName = FmFormPageImpl::setUniqueName( xNewComponent, xParentForm );

        pNewFormControlData->SetText( sName );


        // insert FormComponent
        GetNavModel()->Insert( pNewFormControlData, TREELIST_APPEND, true );
        GetNavModel()->SetModified();

        if (bEditName)
        {

            // switch to EditMode
            SvTreeListEntry* pNewEntry = FindEntry( pNewFormControlData );
            Select( pNewEntry );
            EditEntry( pNewEntry );
        }

        return pNewFormControlData;
    }


    OUString NavigatorTree::GenerateName( FmEntryData* pEntryData )
    {
        const sal_uInt16 nMaxCount = 99;
        OUString aNewName;


        // create base name
        OUString aBaseName;
        if( dynamic_cast<const FmFormData*>( pEntryData) !=  nullptr )
            aBaseName = SVX_RESSTR( RID_STR_STDFORMNAME );
        else if( dynamic_cast<const FmControlData*>( pEntryData) !=  nullptr )
            aBaseName = SVX_RESSTR( RID_STR_CONTROL );


        // create new name
        FmFormData* pFormParentData = static_cast<FmFormData*>(pEntryData->GetParent());

        for( sal_Int32 i=0; i<nMaxCount; i++ )
        {
            aNewName = aBaseName;
            if( i>0 )
            {
                aNewName += " ";
                aNewName += OUString::number(i).getStr();
            }

            if( GetNavModel()->FindData(aNewName, pFormParentData,false) == nullptr )
                break;
        }

        return aNewName;
    }


    bool NavigatorTree::EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText )
    {
        if (EditingCanceled())
            return true;

        GrabFocus();
        FmEntryData* pEntryData = static_cast<FmEntryData*>(pEntry->GetUserData());
        bool bRes = NavigatorTreeModel::Rename( pEntryData, rNewText);
        if( !bRes )
        {
            m_pEditEntry = pEntry;
            nEditEvent = Application::PostUserEvent( LINK(this, NavigatorTree, OnEdit), nullptr, true );
        } else
            SetCursor(pEntry, true);

        return bRes;
    }


    IMPL_LINK_NOARG_TYPED(NavigatorTree, OnEdit, void*, void)
    {
        nEditEvent = nullptr;
        EditEntry( m_pEditEntry );
        m_pEditEntry = nullptr;
    }


    IMPL_LINK_NOARG_TYPED(NavigatorTree, OnDropActionTimer, Timer *, void)
    {
        if (--m_aTimerCounter > 0)
            return;

        switch ( m_aDropActionType )
        {
        case DA_EXPANDNODE:
        {
            SvTreeListEntry* pToExpand = GetEntry(m_aTimerTriggered);
            if (pToExpand && (GetChildCount(pToExpand) > 0) &&  !IsExpanded(pToExpand))
                // normaly, we have to test, if the node is expanded,
                // but there is no method for this either in base class nor the model
                // the base class should tolerate it anyway
                Expand(pToExpand);

            // After expansion there is nothing to do like after scrolling
            m_aDropActionTimer.Stop();
        }
        break;

        case DA_SCROLLUP :
            ScrollOutputArea( 1 );
            m_aTimerCounter = DROP_ACTION_TIMER_SCROLL_TICKS;
            break;

        case DA_SCROLLDOWN :
            ScrollOutputArea( -1 );
            m_aTimerCounter = DROP_ACTION_TIMER_SCROLL_TICKS;
            break;

        }
    }


    IMPL_LINK_NOARG_TYPED(NavigatorTree, OnEntrySelDesel, SvTreeListBox*, void)
    {
        m_sdiState = SDI_DIRTY;

        if (IsSelectionHandlingLocked())
            return;

        if (m_aSynchronizeTimer.IsActive())
            m_aSynchronizeTimer.Stop();

        m_aSynchronizeTimer.SetTimeout(EXPLORER_SYNC_DELAY);
        m_aSynchronizeTimer.Start();
    }


    IMPL_LINK_NOARG_TYPED(NavigatorTree, OnSynchronizeTimer, Timer *, void)
    {
        SynchronizeMarkList();
    }


    IMPL_LINK_NOARG_TYPED(NavigatorTree, OnClipboardAction, OLocalExchange&, void)
    {
        if ( !m_aControlExchange.isClipboardOwner() )
        {
            if ( doingKeyboardCut() )
            {
                for (   ListBoxEntrySet::const_iterator i = m_aCutEntries.begin();
                        i != m_aCutEntries.end();
                        ++i
                    )
                {
                    SvTreeListEntry* pEntry = *i;
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
                if (m_nFormsSelected > 0)
                {   // exactly one form is selected
                    FmFormData* pFormData = static_cast<FmFormData*>((*m_arrCurrentSelection.begin())->GetUserData());
                    aSelection.insert( Reference< XInterface >( pFormData->GetFormIface(), UNO_QUERY ) );
                }
                else
                {   // exactly one control is selected (whatever hidden or normal)
                    FmEntryData* pEntryData = static_cast<FmEntryData*>((*m_arrCurrentSelection.begin())->GetUserData());

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
                        FmFormData* pFormData = static_cast<FmFormData*>((*it)->GetUserData());
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
                            FmEntryData* pEntryData = static_cast<FmEntryData*>((*it)->GetUserData());
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
            pFormShell->GetImpl()->setCurrentSelectionFromMark( pFormShell->GetFormView()->GetMarkedObjectList() );
        else
            pFormShell->GetImpl()->setCurrentSelection( aSelection );

        if ( pFormShell->GetImpl()->IsPropBrwOpen() || bForce )
        {
            // and now deliver all to the PropertyBrowser
            pFormShell->GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_SHOW_PROPERTY_BROWSER, SfxCallMode::ASYNCHRON );
        }
    }


    void NavigatorTree::DeleteSelection()
    {
        // of course, i can't delete root
        bool bRootSelected = IsSelected(m_pRootEntry);
        sal_uIntPtr nSelectedEntries = GetSelectionCount();
        if (bRootSelected && (nSelectedEntries > 1))     // root and other elements ?
            Select(m_pRootEntry, false);                // yes -> remove root from selection

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
        pFormShell->GetImpl()->EnableTrackProperties(false);
        for (SvLBoxEntrySortedArray::reverse_iterator it = m_arrCurrentSelection.rbegin();
             it != m_arrCurrentSelection.rend(); )
        {
            FmEntryData* pCurrent = static_cast<FmEntryData*>((*it)->GetUserData());

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
        pFormShell->GetImpl()->EnableTrackProperties(true);

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
                aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_REMOVE);
                if (m_nFormsSelected)
                    aUndoStr = aUndoStr.replaceFirst( "#", SVX_RESSTR( RID_STR_FORM ) );
                else
                    // it must be a control (else the root would be selected, but it cannot be deleted)
                    aUndoStr = aUndoStr.replaceFirst( "#", SVX_RESSTR( RID_STR_CONTROL ) );
            }
            else
            {
                aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_REMOVE_MULTIPLE);
                aUndoStr = aUndoStr.replaceFirst( "#", OUString::number( m_arrCurrentSelection.size() ) );
            }
            pFormModel->BegUndo(aUndoStr);
        }

        // remove remaining structure
        for (SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
             it != m_arrCurrentSelection.end(); ++it)
        {
            FmEntryData* pCurrent = static_cast<FmEntryData*>((*it)->GetUserData());

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
                if ( pFormShell->GetImpl()->getCurrentForm() == xCurrentForm )  // shell knows form to be deleted ?
                    pFormShell->GetImpl()->forgetCurrentForm();                 // -> take away ...
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

        SvTreeListEntry* pSelectionLoop = FirstSelected();
        while (pSelectionLoop)
        {
            // count different elements
            if (pSelectionLoop == m_pRootEntry)
                m_bRootSelected = true;
            else
            {
                if (IsFormEntry(pSelectionLoop))
                    ++m_nFormsSelected;
                else
                {
                    ++m_nControlsSelected;
                    if (IsHiddenControl(static_cast<FmEntryData*>(pSelectionLoop->GetUserData())))
                        ++m_nHiddenControls;
                }
            }

            if (sdiHow == SDI_NORMALIZED)
            {
                // don't take something with a selected ancestor
                if (pSelectionLoop == m_pRootEntry)
                    m_arrCurrentSelection.insert(pSelectionLoop);
                else
                {
                    SvTreeListEntry* pParentLoop = GetParent(pSelectionLoop);
                    while (pParentLoop)
                    {
                        // actually i would have to test, if parent is part of m_arr_CurrentSelection ...
                        // but if it's selected, than it's in m_arrCurrentSelection
                        // or one of its ancestors, which was selected earlier.
                        // In both cases IsSelected is enough
                        if (IsSelected(pParentLoop))
                            break;
                        else
                        {
                            if (m_pRootEntry == pParentLoop)
                            {
                                // until root (exclusive), there was no selected parent -> entry belongs to normalized list
                                m_arrCurrentSelection.insert(pSelectionLoop);
                                break;
                            }
                            else
                                pParentLoop = GetParent(pParentLoop);
                        }
                    }
                }
            }
            else if (sdiHow == SDI_NORMALIZED_FORMARK)
            {
                SvTreeListEntry* pParent = GetParent(pSelectionLoop);
                if (!pParent || !IsSelected(pParent) || IsFormEntry(pSelectionLoop))
                    m_arrCurrentSelection.insert(pSelectionLoop);
            }
            else
                m_arrCurrentSelection.insert(pSelectionLoop);


            pSelectionLoop = NextSelected(pSelectionLoop);
        }

        m_sdiState = sdiHow;
    }


    void NavigatorTree::SynchronizeSelection(FmEntryDataArray& arredToSelect)
    {
        LockSelectionHandling();
        if (arredToSelect.empty())
        {
            SelectAll(false);
        }
        else
        {
            // compare current selection with requested SelectList
            SvTreeListEntry* pSelection = FirstSelected();
            while (pSelection)
            {
                FmEntryData* pCurrent = static_cast<FmEntryData*>(pSelection->GetUserData());
                if (pCurrent != nullptr)
                {
                    FmEntryDataArray::iterator it = arredToSelect.find(pCurrent);
                    if ( it != arredToSelect.end() )
                    {   // entry already selected, but also in SelectList
                        // remove it from there
                        arredToSelect.erase(it);
                    } else
                    {   // entry selected, but not in SelectList -> remove selection
                        Select(pSelection, false);
                        // make it visible (maybe it's the only modification i do in this handler
                        // so you should see it
                        MakeVisible(pSelection);
                    }
                }
                else
                    Select(pSelection, false);

                pSelection = NextSelected(pSelection);
            }

            // now SelectList contains only entries, which have to be selected
            // two possibilities : 1) run through SelectList, get SvTreeListEntry for every entry and select it (is more intuitive)
            // 2) run through my SvLBoxEntries and select those, i can find in the SelectList
            // 1) needs =(k*n) (k=length of SelectList, n=number of entries),
            // plus the fact, that FindEntry uses extensive IsEqualWithoutChilden instead of comparing pointer to UserData
            // 2) needs =(n*log k), duplicates some code from FindEntry
            // This may be a frequently used code ( at every change in mark of the view!),
            // so i use latter one
            SvTreeListEntry* pLoop = First();
            FmEntryDataArray::const_iterator aEnd = arredToSelect.end();
            while(pLoop)
            {
                FmEntryData* pCurEntryData = static_cast<FmEntryData*>(pLoop->GetUserData());
                FmEntryDataArray::iterator it = arredToSelect.find(pCurEntryData);
                if (it != aEnd)
                {
                    Select(pLoop);
                    MakeVisible(pLoop);
                    SetCursor(pLoop, true);
                }

                pLoop = Next(pLoop);
            }
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
        pFormShell->GetImpl()->EnableTrackProperties(false);

        UnmarkAllViewObj();

        for (SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
             it != m_arrCurrentSelection.end(); ++it)
        {
            SvTreeListEntry* pSelectionLoop = *it;
            // When form selection, mark all controls of form
            if (IsFormEntry(pSelectionLoop) && (pSelectionLoop != m_pRootEntry))
                MarkViewObj(static_cast<FmFormData*>(pSelectionLoop->GetUserData()), false/*deep*/);

            // When control selection, mark Control-SdrObjects
            else if (IsFormComponentEntry(pSelectionLoop))
            {
                FmControlData* pControlData = static_cast<FmControlData*>(pSelectionLoop->GetUserData());
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
        pFormShell->GetImpl()->EnableTrackProperties(true);

        // if exactly one form is selected now, shell should notice it as CurrentForm
        // (if selection handling isn't locked, view cares about it in MarkListHasChanged
        // but mechanism doesn't work, if form is empty for example
        if ((m_arrCurrentSelection.size() == 1) && (m_nFormsSelected == 1))
        {
            FmFormData* pSingleSelectionData = dynamic_cast<FmFormData*>( static_cast< FmEntryData* >( FirstSelected()->GetUserData() )  );
            DBG_ASSERT( pSingleSelectionData, "NavigatorTree::SynchronizeMarkList: invalid selected form!" );
            if ( pSingleSelectionData )
            {
                InterfaceBag aSelection;
                aSelection.insert( Reference< XInterface >( pSingleSelectionData->GetFormIface(), UNO_QUERY ) );
                pFormShell->GetImpl()->setCurrentSelection( aSelection );
            }
        }
    }


    bool NavigatorTree::IsHiddenControl(FmEntryData* pEntryData)
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


    bool NavigatorTree::Select( SvTreeListEntry* pEntry, bool bSelect )
    {
        if (bSelect == IsSelected(pEntry))  // this happens sometimes, maybe base class is to exact ;)
            return true;

        return SvTreeListBox::Select(pEntry, bSelect );
    }


    void NavigatorTree::UnmarkAllViewObj()
    {
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( !pFormShell )
            return;
        FmFormView* pFormView = pFormShell->GetFormView();
        pFormView->UnMarkAll();
    }

    void NavigatorTree::MarkViewObj(FmFormData* pFormData, bool bDeep )
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

        SdrObjListIter aIter( *pPage );
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
        ::Rectangle aMarkRect( pFormView->GetAllMarkedRect());
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

    void NavigatorTree::CollectObjects(FmFormData* pFormData, bool bDeep, ::std::set< Reference< XFormComponent > >& _rObjects)
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

    void NavigatorTree::MarkViewObj( FmControlData* pControlData)
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
        SdrObjListIter aIter( *pPage );
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
            ::Rectangle aMarkRect( pFormView->GetAllMarkedRect());
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
