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
#include <rtl/logfile.hxx>
//............................................................................
namespace svxform
{
//............................................................................

    #define DROP_ACTION_TIMER_INITIAL_TICKS     10
        // solange dauert es, bis das Scrollen anspringt
    #define DROP_ACTION_TIMER_SCROLL_TICKS      3
        // in diesen Intervallen wird jeweils eine Zeile gescrollt
    #define DROP_ACTION_TIMER_TICK_BASE         10
        // das ist die Basis, mit der beide Angaben multipliziert werden (in ms)

    #define EXPLORER_SYNC_DELAY                 200
        // dieser Betrag an Millisekunden wird gewartet, ehe der Explorer nach einem Select oder Deselect die ::com::sun::star::sdbcx::View synchronisiert

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

    //========================================================================
    // helper
    //========================================================================

    typedef ::std::map< Reference< XInterface >, SdrObject*, ::comphelper::OInterfaceCompare< XInterface > >
            MapModelToShape;
    typedef MapModelToShape::value_type ModelShapePair;

    //------------------------------------------------------------------------
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

#ifdef DBG_UTIL
            ::std::pair< MapModelToShape::iterator, bool > aPos =
#endif
            _rMapping.insert( ModelShapePair( xNormalizedModel, pSdrObject ) );
            DBG_ASSERT( aPos.second, "collectShapeModelMapping: model was already existent!" );
                // if this asserts, this would mean we have 2 shapes pointing to the same model
        }
    }

    //========================================================================
    // class NavigatorTree
    //========================================================================

    //------------------------------------------------------------------------
    NavigatorTree::NavigatorTree( const Reference< XMultiServiceFactory >& _xORB,
                           Window* pParent )
        :SvTreeListBox( pParent, WB_HASBUTTONS|WB_HASLINES|WB_BORDER|WB_HSCROLL ) // #100258# OJ WB_HSCROLL added
        ,m_aControlExchange(this)
        ,m_xORB(_xORB)
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
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::NavigatorTree" );
        SetHelpId( HID_FORM_NAVIGATOR );

        m_aNavigatorImages = ImageList( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );

        SetNodeBitmaps(
            m_aNavigatorImages.GetImage( RID_SVXIMG_COLLAPSEDNODE ),
            m_aNavigatorImages.GetImage( RID_SVXIMG_EXPANDEDNODE )
        );

        SetDragDropMode(0xFFFF);
        EnableInplaceEditing( sal_True );
        SetSelectionMode(MULTIPLE_SELECTION);

        m_pNavModel = new NavigatorTreeModel( m_aNavigatorImages );
        Clear();

        StartListening( *m_pNavModel );

        m_aDropActionTimer.SetTimeoutHdl(LINK(this, NavigatorTree, OnDropActionTimer));

        m_aSynchronizeTimer.SetTimeoutHdl(LINK(this, NavigatorTree, OnSynchronizeTimer));
        SetSelectHdl(LINK(this, NavigatorTree, OnEntrySelDesel));
        SetDeselectHdl(LINK(this, NavigatorTree, OnEntrySelDesel));
    }

    //------------------------------------------------------------------------
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

    //------------------------------------------------------------------------
    void NavigatorTree::Clear()
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::Clear" );
        m_pNavModel->Clear();
    }

    //------------------------------------------------------------------------
    void NavigatorTree::UpdateContent( FmFormShell* pFormShell )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::UpdateContent" );
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
            // neue Shell, waehrend ich gerade editiere ?
            if (IsEditingActive())
                CancelTextEditing();

            m_bDragDataDirty = sal_True;    // sicherheitshalber, auch wenn ich gar nicht dragge
        }
        GetNavModel()->UpdateContent( pFormShell );

        // wenn es eine Form gibt, die Root expandieren
        if (m_pRootEntry && !IsExpanded(m_pRootEntry))
            Expand(m_pRootEntry);
        // wenn es GENAU eine Form gibt, auch diese expandieren
        if (m_pRootEntry)
        {
            SvTreeListEntry* pFirst = FirstChild(m_pRootEntry);
            if (pFirst && !NextSibling(pFirst))
                Expand(pFirst);
        }
    }

    //------------------------------------------------------------------------------
    sal_Bool NavigatorTree::implAllowExchange( sal_Int8 _nAction, sal_Bool* _pHasNonHidden )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::implAllowExchange" );
        SvTreeListEntry* pCurEntry = GetCurEntry();
        if (!pCurEntry)
            return sal_False;

        // die Informationen fuer das AcceptDrop und ExecuteDrop
        CollectSelectionData(SDI_ALL);
        if (m_arrCurrentSelection.empty())
            // nothing to do
            return sal_False;

        // testen, ob es sich vielleicht ausschliesslich um hidden controls handelt (dann koennte ich pCtrlExch noch ein
        // zusaetzliches Format geben)
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
            // non-hidden controls need to be moved
            return sal_False;

        if ( _pHasNonHidden )
            *_pHasNonHidden = bHasNonHidden;

        return sal_True;
    }

    //------------------------------------------------------------------------------
    sal_Bool NavigatorTree::implPrepareExchange( sal_Int8 _nAction )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::implPrepareExchange" );

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
            // eine entsprechende Sequenz aufbauen
            Sequence< Reference< XInterface > > seqIFaces(m_arrCurrentSelection.size());
            Reference< XInterface >* pArray = seqIFaces.getArray();
            for ( SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
                  it != m_arrCurrentSelection.end(); ++it, ++pArray )
                *pArray = static_cast< FmEntryData* >( (*it)->GetUserData() )->GetElement();

            // und das neue Format
            m_aControlExchange->addHiddenControlsFormat(seqIFaces);
        }

        m_bDragDataDirty = sal_False;
        return sal_True;
    }

    //------------------------------------------------------------------------------
    void NavigatorTree::StartDrag( sal_Int8 /*nAction*/, const ::Point& /*rPosPixel*/ )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::StartDrag" );
        EndSelection();

        if ( !implPrepareExchange( DND_ACTION_COPYMOVE ) )
            // nothing to do or something went wrong
            return;

        // jetzt haben wir alle in der aktuelle Situation moeglichen Formate eingesammelt, es kann also losgehen ...
        m_aControlExchange.startDrag( DND_ACTION_COPYMOVE );
    }

    //------------------------------------------------------------------------------
    void NavigatorTree::Command( const CommandEvent& rEvt )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::Command" );
        sal_Bool bHandled = sal_False;
        switch( rEvt.GetCommand() )
        {
            case COMMAND_CONTEXTMENU:
            {
                // die Stelle, an der geklickt wurde
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
                    if (m_arrCurrentSelection.empty()) // kann nur bei Kontextmenue ueber Tastatur passieren
                        break;

                    SvTreeListEntry* pCurrent = GetCurEntry();
                    if (!pCurrent)
                        break;
                    ptWhere = GetEntryPosition(pCurrent);
                }

                // meine Selektionsdaten auf den aktuellen Stand
                CollectSelectionData(SDI_ALL);

                // wenn mindestens ein Nicht-Root-Eintrag selektiert ist und die Root auch, dann nehme ich letztere aus der Selektion
                // fix wieder raus
                if ( (m_arrCurrentSelection.size() > 1) && m_bRootSelected )
                {
                    Select( m_pRootEntry, sal_False );
                    SetCursor( *m_arrCurrentSelection.begin(), sal_True);
                }
                sal_Bool bSingleSelection = (m_arrCurrentSelection.size() == 1);


                DBG_ASSERT( (m_arrCurrentSelection.size() > 0) || m_bRootSelected, "keine Eintraege selektiert" );
                    // solte nicht passieren, da ich oben bei der IsSelected-Abfrage auf jeden Fall einen selektiert haette,
                    // wenn das vorher nicht der Fall gewesen waere


                // das Menue zusammenbasteln
                FmFormShell* pFormShell = GetNavModel()->GetFormShell();
                FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;
                if( pFormShell && pFormModel )
                {
                    PopupMenu aContextMenu(SVX_RES(RID_FMEXPLORER_POPUPMENU));
                    PopupMenu* pSubMenuNew = aContextMenu.GetPopupMenu( SID_FM_NEW );

                    // das 'Neu'-Untermenue gibt es nur, wenn genau die Root oder genau ein Formular selektiert sind
                    aContextMenu.EnableItem( SID_FM_NEW, bSingleSelection && (m_nFormsSelected || m_bRootSelected) );

                    // 'Neu'\'Formular' unter genau den selben Bedingungen
                    pSubMenuNew->EnableItem( SID_FM_NEW_FORM, bSingleSelection && (m_nFormsSelected || m_bRootSelected) );
                    pSubMenuNew->SetItemImage(SID_FM_NEW_FORM, m_aNavigatorImages.GetImage(RID_SVXIMG_FORM));
                    pSubMenuNew->SetItemImage(SID_FM_NEW_HIDDEN, m_aNavigatorImages.GetImage(RID_SVXIMG_HIDDEN));

                    // 'Neu'\'verstecktes...', wenn genau ein Formular selektiert ist
                    pSubMenuNew->EnableItem( SID_FM_NEW_HIDDEN, bSingleSelection && m_nFormsSelected );

                    // 'Delete': everything which is not root can be removed
                    aContextMenu.EnableItem( SID_FM_DELETE, !m_bRootSelected );

                    // 'Cut', 'Copy' and 'Paste'
                    aContextMenu.EnableItem( SID_CUT, !m_bRootSelected && implAllowExchange( DND_ACTION_MOVE ) );
                    aContextMenu.EnableItem( SID_COPY, !m_bRootSelected && implAllowExchange( DND_ACTION_COPY ) );
                    aContextMenu.EnableItem( SID_PASTE, implAcceptPaste( ) );

                    // der TabDialog, wenn es genau ein Formular ist ...
                    aContextMenu.EnableItem( SID_FM_TAB_DIALOG, bSingleSelection && m_nFormsSelected );

                    // in XML forms, we don't allow for the properties of a form
                    // #i36484#
                    if ( pFormShell->GetImpl()->isEnhancedForm() && !m_nControlsSelected )
                        aContextMenu.RemoveItem( aContextMenu.GetItemPos( SID_FM_SHOW_PROPERTY_BROWSER ) );

                    // if the property browser is already open, we don't allow for the properties, too
                    if( pFormShell->GetImpl()->IsPropBrwOpen() )
                        aContextMenu.RemoveItem( aContextMenu.GetItemPos( SID_FM_SHOW_PROPERTY_BROWSER ) );
                    // and finally, if there's a mixed selection of forms and controls, disable the entry, too
                    else
                        aContextMenu.EnableItem( SID_FM_SHOW_PROPERTY_BROWSER,
                            (m_nControlsSelected && !m_nFormsSelected) || (!m_nControlsSelected && m_nFormsSelected) );

                    // Umbenennen gdw wenn ein Element und nicht die Root
                    aContextMenu.EnableItem( SID_FM_RENAME_OBJECT, bSingleSelection && !m_bRootSelected );

                    // der Reandonly-Eintrag ist nur auf der Root erlaubt
                    aContextMenu.EnableItem( SID_FM_OPEN_READONLY, m_bRootSelected );
                    // the same for automatic control focus
                    aContextMenu.EnableItem( SID_FM_AUTOCONTROLFOCUS, m_bRootSelected );

                    // die ConvertTo-Slots sind enabled, wenn genau ein Control selektiert ist, der
                    // dem Control entsprechende Slot ist disabled
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
                        aContextMenu.EnableItem( SID_FM_CHANGECONTROLTYPE, sal_False );

                    // jetzt alles, was disabled wurde, wech
                    aContextMenu.RemoveDisabledEntries(sal_True, sal_True);
                    //////////////////////////////////////////////////////////
                    // OpenReadOnly setzen

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
                            // der Slot war nur verfuegbar, wenn es genau einen selektierten Eintrag gibt und dieser die Root
                            // oder ein Formular ist
                            NewForm( *m_arrCurrentSelection.begin() );
                            pFormModel->EndUndo();

                        }   break;
                        case SID_FM_NEW_HIDDEN:
                        {
                            OUString aStr(SVX_RESSTR(RID_STR_CONTROL));
                            OUString aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_INSERT).replaceAll(OUString('#'), aStr);

                            pFormModel->BegUndo(aUndoStr);
                            // dieser Slot war guletig bei (genau) einem selektierten Formular
                            rtl::OUString fControlName = FM_COMPONENT_HIDDEN;
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
                            // dieser Slot galt bei genau einem selektierten Formular
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
                            // das war bei genau einem Nicht-Root-Eintrag erlaubt
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

    //------------------------------------------------------------------------
    SvTreeListEntry* NavigatorTree::FindEntry( FmEntryData* pEntryData )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::FindEntry" );
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

    //------------------------------------------------------------------------
    void NavigatorTree::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::Notify" );
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
            {   // das Image neu setzen
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

            //////////////////////////////////////////////////////////////////////
            // Default-Eintrag "Formulare"
            Image aRootImage( m_aNavigatorImages.GetImage( RID_SVXIMG_FORMS ) );
            m_pRootEntry = InsertEntry( SVX_RES(RID_STR_FORMS), aRootImage, aRootImage,
                NULL, sal_False, 0, NULL );
        }
        else if (!m_bMarkingObjects && rHint.ISA(FmNavRequestSelectHint))
        {   // wenn m_bMarkingObjects sal_True ist, markiere ich gerade selber Objekte, und da der ganze Mechanismus dahinter synchron ist,
            // ist das genau der Hint, der durch mein Markieren ausgeloest wird, also kann ich ihn ignorieren
            FmNavRequestSelectHint* pershHint = (FmNavRequestSelectHint*)&rHint;
            FmEntryDataArray& arredToSelect = pershHint->GetItems();
            SynchronizeSelection(arredToSelect);

            if (pershHint->IsMixedSelection())
                // in diesem Fall habe ich alles deselektiert, obwohl die View u.U. eine gemischte Markierung hatte
                // ich muss also im naechsten Select den Navigator an die View anpassen
                m_bPrevSelectionMixed = sal_True;
        }
    }

    //------------------------------------------------------------------------
    SvTreeListEntry* NavigatorTree::Insert( FmEntryData* pEntryData, sal_uIntPtr nRelPos )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::Insert" );
        //////////////////////////////////////////////////////////////////////
        // Aktuellen Eintrag einfuegen
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

        //////////////////////////////////////////////////////////////////////
        // Wenn Root-Eintrag Root expandieren
        if( !pParentEntry )
            Expand( m_pRootEntry );

        //////////////////////////////////////////////////////////////////////
        // Children einfuegen
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

    //------------------------------------------------------------------------
    void NavigatorTree::Remove( FmEntryData* pEntryData )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::Remove" );
        if( !pEntryData )
            return;

        // der Entry zu den Daten
        SvTreeListEntry* pEntry = FindEntry( pEntryData );
        if (!pEntry)
            return;

        // Eintrag aus TreeListBox entfernen
        // ich darf das Select, das ich ausloese, nicht behandeln :
        // Select aendert die MarkList der View, wenn das gerade auch jemand anders macht und dabei ein Remove
        // triggert, haben wir mit ziemlicher Sicherheit ein Problem - Paradebeispiel war das Gruppieren von Controls mit
        // offenem Navigator ...)
        LockSelectionHandling();

        // ein kleines Problem : ich merke mir meine selektierten Daten, wenn mir jetzt jemand einen selektierten Eintrag
        // unter dem Hintern wegschiesst, werde ich inkonsistent ... was schlecht waere
        Select(pEntry, sal_False);

        // beim eigentlichen Entfernen kann die Selection geaendert werden, da ich aber das SelectionHandling abgeschaltet
        // habe, muss ich mich hinterher darum kuemmern
        sal_uIntPtr nExpectedSelectionCount = GetSelectionCount();

        if( pEntry )
            GetModel()->Remove( pEntry );

        if (nExpectedSelectionCount != GetSelectionCount())
            SynchronizeSelection();

        // und standardmaessig behandle ich das Select natuerlich
        UnlockSelectionHandling();
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::IsFormEntry( SvTreeListEntry* pEntry )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::IsFormEntry" );
        FmEntryData* pEntryData = (FmEntryData*)pEntry->GetUserData();
        return !pEntryData || pEntryData->ISA(FmFormData);
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::IsFormComponentEntry( SvTreeListEntry* pEntry )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::IsFormComponentEntry" );
        FmEntryData* pEntryData = (FmEntryData*)pEntry->GetUserData();
        return pEntryData && pEntryData->ISA(FmControlData);
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::implAcceptPaste( )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::implAcceptPaste" );
        SvTreeListEntry* pFirstSelected = FirstSelected();
        if ( !pFirstSelected || NextSelected( pFirstSelected ) )
            // no selected entry, or at least two selected entries
            return sal_False;

        // get the clipboard
        TransferableDataHelper aClipboardContent( TransferableDataHelper::CreateFromSystemClipboard( this ) );

        sal_Int8 nAction = m_aControlExchange.isClipboardOwner() && doingKeyboardCut( ) ? DND_ACTION_MOVE : DND_ACTION_COPY;
        return ( nAction == implAcceptDataTransfer( aClipboardContent.GetDataFlavorExVector(), nAction, pFirstSelected, sal_False ) );
    }

    //------------------------------------------------------------------------
    sal_Int8 NavigatorTree::implAcceptDataTransfer( const DataFlavorExVector& _rFlavors, sal_Int8 _nAction, const ::Point& _rDropPos, sal_Bool _bDnD )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::implAcceptDataTransfer" );
        return implAcceptDataTransfer( _rFlavors, _nAction, GetEntry( _rDropPos ), _bDnD );
    }

    //------------------------------------------------------------------------
    sal_Int8 NavigatorTree::implAcceptDataTransfer( const DataFlavorExVector& _rFlavors, sal_Int8 _nAction, SvTreeListEntry* _pTargetEntry, sal_Bool _bDnD )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::implAcceptDataTransfer" );
        // no target -> no drop
        if (!_pTargetEntry)
            return DND_ACTION_NONE;

        // format check
        sal_Bool bHasDefControlFormat = OControlExchange::hasFieldExchangeFormat( _rFlavors );
        sal_Bool bHasControlPathFormat = OControlExchange::hasControlPathFormat( _rFlavors );
        sal_Bool bHasHiddenControlsFormat = OControlExchange::hasHiddenControlModelsFormat( _rFlavors );
        if (!bHasDefControlFormat && !bHasControlPathFormat && !bHasHiddenControlsFormat)
            return DND_ACTION_NONE;

        sal_Bool bSelfSource = _bDnD ? m_aControlExchange.isDragSource() : m_aControlExchange.isClipboardOwner();

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

        sal_Bool bForeignCollection = m_aControlExchange->getFormsRoot().get() != GetNavModel()->GetFormPage()->GetForms().get();
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
                // ich befinde mich zwar in der Shell/Page, aus der die Controls stammen, habe aber kein Format, das den stattgefundenen
                // Shell-Wechsel ueberlebt hat (SVX_FM_CONTROLS_AS_PATH)
                return DND_ACTION_NONE;

            // da die Shell waehrend des Draggens umgeschaltet wude, muss ich die Liste des ExchangeObjektes wieder neu aufbauen
            // (dort stehen SvLBoxEntries drin, und die sind bei der Umschaltung floeten gegangen)
            m_aControlExchange->buildListFromPath(this, m_pRootEntry);
            m_bDragDataDirty = sal_False;
        }

        // die Liste der gedroppten Eintraege aus dem DragServer
        const ListBoxEntrySet& aDropped = m_aControlExchange->selected();
        DBG_ASSERT(aDropped.size() >= 1, "NavigatorTree::implAcceptDataTransfer: keine Eintraege !");

        sal_Bool bDropTargetIsComponent = IsFormComponentEntry( _pTargetEntry );
        //SvTreeListEntry* pDropTargetParent = GetParent( _pTargetEntry );

        // conditions to disallow the drop
        // 0) the root entry is part of the list (can't DnD the root!)
        // 1) one of the draged entries is to be dropped onto it's own parent
        // 2) -               "       - is to be dropped onto itself
        // 3) -               "       - is a Form and to be dropped onto one of it's descendants
        // 4) one of the entries is a control and to be dropped onto the root
        // 5) a control or form will be dropped onto a control which is _not_ a sibling (dropping onto a sibling
        //      means moving the control)

        // collect the ancestors of the drop targte (speeds up 3)
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
                if ( bDropTargetIsComponent )   // TODO : die obige Zeile wieder rein, dann muss aber ExecuteDrop das Vertauschen auch beherrschen
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

    //------------------------------------------------------------------------
    sal_Int8 NavigatorTree::AcceptDrop( const AcceptDropEvent& rEvt )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::AcceptDrop" );
        ::Point aDropPos = rEvt.maPosPixel;

        // kuemmern wir uns erst mal um moeglich DropActions (Scrollen und Aufklappen)
        if (rEvt.mbLeaving)
        {
            if (m_aDropActionTimer.IsActive())
                m_aDropActionTimer.Stop();
        } else
        {
            sal_Bool bNeedTrigger = sal_False;
            // auf dem ersten Eintrag ?
            if ((aDropPos.Y() >= 0) && (aDropPos.Y() < GetEntryHeight()))
            {
                m_aDropActionType = DA_SCROLLUP;
                bNeedTrigger = sal_True;
            } else
                // auf dem letzten (bzw. in dem Bereich, den ein Eintrag einnehmen wuerde, wenn er unten genau buendig
                // abschliessen wuerde) ?
                if ((aDropPos.Y() < GetSizePixel().Height()) && (aDropPos.Y() >= GetSizePixel().Height() - GetEntryHeight()))
                {
                    m_aDropActionType = DA_SCROLLDOWN;
                    bNeedTrigger = sal_True;
                } else
                {   // auf einem Entry mit Children, der nicht aufgeklappt ist ?
                    SvTreeListEntry* pDropppedOn = GetEntry(aDropPos);
                    if (pDropppedOn && (GetChildCount(pDropppedOn) > 0) && !IsExpanded(pDropppedOn))
                    {
                        // -> aufklappen
                        m_aDropActionType = DA_EXPANDNODE;
                        bNeedTrigger = sal_True;
                    }
                }

            if (bNeedTrigger && (m_aTimerTriggered != aDropPos))
            {
                // neu anfangen zu zaehlen
                m_aTimerCounter = DROP_ACTION_TIMER_INITIAL_TICKS;
                // die Pos merken, da ich auch AcceptDrops bekomme, wenn sich die Maus gar nicht bewegt hat
                m_aTimerTriggered = aDropPos;
                // und den Timer los
                if (!m_aDropActionTimer.IsActive()) // gibt es den Timer schon ?
                {
                    m_aDropActionTimer.SetTimeout(DROP_ACTION_TIMER_TICK_BASE);
                    m_aDropActionTimer.Start();
                }
            } else if (!bNeedTrigger)
                m_aDropActionTimer.Stop();
        }

        return implAcceptDataTransfer( GetDataFlavorExVector(), rEvt.mnAction, aDropPos, sal_True );
    }

    //------------------------------------------------------------------------
    sal_Int8 NavigatorTree::implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, const ::Point& _rDropPos, sal_Bool _bDnD )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::implExecuteDataTransfer" );
        return implExecuteDataTransfer( _rData, _nAction, GetEntry( _rDropPos ), _bDnD );
    }

    //------------------------------------------------------------------------
    sal_Int8 NavigatorTree::implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, SvTreeListEntry* _pTargetEntry, sal_Bool _bDnD )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::implExecuteDataTransfer" );
        const DataFlavorExVector& rDataFlavors = _rData.GetDataFlavorExVector();

        if ( DND_ACTION_NONE == implAcceptDataTransfer( rDataFlavors, _nAction, _pTargetEntry, _bDnD ) )
            // under some platforms, it may happen that ExecuteDrop is called though AcceptDrop returned DND_ACTION_NONE
            return DND_ACTION_NONE;

        // ware schlecht, wenn nach dem Droppen noch gescrollt wird ...
        if (m_aDropActionTimer.IsActive())
            m_aDropActionTimer.Stop();

        if (!_pTargetEntry)
            // no target -> no drop
            return DND_ACTION_NONE;

        // format checks
#ifdef DBG_UTIL
        sal_Bool bHasHiddenControlsFormat = OControlExchange::hasHiddenControlModelsFormat( rDataFlavors );
        sal_Bool bForeignCollection = _rData.getFormsRoot().get() != GetNavModel()->GetFormPage()->GetForms().get();
        DBG_ASSERT(!bForeignCollection || bHasHiddenControlsFormat, "NavigatorTree::implExecuteDataTransfer: invalid format (AcceptDrop shouldn't have let this pass) !");
        DBG_ASSERT(bForeignCollection || !m_bDragDataDirty, "NavigatorTree::implExecuteDataTransfer: invalid state (shell changed since last exchange resync) !");
            // das sollte in AcceptDrop erledigt worden sein : dort wird in _rData die Liste der Controls aufgebaut und m_bDragDataDirty
            // zurueckgesetzt
#endif

        if ( DND_ACTION_COPY == _nAction )
        {   // bHasHiddenControlsFormat means that only hidden controls are part of the data
            DBG_ASSERT( bHasHiddenControlsFormat, "NavigatorTree::implExecuteDataTransfer: copy allowed for hidden controls only!" );
            DBG_ASSERT( _pTargetEntry && ( _pTargetEntry != m_pRootEntry ) && IsFormEntry( _pTargetEntry ),
                "NavigatorTree::implExecuteDataTransfer: should not be here!" );
                // implAcceptDataTransfer should have caught both cases

            DBG_ASSERT(bHasHiddenControlsFormat, "NavigatorTree::implExecuteDataTransfer: only copying of hidden controls is supported !");
                // das sollte das AcceptDrop abgefangen haben

            // da ich gleich die Zielobjekte alle selektieren will (und nur die)
            SelectAll(sal_False);

            Sequence< Reference< XInterface > > aControls = _rData.hiddenControls();
            sal_Int32 nCount = aControls.getLength();
            const Reference< XInterface >* pControls = aControls.getConstArray();

            FmFormShell* pFormShell = GetNavModel()->GetFormShell();
            FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;

            // innerhalb eines Undo ...
            if (pFormModel)
            {
                OUString aStr(SVX_RESSTR(RID_STR_CONTROL));
                OUString aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_INSERT).replaceAll(OUString('#'), aStr);
                pFormModel->BegUndo(aUndoStr);
            }

            // die Conrtols kopieren
            for (sal_Int32 i=0; i<nCount; ++i)
            {
                // neues Control anlegen
                rtl::OUString fControlName = FM_COMPONENT_HIDDEN;
                FmControlData* pNewControlData = NewControl( fControlName, _pTargetEntry, sal_False);
                Reference< XPropertySet >  xNewPropSet( pNewControlData->GetPropertySet() );

                // und die Properties des alten in das neue kopieren
                Reference< XPropertySet >  xCurrent(pControls[i], UNO_QUERY);
#if (OSL_DEBUG_LEVEL > 1)
                // nur mal eben sehen, ob das Ding tatsaechlich ein hidden control ist
                sal_Int16 nClassId = ::comphelper::getINT16(xCurrent->getPropertyValue(FM_PROP_CLASSID));
                OSL_ENSURE(nClassId == FormComponentType::HIDDENCONTROL, "NavigatorTree::implExecuteDataTransfer: invalid control in drop list !");
                    // wenn das SVX_FM_HIDDEN_CONTROLS-Format vorhanden ist, dann sollten wirklich nur hidden controls in der Sequenz
                    // stecken
#endif // (OSL_DEBUG_LEVEL > 1)
                Reference< XPropertySetInfo >  xPropInfo( xCurrent->getPropertySetInfo());
                Sequence< Property> seqAllCurrentProps = xPropInfo->getProperties();
                Property* pAllCurrentProps = seqAllCurrentProps.getArray();
                for (sal_Int32 j=0; j<seqAllCurrentProps.getLength(); ++j)
                {
                    ::rtl::OUString sCurrentProp = pAllCurrentProps[j].Name;
                    if (((pAllCurrentProps[j].Attributes & PropertyAttribute::READONLY) == 0) && (sCurrentProp != FM_PROP_NAME))
                    {   // (read-only attribs werden natuerlich nicht gesetzt, dito der Name, den hat das NewControl schon eindeutig
                        // festgelegt)
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
            // can't do anything without the internal format here ... usually happens when doing DnD or CnP
            // over navigator boundaries
            return DND_ACTION_NONE;
        }

        // some data for the target
        sal_Bool bDropTargetIsForm = IsFormEntry(_pTargetEntry);
        FmFormData* pTargetData = bDropTargetIsForm ? (FmFormData*)_pTargetEntry->GetUserData() : NULL;

        DBG_ASSERT( DND_ACTION_COPY != _nAction, "NavigatorTree::implExecuteDataTransfer: somebody changed the logics!" );

        // die Liste der gedraggten Eintraege
        ListBoxEntrySet aDropped = _rData.selected();
        DBG_ASSERT(aDropped.size() >= 1, "NavigatorTree::implExecuteDataTransfer: no entries!");

        // die Shell und das Model
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;
        if (!pFormModel)
            return DND_ACTION_NONE;

        // fuer's Undo
        const bool bUndo = pFormModel->IsUndoEnabled();

        if( bUndo )
        {
            OUString strUndoDescription(SVX_RESSTR(RID_STR_UNDO_CONTAINER_REPLACE));
            pFormModel->BegUndo(strUndoDescription);
        }

        // ich nehme vor dem Einfuegen eines Eintrages seine Selection raus, damit die Markierung dabei nicht flackert
        // -> das Handeln des Select locken
        LockSelectionHandling();

        // jetzt durch alle gedroppten Eintraege ...
        for (   ListBoxEntrySet::const_iterator dropped = aDropped.begin();
                dropped != aDropped.end();
                ++dropped
            )
        {
            // ein paar Daten zum aktuellen Element
            SvTreeListEntry* pCurrent = *dropped;
            DBG_ASSERT(pCurrent != NULL, "NavigatorTree::implExecuteDataTransfer: ungueltiger Eintrag");
            DBG_ASSERT(GetParent(pCurrent) != NULL, "NavigatorTree::implExecuteDataTransfer: ungueltiger Eintrag");
                // die Root darf nicht gedraggt werden

            FmEntryData* pCurrentUserData = (FmEntryData*)pCurrent->GetUserData();

            Reference< XChild >  xCurrentChild(pCurrentUserData->GetChildIFace(), UNO_QUERY);
            Reference< XIndexContainer >  xContainer(xCurrentChild->getParent(), UNO_QUERY);

            FmFormData* pCurrentParentUserData = (FmFormData*)pCurrentUserData->GetParent();
            DBG_ASSERT(pCurrentParentUserData == NULL || pCurrentParentUserData->ISA(FmFormData), "NavigatorTree::implExecuteDataTransfer: ungueltiges Parent");

            // beim Vater austragen
            if (pCurrentParentUserData)
                pCurrentParentUserData->GetChildList()->remove( pCurrentUserData );
            else
                GetNavModel()->GetRootList()->remove( pCurrentUserData );

            // aus dem Container entfernen
            sal_Int32 nIndex = getElementPos(Reference< XIndexAccess > (xContainer, UNO_QUERY), xCurrentChild);
            GetNavModel()->m_pPropChangeList->Lock();
            // die Undo-Action fuer das Rausnehmen
            if ( bUndo && GetNavModel()->m_pPropChangeList->CanUndo())
            {
                pFormModel->AddUndo(new FmUndoContainerAction(*pFormModel, FmUndoContainerAction::Removed,
                                                            xContainer, xCurrentChild, nIndex));
            }
            else if( !GetNavModel()->m_pPropChangeList->CanUndo() )
            {
                FmUndoContainerAction::DisposeElement( xCurrentChild );
            }

            // Events mitkopieren
            Reference< XEventAttacherManager >  xManager(xContainer, UNO_QUERY);
            Sequence< ScriptEventDescriptor > aEvts;

            if (xManager.is() && nIndex >= 0)
                aEvts = xManager->getScriptEvents(nIndex);
            xContainer->removeByIndex(nIndex);

            // die Selection raus
            Select(pCurrent, sal_False);
            // und weg
            Remove(pCurrentUserData);

            // die Stelle innerhalb des DropParents, an der ich die gedroppten Eintraege einfuegen muss
            if (pTargetData)
                xContainer = Reference< XIndexContainer > (pTargetData->GetElement(), UNO_QUERY);
            else
                xContainer = Reference< XIndexContainer > (GetNavModel()->GetForms(), UNO_QUERY);

            // immer ganz hinten einfuegen
            nIndex = xContainer->getCount();

            // UndoAction fuer das Einfuegen
            if ( bUndo && GetNavModel()->m_pPropChangeList->CanUndo())
                pFormModel->AddUndo(new FmUndoContainerAction(*pFormModel, FmUndoContainerAction::Inserted,
                                                         xContainer, xCurrentChild, nIndex));

            // einfuegen im neuen Container
            if (pTargetData)
            {
                 // es wird in eine Form eingefuegt, dann brauche ich eine FormComponent
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

            // zuerst dem Eintrag das neue Parent
            pCurrentUserData->SetParent(pTargetData);

            // dann dem Parent das neue Child
            if (pTargetData)
                pTargetData->GetChildList()->insert( pCurrentUserData, nIndex );
            else
                GetNavModel()->GetRootList()->insert( pCurrentUserData, nIndex );

            // dann bei mir selber bekanntgeben und neu selektieren
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

    //------------------------------------------------------------------------
    sal_Int8 NavigatorTree::ExecuteDrop( const ExecuteDropEvent& rEvt )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::ExecuteDrop" );
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

    //------------------------------------------------------------------------
    void NavigatorTree::doPaste()
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::doPaste" );
           try
        {
            if ( m_aControlExchange.isClipboardOwner() )
            {
                implExecuteDataTransfer( *m_aControlExchange, doingKeyboardCut( ) ? DND_ACTION_MOVE : DND_ACTION_COPY, FirstSelected(), sal_False );
            }
            else
            {
                // the clipboard content
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

    //------------------------------------------------------------------------
    void NavigatorTree::doCopy()
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::doCopy" );
        if ( implPrepareExchange( DND_ACTION_COPY ) )
        {
            m_aControlExchange.setClipboardListener( LINK( this, NavigatorTree, OnClipboardAction ) );
            m_aControlExchange.copyToClipboard( );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTree::ModelHasRemoved( SvTreeListEntry* _pEntry )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::ModelHasRemoved" );
        SvTreeListEntry* pTypedEntry = static_cast< SvTreeListEntry* >( _pEntry );
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

    //------------------------------------------------------------------------
    void NavigatorTree::doCut()
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::doCut" );
        if ( implPrepareExchange( DND_ACTION_MOVE ) )
        {
            m_aControlExchange.setClipboardListener( LINK( this, NavigatorTree, OnClipboardAction ) );
            m_aControlExchange.copyToClipboard( );
            m_bKeyboardCut = sal_True;

            // mark all the entries we just "cut" into the clipboard as "nearly moved"
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

    //------------------------------------------------------------------------
    void NavigatorTree::KeyInput(const ::KeyEvent& rKEvt)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::KeyInput" );
        const KeyCode& rCode = rKEvt.GetKeyCode();

        // delete?
        if (rKEvt.GetKeyCode().GetCode() == KEY_DELETE && !rKEvt.GetKeyCode().GetModifier())
        {
            DeleteSelection();
            return;
        }

        // copy'n'paste?
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

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::EditingEntry( SvTreeListEntry* pEntry, Selection& rSelection )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::EditingEntry" );
        if (!SvTreeListBox::EditingEntry( pEntry, rSelection ))
            return sal_False;

        return (pEntry && (pEntry->GetUserData() != NULL));
            // die Wurzel, die ich nicht umbenennen darf, hat als UserData NULL
    }

    //------------------------------------------------------------------------
    void NavigatorTree::NewForm( SvTreeListEntry* pParentEntry )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::NewForm" );
        //////////////////////////////////////////////////////////////////////
        // ParentFormData holen
        if( !IsFormEntry(pParentEntry) )
            return;

        FmFormData* pParentFormData = (FmFormData*)pParentEntry->GetUserData();

        //////////////////////////////////////////////////////////////////////
        // Neue Form erzeugen
        Reference< XForm >  xNewForm(m_xORB->createInstance(FM_SUN_COMPONENT_FORM), UNO_QUERY);
        if (!xNewForm.is())
            return;

        FmFormData* pNewFormData = new FmFormData( xNewForm, m_aNavigatorImages, pParentFormData );

        //////////////////////////////////////////////////////////////////////
        // Namen setzen
        ::rtl::OUString aName = GenerateName(pNewFormData);
        pNewFormData->SetText(aName);

        Reference< XPropertySet >  xPropertySet(xNewForm, UNO_QUERY);
        if (!xPropertySet.is())
            return;
        try
        {
            xPropertySet->setPropertyValue( FM_PROP_NAME, makeAny(aName) );
            // a form should always have the command type table as default
            xPropertySet->setPropertyValue( FM_PROP_COMMANDTYPE, makeAny(sal_Int32(CommandType::TABLE)));
        }
        catch ( const Exception& )
        {
            OSL_FAIL("NavigatorTree::NewForm : could not set esssential properties !");
        }


        //////////////////////////////////////////////////////////////////////
        // Form einfuegen
        GetNavModel()->Insert( pNewFormData, LIST_APPEND, sal_True );

        //////////////////////////////////////////////////////////////////////
        // Neue Form als aktive Form setzen
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( pFormShell )
        {
            InterfaceBag aSelection;
            aSelection.insert( Reference< XInterface >( xNewForm, UNO_QUERY ) );
            pFormShell->GetImpl()->setCurrentSelection( aSelection );

            pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_PROPERTIES,sal_True,sal_True);
        }
        GetNavModel()->SetModified();

        //////////////////////////////////////////////////////////////////////
        // In EditMode schalten
        SvTreeListEntry* pNewEntry = FindEntry( pNewFormData );
        EditEntry( pNewEntry );
    }

    //------------------------------------------------------------------------
    FmControlData* NavigatorTree::NewControl( const ::rtl::OUString& rServiceName, SvTreeListEntry* pParentEntry, sal_Bool bEditName )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::NewControl" );
        //////////////////////////////////////////////////////////////////////
        // ParentForm holen
        if (!GetNavModel()->GetFormShell())
            return NULL;
        if (!IsFormEntry(pParentEntry))
            return NULL;

        FmFormData* pParentFormData = (FmFormData*)pParentEntry->GetUserData();;
        Reference< XForm >  xParentForm( pParentFormData->GetFormIface());

        //////////////////////////////////////////////////////////////////////
        // Neue Component erzeugen
        Reference< XFormComponent >  xNewComponent(::comphelper::getProcessServiceFactory()->createInstance(rServiceName), UNO_QUERY);
        if (!xNewComponent.is())
            return NULL;

        FmControlData* pNewFormControlData = new FmControlData( xNewComponent, m_aNavigatorImages, pParentFormData );

        //////////////////////////////////////////////////////////////////////
        // Namen setzen
        FmFormView*     pFormView       = GetNavModel()->GetFormShell()->GetFormView();
        SdrPageView*    pPageView       = pFormView->GetSdrPageView();
        FmFormPage*     pPage           = (FmFormPage*)pPageView->GetPage();

        ::rtl::OUString sName = pPage->GetImpl().setUniqueName( xNewComponent, xParentForm );

        pNewFormControlData->SetText( sName );

        //////////////////////////////////////////////////////////////////////
        // FormComponent einfuegen
        GetNavModel()->Insert( pNewFormControlData, LIST_APPEND, sal_True );
        GetNavModel()->SetModified();

        if (bEditName)
        {
            //////////////////////////////////////////////////////////////////////
            // In EditMode schalten
            SvTreeListEntry* pNewEntry = FindEntry( pNewFormControlData );
            Select( pNewEntry, sal_True );
            EditEntry( pNewEntry );
        }

        return pNewFormControlData;
    }

    //------------------------------------------------------------------------
    ::rtl::OUString NavigatorTree::GenerateName( FmEntryData* pEntryData )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::GenerateName" );
        const sal_uInt16 nMaxCount = 99;
        ::rtl::OUString aNewName;

        //////////////////////////////////////////////////////////////////////
        // BasisNamen erzeugen
        OUString aBaseName;
        if( pEntryData->ISA(FmFormData) )
            aBaseName = SVX_RESSTR( RID_STR_STDFORMNAME );
        else if( pEntryData->ISA(FmControlData) )
            aBaseName = SVX_RESSTR( RID_STR_CONTROL );

        //////////////////////////////////////////////////////////////////////
        // Neuen Namen erstellen
        FmFormData* pFormParentData = (FmFormData*)pEntryData->GetParent();

        for( sal_Int32 i=0; i<nMaxCount; i++ )
        {
            aNewName = aBaseName;
            if( i>0 )
            {
                aNewName += ::rtl::OUString(" ");
                aNewName += ::rtl::OUString::valueOf(i).getStr();
            }

            if( GetNavModel()->FindData(aNewName, pFormParentData,sal_False) == NULL )
                break;
        }

        return aNewName;
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::EditedEntry( SvTreeListEntry* pEntry, const rtl::OUString& rNewText )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::EditedEntry" );
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

    //------------------------------------------------------------------------
    IMPL_LINK_NOARG(NavigatorTree, OnEdit)
    {
        nEditEvent = 0;
        EditEntry( m_pEditEntry );
        m_pEditEntry = NULL;

        return 0L;
    }

    //------------------------------------------------------------------------
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
                // tja, eigentlich muesste ich noch testen, ob die Node nicht schon expandiert ist, aber ich
                // habe dazu weder in den Basisklassen noch im Model eine Methode gefunden ...
                // aber ich denke, die BK sollte es auch so vertragen
                Expand(pToExpand);

            // nach dem Expand habe ich im Gegensatz zum Scrollen natuerlich nix mehr zu tun
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

    //------------------------------------------------------------------------
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

    //------------------------------------------------------------------------
    IMPL_LINK_NOARG(NavigatorTree, OnSynchronizeTimer)
    {
        SynchronizeMarkList();
        return 0L;
    }


    //------------------------------------------------------------------------
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

    //------------------------------------------------------------------------
    void NavigatorTree::ShowSelectionProperties(sal_Bool bForce)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::ShowSelectionProperties" );
        // zuerst brauche ich die FormShell
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell)
            // keine Shell -> ich koennte kein curObject setzen -> raus
            return;

        CollectSelectionData(SDI_ALL);
        SAL_WARN_IF(static_cast<size_t>(m_nFormsSelected + m_nControlsSelected
                + (m_bRootSelected ? 1 : 0)) != m_arrCurrentSelection.size(),
            "svx.form",
            "NavigatorTree::ShowSelectionProperties : selection meta data invalid !");


        InterfaceBag aSelection;
        sal_Bool bSetSelectionAsMarkList = sal_False;

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
                {   // es ist genau eine Form selektiert
                    FmFormData* pFormData = (FmFormData*)(*m_arrCurrentSelection.begin())->GetUserData();
                    aSelection.insert( Reference< XInterface >( pFormData->GetFormIface(), UNO_QUERY ) );
                }
                else
                {   // es ist genau ein Control selektiert (egal ob hidden oder normal)
                    FmEntryData* pEntryData = (FmEntryData*)(*m_arrCurrentSelection.begin())->GetUserData();

                    aSelection.insert( Reference< XInterface >( pEntryData->GetElement(), UNO_QUERY ) );
                }
            }
            else
            {   // wir haben eine MultiSelection, also muessen wir ein MultiSet dafuer aufbauen
                if (m_nFormsSelected > 0)
                {   // ... nur Forms
                    // erstmal die PropertySet-Interfaces der Forms einsammeln
                   SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
                    for ( sal_Int32 i = 0; i < m_nFormsSelected; ++i )
                    {
                        FmFormData* pFormData = (FmFormData*)(*it)->GetUserData();
                        aSelection.insert( pFormData->GetPropertySet().get() );
                        ++it;
                    }
                }
                else
                {   // ... nur Controls
                    if (m_nHiddenControls == m_nControlsSelected)
                    {   // ein MultiSet fuer die Properties der hidden controls
                        SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
                        for ( sal_Int32 i = 0; i < m_nHiddenControls; ++i )
                        {
                            FmEntryData* pEntryData = (FmEntryData*)(*it)->GetUserData();
                            aSelection.insert( pEntryData->GetPropertySet().get() );
                            ++it;
                        }
                    }
                    else if (m_nHiddenControls == 0)
                    {   // nur normale Controls
                        bSetSelectionAsMarkList = sal_True;
                    }
                }
            }

        }

        // und dann meine Form und mein SelObject
        if ( bSetSelectionAsMarkList )
            pFormShell->GetImpl()->setCurrentSelectionFromMark( pFormShell->GetFormView()->GetMarkedObjectList() );
        else
            pFormShell->GetImpl()->setCurrentSelection( aSelection );

        if ( pFormShell->GetImpl()->IsPropBrwOpen() || bForce )
        {
            // und jetzt kann ich das Ganze dem PropertyBrowser uebergeben
            pFormShell->GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_SHOW_PROPERTY_BROWSER, SFX_CALLMODE_ASYNCHRON );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTree::DeleteSelection()
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::DeleteSelection" );
        // die Root darf ich natuerlich nicht mitloeschen
        sal_Bool bRootSelected = IsSelected(m_pRootEntry);
        sal_uIntPtr nSelectedEntries = GetSelectionCount();
        if (bRootSelected && (nSelectedEntries > 1))     // die Root plus andere Elemente ?
            Select(m_pRootEntry, sal_False);                // ja -> die Root raus

        if ((nSelectedEntries == 0) || bRootSelected)    // immer noch die Root ?
            return;                                     // -> sie ist das einzige selektierte -> raus

        DBG_ASSERT(!m_bPrevSelectionMixed, "NavigatorTree::DeleteSelection() : loeschen nicht erlaubt wenn Markierung und Selektion nciht konsistent");

        // ich brauche unten das FormModel ...
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell)
            return;
        FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;
        if (!pFormModel)
            return;

        // jetzt muss ich noch die DeleteList etwas absichern : wenn man ein Formular und ein abhaengiges
        // Element loescht - und zwar in dieser Reihenfolge - dann ist der SvLBoxEntryPtr des abhaengigen Elementes
        // natuerlich schon ungueltig, wenn es geloescht werden soll ... diesen GPF, den es dann mit Sicherheit gibt,
        // gilt es zu verhindern, also die 'normalisierte' Liste
        CollectSelectionData( SDI_NORMALIZED );

        // see below for why we need this mapping from models to shapes
        FmFormView*     pFormView       = pFormShell->GetFormView();
        SdrPageView*    pPageView       = pFormView ? pFormView->GetSdrPageView() : NULL;
        SdrPage*        pPage           = pPageView ? pPageView->GetPage() : NULL;
        DBG_ASSERT( pPage, "NavigatorTree::DeleteSelection: invalid form page!" );

        MapModelToShape aModelShapes;
        if ( pPage )
            collectShapeModelMapping( pPage, aModelShapes );

        // problem: we have to use ExplorerModel::Remove, since only this one properly deletes Form objects.
        // But, the controls themself must be deleted via DeleteMarked (else, the Writer has some problems
        // somewhere). In case I'd first delete the structure, then the controls, the UNDO would not work
        // (since UNDO then would mean to first restore the controls, then the structure, means their parent
        // form). The other way round, the EntryDatas would be invalid, if I'd first delete the controls and
        // then go on to the strucure. This means I have to delete the forms *after* the normal controls, so
        // that during UNDO, they're restored in the proper order.
        pFormShell->GetImpl()->EnableTrackProperties(sal_False);
        for (SvLBoxEntrySortedArray::reverse_iterator it = m_arrCurrentSelection.rbegin();
             it != m_arrCurrentSelection.rend(); )
        {
            FmEntryData* pCurrent = (FmEntryData*)((*it)->GetUserData());

            // eine Form ?
            sal_Bool bIsForm = pCurrent->ISA(FmFormData);

            // da ich das Loeschen im folgenden der View ueberlasse und dabei auf deren MarkList aufbaue, im Normalfall aber bei
            // einem makierten Formular nur die direkt, nicht die indirekt abhaengigen Controls markiert werden, muss ich das hier
            // noch nachholen
            if (bIsForm)
                MarkViewObj((FmFormData*)pCurrent, sal_True, sal_True);     // das zweite sal_True heisst "deep"

            // ein hidden control ?
            sal_Bool bIsHidden = IsHiddenControl(pCurrent);

            // Forms und hidden Controls muss ich behalten, alles andere nicht
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
                // #103597#
            }
            else
                ++it;
        }
        pFormShell->GetImpl()->EnableTrackProperties(sal_True);

        // let the view delete the marked controls
        pFormShell->GetFormView()->DeleteMarked();

        // start UNDO at this point. Unfortunately, this results in 2 UNDO actions, since DeleteMarked is
        // creating an own one. However, if we'd move it before DeleteMarked, Writer does not really like
        // this ... :(
        // #i31038#
        {
            // ---------------
            // initialize UNDO
            String aUndoStr;
            if ( m_arrCurrentSelection.size() == 1 )
            {
                aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_REMOVE);
                if (m_nFormsSelected)
                    aUndoStr.SearchAndReplaceAscii( "#", SVX_RESSTR( RID_STR_FORM ) );
                else
                    // it must be a control (else the root would be selected, but it cannot be deleted)
                    aUndoStr.SearchAndReplaceAscii( "#", SVX_RESSTR( RID_STR_CONTROL ) );
            }
            else
            {
                aUndoStr = SVX_RESSTR(RID_STR_UNDO_CONTAINER_REMOVE_MULTIPLE);
                aUndoStr.SearchAndReplaceAscii( "#", String::CreateFromInt32( m_arrCurrentSelection.size() ) );
            }
            pFormModel->BegUndo(aUndoStr);
        }

        // remove remaining structure
        for (SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
             it != m_arrCurrentSelection.end(); ++it)
        {
            FmEntryData* pCurrent = (FmEntryData*)((*it)->GetUserData());

            // if the entry still has children, we skipped deletion of one of those children.
            // This may for instance be because the shape is in a hidden layer, where we're unable
            // to remove it
            if ( pCurrent->GetChildList()->size() )
                continue;

            // noch ein kleines Problem, bevor ich das ganz loesche : wenn es eine Form ist und die Shell diese als CurrentObject
            // kennt, dann muss ich ihr das natuerlich ausreden
            if (pCurrent->ISA(FmFormData))
            {
                Reference< XForm >  xCurrentForm( static_cast< FmFormData* >( pCurrent )->GetFormIface() );
                if ( pFormShell->GetImpl()->getCurrentForm() == xCurrentForm )  // die Shell kennt die zu loeschende Form ?
                    pFormShell->GetImpl()->forgetCurrentForm();                 // -> wegnehmen ...
            }
            GetNavModel()->Remove(pCurrent, sal_True);
        }
        pFormModel->EndUndo();
    }

    //------------------------------------------------------------------------
    void NavigatorTree::CollectSelectionData(SELDATA_ITEMS sdiHow)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::CollectSelectionData" );
        DBG_ASSERT(sdiHow != SDI_DIRTY, "NavigatorTree::CollectSelectionData : ever thought about your parameter ? DIRTY ?");
        if (sdiHow == m_sdiState)
            return;

        m_arrCurrentSelection.clear();
        m_nFormsSelected = m_nControlsSelected = m_nHiddenControls = 0;
        m_bRootSelected = sal_False;

        SvTreeListEntry* pSelectionLoop = FirstSelected();
        while (pSelectionLoop)
        {
            // erst mal die Zaehlung der verschiedenen Elemente
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
                // alles, was schon einen selektierten Vorfahr hat, nicht mitnehmen
                if (pSelectionLoop == m_pRootEntry)
                    m_arrCurrentSelection.insert(pSelectionLoop);
                else
                {
                    SvTreeListEntry* pParentLoop = GetParent(pSelectionLoop);
                    while (pParentLoop)
                    {
                        // eigentlich muesste ich testen, ob das Parent in der m_arrCurrentSelection steht ...
                        // Aber wenn es selektiert ist, dann steht es in m_arrCurrentSelection, oder wenigstens einer seiner Vorfahren,
                        // wenn der auch schon selektiert war. In beiden Faellen reicht also die Abfrage IsSelected
                        if (IsSelected(pParentLoop))
                            break;
                        else
                        {
                            if (m_pRootEntry == pParentLoop)
                            {
                                // bis (exclusive) zur Root gab es kein selektiertes Parent -> der Eintrag gehoert in die normalisierte Liste
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

    //------------------------------------------------------------------------
    void NavigatorTree::SynchronizeSelection(FmEntryDataArray& arredToSelect)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::SynchronizeSelection" );
        LockSelectionHandling();
        if (arredToSelect.empty())
        {
            SelectAll(sal_False);
        }
        else
        {
            // erst mal gleiche ich meine aktuelle Selektion mit der geforderten SelectList ab
            SvTreeListEntry* pSelection = FirstSelected();
            while (pSelection)
            {
                FmEntryData* pCurrent = (FmEntryData*)pSelection->GetUserData();
                if (pCurrent != NULL)
                {
                    FmEntryDataArray::iterator it = arredToSelect.find(pCurrent);
                    if ( it != arredToSelect.end() )
                    {   // der Entry ist schon selektiert, steht aber auch in der SelectList -> er kann aus letzterer
                        // raus
                        arredToSelect.erase(it);
                    } else
                    {   // der Entry ist selektiert, aber steht nicht in der SelectList -> Selektion rausnehmen
                        Select(pSelection, sal_False);
                        // und sichtbar machen (kann ja sein, dass das die einzige Modifikation ist, die ich hier in dem
                        // ganzen Handler mache, dann sollte das zu sehen sein)
                        MakeVisible(pSelection);
                    }
                }
                else
                    Select(pSelection, sal_False);

                pSelection = NextSelected(pSelection);
            }

            // jetzt habe ich in der SelectList genau die Eintraege, die noch selektiert werden muessen
            // zwei Moeglichkeiten : 1) ich gehe durch die SelectList, besorge mir zu jedem Eintrag meinen SvTreeListEntry
            // und selektiere diesen (waere irgendwie intuitiver ;)) 2) ich gehe durch alle meine SvLBoxEntries und selektiere
            // genau die, die ich in der SelectList finde
            // 1) braucht O(k*n) (k=Laenge der SelectList, n=Anzahl meiner Entries), plus den Fakt, dass FindEntry nicht den
            // Pointer auf die UserDaten vergleicht, sondern ein aufwendigeres IsEqualWithoutChildren durchfuehrt
            // 2) braucht O(n*log k), dupliziert aber etwas Code (naemlich den aus FindEntry)
            // da das hier eine relativ oft aufgerufenen Stelle sein koennte (bei jeder Aenderung in der Markierung in der View !),
            // nehme ich doch lieber letzteres
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

    //------------------------------------------------------------------------
    void NavigatorTree::SynchronizeSelection()
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::SynchronizeSelection" );
        // Shell und View
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if(!pFormShell) return;

        FmFormView* pFormView = pFormShell->GetFormView();
        if (!pFormView) return;

        GetNavModel()->BroadcastMarkedObjects(pFormView->GetMarkedObjectList());
    }

    //------------------------------------------------------------------------
    void NavigatorTree::SynchronizeMarkList()
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::SynchronizeMarkList" );
        // die Shell werde ich brauchen ...
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell) return;

        CollectSelectionData(SDI_NORMALIZED_FORMARK);

        // Die View soll jetzt kein Notify bei einer Aenderung der MarkList rauslassen
        pFormShell->GetImpl()->EnableTrackProperties(sal_False);

        UnmarkAllViewObj();

        for (SvLBoxEntrySortedArray::const_iterator it = m_arrCurrentSelection.begin();
             it != m_arrCurrentSelection.end(); ++it)
        {
            SvTreeListEntry* pSelectionLoop = *it;
            // Bei Formselektion alle Controls dieser Form markieren
            if (IsFormEntry(pSelectionLoop) && (pSelectionLoop != m_pRootEntry))
                MarkViewObj((FmFormData*)pSelectionLoop->GetUserData(), sal_True, sal_False);

            // Bei Controlselektion Control-SdrObjects markieren
            else if (IsFormComponentEntry(pSelectionLoop))
            {
                FmControlData* pControlData = (FmControlData*)pSelectionLoop->GetUserData();
                if (pControlData)
                {
                    /////////////////////////////////////////////////////////////////
                    // Beim HiddenControl kann kein Object selektiert werden
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

        // wenn der PropertyBrowser offen ist, muss ich den entsprechend meiner Selektion anpassen
        // (NICHT entsprechend der MarkList der View : wenn ich ein Formular selektiert habe, sind in der
        // View alle zugehoerigen Controls markiert, trotzdem moechte ich natuerlich die Formular-Eigenschaften
        // sehen)
        ShowSelectionProperties(sal_False);

        // Flag an View wieder zuruecksetzen
        pFormShell->GetImpl()->EnableTrackProperties(sal_True);

        // wenn jetzt genau eine Form selektiert ist, sollte die Shell das als CurrentForm mitbekommen
        // (wenn SelectionHandling nicht locked ist, kuemmert sich die View eigentlich in MarkListHasChanged drum,
        // aber der Mechanismus greift zum Beispiel nicht, wenn die Form leer ist)
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

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::IsHiddenControl(FmEntryData* pEntryData)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::IsHiddenControl" );
        if (pEntryData == NULL) return sal_False;

        Reference< XPropertySet > xProperties( pEntryData->GetPropertySet() );
        if (::comphelper::hasProperty(FM_PROP_CLASSID, xProperties))
        {
            Any aClassID = xProperties->getPropertyValue( FM_PROP_CLASSID );
            return (::comphelper::getINT16(aClassID) == FormComponentType::HIDDENCONTROL);
        }
        return sal_False;
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::Select( SvTreeListEntry* pEntry, sal_Bool bSelect )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::Select" );
        if (bSelect == IsSelected(pEntry))  // das passiert manchmal, ich glaube, die Basisklasse geht zu sehr auf Nummer sicher ;)
            return sal_True;

        return SvTreeListBox::Select(pEntry, bSelect );
    }

    //------------------------------------------------------------------------
    void NavigatorTree::UnmarkAllViewObj()
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::UnmarkAllViewObj" );
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( !pFormShell )
            return;
        FmFormView* pFormView = pFormShell->GetFormView();
        pFormView->UnMarkAll();
    }
    //------------------------------------------------------------------------
    void NavigatorTree::MarkViewObj(FmFormData* pFormData, sal_Bool bMark, sal_Bool bDeep )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::MarkViewObjects" );
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( !pFormShell )
            return;

        // first collect all sdrobjects
        ::std::set< Reference< XFormComponent > > aObjects;
        CollectObjects(pFormData,bDeep,aObjects);

        //////////////////////////////////////////////////////////////////////
        // In der Page das entsprechende SdrObj finden und selektieren
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
            if ( xControlModel.is() && aObjects.find(xControlModel) != aObjects.end() && bMark != pFormView->IsObjMarked( pSdrObject ) )
            {
                // unfortunately, the writer doesn't like marking an already-marked object, again, so reset the mark first
                pFormView->MarkObj( pSdrObject, pPageView, !bMark, sal_False );
            }
        } // while ( aIter.IsMore() )
        if ( bMark )
        {
            // make the mark visible
            ::Rectangle aMarkRect( pFormView->GetAllMarkedRect());
            for ( sal_uInt32 i = 0; i < pFormView->PaintWindowCount(); ++i )
            {
                SdrPaintWindow* pPaintWindow = pFormView->GetPaintWindow( i );
                OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
                if ( ( OUTDEV_WINDOW == rOutDev.GetOutDevType() ) && !aMarkRect.IsEmpty() )
                {
                    pFormView->MakeVisible( aMarkRect, (Window&)rOutDev );
                }
            } // for ( sal_uInt32 i = 0; i < pFormView->PaintWindowCount(); ++i )
        }
    }
    //------------------------------------------------------------------------
    void NavigatorTree::CollectObjects(FmFormData* pFormData, sal_Bool bDeep, ::std::set< Reference< XFormComponent > >& _rObjects)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::MarkViewObjects" );
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
            } // if( pEntryData->ISA(FmControlData) )
            else if (bDeep && (pEntryData->ISA(FmFormData)))
                CollectObjects((FmFormData*)pEntryData,bDeep,_rObjects);
        } // for( sal_uInt32 i=0; i<pChildList->Count(); i++ )
    }
    //------------------------------------------------------------------------
    void NavigatorTree::MarkViewObj( FmControlData* pControlData, sal_Bool bMarkHandles, sal_Bool bMark)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTree::MarkViewObj" );
        if( !pControlData )
            return;
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( !pFormShell )
            return;

        //////////////////////////////////////////////////////////////////////
        // In der Page das entsprechende SdrObj finden und selektieren
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
            if ( bMark != pFormView->IsObjMarked( pSdrObject ) )
                // unfortunately, the writer doesn't like marking an already-marked object, again, so reset the mark first
                pFormView->MarkObj( pSdrObject, pPageView, !bMark, sal_False );

            if ( !bMarkHandles || !bMark )
                continue;

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
                    pFormView->MakeVisible( aMarkRect, (Window&)rOutDev );
                }
            } // for ( sal_uInt32 i = 0; i < pFormView->PaintWindowCount(); ++i )
        }
    }

//............................................................................
}   // namespace svxform
//............................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
