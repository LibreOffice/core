/*************************************************************************
 *
 *  $RCSfile: navigatortree.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:31:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_DIALMGR_HXX //autogen
#include "dialmgr.hxx"
#endif
#ifndef _SVX_FMSHELL_HXX
#include "fmshell.hxx"
#endif
#ifndef _SVX_FMMODEL_HXX
#include "fmmodel.hxx"
#endif
#ifndef _SVX_FMPAGE_HXX
#include "fmpage.hxx"
#endif
#ifndef _SVDPAGV_HXX
#include "svdpagv.hxx"
#endif
#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif

#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif
#ifndef _SVX_FMEXPL_HRC
#include "fmexpl.hrc"
#endif
#ifndef _SVX_FMEXPL_HXX
#include "fmexpl.hxx"
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _SVX_FMSHIMP_HXX
#include "fmshimp.hxx"
#endif
#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif
#ifndef _SVX_TABORDER_HXX
#include "taborder.hxx"
#endif
#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif
#ifndef _SVX_FMUNOPGE_HXX
#include "fmpgeimp.hxx"
#endif
#ifndef _MULTIPRO_HXX
#include "multipro.hxx"
#endif
#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
#endif

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

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
            // get the shape
            SdrObject* pShape = aIter.Next();

            // is it a UNO control shape?
            if ( pShape->IsUnoObj() )
            {
                Reference< XInterface > xNormalizedModel;
                xNormalizedModel = xNormalizedModel.query( ( static_cast< SdrUnoObj* >( pShape )->GetUnoControlModel() ) );
                    // note that this is normalized (i.e. queried for XInterface explicitly)

#ifdef DBG_UTIL
                ::std::pair< MapModelToShape::iterator, bool > aPos =
#endif
                _rMapping.insert( ModelShapePair( xNormalizedModel, pShape ) );
                DBG_ASSERT( aPos.second, "collectShapeModelMapping: model was already existent!" );
                    // if this asserts, this would mean we have 2 shapes pointing to the same model
            }
        }
    }

    //------------------------------------------------------------------------
    sal_Bool isModelShapeMarked( FmEntryData* _pEntry, const MapModelToShape& _rModelMap, SdrMarkView* _pView )
    {
        DBG_ASSERT( _pEntry && _pView, "isModelShapeMarked: invalid arguments!" );
        if ( !_pEntry || !_pView )
            return sal_False;

        DBG_ASSERT( _pEntry->GetElement().get() == Reference< XInterface >( _pEntry->GetElement(), UNO_QUERY ).get(),
            "isModelShapeMarked: element of the FmEntryData is not normalized!" );
            // normalization of the XInterface is a prerequisite for properly finding it in the map

        sal_Bool bIsMarked = sal_False;

        MapModelToShape::const_iterator aPos = _rModelMap.find( _pEntry->GetElement() );
        if ( _rModelMap.end() != aPos )
        {   // there is a shape for this model ....
            bIsMarked = _pView->IsObjMarked( aPos->second );
            if ( !bIsMarked )
            {
                // IsObjMarked does not step down grouped objects, so the sal_False we
                // have is not really reliable (while a sal_True would have been)
                // Okay, travel the mark list, and see if there is a group marked, and our shape
                // is a part of this group
                sal_uInt32 nMarked = _pView->GetMarkList().GetMarkCount();
                for ( sal_uInt32 i = 0; (i<nMarked ) && !bIsMarked; ++i )
                {
                    SdrMark* pMark = _pView->GetMarkList().GetMark( i );
                    SdrObject* pObj = pMark ? pMark->GetObj() : NULL;
                    if ( pObj && pObj->IsGroupObject() )
                    {   // the i-th marked shape is a group shape
                        SdrObjListIter aIter( *pObj );
                        while ( aIter.IsMore() )
                        {
                            if ( aIter.Next() == aPos->second )
                            {
                                bIsMarked = sal_True;
                                break;
                            }
                        }
                    }
                }
            }
        }

        return bIsMarked;
    }

    //========================================================================
    // class NavigatorTree
    //========================================================================

    //------------------------------------------------------------------------
    NavigatorTree::NavigatorTree( const Reference< XMultiServiceFactory >& _xORB,
                           Window* pParent )
        :SvTreeListBox( pParent, WB_HASBUTTONS|WB_HASLINES|WB_BORDER|WB_HSCROLL ) // #100258# OJ WB_HSCROLL added
        ,m_aControlExchange(this)
        ,nEditEvent(0)
        ,m_pEditEntry(NULL)
        ,m_nSelectLock(0)
        ,m_aTimerTriggered(-1,-1)
        ,m_bRootSelected(sal_False)
        ,m_nFormsSelected(0)
        ,m_nControlsSelected(0)
        ,m_nHiddenControls(0)
        ,m_bPrevSelectionMixed(sal_False)
        ,m_bInitialUpdate(sal_True)
        ,m_bMarkingObjects(sal_False)
        ,m_sdiState(SDI_DIRTY)
        ,m_bDragDataDirty(sal_False)
        ,m_bKeyboardCut( sal_False )
        ,m_pRootEntry(NULL)
        ,m_xORB(_xORB)
    {
        SetHelpId( HID_FORM_NAVIGATOR );

        m_aNavigatorImages = ImageList( SVX_RES( RID_SVXIMGLIST_FMEXPL ) );
        m_aNavigatorImagesHC = ImageList( SVX_RES( RID_SVXIMGLIST_FMEXPL_HC ) );

        SetNodeBitmaps(
            m_aNavigatorImages.GetImage( RID_SVXIMG_COLLAPSEDNODE ),
            m_aNavigatorImages.GetImage( RID_SVXIMG_EXPANDEDNODE ),
            BMP_COLOR_NORMAL
        );
        SetNodeBitmaps(
            m_aNavigatorImagesHC.GetImage( RID_SVXIMG_COLLAPSEDNODE ),
            m_aNavigatorImagesHC.GetImage( RID_SVXIMG_EXPANDEDNODE ),
            BMP_COLOR_HIGHCONTRAST
        );

        SetDragDropMode(0xFFFF);
        EnableInplaceEditing( sal_True );
        SetSelectionMode(MULTIPLE_SELECTION);

        m_pNavModel = new NavigatorTreeModel( m_aNavigatorImages, m_aNavigatorImagesHC );
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
        m_pNavModel->Clear();
    }

    //------------------------------------------------------------------------
    void NavigatorTree::Update( FmFormShell* pFormShell )
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
            // neue Shell, waehrend ich gerade editiere ?
            if (IsEditingActive())
                CancelTextEditing();

            m_bDragDataDirty = sal_True;    // sicherheitshalber, auch wenn ich gar nicht dragge
        }
        GetNavModel()->Update( pFormShell );

        // wenn es eine Form gibt, die Root expandieren
        if (m_pRootEntry && !IsExpanded(m_pRootEntry))
            Expand(m_pRootEntry);
        // wenn es GENAU eine Form gibt, auch diese expandieren
        if (m_pRootEntry)
        {
            SvLBoxEntry* pFirst = FirstChild(m_pRootEntry);
            if (pFirst && !NextSibling(pFirst))
                Expand(pFirst);
        }
    }

    //------------------------------------------------------------------------------
    sal_Bool NavigatorTree::implAllowExchange( sal_Int8 _nAction, sal_Bool* _pHasNonHidden )
    {
        SvLBoxEntry* pCurEntry = GetCurEntry();
        if (!pCurEntry)
            return sal_False;

        // die Informationen fuer das AcceptDrop und ExecuteDrop
        CollectSelectionData(SDI_ALL);
        if (!m_arrCurrentSelection.Count())
            // nothing to do
            return sal_False;

        // testen, ob es sich vielleicht ausschliesslich um hidden controls handelt (dann koennte ich pCtrlExch noch ein
        // zusaetzliches Format geben)
        sal_Bool bHasNonHidden = sal_False;
        for (sal_Int32 i=0; i<m_arrCurrentSelection.Count(); i++)
        {
            FmEntryData* pCurrent = static_cast< FmEntryData* >( m_arrCurrentSelection[(sal_uInt16)i]->GetUserData() );
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
        EndSelection();

        sal_Bool bHasNonHidden = sal_False;
        if ( !implAllowExchange( _nAction, &bHasNonHidden ) )
            return sal_False;

        m_aControlExchange.prepareDrag();
        m_aControlExchange->setFocusEntry( GetCurEntry() );

        for (sal_Int32 i=0; i<m_arrCurrentSelection.Count(); ++i)
            m_aControlExchange->addSelectedEntry(m_arrCurrentSelection[(sal_uInt16)i]);

        m_aControlExchange->setFormsRoot( GetNavModel()->GetFormPage()->GetForms() );
        m_aControlExchange->buildPathFormat( this, m_pRootEntry );

        if (!bHasNonHidden)
        {
            // eine entsprechende Sequenz aufbauen
            Sequence< Reference< XInterface > > seqIFaces(m_arrCurrentSelection.Count());
            Reference< XInterface >* pArray = seqIFaces.getArray();
            for (i=0; i<m_arrCurrentSelection.Count(); ++i, ++pArray)
                *pArray = static_cast< FmEntryData* >( m_arrCurrentSelection[(sal_uInt16)i]->GetUserData() )->GetElement();

            // und das neue Format
            m_aControlExchange->addHiddenControlsFormat(seqIFaces);
        }

        m_bDragDataDirty = sal_False;
        return sal_True;
    }

    //------------------------------------------------------------------------------
    void NavigatorTree::StartDrag( sal_Int8 nAction, const ::Point& rPosPixel )
    {
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
                    SvLBoxEntry* ptClickedOn = GetEntry(ptWhere);
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
                    if (m_arrCurrentSelection.Count() == 0) // kann nur bei Kontextmenue ueber Tastatur passieren
                        break;

                    SvLBoxEntry* pCurrent = GetCurEntry();
                    if (!pCurrent)
                        break;
                    ptWhere = GetEntryPos(pCurrent);
                }

                // meine Selektionsdaten auf den aktuellen Stand
                CollectSelectionData(SDI_ALL);

                // wenn mindestens ein Nicht-Root-Eintrag selektiert ist und die Root auch, dann nehme ich letztere aus der Selektion
                // fix wieder raus
                if ( (m_arrCurrentSelection.Count() > 1) && m_bRootSelected )
                {
                    Select( m_pRootEntry, sal_False );
                    SetCursor( m_arrCurrentSelection.GetObject(0), sal_True);
                }
                sal_Bool bSingleSelection = (m_arrCurrentSelection.Count() == 1);


                DBG_ASSERT( (m_arrCurrentSelection.Count() > 0) | m_bRootSelected, "keine Eintraege selektiert" );
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

                    // Properties, wenn es nur Controls oder genau ein Formular ist
                    // (und der ProBrowser nicht sowieso schon offen ist)
                    if( pFormShell->GetImpl()->IsPropBrwOpen() )
                        aContextMenu.RemoveItem( aContextMenu.GetItemPos(SID_FM_SHOW_PROPERTY_BROWSER) );
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
                        FmControlData* pCurrent = (FmControlData*)(m_arrCurrentSelection[0]->GetUserData());
                        Reference< XPropertySet >  xCurrentProps(pCurrent->GetFormComponent(), UNO_QUERY);
                        pFormShell->GetImpl()->CheckControlConversionSlots(xCurrentProps, *aContextMenu.GetPopupMenu(SID_FM_CHANGECONTROLTYPE));
                            // die Shell filtert nach weiteren Bedingungen, zum Beispiel kein 'hidden control'
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
                            XubString aStr(SVX_RES(RID_STR_FORM));
                            XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
                            aUndoStr.SearchAndReplace('#', aStr);

                            pFormModel->BegUndo(aUndoStr);
                            // der Slot war nur verfuegbar, wenn es genau einen selektierten Eintrag gibt und dieser die Root
                            // oder ein Formular ist
                            NewForm( m_arrCurrentSelection.GetObject(0) );
                            pFormModel->EndUndo();

                        }   break;
                        case SID_FM_NEW_HIDDEN:
                        {
                            XubString aStr(SVX_RES(RID_STR_CONTROL));
                            XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
                            aUndoStr.SearchAndReplace('#', aStr);

                            pFormModel->BegUndo(aUndoStr);
                            // dieser Slot war guletig bei (genau) einem selektierten Formular
                            rtl::OUString fControlName = FM_COMPONENT_HIDDEN;
                            NewControl( fControlName, m_arrCurrentSelection.GetObject(0) );
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
                            SvLBoxEntry* pSelectedForm = m_arrCurrentSelection.GetObject(0);
                            DBG_ASSERT( IsFormEntry(pSelectedForm), "NavigatorTree::Command: Dieser Eintrag muss ein FormEntry sein." );

                            FmFormData* pFormData = (FmFormData*)pSelectedForm->GetUserData();
                            Reference< XForm >  xForm(  pFormData->GetFormIface());

                            Reference< XTabControllerModel >  xTabController(xForm, UNO_QUERY);
                            if( !xTabController.is() ) break;
                            FmTabOrderDlg aTabDlg(m_xORB, GetpApp()->GetAppWindow(), GetNavModel()->GetFormShell() );
                            aTabDlg.Execute();

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
                            EditEntry( m_arrCurrentSelection.GetObject(0) );
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
                            if (pFormShell->GetImpl()->IsControlConversionSlot(nSlotId))
                            {
                                FmControlData* pCurrent = (FmControlData*)(m_arrCurrentSelection[0]->GetUserData());
                                if (pFormShell->GetImpl()->ExecuteControlConversionSlot(pCurrent->GetFormComponent(), nSlotId))
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
    sal_Bool NavigatorTree::IsDeleteAllowed()
    {
        //////////////////////////////////////////////////////////////////////
        // Haben wir eine Form...
        SvLBoxEntry* pCurEntry = GetCurEntry();
        sal_uInt32 nCurEntryPos = GetModel()->GetAbsPos( pCurEntry );

        if( nCurEntryPos==0 )           // Root kann nicht geloescht werden
            return sal_False;
        else
            return IsFormEntry(pCurEntry) || IsFormComponentEntry(pCurEntry);
    }

    //------------------------------------------------------------------------
    SvLBoxEntry* NavigatorTree::FindEntry( FmEntryData* pEntryData )
    {
        if( !pEntryData ) return NULL;
        SvLBoxEntry* pCurEntry = First();
        FmEntryData* pCurEntryData;
        while( pCurEntry )
        {
            pCurEntryData = (FmEntryData*)pCurEntry->GetUserData();
            if( pCurEntryData && pCurEntryData->IsEqualWithoutChilds(pEntryData) )
                return pCurEntry;

            pCurEntry = Next( pCurEntry );
        }

        return NULL;
    }

    //------------------------------------------------------------------------
    void NavigatorTree::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
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
            SvLBoxEntry* pEntry = FindEntry( pData );
            if (pEntry)
            {   // das Image neu setzen
                SetCollapsedEntryBmp( pEntry, pData->GetNormalImage(), BMP_COLOR_NORMAL );
                SetExpandedEntryBmp( pEntry, pData->GetNormalImage(), BMP_COLOR_NORMAL );

                SetCollapsedEntryBmp( pEntry, pData->GetHCImage(), BMP_COLOR_HIGHCONTRAST );
                SetExpandedEntryBmp( pEntry, pData->GetHCImage(), BMP_COLOR_HIGHCONTRAST );
            }
        }

        else if( rHint.ISA(FmNavNameChangedHint) )
        {
            FmNavNameChangedHint* pNameChangedHint = (FmNavNameChangedHint*)&rHint;
            SvLBoxEntry* pEntry = FindEntry( pNameChangedHint->GetEntryData() );
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

            if ( m_pRootEntry )
            {
                Image aHCRootImage( m_aNavigatorImagesHC.GetImage( RID_SVXIMG_FORMS ) );
                SetExpandedEntryBmp( m_pRootEntry, aHCRootImage, BMP_COLOR_HIGHCONTRAST );
                SetCollapsedEntryBmp( m_pRootEntry, aHCRootImage, BMP_COLOR_HIGHCONTRAST );
            }
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
    SvLBoxEntry* NavigatorTree::Insert( FmEntryData* pEntryData, sal_uInt32 nRelPos )
    {
        //////////////////////////////////////////////////////////////////////
        // Aktuellen Eintrag einfuegen
        SvLBoxEntry* pParentEntry = FindEntry( pEntryData->GetParent() );
        SvLBoxEntry* pNewEntry;

        if( !pParentEntry )
            pNewEntry = InsertEntry( pEntryData->GetText(),
                pEntryData->GetNormalImage(), pEntryData->GetNormalImage(),
                m_pRootEntry, sal_False, nRelPos, pEntryData );

        else
            pNewEntry = InsertEntry( pEntryData->GetText(),
                pEntryData->GetNormalImage(), pEntryData->GetNormalImage(),
                pParentEntry, sal_False, nRelPos, pEntryData );

        if ( pNewEntry )
        {
            SetExpandedEntryBmp( pNewEntry, pEntryData->GetHCImage(), BMP_COLOR_HIGHCONTRAST );
            SetCollapsedEntryBmp( pNewEntry, pEntryData->GetHCImage(), BMP_COLOR_HIGHCONTRAST );
        }

        //////////////////////////////////////////////////////////////////////
        // Wenn Root-Eintrag Root expandieren
        if( !pParentEntry )
            Expand( m_pRootEntry );

        //////////////////////////////////////////////////////////////////////
        // Childs einfuegen
        FmEntryDataList* pChildList = pEntryData->GetChildList();
        sal_uInt32 nChildCount = pChildList->Count();
        FmEntryData* pChildData;
        for( sal_uInt32 i=0; i<nChildCount; i++ )
        {
            pChildData = pChildList->GetObject(i);
            Insert( pChildData, LIST_APPEND );
        }

        return pNewEntry;
    }

    //------------------------------------------------------------------------
    void NavigatorTree::Remove( FmEntryData* pEntryData )
    {
        if( !pEntryData )
            return;

        // der Entry zu den Daten
        SvLBoxEntry* pEntry = FindEntry( pEntryData );
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
        ULONG nExpectedSelectionCount = GetSelectionCount();

        if( pEntry )
            GetModel()->Remove( pEntry );

        if (nExpectedSelectionCount != GetSelectionCount())
            SynchronizeSelection();

        // und standardmaessig behandle ich das Select natuerlich
        UnlockSelectionHandling();
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::IsFormEntry( SvLBoxEntry* pEntry )
    {
        FmEntryData* pEntryData = (FmEntryData*)pEntry->GetUserData();
        return !pEntryData || pEntryData->ISA(FmFormData);
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::IsFormComponentEntry( SvLBoxEntry* pEntry )
    {
        FmEntryData* pEntryData = (FmEntryData*)pEntry->GetUserData();
        return pEntryData && pEntryData->ISA(FmControlData);
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::implAcceptPaste( )
    {
        SvLBoxEntry* pFirstSelected = FirstSelected();
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
        return implAcceptDataTransfer( _rFlavors, _nAction, GetEntry( _rDropPos ), _bDnD );
    }

    //------------------------------------------------------------------------
    sal_Int8 NavigatorTree::implAcceptDataTransfer( const DataFlavorExVector& _rFlavors, sal_Int8 _nAction, SvLBoxEntry* _pTargetEntry, sal_Bool _bDnD )
    {
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
        ListBoxEntryArray aDropped = m_aControlExchange->selected();
        DBG_ASSERT(aDropped.size() >= 1, "NavigatorTree::implAcceptDataTransfer: keine Eintraege !");

        sal_Bool bDropTargetIsComponent = IsFormComponentEntry( _pTargetEntry );
        SvLBoxEntry* pDropTargetParent = GetParent( _pTargetEntry );

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
        SvLBoxEntry* pLoop = _pTargetEntry;
        while (pLoop)
        {
            arrDropAnchestors.Insert(pLoop);
            pLoop = GetParent(pLoop);
        }

        for (size_t i=0; i<aDropped.size(); i++)
        {
            SvLBoxEntry* pCurrent = aDropped[i];
            SvLBoxEntry* pCurrentParent = GetParent(pCurrent);

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
                sal_uInt16 nPosition;
                if ( arrDropAnchestors.Seek_Entry(pCurrent, &nPosition) )
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
                {   // auf einem Entry mit Childs, der nicht aufgeklappt ist ?
                    SvLBoxEntry* pDropppedOn = GetEntry(aDropPos);
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
        return implExecuteDataTransfer( _rData, _nAction, GetEntry( _rDropPos ), _bDnD );
    }

    //------------------------------------------------------------------------
    sal_Int8 NavigatorTree::implExecuteDataTransfer( const OControlTransferData& _rData, sal_Int8 _nAction, SvLBoxEntry* _pTargetEntry, sal_Bool _bDnD )
    {
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
        sal_Bool bHasHiddenControlsFormat = OControlExchange::hasHiddenControlModelsFormat( rDataFlavors );
        sal_Bool bForeignCollection = _rData.getFormsRoot().get() != GetNavModel()->GetFormPage()->GetForms().get();
#ifdef DBG_UTIL
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
                XubString aStr(SVX_RES(RID_STR_CONTROL));
                XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
                aUndoStr.SearchAndReplace('#', aStr);
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
#if (OSL_DEBUG_LEVEL > 1) || DBG_UTIL
                // nur mal eben sehen, ob das Ding tatsaechlich ein hidden control ist
                sal_Int16 nClassId = ::comphelper::getINT16(xCurrent->getPropertyValue(FM_PROP_CLASSID));
                DBG_ASSERT(nClassId == FormComponentType::HIDDENCONTROL, "NavigatorTree::implExecuteDataTransfer: invalid control in drop list !");
                    // wenn das SVX_FM_HIDDEN_CONTROLS-Format vorhanden ist, dann sollten wirklich nur hidden controls in der Sequenz
                    // stecken
#endif // (OSL_DEBUG_LEVEL > 1) || DBG_UTIL
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

                SvLBoxEntry* pToSelect = FindEntry(pNewControlData);
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
        const ListBoxEntryArray& aDropped = _rData.selected();
        DBG_ASSERT(aDropped.size() >= 1, "NavigatorTree::implExecuteDataTransfer: no entries!");

        // die Shell und das Model
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;
        if (!pFormModel)
            return DND_ACTION_NONE;

        // fuer's Undo
        XubString strUndoDescription(SVX_RES(RID_STR_UNDO_CONTAINER_REPLACE));
            // TODO : den ::rtl::OUString aussagekraeftiger machen
        pFormModel->BegUndo(strUndoDescription);

        // ich nehme vor dem Einfuegen eines Eintrages seine Selection raus, damit die Markierung dabei nicht flackert
        // -> das Handeln des Select locken
        LockSelectionHandling();

        // jetzt durch alle gedroppten Eintraege ...
        for (size_t i=0; i<aDropped.size(); ++i)
        {
            // ein paar Daten zum aktuellen Element
            SvLBoxEntry* pCurrent = aDropped[i];
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
                pCurrentParentUserData->GetChildList()->Remove(pCurrentUserData);
            else
                GetNavModel()->GetRootList()->Remove(pCurrentUserData);

            // aus dem Container entfernen
            sal_Int32 nIndex = getElementPos(Reference< XIndexAccess > (xContainer, UNO_QUERY), xCurrentChild);
            GetNavModel()->m_pPropChangeList->Lock();
            // die Undo-Action fuer das Rausnehmen
            if (GetNavModel()->m_pPropChangeList->CanUndo())
                pFormModel->AddUndo(new FmUndoContainerAction(*pFormModel, FmUndoContainerAction::Removed,
                                                            xContainer, xCurrentChild, nIndex));

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
            if (GetNavModel()->m_pPropChangeList->CanUndo())
                pFormModel->AddUndo(new FmUndoContainerAction(*pFormModel, FmUndoContainerAction::Inserted,
                                                         xContainer, xCurrentChild, nIndex));

            // einfuegen im neuen Container
            if (pTargetData)
            {
                 // es wird in eine Form eingefuegt, dann brauche ich eine FormComponent
                xContainer->insertByIndex(nIndex,
                    makeAny(Reference<
                    XFormComponent>(xCurrentChild, UNO_QUERY)));
            }
            else
            {
                xContainer->insertByIndex(nIndex,
                    makeAny(Reference<
                    XForm>(xCurrentChild, UNO_QUERY)));
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
                pTargetData->GetChildList()->Insert(pCurrentUserData, nIndex);
            else
                GetNavModel()->GetRootList()->Insert(pCurrentUserData, nIndex);

            // dann bei mir selber bekanntgeben und neu selektieren
            SvLBoxEntry* pNew = Insert( pCurrentUserData, nIndex );
            if ( ( 0 == i ) && pNew )
            {
                SvLBoxEntry* pParent = GetParent( pNew );
                if ( pParent )
                    Expand( pParent );
            }
        }

        UnlockSelectionHandling();
        pFormModel->EndUndo();

        // During the move, the markings of the underlying view did not change (because the view is not affected by the logical
        // hierarchy of the form/control models. But my selection changed - which means I have to adjust it according to the
        // view marks, again.
        SynchronizeSelection();

        // in addition, with the move of controls such things as "the current form" may have changed - force the shell
        // to update itself accordingly
        if( pFormShell && pFormShell->GetImpl() && pFormShell->GetFormView() )
            pFormShell->GetImpl()->DetermineSelection( pFormShell->GetFormView()->GetMarkList() );

        if ( m_aControlExchange.isClipboardOwner() && ( DND_ACTION_MOVE == _nAction ) )
            m_aControlExchange->clear();

        return _nAction;
    }

    //------------------------------------------------------------------------
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

    //------------------------------------------------------------------------
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
            DBG_ERROR( "NavigatorTree::doPaste: caught an exception!" );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTree::doCopy()
    {
        if ( implPrepareExchange( DND_ACTION_COPY ) )
        {
            m_aControlExchange.setClipboardListener( LINK( this, NavigatorTree, OnClipboardAction ) );
            m_aControlExchange.copyToClipboard( );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTree::ModelHasRemoved( SvListEntry* _pEntry )
    {
        sal_uInt16 nPosition;
        if ( m_aCutEntries.Seek_Entry( static_cast< SvLBoxEntry* >( _pEntry ), &nPosition ) )
        {
            m_aCutEntries.Remove( nPosition );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTree::doCut()
    {
        if ( implPrepareExchange( DND_ACTION_MOVE ) )
        {
            m_aControlExchange.setClipboardListener( LINK( this, NavigatorTree, OnClipboardAction ) );
            m_aControlExchange.copyToClipboard( );
            m_bKeyboardCut = sal_True;

            // mark all the entries we just "cut" into the clipboard as "nearly moved"
            for ( sal_Int32 i=0; i<m_arrCurrentSelection.Count(); ++i )
            {
                SvLBoxEntry* pEntry = m_arrCurrentSelection[ (sal_uInt16)i ];
                if ( pEntry )
                {
                    m_aCutEntries.Insert( pEntry );
                    pEntry->SetFlags( pEntry->GetFlags() | SV_ENTRYFLAG_SEMITRANSPARENT );
                    InvalidateEntry( pEntry );
                }
            }
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTree::KeyInput(const ::KeyEvent& rKEvt)
    {
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
        }

        SvTreeListBox::KeyInput(rKEvt);
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::EditingEntry( SvLBoxEntry* pEntry, Selection& rSelection )
    {
        if (!SvTreeListBox::EditingEntry( pEntry, rSelection ))
            return sal_False;

        return (pEntry && (pEntry->GetUserData() != NULL));
            // die Wurzel, die ich nicht umbenennen darf, hat als UserData NULL
    }

    //------------------------------------------------------------------------
    void NavigatorTree::NewForm( SvLBoxEntry* pParentEntry )
    {
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

        FmFormData* pNewFormData = new FmFormData( xNewForm, m_aNavigatorImages, m_aNavigatorImagesHC, pParentFormData );

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
            DBG_ERROR("NavigatorTree::NewForm : could not set esssential properties !");
        }


        //////////////////////////////////////////////////////////////////////
        // Form einfuegen
        GetNavModel()->Insert( pNewFormData, LIST_APPEND, sal_True );

        //////////////////////////////////////////////////////////////////////
        // Neue Form als aktive Form setzen
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( pFormShell )
        {
            pFormShell->GetImpl()->setCurForm( xNewForm );
            pFormShell->GetCurPage()->GetImpl()->setCurForm( xNewForm );

            pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_PROPERTIES,sal_True,sal_True);
        }
        GetNavModel()->SetModified();

        //////////////////////////////////////////////////////////////////////
        // In EditMode schalten
        SvLBoxEntry* pNewEntry = FindEntry( pNewFormData );
        EditEntry( pNewEntry );
    }

    //------------------------------------------------------------------------
    FmControlData* NavigatorTree::NewControl( const ::rtl::OUString& rServiceName, SvLBoxEntry* pParentEntry, sal_Bool bEditName )
    {
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

        FmControlData* pNewFormControlData = new FmControlData( xNewComponent, m_aNavigatorImages, m_aNavigatorImagesHC, pParentFormData );

        //////////////////////////////////////////////////////////////////////
        // Namen setzen
        FmFormView*     pFormView       = GetNavModel()->GetFormShell()->GetFormView();
        SdrPageView*    pPageView       = pFormView->GetPageViewPvNum(0);
        FmFormPage*     pPage           = (FmFormPage*)pPageView->GetPage();

        ::rtl::OUString sName = pPage->GetImpl()->setUniqueName( xNewComponent, xParentForm );

        pNewFormControlData->SetText( sName );

        //////////////////////////////////////////////////////////////////////
        // FormComponent einfuegen
        GetNavModel()->Insert( pNewFormControlData, LIST_APPEND, sal_True );
        GetNavModel()->SetModified();

        if (bEditName)
        {
            //////////////////////////////////////////////////////////////////////
            // In EditMode schalten
            SvLBoxEntry* pNewEntry = FindEntry( pNewFormControlData );
            Select( pNewEntry, sal_True );
            EditEntry( pNewEntry );
        }

        return pNewFormControlData;
    }

    //------------------------------------------------------------------------
    ::rtl::OUString NavigatorTree::GenerateName( FmEntryData* pEntryData )
    {
        const sal_uInt16 nMaxCount = 99;
        ::rtl::OUString aNewName;

        //////////////////////////////////////////////////////////////////////
        // BasisNamen erzeugen
        UniString aBaseName;
        if( pEntryData->ISA(FmFormData) )
            aBaseName = SVX_RES( RID_STR_STDFORMNAME );

        else if( pEntryData->ISA(FmControlData) )
            aBaseName = SVX_RES( RID_STR_CONTROL_CLASSNAME );

        //////////////////////////////////////////////////////////////////////
        // Neuen Namen erstellen
        FmFormData* pFormParentData = (FmFormData*)pEntryData->GetParent();

        for( sal_Int32 i=0; i<nMaxCount; i++ )
        {
            aNewName = aBaseName;
            if( i>0 )
            {
                aNewName += ::rtl::OUString::createFromAscii(" ");
                aNewName += ::rtl::OUString::valueOf(i).getStr();
            }

            if( GetNavModel()->FindData(aNewName, pFormParentData,sal_False) == NULL )
                break;
        }

        return aNewName;
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText )
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

    //------------------------------------------------------------------------
    IMPL_LINK( NavigatorTree, OnEdit, void*, EMPTYARG )
    {
        nEditEvent = 0;
        EditEntry( m_pEditEntry );
        m_pEditEntry = NULL;

        return 0L;
    }

    //------------------------------------------------------------------------
    IMPL_LINK( NavigatorTree, OnDropActionTimer, void*, EMPTYARG )
    {
        if (--m_aTimerCounter > 0)
            return 0L;

        if (m_aDropActionType == DA_EXPANDNODE)
        {
            SvLBoxEntry* pToExpand = GetEntry(m_aTimerTriggered);
            if (pToExpand && (GetChildCount(pToExpand) > 0) &&  !IsExpanded(pToExpand))
                // tja, eigentlich muesste ich noch testen, ob die Node nicht schon expandiert ist, aber ich
                // habe dazu weder in den Basisklassen noch im Model eine Methode gefunden ...
                // aber ich denke, die BK sollte es auch so vertragen
                Expand(pToExpand);

            // nach dem Expand habe ich im Gegensatz zum Scrollen natuerlich nix mehr zu tun
            m_aDropActionTimer.Stop();
        } else
        {
            switch (m_aDropActionType)
            {
                case DA_SCROLLUP :
                {
                    ScrollOutputArea(1);
                    break;
                }

                case DA_SCROLLDOWN :
                    ScrollOutputArea(-1);
                    break;
            }

            m_aTimerCounter = DROP_ACTION_TIMER_SCROLL_TICKS;
        }

        return 0L;
    }

    //------------------------------------------------------------------------
    IMPL_LINK(NavigatorTree, OnEntrySelDesel, NavigatorTree*, pThis)
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
    IMPL_LINK(NavigatorTree, OnSynchronizeTimer, void*, EMPTYARG)
    {
        SynchronizeMarkList();
        return 0L;
    }


    //------------------------------------------------------------------------
    IMPL_LINK( NavigatorTree, OnClipboardAction, void*, EMPTYARG )
    {
        if ( !m_aControlExchange.isClipboardOwner() )
        {
            if ( doingKeyboardCut() )
            {
                for ( sal_Int32 i=0; i<m_aCutEntries.Count(); ++i )
                {
                    SvLBoxEntry* pEntry = m_aCutEntries[ (sal_uInt16)i ];
                    if ( pEntry )
                    {
                        pEntry->SetFlags( pEntry->GetFlags() & ~SV_ENTRYFLAG_SEMITRANSPARENT );
                        InvalidateEntry( pEntry );
                    }
                }
                m_aCutEntries.Remove( (USHORT)0, (USHORT)m_aCutEntries.Count() );

                m_bKeyboardCut = sal_False;
            }
        }
        return 0L;
    }

    //------------------------------------------------------------------------
    void NavigatorTree::ShowSelectionProperties(sal_Bool bForce)
    {
        // zuerst brauche ich die FormShell
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell)
            // keine Shell -> ich koennte kein curObject setzen -> raus
            return;

        CollectSelectionData(SDI_ALL);
        DBG_ASSERT( (m_nFormsSelected >= 0) && (m_nControlsSelected >= 0) && (m_nHiddenControls >= 0),
            "NavigatorTree::ShowSelectionProperties : selection counter(s) invalid !");
        DBG_ASSERT( m_nFormsSelected + m_nControlsSelected + (m_bRootSelected ? 1 : 0) == m_arrCurrentSelection.Count(),
            "NavigatorTree::ShowSelectionProperties : selection meta data invalid !");


        Reference< XInterface >  xInterfaceSelected;
        Reference< XForm >  xFormSelected;
        sal_Bool bPropertiesAvailable = sal_True;
        sal_Bool bNeedSetCurControl = sal_True;

        if (m_bRootSelected)
            bPropertiesAvailable = sal_False;   // fuer die Root habe ich keine - weder einzeln noch in einer Gruppe - Properties
        else if (m_nFormsSelected + m_nControlsSelected == 0)   // keines der beiden sollte kleiner 0 sein !
            bPropertiesAvailable = sal_False;   // keine Selektion -> keine Properties
        else if (m_nFormsSelected * m_nControlsSelected != 0)
            bPropertiesAvailable = sal_False;   // gemischte Selektion -> keine Properties
        else
        {   // so, hier bin ich, wenn entweder nur Forms oder nur Controls selektiert sind
           if (m_arrCurrentSelection.Count() == 1)
            {
                if (m_nFormsSelected > 0)
                {   // es ist genau eine Form selektiert
                    FmFormData* pFormData = (FmFormData*)m_arrCurrentSelection.GetObject(0)->GetUserData();
                    xFormSelected = pFormData->GetFormIface();
                    xInterfaceSelected = xFormSelected;
                } else
                {   // es ist genau ein Control selektiert (egal ob hidden oder normal)
                    FmEntryData* pEntryData = (FmEntryData*)m_arrCurrentSelection.GetObject(0)->GetUserData();
                    xInterfaceSelected = pEntryData->GetElement();
                    pFormShell->GetImpl()->setCurControl(xInterfaceSelected);
                    bNeedSetCurControl = sal_False;

                    if (pEntryData->GetParent())
                        xFormSelected = ((FmFormData*)pEntryData->GetParent())->GetFormIface();
                }
            } else
            {   // wir haben eine MultiSelection, also muessen wir ein MultiSet dafuer aufbauen
                if (m_nFormsSelected > 0)
                {   // ... nur Forms
                    // erstmal die PropertySet-Interfaces der Forms einsammeln
                    Sequence< Reference< XPropertySet > > seqForms(m_nFormsSelected);
                    Reference< XPropertySet > * pPropSets = seqForms.getArray();
                    for (int i=0; i<m_nFormsSelected; i++)
                    {
                        FmFormData* pFormData = (FmFormData*)m_arrCurrentSelection.GetObject(i)->GetUserData();
                        pPropSets[i] = pFormData->GetPropertySet();
                    }
                    // dann diese in ein MultiSet packen
                    FmXMultiSet* pSelectionSet = new FmXMultiSet( seqForms );
                    xInterfaceSelected = Reference< XInterface > ( (XPropertySet*)pSelectionSet );
                }
                else
                {   // ... nur Controls
                    if (m_nHiddenControls == m_nControlsSelected)
                    {   // ein MultiSet fuer die Properties der hidden controls
                        Sequence< Reference< XPropertySet > > seqHiddenControls(m_nHiddenControls);
                        Reference< XPropertySet > * pPropSets = seqHiddenControls.getArray();
                        FmEntryData* pParentData = ((FmEntryData*)m_arrCurrentSelection.GetObject(0)->GetUserData())->GetParent();
                        for (int i=0; i<m_nHiddenControls; i++)
                        {
                            FmEntryData* pEntryData = (FmEntryData*)m_arrCurrentSelection.GetObject(i)->GetUserData();
                            pPropSets[i] = pEntryData->GetPropertySet();

                            if (pParentData && pParentData != pEntryData->GetParent())
                                pParentData = NULL;
                        }

                        // dann diese in ein MultiSet packen
                        FmXMultiSet* pSelectionSet = new FmXMultiSet( seqHiddenControls );
                        xInterfaceSelected = Reference< XInterface > ( (XPropertySet*)pSelectionSet );
                        // und der Shell als aktuelles Objekt anzeigen
                        pFormShell->GetImpl()->setCurControl( xInterfaceSelected );
                        bNeedSetCurControl = sal_False;

                        if (pParentData)
                            // alle hidden controls gehoeren zu einer Form
                            xFormSelected = ((FmFormData*)pParentData)->GetFormIface();
                    }
                    else if (m_nHiddenControls == 0)
                    {   // nur normale Controls
                        // ein MultiSet aus der MarkList der View aufbauen ...
                        const SdrMarkList& mlMarked = pFormShell->GetFormView()->GetMarkList();
                        FmXMultiSet* pSelectionSet = FmXMultiSet::Create( mlMarked );
                        xInterfaceSelected = Reference< XInterface > ( (XPropertySet*)pSelectionSet );
                        pFormShell->GetImpl()->setCurControl( xInterfaceSelected );
                        bNeedSetCurControl = sal_False;

                        // jetzt das Formular setzen
                        sal_Bool bMixedWithFormControls;
                        xFormSelected = pFormShell->GetImpl()->DetermineCurForm(mlMarked,bMixedWithFormControls);
                    } else
                        // gemischte Selektion aus hidden und normalen Controls -> keine Properties
                        bPropertiesAvailable = sal_False;
                }
            }

        }

        // um das Setzen des current Controls kann sich die Shell kuemmern (da gibt es einige Feinheiten, die ich hier nicht
        // neu implementieren moechte)
        if (bNeedSetCurControl)
            pFormShell->GetImpl()->DetermineSelection(pFormShell->GetFormView()->GetMarkList());
        // und dann meine Form und mein SelObject
        pFormShell->GetImpl()->setSelObject(xInterfaceSelected);
        pFormShell->GetImpl()->setCurForm(xFormSelected);

        if (pFormShell->GetImpl()->IsPropBrwOpen() || bForce)
        {
            // und jetzt kann ich das Ganze dem PropertyBrowser uebergeben
            FmInterfaceItem aInterfaceItem( SID_FM_SHOW_PROPERTY_BROWSER, xInterfaceSelected );
            pFormShell->GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_SHOW_PROPERTY_BROWSER, SFX_CALLMODE_ASYNCHRON,
                &aInterfaceItem, 0L );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTree::DeleteSelection()
    {
        // die Root darf ich natuerlich nicht mitloeschen
        sal_Bool bRootSelected = IsSelected(m_pRootEntry);
        int nSelectionCount = GetSelectionCount();
        if (bRootSelected && (nSelectionCount > 1))     // die Root plus andere Elemente ?
            Select(m_pRootEntry, sal_False);                // ja -> die Root raus

        if ((nSelectionCount == 0) || bRootSelected)    // immer noch die Root ?
            return;                                     // -> sie ist das einzige selektierte -> raus

        DBG_ASSERT(!m_bPrevSelectionMixed, "NavigatorTree::DeleteSelection() : loeschen nicht erlaubt wenn Markierung und Selektion nciht konsistent");

        // ich brauche unten das FormModel ...
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell)
            return;
        FmFormModel* pFormModel = pFormShell ? pFormShell->GetFormModel() : NULL;
        if (!pFormModel)
            return;

        // die Undo-Beschreibung
        UniString aUndoStr = SVX_RES(RID_STR_UNDO_DELETE_LOGICAL);
    /*  falls irgendwann mal Ordnung in das Undo kommt (so dass wir wirklich die aeusserste Klammer sind und beim Loeschen nur
    //  eine, nicht zwei UndoActions erzeugen), koennen wir folgenden Code ja wieder benutzen ...
        if (m_arrCurrentSelection.Count() == 1)
        {
            aUndoStr = SVX_RES(RID_STR_UNDO_CONTAINER_REMOVE);
            if (m_nFormsSelected)
                aUndoStr.SearchAndReplace("#", SVX_RES(RID_STR_FORM));
            else
                // bei selektierter Root war loeschen nicht erlaubt, also ist es ein Control
                aUndoStr.SearchAndReplace("#", SVX_RES(RID_STR_CONTROL));
        }
        else
        {
            aUndoStr = SVX_RES(RID_STR_UNDO_CONTAINER_REMOVE_MULTIPLE);
            aUndoStr.SearchAndReplace("#", UniString(::rtl::OUString(m_arrCurrentSelection.Count())));
                // der Umweg ueber den ::rtl::OUString ist notwendig, da nur der den Constructor hat, der gleich eine Zahl formatiert
        }
    */

        // see below for why we need this mapping from models to shapes
        FmFormView*     pFormView       = pFormShell->GetFormView();
        SdrPageView*    pPageView       = pFormView ? pFormView->GetPageViewPvNum(0) : NULL;
        SdrPage*        pPage           = pPageView ? pPageView->GetPage() : NULL;
        DBG_ASSERT( pPage, "NavigatorTree::DeleteSelection: invalid form page!" );

        MapModelToShape aModelShapes;
        if ( pPage )
            collectShapeModelMapping( pPage, aModelShapes );

        // jetzt muss ich noch die DeleteList etwas absichern : wenn man ein Formular und ein abhaengiges
        // Element loescht - und zwar in dieser Reihenfolge - dann ist der SvLBoxEntryPtr des abhaengigen Elementes
        // natuerlich schon ungueltig, wenn es geloescht werden soll ... diesen GPF, den es dann mit Sicherheit gibt,
        // gilt es zu verhindern, also die 'normalisierte' Liste
        CollectSelectionData(SDI_NORMALIZED);

        // folgendes Problem : ich muss das ExplorerModel::Remove verwenden, da nur dieses sich um das korrekte Loeschen von Form-
        // Objekten kuemmert. Andererseits muss ich die Controls selber ueber DeleteMarked loeschen (irgendwo im Writer gibt
        // es sonst Probleme). Wenn ich erst die Struktur, dann die Controls loesche, klappt das Undo nicht (da dann erst die Controls
        // eingefuegt werden, dann die Struktur, sprich ihr Parent-Formular). Andersrum sind die EntryDatas ungueltig, wenn ich die
        // Controls geloescht habe und zur Struktur gehe. Also muss ich die Formulare NACH den normalen Controls loeschen, damit sie
        // beim Undo VOR denen wiederhergestellt werden.
        pFormShell->GetImpl()->EnableTrackProperties(sal_False);
        int i;
        for (i = m_arrCurrentSelection.Count()-1; i>=0; --i)
        {
            FmEntryData* pCurrent = (FmEntryData*)(m_arrCurrentSelection.GetObject(i)->GetUserData());

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
                // well, not form and no hidden control -> we can remove it from m_arrCurrentSelection, as it will
                // be deleted automatically. This is because for every model (except forms and hidden control models)
                // there exist a shape, which is marked _if_and_only_if_ the model is selected in our tree.
                // This is the theory.
                // Now the practice. There may be models which do not have a shape at all. This may be because of
                // explicit API programming (in this case it was intentional by the user, or a failuer, but this
                // does not matter), or by bugs :). One of these bugs is 103597.
                // Because of this "dead" models (means not connected to a shape), we have to do an extra test
                // 103597 - 2002-09-25 - fs@openoffice.org

                if ( isModelShapeMarked( pCurrent, aModelShapes, pFormView ) )
                    // there indeed is a _marked_ shape for this model
                    m_arrCurrentSelection.Remove( (sal_uInt16)i, 1 );
            }
        }
        pFormShell->GetImpl()->EnableTrackProperties(sal_True);

        // und jetzt das eigentliche Loeschen
        // die Controls wech
        pFormShell->GetFormView()->DeleteMarked();

        // das UNDO beginne ich erst jetzt : Das DeleteMarked erzeugt eine eigene Undo-Action, in die ich mich eigentlich einklinken
        // muesste, was leider nicht geht (das laeuft irgendwo im SwDoc), also erzeuge ich eine zweite, mit der man das Loeschen der
        // logischen Struktur zuruecknehmen kann (das ist nicht schoen, dass eine eigentlich atomare Aktion zwei UndoActions erzeugt,
        // aber das einzige, was wir gefunden haben)
        pFormModel->BegUndo(aUndoStr);

        // die Struktur wech
        for (i=0; i<m_arrCurrentSelection.Count(); ++i)
        {
            FmEntryData* pCurrent = (FmEntryData*)(m_arrCurrentSelection.GetObject(i)->GetUserData());
            // noch ein kleines Problem, bevor ich das ganz loesche : wenn es eine Form ist und die Shell diese als CurrentObject
            // kennt, dann muss ich ihr das natuerlich ausreden
            if (pCurrent->ISA(FmFormData))
            {
                Reference< XForm >  xCurrentForm( static_cast< FmFormData* >( pCurrent )->GetFormIface() );
                if (pFormShell->GetImpl()->getCurForm() == xCurrentForm)    // die Shell kennt die zu loeschende Form ?
                    pFormShell->GetImpl()->setCurForm( NULL );              // -> wegnehmen ...
            }
            GetNavModel()->Remove(pCurrent, sal_True);
        }
        pFormModel->EndUndo();
    }

    //------------------------------------------------------------------------
    void NavigatorTree::CollectSelectionData(SELDATA_ITEMS sdiHow)
    {
        DBG_ASSERT(sdiHow != SDI_DIRTY, "NavigatorTree::CollectSelectionData : ever thought about your parameter ? DIRTY ?");
        if (sdiHow == m_sdiState)
            return;

        m_arrCurrentSelection.Remove((sal_uInt16)0, m_arrCurrentSelection.Count());
        m_nFormsSelected = m_nControlsSelected = m_nHiddenControls = 0;
        m_bRootSelected = sal_False;

        SvLBoxEntry* pSelectionLoop = FirstSelected();
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
                    m_arrCurrentSelection.Insert(pSelectionLoop);
                else
                {
                    SvLBoxEntry* pParentLoop = GetParent(pSelectionLoop);
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
                                m_arrCurrentSelection.Insert(pSelectionLoop);
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
                SvLBoxEntry* pParent = GetParent(pSelectionLoop);
                if (!pParent || !IsSelected(pParent) || IsFormEntry(pSelectionLoop))
                    m_arrCurrentSelection.Insert(pSelectionLoop);
            }
            else
                m_arrCurrentSelection.Insert(pSelectionLoop);


            pSelectionLoop = NextSelected(pSelectionLoop);
        }

        m_sdiState = sdiHow;
    }

    //------------------------------------------------------------------------
    void NavigatorTree::SynchronizeSelection(FmEntryDataArray& arredToSelect)
    {
        LockSelectionHandling();
        if (arredToSelect.Count() == 0)
        {
            SelectAll(sal_False);
        }
        else
        {
            // erst mal gleiche ich meine aktuelle Selektion mit der geforderten SelectList ab
            SvLBoxEntry* pSelection = FirstSelected();
            while (pSelection)
            {
                FmEntryData* pCurrent = (FmEntryData*)pSelection->GetUserData();
                if (pCurrent != NULL)
                {
                    sal_uInt16 nPosition;
                    if ( arredToSelect.Seek_Entry(pCurrent, &nPosition) )
                    {   // der Entry ist schon selektiert, steht aber auch in der SelectList -> er kann aus letzterer
                        // raus
                        arredToSelect.Remove(nPosition, 1);
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
            // zwei Moeglichkeiten : 1) ich gehe durch die SelectList, besorge mir zu jedem Eintrag meinen SvLBoxEntry
            // und selektiere diesen (waere irgendwie intuitiver ;)) 2) ich gehe durch alle meine SvLBoxEntries und selektiere
            // genau die, die ich in der SelectList finde
            // 1) braucht O(k*n) (k=Laenge der SelectList, n=Anzahl meiner Entries), plus den Fakt, dass FindEntry nicht den
            // Pointer auf die UserDaten vergleicht, sondern ein aufwendigeres IsEqualWithoutChilds durchfuehrt
            // 2) braucht O(n*log k), dupliziert aber etwas Code (naemlich den aus FindEntry)
            // da das hier eine relativ oft aufgerufenen Stelle sein koennte (bei jeder Aenderung in der Markierung in der View !),
            // nehme ich doch lieber letzteres
            SvLBoxEntry* pLoop = First();
            while( pLoop )
            {
                FmEntryData* pCurEntryData = (FmEntryData*)pLoop->GetUserData();
                sal_uInt16 nPosition;
                if ( arredToSelect.Seek_Entry(pCurEntryData, &nPosition) )
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
        // Shell und View
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if(!pFormShell) return;

        FmFormView* pFormView = pFormShell->GetFormView();
        if (!pFormView) return;

        GetNavModel()->BroadcastMarkedObjects(pFormView->GetMarkList());
    }

    //------------------------------------------------------------------------
    void NavigatorTree::SynchronizeMarkList()
    {
        // die Shell werde ich brauchen ...
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if (!pFormShell) return;

        CollectSelectionData(SDI_NORMALIZED_FORMARK);

        // Die View soll jetzt kein Notify bei einer Aenderung der MarkList rauslassen
        pFormShell->GetImpl()->EnableTrackProperties(sal_False);

        UnmarkAllViewObj();

        for (sal_uInt32 i=0; i<m_arrCurrentSelection.Count(); ++i)
        {
            SvLBoxEntry* pSelectionLoop = m_arrCurrentSelection.GetObject((USHORT)i);
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
        if ((m_arrCurrentSelection.Count() == 1) && (m_nFormsSelected == 1))
        {
            FmFormData* pSingleSelectionData = PTR_CAST( FmFormData, static_cast< FmEntryData* >( FirstSelected()->GetUserData() ) );
            DBG_ASSERT( pSingleSelectionData, "NavigatorTree::SynchronizeMarkList: invalid selected form!" );
            if ( pSingleSelectionData )
                pFormShell->GetImpl()->setCurForm( pSingleSelectionData->GetFormIface() );
        }
    }

    //------------------------------------------------------------------------
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

    //------------------------------------------------------------------------
    sal_Bool NavigatorTree::Select( SvLBoxEntry* pEntry, sal_Bool bSelect )
    {
        if (bSelect == IsSelected(pEntry))  // das passiert manchmal, ich glaube, die Basisklasse geht zu sehr auf Nummer sicher ;)
            return sal_True;

        return SvTreeListBox::Select(pEntry, bSelect );
    }

    //------------------------------------------------------------------------
    void NavigatorTree::UnmarkAllViewObj()
    {
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( !pFormShell )
            return;
        FmFormView* pFormView = pFormShell->GetFormView();
        pFormView->UnMarkAll();
    }

    //------------------------------------------------------------------------
    void NavigatorTree::MarkViewObj( FmFormData* pFormData, sal_Bool bMark, sal_Bool bDeep )
    {
        //////////////////////////////////////////////////////////////////////
        // Alle Controls der Form markieren
        FmEntryDataList* pChildList = pFormData->GetChildList();
        FmEntryData* pEntryData;
        FmControlData* pControlData;

        for( sal_uInt32 i=0; i<pChildList->Count(); i++ )
        {
            pEntryData = pChildList->GetObject(i);
            if( pEntryData->ISA(FmControlData) )
            {
                pControlData = (FmControlData*)pEntryData;
                MarkViewObj( pControlData, (i==pChildList->Count()-1), bMark );
            } else if (bDeep && (pEntryData->ISA(FmFormData)))
                MarkViewObj((FmFormData*)pEntryData, bMark, bDeep);
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTree::MarkViewObj( FmControlData* pControlData, sal_Bool bMarkHandles, sal_Bool bMark)
    {
        if( !pControlData ) return;
        FmFormShell* pFormShell = GetNavModel()->GetFormShell();
        if( !pFormShell ) return;

        //////////////////////////////////////////////////////////////////////
        // In der Page das entsprechende SdrObj finden und selektieren
        Reference< XFormComponent >  xFormComponent( pControlData->GetFormComponent());
        FmFormView*     pFormView       = pFormShell->GetFormView();
        SdrPageView*    pPageView       = pFormView->GetPageViewPvNum(0);
        SdrPage*        pPage           = pPageView->GetPage();

        SdrObjListIter  aIter( *pPage );
        while( aIter.IsMore() )
        {
            SdrObject* pObj = aIter.Next();

            //////////////////////////////////////////////////////////////////////
            // Es interessieren nur Uno-Objekte
            if( pObj->IsUnoObj() )
            {
                Reference< XInterface >  xControlModel(((SdrUnoObj*)pObj)->GetUnoControlModel());

                //////////////////////////////////////////////////////////////////////
                // Ist dieses Objekt ein XFormComponent?
                Reference< XFormComponent >  xFormViewControl(xControlModel, UNO_QUERY);
                if( !xFormViewControl.is() )
                    return;

                if (xFormViewControl == xFormComponent )
                {
                    // Objekt markieren
                    if (bMark != pFormView->IsObjMarked(pObj))
                        // der Writer mag das leider nicht, wenn schon markierte Objekte noch mal markiert werden ...
                        pFormView->MarkObj( pObj, pPageView, !bMark, sal_False );

                    // Markierung in allen Fenstern in den sichtbaren Bereich verschieben
                    if( bMarkHandles && bMark)
                    {
                        ::Rectangle aMarkRect( pFormView->GetAllMarkedRect());

                        for( sal_uInt16 i=0; i<pFormView->GetWinCount(); i++ )
                            pFormView->MakeVisible( aMarkRect, *(Window*)pFormView->GetWin(i) );
                    }
                }
            }
        }
    }

//............................................................................
}   // namespace svxform
//............................................................................


