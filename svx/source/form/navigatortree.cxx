/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/dialmgr.hxx>
#include <svx/fmshell.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svx/svdpagv.hxx>
#include "svx/svditer.hxx"

#include "fmhelp.hrc"
#include "fmexpl.hrc"
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
        
    #define DROP_ACTION_TIMER_SCROLL_TICKS      3
        
    #define DROP_ACTION_TIMER_TICK_BASE         10
        

    #define EXPLORER_SYNC_DELAY                 200
        

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
                

#ifdef DBG_UTIL
            ::std::pair< MapModelToShape::iterator, bool > aPos =
#endif
            _rMapping.insert( ModelShapePair( xNormalizedModel, pSdrObject ) );
            DBG_ASSERT( aPos.second, "collectShapeModelMapping: model was already existent!" );
                
        }
    }

    
    
    

    
    NavigatorTree::NavigatorTree( Window* pParent )
        :SvTreeListBox( pParent, WB_HASBUTTONS|WB_HASLINES|WB_BORDER|WB_HSCROLL ) 
        ,m_aControlExchange(this)
        ,m_pNavModel( NULL )
        ,m_pRootEntry(NULL)
        ,m_pEditEntry(NULL)
        ,nEditEvent(0)
        ,m_sdiState(SDI_DIRTY)
        ,m_aTimerTriggered(-1,-1)
        ,m_aDropActionType( DA_SCROLLUP )
        ,m_nSelectLock(0)
        ,m_nFormsSelected(0)
        ,m_nControlsSelected(0)
        ,m_nHiddenControls(0)
        ,m_aTimerCounter( DROP_ACTION_TIMER_INITIAL_TICKS )
        ,m_bDragDataDirty(sal_False)
        ,m_bPrevSelectionMixed(sal_False)
        ,m_bMarkingObjects(sal_False)
        ,m_bRootSelected(sal_False)
        ,m_bInitialUpdate(sal_True)
        ,m_bKeyboardCut( sal_False )
    {
        SetHelpId( HID_FORM_NAVIGATOR );

        m_aNavigatorImages = ImageList( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );

        SetNodeBitmaps(
            m_aNavigatorImages.GetImage( RID_SVXIMG_COLLAPSEDNODE ),
            m_aNavigatorImages.GetImage( RID_SVXIMG_EXPANDEDNODE )
        );

        SetDragDropMode(0xFFFF);
        EnableInplaceEditing( true );
        SetSelectionMode(MULTIPLE_SELECTION);

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
        if( nEditEvent )
            Application::RemoveUserEvent( nEditEvent );

        if (m_aSynchronizeTimer.IsActive())
            m_aSynchronizeTimer.Stop();

        DBG_ASSERT(GetNavModel() != NULL, "NavigatorTree::~NavigatorTree : unerwartet : kein ExplorerModel");
        EndListening( *m_pNavModel );
        Clear();
        delete m_pNavModel;
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
            m_bInitialUpdate = sal_False;
        }

        FmFormShell* pOldShell = GetNavModel()->GetFormShell();
        FmFormPage* pOldPage = GetNavModel()->GetFormPage();
        FmFormPage* pNewPage = pFormShell ? pFormShell->GetCurPage() : NULL;

        if ((pOldShell != pFormShell) || (pOldPage != pNewPage))
        {
            
            if (IsEditingActive())
                CancelTextEditing();

            m_bDragDataDirty = sal_True;    
        }
        GetNavModel()->UpdateContent( pFormShell );

        
        if (m_pRootEntry && !IsExpanded(m_pRootEntry))
            Expand(m_pRootEntry);
        
        if (m_pRootEntry)
        {
            SvTreeListEntry* pFirst = FirstChild(m_pRootEntry);
            if (pFirst && !NextSibling(pFirst))
                Expand(pFirst);
        }
    }

    
    sal_Bool NavigatorTree::implAllowExchange( sal_Int8 _nAction, sal_Bool* _pHasNonHidden )
    {
        SvTreeListEntry* pCurEntry = GetCurEntry();
        if (!pCurEntry)
            return sal_False;

        
        CollectSelectionData(SDI_ALL);
        if (m_arrCurrentSelection.empty())
            
            return sal_False;

        
        
        sal_Bool bHasNonHidden = sal_False;
        for ( SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
              it != m_arrCurrentSelection.end(); ++it )
        {
            FmEntryData* pCurrent = static_cast< FmEntryData* >( (*it)->GetUserData() );
            if ( IsHiddenControl( pCurrent ) )
                continue;
            bHasNonHidden = sal_True;
            break;
        }

        if ( bHasNonHidden && ( 0 == ( _nAction & DND_ACTION_MOVE ) ) )
            
            return sal_False;

        if ( _pHasNonHidden )
            *_pHasNonHidden = bHasNonHidden;

        return sal_True;
    }

    
    sal_Bool NavigatorTree::implPrepareExchange( sal_Int8 _nAction )
    {
        EndSelection();

        sal_Bool bHasNonHidden = sal_False;
        if ( !implAllowExchange( _nAction, &bHasNonHidden ) )
            return sal_False;

        m_aControlExchange.prepareDrag();
        m_aControlExchange->setFocusEntry( GetCurEntry() );

        for ( SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
              it != m_arrCurrentSelection.end(); ++it )
            m_aControlExchange->addSelectedEntry(*it);

        m_aControlExchange->setFormsRoot( GetNavModel()->GetFormPage()->GetForms() );
        m_aControlExchange->buildPathFormat( this, m_pRootEntry );

        if (!bHasNonHidden)
        {
            
            Sequence< Reference< XInterface > > seqIFaces(m_arrCurrentSelection.size());
            Reference< XInterface >* pArray = seqIFaces.getArray();
            for ( SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
                  it != m_arrCurrentSelection.end(); ++it, ++pArray )
                *pArray = static_cast< FmEntryData* >( (*it)->GetUserData() )->GetElement();

            
            m_aControlExchange->addHiddenControlsFormat(seqIFaces);
        }

        m_bDragDataDirty = sal_False;
        return sal_True;
    }

    
    void NavigatorTree::StartDrag( sal_Int8 /*nAction*/, const ::Point& /*rPosPixel*/ )
    {
        EndSelection();

        if ( !implPrepareExchange( DND_ACTION_COPYMOVE ) )
            
            return;

        
        m_aControlExchange.startDrag( DND_ACTION_COPYMOVE );
    }

    
    void NavigatorTree::Command( const CommandEvent& rEvt )
    {
        sal_Bool bHandled = sal_False;
        switch( rEvt.GetCommand() )
        {
            case COMMAND_CONTEXTMENU:
            {
                
                ::Point ptWhere;
                if (rEvt.IsMouseEvent())
                {
                    ptWhere = rEvt.GetMousePosPixel();
                    SvTreeListEntry* ptClickedOn = GetEntry(ptWhere);
                    if (ptClickedOn == NULL)
                        break;
                    if ( !IsSelected(ptClickedOn) )
                    {
                        SelectAll(sal_False);
                        Select(ptClickedOn, sal_True);
                        SetCurEntry(ptClickedOn);
                    }
                }
                else
                {
                    if (m_arrCurrentSelection.empty()) 
                        break;

                    SvTreeListEntry* pCurrent = GetCurEntry();
                    if (!pCurrent)
                        break;
                    ptWhere = GetEntryPosition(pCurrent);
                }

                
                CollectSelectionData(SDI_ALL);

                
                
                if ( (m_arrCurrentSelection.size() > 1) && m_bRootSelected )
                {
                    Select( m_pRootEntry, sal_False );
                    SetCursor( *m_arrCurrentSelection.begin(), sal_True);
                }
                sal_Bool bSingleSelection = (m_arrCurrentSelection.size() == 1);


                DBG_ASSERT( (m_arrCurrentSelection.size() > 0) || m_bRootSelected, "keine Eintraege selektiert" );
                    
                    


                
                FmFormShell* pFormShell = GetNavModel()->GetFormShell();
                FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;
                if( pFormShell && pFormModel )
                {
                    PopupMenu aContextMenu(SVX_RES(RID_FMEXPLORER_POPUPMENU));
                    PopupMenu* pSubMenuNew = aContextMenu.GetPopupMenu( SID_FM_NEW );

                    
                    aContextMenu.EnableItem( SID_FM_NEW, bSingleSelection && (m_nFormsSelected || m_bRootSelected) );

                    
                    pSubMenuNew->EnableItem( SID_FM_NEW_FORM, bSingleSelection && (m_nFormsSelected || m_bRootSelected) );
                    pSubMenuNew->SetItemImage(SID_FM_NEW_FORM, m_aNavigatorImages.GetImage(RID_SVXIMG_FORM));
                    pSubMenuNew->SetItemImage(SID_FM_NEW_HIDDEN, m_aNavigatorImages.GetImage(RID_SVXIMG_HIDDEN));

                    
                    pSubMenuNew->EnableItem( SID_FM_NEW_HIDDEN, bSingleSelection && m_nFormsSelected );

                    
                    aContextMenu.EnableItem( SID_FM_DELETE, !m_bRootSelected );

                    
                    aContextMenu.EnableItem( SID_CUT, !m_bRootSelected && implAllowExchange( DND_ACTION_MOVE ) );
                    aContextMenu.EnableItem( SID_COPY, !m_bRootSelected && implAllowExchange( DND_ACTION_COPY ) );
                    aContextMenu.EnableItem( SID_PASTE, implAcceptPaste( ) );

                    
                    aContextMenu.EnableItem( SID_FM_TAB_DIALOG, bSingleSelection && m_nFormsSelected );

                    
                    
                    if ( pFormShell->GetImpl()->isEnhancedForm() && !m_nControlsSelected )
                        aContextMenu.RemoveItem( aContextMenu.GetItemPos( SID_FM_SHOW_PROPERTY_BROWSER ) );

                    
                    if( pFormShell->GetImpl()->IsPropBrwOpen() )
                        aContextMenu.RemoveItem( aContextMenu.GetItemPos( SID_FM_SHOW_PROPERTY_BROWSER ) );
                    
                    else
                        aContextMenu.EnableItem( SID_FM_SHOW_PROPERTY_BROWSER,
                            (m_nControlsSelected && !m_nFormsSelected) || (!m_nControlsSelected && m_nFormsSelected) );

                    
                    aContextMenu.EnableItem( SID_FM_RENAME_OBJECT, bSingleSelection && !m_bRootSelected );

                    
                    aContextMenu.EnableItem( SID_FM_OPEN_READONLY, m_bRootSelected );
                    
                    aContextMenu.EnableItem( SID_FM_AUTOCONTROLFOCUS, m_bRootSelected );

                    
                    
                    if (!m_bRootSelected && !m_nFormsSelected && (m_nControlsSelected == 1))
                    {
                        aContextMenu.SetPopupMenu( SID_FM_CHANGECONTROLTYPE, FmXFormShell::GetConversionMenu() );
#if OSL_DEBUG_LEVEL > 0
                        FmControlData* pCurrent = (FmControlData*)(*m_arrCurrentSelection.begin())->GetUserData();
                        OSL_ENSURE( pFormShell->GetImpl()->isSolelySelected( pCurrent->GetFormComponent() ),
                            "NavigatorTree::Command: inconsistency between the navigator selection, and the selection as the shell knows it!" );
#endif

                        pFormShell->GetImpl()->checkControlConversionSlotsForCurrentSelection( *aContextMenu.GetPopupMenu( SID_FM_CHANGECONTROLTYPE ) );
                    }
                    else
                        aContextMenu.EnableItem( SID_FM_CHANGECONTROLTYPE, false );

                    
                    aContextMenu.RemoveDisabledEntries(true, true);
                    
                    

                    aContextMenu.CheckItem( SID_FM_OPEN_READONLY, pFormModel->GetOpenInDesignMode() );
                    aContextMenu.CheckItem( SID_FM_AUTOCONTROLFOCUS, pFormModel->GetAutoControlFocus() );

                    sal_uInt16 nSlotId = aContextMenu.Execute( this, ptWhere );
                    switch( nSlotId )
                    {
                        case SID_FM_NEW_FORM:
                        {
                            OUString aStr(SVX_RESSTR(RID_STR_FORM));
                            OUString aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_INSERT).replaceAll(OUString('#'), aStr);

                            pFormModel->BegUndo(aUndoStr);
                            
                            
                            NewForm( *m_arrCurrentSelection.begin() );
                            pFormModel->EndUndo();

                        }   break;
                        case SID_FM_NEW_HIDDEN:
                        {
                            OUString aStr(SVX_RESSTR(RID_STR_CONTROL));
                            OUString aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_INSERT).replaceAll(OUString('#'), aStr);

                            pFormModel->BegUndo(aUndoStr);
                            
                            OUString fControlName = FM_COMPONENT_HIDDEN;
                            NewControl( fControlName, *m_arrCurrentSelection.begin() );
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
                            
                            SvTreeListEntry* pSelectedForm = *m_arrCurrentSelection.begin();
                            DBG_ASSERT( IsFormEntry(pSelectedForm), "NavigatorTree::Command: Dieser Eintrag muss ein FormEntry sein." );

                            FmFormData* pFormData = (FmFormData*)pSelectedForm->GetUserData();
                            Reference< XForm >  xForm(  pFormData->GetFormIface());

                            Reference< XTabControllerModel >  xTabController(xForm, UNO_QUERY);
                            if( !xTabController.is() )
                                break;
                            GetNavModel()->GetFormShell()->GetImpl()->ExecuteTabOrderDialog( xTabController );
                        }
                        break;

                        case SID_FM_SHOW_PROPERTY_BROWSER:
                        {
                            ShowSelectionProperties(sal_True);
                        }
                        break;
                        case SID_FM_RENAME_OBJECT:
                        {
                            
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
                            if (pFormShell->GetImpl()->isControlConversionSlot(nSlotId))
                            {
                                FmControlData* pCurrent = (FmControlData*)(*m_arrCurrentSelection.begin())->GetUserData();
                                if ( pFormShell->GetImpl()->executeControlConversionSlot( pCurrent->GetFormComponent(), nSlotId ) )
                                    ShowSelectionProperties();
                            }
                    }
                }
                bHandled = sal_True;
            } break;
        }

        if (!bHandled)
            SvTreeListBox::Command( rEvt );
    }

    
    SvTreeListEntry* NavigatorTree::FindEntry( FmEntryData* pEntryData )
    {
        if( !pEntryData ) return NULL;
        SvTreeListEntry* pCurEntry = First();
        FmEntryData* pCurEntryData;
        while( pCurEntry )
        {
            pCurEntryData = (FmEntryData*)pCurEntry->GetUserData();
            if( pCurEntryData && pCurEntryData->IsEqualWithoutChildren(pEntryData) )
                return pCurEntry;

            pCurEntry = Next( pCurEntry );
        }

        return NULL;
    }

    
    void NavigatorTree::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
    {
        if( rHint.ISA(FmNavRemovedHint) )
        {
            FmNavRemovedHint* pRemovedHint = (FmNavRemovedHint*)&rHint;
            FmEntryData* pEntryData = pRemovedHint->GetEntryData();
            Remove( pEntryData );
        }

        else if( rHint.ISA(FmNavInsertedHint) )
        {
            FmNavInsertedHint* pInsertedHint = (FmNavInsertedHint*)&rHint;
            FmEntryData* pEntryData = pInsertedHint->GetEntryData();
            sal_uInt32 nRelPos = pInsertedHint->GetRelPos();
            Insert( pEntryData, nRelPos );
        }

        else if( rHint.ISA(FmNavModelReplacedHint) )
        {
            FmEntryData* pData = ((FmNavModelReplacedHint*)&rHint)->GetEntryData();
            SvTreeListEntry* pEntry = FindEntry( pData );
            if (pEntry)
            {   
                SetCollapsedEntryBmp( pEntry, pData->GetNormalImage() );
                SetExpandedEntryBmp( pEntry, pData->GetNormalImage() );
            }
        }

        else if( rHint.ISA(FmNavNameChangedHint) )
        {
            FmNavNameChangedHint* pNameChangedHint = (FmNavNameChangedHint*)&rHint;
            SvTreeListEntry* pEntry = FindEntry( pNameChangedHint->GetEntryData() );
            SetEntryText( pEntry, pNameChangedHint->GetNewName() );
        }

        else if( rHint.ISA(FmNavClearedHint) )
        {
            SvTreeListBox::Clear();

            
            
            Image aRootImage( m_aNavigatorImages.GetImage( RID_SVXIMG_FORMS ) );
            m_pRootEntry = InsertEntry( SVX_RESSTR(RID_STR_FORMS), aRootImage, aRootImage,
                NULL, sal_False, 0, NULL );
        }
        else if (!m_bMarkingObjects && rHint.ISA(FmNavRequestSelectHint))
        {   
            
            FmNavRequestSelectHint* pershHint = (FmNavRequestSelectHint*)&rHint;
            FmEntryDataArray& arredToSelect = pershHint->GetItems();
            SynchronizeSelection(arredToSelect);

            if (pershHint->IsMixedSelection())
                
                
                m_bPrevSelectionMixed = sal_True;
        }
    }

    
    SvTreeListEntry* NavigatorTree::Insert( FmEntryData* pEntryData, sal_uIntPtr nRelPos )
    {
        
        
        SvTreeListEntry* pParentEntry = FindEntry( pEntryData->GetParent() );
        SvTreeListEntry* pNewEntry;

        if( !pParentEntry )
            pNewEntry = InsertEntry( pEntryData->GetText(),
                pEntryData->GetNormalImage(), pEntryData->GetNormalImage(),
                m_pRootEntry, sal_False, nRelPos, pEntryData );

        else
            pNewEntry = InsertEntry( pEntryData->GetText(),
                pEntryData->GetNormalImage(), pEntryData->GetNormalImage(),
                pParentEntry, sal_False, nRelPos, pEntryData );

        
        
        if( !pParentEntry )
            Expand( m_pRootEntry );

        
        
        FmEntryDataList* pChildList = pEntryData->GetChildList();
        size_t nChildCount = pChildList->size();
        FmEntryData* pChildData;
        for( size_t i = 0; i < nChildCount; i++ )
        {
            pChildData = pChildList->at( i );
            Insert( pChildData, LIST_APPEND );
        }

        return pNewEntry;
    }

    
    void NavigatorTree::Remove( FmEntryData* pEntryData )
    {
        if( !pEntryData )
            return;

        
        SvTreeListEntry* pEntry = FindEntry( pEntryData );
        if (!pEntry)
            return;

        
        
        
        
        
        LockSelectionHandling();

        
        
        Select(pEntry, sal_False);

        
        
        sal_uIntPtr nExpectedSelectionCount = GetSelectionCount();

        if( pEntry )
            GetModel()->Remove( pEntry );

        if (nExpectedSelectionCount != GetSelectionCount())
            SynchronizeSelection();

        
        UnlockSelectionHandling();
    }

    
    sal_Bool NavigatorTree::IsFormEntry( SvTreeListEntry* pEntry )
    {
        FmEntryData* pEntryData = (FmEntryData*)pEntry->GetUserData();
        return !pEntryData || pEntryData->ISA(FmFormData);
    }

    
    sal_Bool NavigatorTree::IsFormComponentEntry( SvTreeListEntry* pEntry )
    {
        FmEntryData* pEntryData = (FmEntryData*)pEntry->GetUserData();
        return pEntryData && pEntryData->ISA(FmControlData);
    }

    
    sal_Bool NavigatorTree::implAcceptPaste( )
    {
        SvTreeListEntry* pFirstSelected = FirstSelected();
        if ( !pFirstSelected || NextSelected( pFirstSelected ) )
            
            return sal_False;

        
        TransferableDataHelper aClipboardContent( TransferableDataHelper::CreateFromSystemClipboard( this ) );

        sal_Int8 nAction = m_aControlExchange.isClipboardOwner() && doingKeyboardCut( ) ? DND_ACTION_MOVE : DND_ACTION_COPY;
        return ( nAction == implAcceptDataTransfer( aClipboardContent.GetDataFlavorExVector(), nAction, pFirstSelected, sal_False ) );
    }

    
    sal_Int8 NavigatorTree::implAcceptDataTransfer( const DataFlavorExVector& _rFlavors, sal_Int8 _nAction, const ::Point& _rDropPos, sal_Bool _bDnD )
    {
        return implAcceptDataTransfer( _rFlavors, _nAction, GetEntry( _rDropPos ), _bDnD );
    }

    
    sal_Int8 NavigatorTree::implAcceptDataTransfer( const DataFlavorExVector& _rFlavors, sal_Int8 _nAction, SvTreeListEntry* _pTargetEntry, sal_Bool _bDnD )
    {
        
        if (!_pTargetEntry)
            return DND_ACTION_NONE;

        
        sal_Bool bHasDefControlFormat = OControlExchange::hasFieldExchangeFormat( _rFlavors );
        sal_Bool bHasControlPathFormat = OControlExchange::hasControlPathFormat( _rFlavors );
        sal_Bool bHasHiddenControlsFormat = OControlExchange::hasHiddenControlModelsFormat( _rFlavors );
        if (!bHasDefControlFormat && !bHasControlPathFormat && !bHasHiddenControlsFormat)
            return DND_ACTION_NONE;

        sal_Bool bSelfSource = _bDnD ? m_aControlExchange.isDragSource() : m_aControlExchange.isClipboardOwner();

        if ( bHasHiddenControlsFormat )
        {   

            
            if ( !_pTargetEntry || ( _pTargetEntry == m_pRootEntry ) || !IsFormEntry( _pTargetEntry ) )
                return DND_ACTION_NONE;

            return bSelfSource ? ( DND_ACTION_COPYMOVE & _nAction ) : DND_ACTION_COPY;
        }

        if  ( !bSelfSource )
        {
            
            
            

            
            

            return DND_ACTION_NONE;
        }

        DBG_ASSERT( _bDnD ? m_aControlExchange.isDragSource() : m_aControlExchange.isClipboardOwner(),
            "NavigatorTree::implAcceptDataTransfer: here only with source=dest!" );
            

        

        sal_Bool bForeignCollection = m_aControlExchange->getFormsRoot().get() != GetNavModel()->GetFormPage()->GetForms().get();
        if ( bForeignCollection )
        {
            
            
            
            DBG_ASSERT( !bHasHiddenControlsFormat, "NavigatorTree::implAcceptDataTransfer: still hidden controls format!" );
                

            return DND_ACTION_COPY;
        }

        if (DND_ACTION_MOVE != _nAction) 
            return DND_ACTION_NONE;

        if ( m_bDragDataDirty || !bHasDefControlFormat )
        {
            if (!bHasControlPathFormat)
                
                
                return DND_ACTION_NONE;

            
            
            m_aControlExchange->buildListFromPath(this, m_pRootEntry);
            m_bDragDataDirty = sal_False;
        }

        
        const ListBoxEntrySet& aDropped = m_aControlExchange->selected();
        DBG_ASSERT(aDropped.size() >= 1, "NavigatorTree::implAcceptDataTransfer: keine Eintraege !");

        sal_Bool bDropTargetIsComponent = IsFormComponentEntry( _pTargetEntry );
        

        
        
        
        
        
        
        
        

        
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

            
            if (pCurrent == m_pRootEntry)
                return DND_ACTION_NONE;

            
            if ( _pTargetEntry == pCurrentParent )
                return DND_ACTION_NONE;

            
            if (pCurrent == _pTargetEntry)
                return DND_ACTION_NONE;

            
    
            if ( bDropTargetIsComponent )   
                return DND_ACTION_NONE;

            
            if ( IsFormEntry(pCurrent) )
            {
                if ( arrDropAnchestors.find(pCurrent) != arrDropAnchestors.end() )
                    return DND_ACTION_NONE;
            } else if ( IsFormComponentEntry(pCurrent) )
            {
                
                if (_pTargetEntry == m_pRootEntry)
                    return DND_ACTION_NONE;
            }
        }

        return DND_ACTION_MOVE;
    }

    
    sal_Int8 NavigatorTree::AcceptDrop( const AcceptDropEvent& rEvt )
    {
        ::Point aDropPos = rEvt.maPosPixel;

        
        if (rEvt.mbLeaving)
        {
            if (m_aDropActionTimer.IsActive())
                m_aDropActionTimer.Stop();
        } else
        {
            sal_Bool bNeedTrigger = sal_False;
            
            if ((aDropPos.Y() >= 0) && (aDropPos.Y() < GetEntryHeight()))
            {
                m_aDropActionType = DA_SCROLLUP;
                bNeedTrigger = sal_True;
            } else
                
                
                if ((aDropPos.Y() < GetSizePixel().Height()) && (aDropPos.Y() >= GetSizePixel().Height() - GetEntryHeight()))
                {
                    m_aDropActionType = DA_SCROLLDOWN;
                    bNeedTrigger = sal_True;
                } else
                {   
                    SvTreeListEntry* pDropppedOn = GetEntry(aDropPos);
                    if (pDropppedOn && (GetChildCount(pDropppedOn) > 0) && !IsExpanded(pDropppedOn))
                    {
                        
                        m_aDropActionType = DA_EXPANDNODE;
                        bNeedTrigger = sal_True;
                    }
                }

            if (bNeedTrigger && (m_aTimerTriggered != aDropPos))
            {
                
                m_aTimerCounter = DROP_ACTION_TIMER_INITIAL_TICKS;
                
                m_aTimerTriggered = aDropPos;
                
                if (!m_aDropActionTimer.IsActive()) 
                {
                    m_aDropActionTimer.SetTimeout(DROP_ACTION_TIMER_TICK_BASE);
                    m_aDropActionTimer.Start();
                }
            } else if (!bNeedTrigger)
                m_aDropActionTimer.Stop();
        }

        return implAcceptDataTransfer( GetDataFlavorExVector(), rEvt.mnAction, aDropPos, sal_True );
    }

    
    sal_Int8 NavigatorTree::implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, const ::Point& _rDropPos, sal_Bool _bDnD )
    {
        return implExecuteDataTransfer( _rData, _nAction, GetEntry( _rDropPos ), _bDnD );
    }

    
    sal_Int8 NavigatorTree::implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, SvTreeListEntry* _pTargetEntry, sal_Bool _bDnD )
    {
        const DataFlavorExVector& rDataFlavors = _rData.GetDataFlavorExVector();

        if ( DND_ACTION_NONE == implAcceptDataTransfer( rDataFlavors, _nAction, _pTargetEntry, _bDnD ) )
            
            return DND_ACTION_NONE;

        
        if (m_aDropActionTimer.IsActive())
            m_aDropActionTimer.Stop();

        if (!_pTargetEntry)
            
            return DND_ACTION_NONE;

        
#ifdef DBG_UTIL
        sal_Bool bHasHiddenControlsFormat = OControlExchange::hasHiddenControlModelsFormat( rDataFlavors );
        sal_Bool bForeignCollection = _rData.getFormsRoot().get() != GetNavModel()->GetFormPage()->GetForms().get();
        DBG_ASSERT(!bForeignCollection || bHasHiddenControlsFormat, "NavigatorTree::implExecuteDataTransfer: invalid format (AcceptDrop shouldn't have let this pass) !");
        DBG_ASSERT(bForeignCollection || !m_bDragDataDirty, "NavigatorTree::implExecuteDataTransfer: invalid state (shell changed since last exchange resync) !");
            
            
#endif

        if ( DND_ACTION_COPY == _nAction )
        {   
            DBG_ASSERT( bHasHiddenControlsFormat, "NavigatorTree::implExecuteDataTransfer: copy allowed for hidden controls only!" );
            DBG_ASSERT( _pTargetEntry && ( _pTargetEntry != m_pRootEntry ) && IsFormEntry( _pTargetEntry ),
                "NavigatorTree::implExecuteDataTransfer: should not be here!" );
                

            DBG_ASSERT(bHasHiddenControlsFormat, "NavigatorTree::implExecuteDataTransfer: only copying of hidden controls is supported !");
                

            
            SelectAll(sal_False);

            Sequence< Reference< XInterface > > aControls = _rData.hiddenControls();
            sal_Int32 nCount = aControls.getLength();
            const Reference< XInterface >* pControls = aControls.getConstArray();

            FmFormShell* pFormShell = GetNavModel()->GetFormShell();
            FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;

            
            if (pFormModel)
            {
                OUString aStr(SVX_RESSTR(RID_STR_CONTROL));
                OUString aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_INSERT).replaceAll(OUString('#'), aStr);
                pFormModel->BegUndo(aUndoStr);
            }

            
            for (sal_Int32 i=0; i<nCount; ++i)
            {
                
                OUString fControlName = FM_COMPONENT_HIDDEN;
                FmControlData* pNewControlData = NewControl( fControlName, _pTargetEntry, sal_False);
                Reference< XPropertySet >  xNewPropSet( pNewControlData->GetPropertySet() );

                
                Reference< XPropertySet >  xCurrent(pControls[i], UNO_QUERY);
#if (OSL_DEBUG_LEVEL > 1)
                
                sal_Int16 nClassId = ::comphelper::getINT16(xCurrent->getPropertyValue(FM_PROP_CLASSID));
                OSL_ENSURE(nClassId == FormComponentType::HIDDENCONTROL, "NavigatorTree::implExecuteDataTransfer: invalid control in drop list !");
                    
                    
#endif 
                Reference< XPropertySetInfo >  xPropInfo( xCurrent->getPropertySetInfo());
                Sequence< Property> seqAllCurrentProps = xPropInfo->getProperties();
                Property* pAllCurrentProps = seqAllCurrentProps.getArray();
                for (sal_Int32 j=0; j<seqAllCurrentProps.getLength(); ++j)
                {
                    OUString sCurrentProp = pAllCurrentProps[j].Name;
                    if (((pAllCurrentProps[j].Attributes & PropertyAttribute::READONLY) == 0) && (sCurrentProp != FM_PROP_NAME))
                    {   
                        
                        xNewPropSet->setPropertyValue(sCurrentProp, xCurrent->getPropertyValue(sCurrentProp));
                    }
                }

                SvTreeListEntry* pToSelect = FindEntry(pNewControlData);
                Select(pToSelect, sal_True);
                if (i == 0)
                    SetCurEntry(pToSelect);
            }

            if (pFormModel)
                pFormModel->EndUndo();

            return _nAction;
        }

        if ( !OControlExchange::hasFieldExchangeFormat( _rData.GetDataFlavorExVector() ) )
        {
            
            
            return DND_ACTION_NONE;
        }

        
        sal_Bool bDropTargetIsForm = IsFormEntry(_pTargetEntry);
        FmFormData* pTargetData = bDropTargetIsForm ? (FmFormData*)_pTargetEntry->GetUserData() : NULL;

        DBG_ASSERT( DND_ACTION_COPY != _nAction, "NavigatorTree::implExecuteDataTransfer: somebody changed the logics!" );

        
        ListBoxEntrySet aDropped = _rData.selected();
        DBG_ASSERT(aDropped.size() >= 1, "NavigatorTree::implExecuteDataTransfer: no entries!");

        
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;
        if (!pFormModel)
            return DND_ACTION_NONE;

        
        const bool bUndo = pFormModel->IsUndoEnabled();

        if( bUndo )
        {
            OUString strUndoDescription(SVX_RESSTR(RID_STR_UNDO_CONTAINER_REPLACE));
            pFormModel->BegUndo(strUndoDescription);
        }

        
        
        LockSelectionHandling();

        
        for (   ListBoxEntrySet::const_iterator dropped = aDropped.begin();
                dropped != aDropped.end();
                ++dropped
            )
        {
            
            SvTreeListEntry* pCurrent = *dropped;
            DBG_ASSERT(pCurrent != NULL, "NavigatorTree::implExecuteDataTransfer: ungueltiger Eintrag");
            DBG_ASSERT(GetParent(pCurrent) != NULL, "NavigatorTree::implExecuteDataTransfer: ungueltiger Eintrag");
                

            FmEntryData* pCurrentUserData = (FmEntryData*)pCurrent->GetUserData();

            Reference< XChild >  xCurrentChild(pCurrentUserData->GetChildIFace(), UNO_QUERY);
            Reference< XIndexContainer >  xContainer(xCurrentChild->getParent(), UNO_QUERY);

            FmFormData* pCurrentParentUserData = (FmFormData*)pCurrentUserData->GetParent();
            DBG_ASSERT(pCurrentParentUserData == NULL || pCurrentParentUserData->ISA(FmFormData), "NavigatorTree::implExecuteDataTransfer: ungueltiges Parent");

            
            if (pCurrentParentUserData)
                pCurrentParentUserData->GetChildList()->remove( pCurrentUserData );
            else
                GetNavModel()->GetRootList()->remove( pCurrentUserData );

            
            sal_Int32 nIndex = getElementPos(xContainer, xCurrentChild);
            GetNavModel()->m_pPropChangeList->Lock();
            
            if ( bUndo && GetNavModel()->m_pPropChangeList->CanUndo())
            {
                pFormModel->AddUndo(new FmUndoContainerAction(*pFormModel, FmUndoContainerAction::Removed,
                                                            xContainer, xCurrentChild, nIndex));
            }
            else if( !GetNavModel()->m_pPropChangeList->CanUndo() )
            {
                FmUndoContainerAction::DisposeElement( xCurrentChild );
            }

            
            Reference< XEventAttacherManager >  xManager(xContainer, UNO_QUERY);
            Sequence< ScriptEventDescriptor > aEvts;

            if (xManager.is() && nIndex >= 0)
                aEvts = xManager->getScriptEvents(nIndex);
            xContainer->removeByIndex(nIndex);

            
            Select(pCurrent, sal_False);
            
            Remove(pCurrentUserData);

            
            if (pTargetData)
                xContainer = Reference< XIndexContainer > (pTargetData->GetElement(), UNO_QUERY);
            else
                xContainer = Reference< XIndexContainer > (GetNavModel()->GetForms(), UNO_QUERY);

            
            nIndex = xContainer->getCount();

            
            if ( bUndo && GetNavModel()->m_pPropChangeList->CanUndo())
                pFormModel->AddUndo(new FmUndoContainerAction(*pFormModel, FmUndoContainerAction::Inserted,
                                                         xContainer, xCurrentChild, nIndex));

            
            if (pTargetData)
            {
                 
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
                xManager = Reference< XEventAttacherManager > (xContainer, UNO_QUERY);
                if (xManager.is())
                    xManager->registerScriptEvents(nIndex, aEvts);
            }

            GetNavModel()->m_pPropChangeList->UnLock();

            
            pCurrentUserData->SetParent(pTargetData);

            
            if (pTargetData)
                pTargetData->GetChildList()->insert( pCurrentUserData, nIndex );
            else
                GetNavModel()->GetRootList()->insert( pCurrentUserData, nIndex );

            
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

        
        
        
        SynchronizeSelection();

        
        
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
            nResult = implExecuteDataTransfer( *m_aControlExchange, rEvt.mnAction, rEvt.maPosPixel, sal_True );
        else
        {
            OControlTransferData aDroppedData( rEvt.maDropEvent.Transferable );
            nResult = implExecuteDataTransfer( aDroppedData, rEvt.mnAction, rEvt.maPosPixel, sal_True );
        }

        return nResult;
    }

    
    void NavigatorTree::doPaste()
    {
           try
        {
            if ( m_aControlExchange.isClipboardOwner() )
            {
                implExecuteDataTransfer( *m_aControlExchange, doingKeyboardCut( ) ? DND_ACTION_MOVE : DND_ACTION_COPY, FirstSelected(), sal_False );
            }
            else
            {
                
                Reference< XClipboard > xClipboard( GetClipboard() );
                Reference< XTransferable > xTransferable;
                if ( xClipboard.is() )
                    xTransferable = xClipboard->getContents();

                OControlTransferData aClipboardContent( xTransferable );
                implExecuteDataTransfer( aClipboardContent, DND_ACTION_COPY, FirstSelected(), sal_False );
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
        SvTreeListEntry* pTypedEntry = static_cast< SvTreeListEntry* >( _pEntry );
        if ( doingKeyboardCut() )
            m_aCutEntries.erase( pTypedEntry );

        if ( m_aControlExchange.isDataExchangeActive() )
        {
            if ( 0 == m_aControlExchange->onEntryRemoved( pTypedEntry ) )
            {
                
                
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
            m_bKeyboardCut = sal_True;

            
            for ( SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
                  it != m_arrCurrentSelection.end(); ++it )
            {
                SvTreeListEntry* pEntry = *it;
                if ( pEntry )
                {
                    m_aCutEntries.insert( pEntry );
                    pEntry->SetFlags( pEntry->GetFlags() | SV_ENTRYFLAG_SEMITRANSPARENT );
                    InvalidateEntry( pEntry );
                }
            }
        }
    }

    
    void NavigatorTree::KeyInput(const ::KeyEvent& rKEvt)
    {
        const KeyCode& rCode = rKEvt.GetKeyCode();

        
        if (rKEvt.GetKeyCode().GetCode() == KEY_DELETE && !rKEvt.GetKeyCode().GetModifier())
        {
            DeleteSelection();
            return;
        }

        
        switch ( rCode.GetFunction() )
        {
            case KEYFUNC_CUT:
                doCut();
                break;

            case KEYFUNC_PASTE:
                if ( implAcceptPaste() )
                    doPaste();
                break;

            case KEYFUNC_COPY:
                doCopy();
                break;

            default:
                break;
        }

        SvTreeListBox::KeyInput(rKEvt);
    }

    
    sal_Bool NavigatorTree::EditingEntry( SvTreeListEntry* pEntry, Selection& rSelection )
    {
        if (!SvTreeListBox::EditingEntry( pEntry, rSelection ))
            return sal_False;

        return (pEntry && (pEntry->GetUserData() != NULL));
            
    }

    
    void NavigatorTree::NewForm( SvTreeListEntry* pParentEntry )
    {
        
        
        if( !IsFormEntry(pParentEntry) )
            return;

        FmFormData* pParentFormData = (FmFormData*)pParentEntry->GetUserData();

        
        
        Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
        Reference< XForm >  xNewForm(xContext->getServiceManager()->createInstanceWithContext(FM_SUN_COMPONENT_FORM, xContext), UNO_QUERY);
        if (!xNewForm.is())
            return;

        FmFormData* pNewFormData = new FmFormData( xNewForm, m_aNavigatorImages, pParentFormData );

        
        
        OUString aName = GenerateName(pNewFormData);
        pNewFormData->SetText(aName);

        Reference< XPropertySet >  xPropertySet(xNewForm, UNO_QUERY);
        if (!xPropertySet.is())
            return;
        try
        {
            xPropertySet->setPropertyValue( FM_PROP_NAME, makeAny(aName) );
            
            xPropertySet->setPropertyValue( FM_PROP_COMMANDTYPE, makeAny(sal_Int32(CommandType::TABLE)));
        }
        catch ( const Exception& )
        {
            OSL_FAIL("NavigatorTree::NewForm : could not set esssential properties !");
        }


        
        
        GetNavModel()->Insert( pNewFormData, LIST_APPEND, sal_True );

        
        
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( pFormShell )
        {
            InterfaceBag aSelection;
            aSelection.insert( xNewForm );
            pFormShell->GetImpl()->setCurrentSelection( aSelection );

            pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_PROPERTIES,sal_True,sal_True);
        }
        GetNavModel()->SetModified();

        
        
        SvTreeListEntry* pNewEntry = FindEntry( pNewFormData );
        EditEntry( pNewEntry );
    }

    
    FmControlData* NavigatorTree::NewControl( const OUString& rServiceName, SvTreeListEntry* pParentEntry, sal_Bool bEditName )
    {
        
        
        if (!GetNavModel()->GetFormShell())
            return NULL;
        if (!IsFormEntry(pParentEntry))
            return NULL;

        FmFormData* pParentFormData = (FmFormData*)pParentEntry->GetUserData();;
        Reference< XForm >  xParentForm( pParentFormData->GetFormIface());

        
        
        Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
        Reference<XFormComponent> xNewComponent( xContext->getServiceManager()->createInstanceWithContext(rServiceName, xContext), UNO_QUERY);
        if (!xNewComponent.is())
            return NULL;

        FmControlData* pNewFormControlData = new FmControlData( xNewComponent, m_aNavigatorImages, pParentFormData );

        
        
        FmFormView*     pFormView       = GetNavModel()->GetFormShell()->GetFormView();
        SdrPageView*    pPageView       = pFormView->GetSdrPageView();
        FmFormPage*     pPage           = (FmFormPage*)pPageView->GetPage();

        OUString sName = pPage->GetImpl().setUniqueName( xNewComponent, xParentForm );

        pNewFormControlData->SetText( sName );

        
        
        GetNavModel()->Insert( pNewFormControlData, LIST_APPEND, sal_True );
        GetNavModel()->SetModified();

        if (bEditName)
        {
            
            
            SvTreeListEntry* pNewEntry = FindEntry( pNewFormControlData );
            Select( pNewEntry, sal_True );
            EditEntry( pNewEntry );
        }

        return pNewFormControlData;
    }

    
    OUString NavigatorTree::GenerateName( FmEntryData* pEntryData )
    {
        const sal_uInt16 nMaxCount = 99;
        OUString aNewName;

        
        
        OUString aBaseName;
        if( pEntryData->ISA(FmFormData) )
            aBaseName = SVX_RESSTR( RID_STR_STDFORMNAME );
        else if( pEntryData->ISA(FmControlData) )
            aBaseName = SVX_RESSTR( RID_STR_CONTROL );

        
        
        FmFormData* pFormParentData = (FmFormData*)pEntryData->GetParent();

        for( sal_Int32 i=0; i<nMaxCount; i++ )
        {
            aNewName = aBaseName;
            if( i>0 )
            {
                aNewName += " ";
                aNewName += OUString::number(i).getStr();
            }

            if( GetNavModel()->FindData(aNewName, pFormParentData,sal_False) == NULL )
                break;
        }

        return aNewName;
    }

    
    sal_Bool NavigatorTree::EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText )
    {
        if (EditingCanceled())
            return sal_True;

        GrabFocus();
        FmEntryData* pEntryData = (FmEntryData*)pEntry->GetUserData();
        sal_Bool bRes = GetNavModel()->Rename( pEntryData, rNewText);
        if( !bRes )
        {
            m_pEditEntry = pEntry;
            nEditEvent = Application::PostUserEvent( LINK(this, NavigatorTree, OnEdit) );
        } else
            SetCursor(pEntry, sal_True);

        return bRes;
    }

    
    IMPL_LINK_NOARG(NavigatorTree, OnEdit)
    {
        nEditEvent = 0;
        EditEntry( m_pEditEntry );
        m_pEditEntry = NULL;

        return 0L;
    }

    
    IMPL_LINK_NOARG(NavigatorTree, OnDropActionTimer)
    {
        if (--m_aTimerCounter > 0)
            return 0L;

        switch ( m_aDropActionType )
        {
        case DA_EXPANDNODE:
        {
            SvTreeListEntry* pToExpand = GetEntry(m_aTimerTriggered);
            if (pToExpand && (GetChildCount(pToExpand) > 0) &&  !IsExpanded(pToExpand))
                
                
                
                Expand(pToExpand);

            
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

        return 0L;
    }

    
    IMPL_LINK(NavigatorTree, OnEntrySelDesel, NavigatorTree*, /*pThis*/)
    {
        m_sdiState = SDI_DIRTY;

        if (IsSelectionHandlingLocked())
            return 0L;

        if (m_aSynchronizeTimer.IsActive())
            m_aSynchronizeTimer.Stop();

        m_aSynchronizeTimer.SetTimeout(EXPLORER_SYNC_DELAY);
        m_aSynchronizeTimer.Start();

        return 0L;
    }

    
    IMPL_LINK_NOARG(NavigatorTree, OnSynchronizeTimer)
    {
        SynchronizeMarkList();
        return 0L;
    }


    
    IMPL_LINK_NOARG(NavigatorTree, OnClipboardAction)
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

                    pEntry->SetFlags( pEntry->GetFlags() & ~SV_ENTRYFLAG_SEMITRANSPARENT );
                    InvalidateEntry( pEntry );
                }
                ListBoxEntrySet aEmpty;
                m_aCutEntries.swap( aEmpty );

                m_bKeyboardCut = sal_False;
            }
        }
        return 0L;
    }

    
    void NavigatorTree::ShowSelectionProperties(sal_Bool bForce)
    {
        
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell)
            
            return;

        CollectSelectionData(SDI_ALL);
        SAL_WARN_IF(static_cast<size_t>(m_nFormsSelected + m_nControlsSelected
                + (m_bRootSelected ? 1 : 0)) != m_arrCurrentSelection.size(),
            "svx.form",
            "NavigatorTree::ShowSelectionProperties : selection meta data invalid !");


        InterfaceBag aSelection;
        sal_Bool bSetSelectionAsMarkList = sal_False;

        if (m_bRootSelected)
            ;                                   
        else if ( m_nFormsSelected + m_nControlsSelected == 0 )   
            ;                                   
        else if ( m_nFormsSelected * m_nControlsSelected != 0 )
            ;                                   
        else
        {   
            if (m_arrCurrentSelection.size() == 1)
            {
                if (m_nFormsSelected > 0)
                {   
                    FmFormData* pFormData = (FmFormData*)(*m_arrCurrentSelection.begin())->GetUserData();
                    aSelection.insert( Reference< XInterface >( pFormData->GetFormIface(), UNO_QUERY ) );
                }
                else
                {   
                    FmEntryData* pEntryData = (FmEntryData*)(*m_arrCurrentSelection.begin())->GetUserData();

                    aSelection.insert( Reference< XInterface >( pEntryData->GetElement(), UNO_QUERY ) );
                }
            }
            else
            {   
                if (m_nFormsSelected > 0)
                {   
                    
                   SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
                    for ( sal_Int32 i = 0; i < m_nFormsSelected; ++i )
                    {
                        FmFormData* pFormData = (FmFormData*)(*it)->GetUserData();
                        aSelection.insert( pFormData->GetPropertySet().get() );
                        ++it;
                    }
                }
                else
                {   
                    if (m_nHiddenControls == m_nControlsSelected)
                    {   
                        SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
                        for ( sal_Int32 i = 0; i < m_nHiddenControls; ++i )
                        {
                            FmEntryData* pEntryData = (FmEntryData*)(*it)->GetUserData();
                            aSelection.insert( pEntryData->GetPropertySet().get() );
                            ++it;
                        }
                    }
                    else if (m_nHiddenControls == 0)
                    {   
                        bSetSelectionAsMarkList = sal_True;
                    }
                }
            }

        }

        
        if ( bSetSelectionAsMarkList )
            pFormShell->GetImpl()->setCurrentSelectionFromMark( pFormShell->GetFormView()->GetMarkedObjectList() );
        else
            pFormShell->GetImpl()->setCurrentSelection( aSelection );

        if ( pFormShell->GetImpl()->IsPropBrwOpen() || bForce )
        {
            
            pFormShell->GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_SHOW_PROPERTY_BROWSER, SFX_CALLMODE_ASYNCHRON );
        }
    }

    
    void NavigatorTree::DeleteSelection()
    {
        
        sal_Bool bRootSelected = IsSelected(m_pRootEntry);
        sal_uIntPtr nSelectedEntries = GetSelectionCount();
        if (bRootSelected && (nSelectedEntries > 1))     
            Select(m_pRootEntry, sal_False);                

        if ((nSelectedEntries == 0) || bRootSelected)    
            return;                                     

        DBG_ASSERT(!m_bPrevSelectionMixed, "NavigatorTree::DeleteSelection() : loeschen nicht erlaubt wenn Markierung und Selektion nciht konsistent");

        
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell)
            return;
        FmFormModel* pFormModel = pFormShell->GetFormModel();
        if (!pFormModel)
            return;

        
        
        
        
        CollectSelectionData( SDI_NORMALIZED );

        
        FmFormView*     pFormView       = pFormShell->GetFormView();
        SdrPageView*    pPageView       = pFormView ? pFormView->GetSdrPageView() : NULL;
        SdrPage*        pPage           = pPageView ? pPageView->GetPage() : NULL;
        DBG_ASSERT( pPage, "NavigatorTree::DeleteSelection: invalid form page!" );

        MapModelToShape aModelShapes;
        if ( pPage )
            collectShapeModelMapping( pPage, aModelShapes );

        
        
        
        
        
        
        
        pFormShell->GetImpl()->EnableTrackProperties(sal_False);
        for (SvLBoxEntrySortedArray::reverse_iterator it = m_arrCurrentSelection.rbegin();
             it != m_arrCurrentSelection.rend(); )
        {
            FmEntryData* pCurrent = (FmEntryData*)((*it)->GetUserData());

            
            sal_Bool bIsForm = pCurrent->ISA(FmFormData);

            
            
            
            if (bIsForm)
                MarkViewObj((FmFormData*)pCurrent, sal_True, sal_True);     

            
            sal_Bool bIsHidden = IsHiddenControl(pCurrent);

            
            if (!bIsForm && !bIsHidden)
            {
                
                
                
                if ( aModelShapes.find( pCurrent->GetElement() ) != aModelShapes.end() )
                {
                    
                    
                    
                    
                    m_arrCurrentSelection.erase( --(it.base()) );
                }
                else
                   ++it;
                
                
                
            }
            else
                ++it;
        }
        pFormShell->GetImpl()->EnableTrackProperties(sal_True);

        
        pFormShell->GetFormView()->DeleteMarked();

        
        
        
        
        {
            
            
            OUString aUndoStr;
            if ( m_arrCurrentSelection.size() == 1 )
            {
                aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_REMOVE);
                if (m_nFormsSelected)
                    aUndoStr = aUndoStr.replaceFirst( "#", SVX_RESSTR( RID_STR_FORM ) );
                else
                    
                    aUndoStr = aUndoStr.replaceFirst( "#", SVX_RESSTR( RID_STR_CONTROL ) );
            }
            else
            {
                aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_REMOVE_MULTIPLE);
                aUndoStr = aUndoStr.replaceFirst( "#", OUString::number( m_arrCurrentSelection.size() ) );
            }
            pFormModel->BegUndo(aUndoStr);
        }

        
        for (SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
             it != m_arrCurrentSelection.end(); ++it)
        {
            FmEntryData* pCurrent = (FmEntryData*)((*it)->GetUserData());

            
            
            
            if ( pCurrent->GetChildList()->size() )
                continue;

            
            
            if (pCurrent->ISA(FmFormData))
            {
                Reference< XForm >  xCurrentForm( static_cast< FmFormData* >( pCurrent )->GetFormIface() );
                if ( pFormShell->GetImpl()->getCurrentForm() == xCurrentForm )  
                    pFormShell->GetImpl()->forgetCurrentForm();                 
            }
            GetNavModel()->Remove(pCurrent, sal_True);
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
        m_bRootSelected = sal_False;

        SvTreeListEntry* pSelectionLoop = FirstSelected();
        while (pSelectionLoop)
        {
            
            if (pSelectionLoop == m_pRootEntry)
                m_bRootSelected = sal_True;
            else
            {
                if (IsFormEntry(pSelectionLoop))
                    ++m_nFormsSelected;
                else
                {
                    ++m_nControlsSelected;
                    if (IsHiddenControl((FmEntryData*)(pSelectionLoop->GetUserData())))
                        ++m_nHiddenControls;
                }
            }

            if (sdiHow == SDI_NORMALIZED)
            {
                
                if (pSelectionLoop == m_pRootEntry)
                    m_arrCurrentSelection.insert(pSelectionLoop);
                else
                {
                    SvTreeListEntry* pParentLoop = GetParent(pSelectionLoop);
                    while (pParentLoop)
                    {
                        
                        
                        
                        if (IsSelected(pParentLoop))
                            break;
                        else
                        {
                            if (m_pRootEntry == pParentLoop)
                            {
                                
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
            SelectAll(sal_False);
        }
        else
        {
            
            SvTreeListEntry* pSelection = FirstSelected();
            while (pSelection)
            {
                FmEntryData* pCurrent = (FmEntryData*)pSelection->GetUserData();
                if (pCurrent != NULL)
                {
                    FmEntryDataArray::iterator it = arredToSelect.find(pCurrent);
                    if ( it != arredToSelect.end() )
                    {   
                        
                        arredToSelect.erase(it);
                    } else
                    {   
                        Select(pSelection, sal_False);
                        
                        
                        MakeVisible(pSelection);
                    }
                }
                else
                    Select(pSelection, sal_False);

                pSelection = NextSelected(pSelection);
            }

            
            
            
            
            
            
            
            
            
            SvTreeListEntry* pLoop = First();
            while( pLoop )
            {
                FmEntryData* pCurEntryData = (FmEntryData*)pLoop->GetUserData();
                FmEntryDataArray::iterator it = arredToSelect.find(pCurEntryData);
                if ( it != arredToSelect.end() )
                {
                    Select(pLoop, sal_True);
                    MakeVisible(pLoop);
                    SetCursor(pLoop, sal_True);
                }

                pLoop = Next( pLoop );
            }
        }
        UnlockSelectionHandling();
    }

    
    void NavigatorTree::SynchronizeSelection()
    {
        
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if(!pFormShell) return;

        FmFormView* pFormView = pFormShell->GetFormView();
        if (!pFormView) return;

        GetNavModel()->BroadcastMarkedObjects(pFormView->GetMarkedObjectList());
    }

    
    void NavigatorTree::SynchronizeMarkList()
    {
        
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell) return;

        CollectSelectionData(SDI_NORMALIZED_FORMARK);

        
        pFormShell->GetImpl()->EnableTrackProperties(sal_False);

        UnmarkAllViewObj();

        for (SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
             it != m_arrCurrentSelection.end(); ++it)
        {
            SvTreeListEntry* pSelectionLoop = *it;
            
            if (IsFormEntry(pSelectionLoop) && (pSelectionLoop != m_pRootEntry))
                MarkViewObj((FmFormData*)pSelectionLoop->GetUserData(), sal_True, sal_False);

            
            else if (IsFormComponentEntry(pSelectionLoop))
            {
                FmControlData* pControlData = (FmControlData*)pSelectionLoop->GetUserData();
                if (pControlData)
                {
                    
                    
                    Reference< XFormComponent >  xFormComponent( pControlData->GetFormComponent());
                    if (!xFormComponent.is())
                        continue;
                    Reference< XPropertySet >  xSet(xFormComponent, UNO_QUERY);
                    if (!xSet.is())
                        continue;

                    sal_uInt16 nClassId = ::comphelper::getINT16(xSet->getPropertyValue(FM_PROP_CLASSID));
                    if (nClassId != FormComponentType::HIDDENCONTROL)
                        MarkViewObj(pControlData, sal_True, sal_True);
                }
            }
        }

        
        
        
        
        ShowSelectionProperties(sal_False);

        
        pFormShell->GetImpl()->EnableTrackProperties(sal_True);

        
        
        
        if ((m_arrCurrentSelection.size() == 1) && (m_nFormsSelected == 1))
        {
            FmFormData* pSingleSelectionData = PTR_CAST( FmFormData, static_cast< FmEntryData* >( FirstSelected()->GetUserData() ) );
            DBG_ASSERT( pSingleSelectionData, "NavigatorTree::SynchronizeMarkList: invalid selected form!" );
            if ( pSingleSelectionData )
            {
                InterfaceBag aSelection;
                aSelection.insert( Reference< XInterface >( pSingleSelectionData->GetFormIface(), UNO_QUERY ) );
                pFormShell->GetImpl()->setCurrentSelection( aSelection );
            }
        }
    }

    
    sal_Bool NavigatorTree::IsHiddenControl(FmEntryData* pEntryData)
    {
        if (pEntryData == NULL) return sal_False;

        Reference< XPropertySet > xProperties( pEntryData->GetPropertySet() );
        if (::comphelper::hasProperty(FM_PROP_CLASSID, xProperties))
        {
            Any aClassID = xProperties->getPropertyValue( FM_PROP_CLASSID );
            return (::comphelper::getINT16(aClassID) == FormComponentType::HIDDENCONTROL);
        }
        return sal_False;
    }

    
    sal_Bool NavigatorTree::Select( SvTreeListEntry* pEntry, sal_Bool bSelect )
    {
        if (bSelect == IsSelected(pEntry))  
            return sal_True;

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
    
    void NavigatorTree::MarkViewObj(FmFormData* pFormData, sal_Bool bMark, sal_Bool bDeep )
    {
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( !pFormShell )
            return;

        
        ::std::set< Reference< XFormComponent > > aObjects;
        CollectObjects(pFormData,bDeep,aObjects);

        
        
        FmFormView*     pFormView       = pFormShell->GetFormView();
        SdrPageView*    pPageView       = pFormView->GetSdrPageView();
        SdrPage*        pPage           = pPageView->GetPage();
        

        SdrObjListIter aIter( *pPage );
        while ( aIter.IsMore() )
        {
            SdrObject* pSdrObject = aIter.Next();
            FmFormObj* pFormObject = FmFormObj::GetFormObject( pSdrObject );
            if ( !pFormObject )
                continue;

            Reference< XFormComponent > xControlModel( pFormObject->GetUnoControlModel(),UNO_QUERY );
            if ( xControlModel.is() && aObjects.find(xControlModel) != aObjects.end() && bMark != pFormView->IsObjMarked( pSdrObject ) )
            {
                
                pFormView->MarkObj( pSdrObject, pPageView, !bMark, sal_False );
            }
        } 
        if ( bMark )
        {
            
            ::Rectangle aMarkRect( pFormView->GetAllMarkedRect());
            for ( sal_uInt32 i = 0; i < pFormView->PaintWindowCount(); ++i )
            {
                SdrPaintWindow* pPaintWindow = pFormView->GetPaintWindow( i );
                OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
                if ( ( OUTDEV_WINDOW == rOutDev.GetOutDevType() ) && !aMarkRect.IsEmpty() )
                {
                    pFormView->MakeVisible( aMarkRect, (Window&)rOutDev );
                }
            } 
        }
    }
    
    void NavigatorTree::CollectObjects(FmFormData* pFormData, sal_Bool bDeep, ::std::set< Reference< XFormComponent > >& _rObjects)
    {
        FmEntryDataList* pChildList = pFormData->GetChildList();
        FmEntryData* pEntryData;
        FmControlData* pControlData;
        for( size_t i = 0; i < pChildList->size(); ++i )
        {
            pEntryData = pChildList->at( i );
            if( pEntryData->ISA(FmControlData) )
            {
                pControlData = (FmControlData*)pEntryData;
                _rObjects.insert(pControlData->GetFormComponent());
            } 
            else if (bDeep && (pEntryData->ISA(FmFormData)))
                CollectObjects((FmFormData*)pEntryData,bDeep,_rObjects);
        } 
    }
    
    void NavigatorTree::MarkViewObj( FmControlData* pControlData, sal_Bool bMarkHandles, sal_Bool bMark)
    {
        if( !pControlData )
            return;
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( !pFormShell )
            return;

        
        
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

            
            if ( bMark != pFormView->IsObjMarked( pSdrObject ) )
                
                pFormView->MarkObj( pSdrObject, pPageView, !bMark, sal_False );

            if ( !bMarkHandles || !bMark )
                continue;

            bPaint = true;

        } 
        if ( bPaint )
        {
            
            ::Rectangle aMarkRect( pFormView->GetAllMarkedRect());
            for ( sal_uInt32 i = 0; i < pFormView->PaintWindowCount(); ++i )
            {
                SdrPaintWindow* pPaintWindow = pFormView->GetPaintWindow( i );
                OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
                if ( OUTDEV_WINDOW == rOutDev.GetOutDevType() )
                {
                    pFormView->MakeVisible( aMarkRect, (Window&)rOutDev );
                }
            } 
        }
    }


}   



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
