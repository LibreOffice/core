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
#include <svx/fmglob.hxx>
#include "svx/svditer.hxx"
#include <svx/svdogrp.hxx>
#include <svx/svdpagv.hxx>

#include "fmprop.hrc"

#include "fmundo.hxx"
#include "fmhelp.hrc"
#include "fmexpl.hrc"
#include "fmexpl.hxx"
#include "svx/fmresids.hrc"
#include "fmshimp.hxx"
#include "fmobj.hxx"
#include <sfx2/objsh.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/container/XContainer.hpp>

//............................................................................
namespace svxform
{
//............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::sdb;

    //========================================================================
    // class OFormComponentObserver
    //========================================================================
    //------------------------------------------------------------------------
    OFormComponentObserver::OFormComponentObserver(NavigatorTreeModel* _pModel)
        :m_pNavModel(_pModel)
        ,m_nLocks(0)
        ,m_bCanUndo(sal_True)
    {
    }

    // XPropertyChangeListener
    //------------------------------------------------------------------------
    void SAL_CALL OFormComponentObserver::disposing(const EventObject& Source) throw( RuntimeException )
    {
        Remove( Source.Source );
    }

    //------------------------------------------------------------------------
    void SAL_CALL OFormComponentObserver::propertyChange(const PropertyChangeEvent& evt) throw(RuntimeException)
    {
        if( !m_pNavModel ) return;
        if( evt.PropertyName != FM_PROP_NAME ) return;

        Reference< XFormComponent >  xFormComponent(evt.Source, UNO_QUERY);
        Reference< XForm >  xForm(evt.Source, UNO_QUERY);

        FmEntryData* pEntryData( NULL );
        if( xForm.is() )
            pEntryData = m_pNavModel->FindData( xForm, m_pNavModel->GetRootList() );
        else if( xFormComponent.is() )
            pEntryData = m_pNavModel->FindData( xFormComponent, m_pNavModel->GetRootList() );

        if( pEntryData )
        {
            OUString aNewName =  ::comphelper::getString(evt.NewValue);
            pEntryData->SetText( aNewName );
            FmNavNameChangedHint aNameChangedHint( pEntryData, aNewName );
            m_pNavModel->Broadcast( aNameChangedHint );
        }
    }

    // XContainerListener
    //------------------------------------------------------------------------------
    void SAL_CALL OFormComponentObserver::elementInserted(const ContainerEvent& evt) throw(RuntimeException)
    {
        if (IsLocked() || !m_pNavModel)
            return;

        // keine Undoaction einfuegen
        m_bCanUndo = sal_False;

        Reference< XInterface > xTemp;
        evt.Element >>= xTemp;
        Insert(xTemp, ::comphelper::getINT32(evt.Accessor));

        m_bCanUndo = sal_True;
    }

    //------------------------------------------------------------------------------
    void OFormComponentObserver::Insert(const Reference< XInterface > & xIface, sal_Int32 nIndex)
    {
        Reference< XForm >  xForm(xIface, UNO_QUERY);
        if (xForm.is())
        {
            m_pNavModel->InsertForm(xForm, sal_uInt32(nIndex));
            Reference< XIndexContainer >  xContainer(xForm, UNO_QUERY);
            Reference< XInterface > xTemp;
            for (sal_Int32 i = 0; i < xContainer->getCount(); i++)
            {
                xContainer->getByIndex(i) >>= xTemp;
                Insert(xTemp, i);
            }
        }
        else
        {
            Reference< XFormComponent >  xFormComp(xIface, UNO_QUERY);
            if (xFormComp.is())
                m_pNavModel->InsertFormComponent(xFormComp, sal_uInt32(nIndex));
        }
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OFormComponentObserver::elementReplaced(const ContainerEvent& evt) throw(RuntimeException)
    {
        if (IsLocked() || !m_pNavModel)
            return;

        m_bCanUndo = sal_False;

        // EntryData loeschen
        Reference< XFormComponent >  xReplaced;
        evt.ReplacedElement >>= xReplaced;
        FmEntryData* pEntryData = m_pNavModel->FindData(xReplaced, m_pNavModel->GetRootList(), sal_True);
        if (pEntryData)
        {
            if (pEntryData->ISA(FmControlData))
            {
                Reference< XFormComponent >  xComp;
                evt.Element >>= xComp;
                DBG_ASSERT(xComp.is(), "OFormComponentObserver::elementReplaced : invalid argument !");
                    // an einer FmControlData sollte eine XFormComponent haengen
                m_pNavModel->ReplaceFormComponent(xReplaced, xComp);
            }
            else if (pEntryData->ISA(FmFormData))
            {
                OSL_FAIL("replacing forms not implemented yet !");
            }
        }

        m_bCanUndo = sal_True;
    }

    //------------------------------------------------------------------------------
    void OFormComponentObserver::Remove( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement )
    {
        if (IsLocked() || !m_pNavModel)
            return;

        m_bCanUndo = sal_False;

        //////////////////////////////////////////////////////////
        // EntryData loeschen
        FmEntryData* pEntryData = m_pNavModel->FindData( _rxElement, m_pNavModel->GetRootList(), sal_True );
        if (pEntryData)
            m_pNavModel->Remove(pEntryData);

        m_bCanUndo = sal_True;
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OFormComponentObserver::elementRemoved(const ContainerEvent& evt) throw(RuntimeException)
    {
        Reference< XInterface > xElement;
        evt.Element >>= xElement;
        Remove( xElement );
    }

    //========================================================================
    // class NavigatorTreeModel
    //========================================================================

    //------------------------------------------------------------------------
    NavigatorTreeModel::NavigatorTreeModel( const ImageList& _rNormalImages )
                    :m_pFormShell(NULL)
                    ,m_pFormPage(NULL)
                    ,m_pFormModel(NULL)
                    ,m_aNormalImages( _rNormalImages )
    {
        m_pPropChangeList = new OFormComponentObserver(this);
        m_pPropChangeList->acquire();
        m_pRootList = new FmEntryDataList();
    }

    //------------------------------------------------------------------------
    NavigatorTreeModel::~NavigatorTreeModel()
    {
        //////////////////////////////////////////////////////////////////////
        // Als Listener abmelden
        if( m_pFormShell)
        {
            FmFormModel* pFormModel = m_pFormShell->GetFormModel();
            if( pFormModel && IsListening(*pFormModel))
                EndListening( *pFormModel );

            if (IsListening(*m_pFormShell))
                EndListening(*m_pFormShell);
        }

        Clear();
        delete m_pRootList;
        m_pPropChangeList->ReleaseModel();
        m_pPropChangeList->release();
    }


    //------------------------------------------------------------------------
    void NavigatorTreeModel::SetModified( sal_Bool bMod )
    {
        if( !m_pFormShell ) return;
        SfxObjectShell* pObjShell = m_pFormShell->GetFormModel()->GetObjectShell();
        if( !pObjShell ) return;
        pObjShell->SetModified( bMod );
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::Clear()
    {
        Reference< css::form::XForms >  xForms( GetForms());
        if(xForms.is())
            xForms->removeContainerListener((XContainerListener*)m_pPropChangeList);

        //////////////////////////////////////////////////////////////////////
        // RootList loeschen
        GetRootList()->clear();

        //////////////////////////////////////////////////////////////////////
        // UI benachrichtigen
        FmNavClearedHint aClearedHint;
        Broadcast( aClearedHint );
    }

    //------------------------------------------------------------------------
    Reference< css::form::XForms >  NavigatorTreeModel::GetForms() const
    {
        if( !m_pFormShell || !m_pFormShell->GetCurPage())
            return NULL;
        else
            return m_pFormShell->GetCurPage()->GetForms();
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::Insert(FmEntryData* pEntry, sal_uLong nRelPos, sal_Bool bAlterModel)
    {
        if (IsListening(*m_pFormModel))
            EndListening(*m_pFormModel);

        m_pPropChangeList->Lock();
        FmFormData* pFolder     = (FmFormData*) pEntry->GetParent();
        Reference< XChild > xElement( pEntry->GetChildIFace() );
        if (bAlterModel)
        {
            OUString aStr;
            if (pEntry->ISA(FmFormData))
                aStr = SVX_RESSTR(RID_STR_FORM);
            else
                aStr = SVX_RESSTR(RID_STR_CONTROL);

            Reference< XIndexContainer >  xContainer;
            if (pFolder)
                xContainer = Reference< XIndexContainer > (pFolder->GetFormIface(), UNO_QUERY);
            else
                xContainer = Reference< XIndexContainer > (GetForms(), UNO_QUERY);

            bool bUndo = m_pFormModel->IsUndoEnabled();

            if( bUndo )
            {
                OUString aUndoStr(SVX_RESSTR(RID_STR_UNDO_CONTAINER_INSERT));
                aUndoStr = aUndoStr.replaceFirst("#", aStr);
                m_pFormModel->BegUndo(aUndoStr);
            }

            if (nRelPos >= (sal_uInt32)xContainer->getCount())
                nRelPos = (sal_uInt32)xContainer->getCount();

            // UndoAction
            if ( bUndo && m_pPropChangeList->CanUndo())
            {
                m_pFormModel->AddUndo(new FmUndoContainerAction(*m_pFormModel,
                                                         FmUndoContainerAction::Inserted,
                                                         xContainer,
                                                         xElement,
                                                         nRelPos));
            }

            // das Element muss den Typ haben, den der Container erwartet
            if (xContainer->getElementType() ==
                ::getCppuType((const Reference< XForm>*)0))

            {
                Reference< XForm >  xElementAsForm(xElement, UNO_QUERY);
                xContainer->insertByIndex(nRelPos, makeAny(xElementAsForm));
            }
            else if (xContainer->getElementType() ==
                ::getCppuType((const Reference< XFormComponent>*)0))

            {
                Reference< XFormComponent >  xElementAsComponent(xElement, UNO_QUERY);
                xContainer->insertByIndex(nRelPos, makeAny(xElementAsComponent));
            }
            else
            {
                OSL_FAIL("NavigatorTreeModel::Insert : the parent container needs an elementtype I don't know !");
            }

            if( bUndo )
                m_pFormModel->EndUndo();
        }

        //////////////////////////////////////////////////////////////////////
        // Als PropertyChangeListener anmelden
        Reference< XPropertySet >  xSet(xElement, UNO_QUERY);
        if( xSet.is() )
            xSet->addPropertyChangeListener( FM_PROP_NAME, m_pPropChangeList );

        //////////////////////////////////////////////////////////////////////
        // Daten aus Model entfernen
        if (pEntry->ISA(FmFormData))
        {
            Reference< XContainer >  xContainer(xElement, UNO_QUERY);
            if (xContainer.is())
                xContainer->addContainerListener((XContainerListener*)m_pPropChangeList);
        }

        if (pFolder)
            pFolder->GetChildList()->insert( pEntry, nRelPos );
        else
            GetRootList()->insert( pEntry, nRelPos );

        //////////////////////////////////////////////////////////////////////
        // UI benachrichtigen
        FmNavInsertedHint aInsertedHint( pEntry, nRelPos );
        Broadcast( aInsertedHint );

        m_pPropChangeList->UnLock();
        if (IsListening(*m_pFormModel))
            StartListening(*m_pFormModel);
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::Remove(FmEntryData* pEntry, sal_Bool bAlterModel)
    {
        //////////////////////////////////////////////////////////////////////
        // Form und Parent holen
        if (!pEntry || !m_pFormModel)
            return;

        if (IsListening(*m_pFormModel))
            EndListening(*m_pFormModel);

        const bool bUndo = m_pFormModel->IsUndoEnabled();

        m_pPropChangeList->Lock();
        FmFormData*     pFolder     = (FmFormData*) pEntry->GetParent();
        Reference< XChild > xElement ( pEntry->GetChildIFace() );
        if (bAlterModel)
        {
            OUString        aStr;
            if (pEntry->ISA(FmFormData))
                aStr = SVX_RESSTR(RID_STR_FORM);
            else
                aStr = SVX_RESSTR(RID_STR_CONTROL);

            if( bUndo )
            {
                OUString aUndoStr(SVX_RESSTR(RID_STR_UNDO_CONTAINER_REMOVE));
                aUndoStr = aUndoStr.replaceFirst("#", aStr);
                m_pFormModel->BegUndo(aUndoStr);
            }
        }

        // jetzt die eigentliche Entfernung der Daten aus dem Model
        if (pEntry->ISA(FmFormData))
            RemoveForm((FmFormData*)pEntry);
        else
            RemoveFormComponent((FmControlData*)pEntry);


        if (bAlterModel)
        {
            Reference< XIndexContainer >  xContainer(xElement->getParent(), UNO_QUERY);
            // aus dem Container entfernen
            sal_Int32 nContainerIndex = getElementPos(xContainer.get(), xElement);
            // UndoAction
            if (nContainerIndex >= 0)
            {
                if ( bUndo && m_pPropChangeList->CanUndo())
                {
                    m_pFormModel->AddUndo(new FmUndoContainerAction(*m_pFormModel,
                                                          FmUndoContainerAction::Removed,
                                                          xContainer,
                                                          xElement, nContainerIndex ));
                }
                else if( !m_pPropChangeList->CanUndo() )
                {
                    FmUndoContainerAction::DisposeElement( xElement );
                }

                xContainer->removeByIndex(nContainerIndex );
            }

            if( bUndo )
                m_pFormModel->EndUndo();
        }

        // beim Vater austragen
        if (pFolder)
            pFolder->GetChildList()->remove( pEntry );
        else
        {
            GetRootList()->remove( pEntry );
            //////////////////////////////////////////////////////////////////////
            // Wenn keine Form mehr in der Root, an der Shell CurForm zuruecksetzen
            if ( !GetRootList()->size() )
                m_pFormShell->GetImpl()->forgetCurrentForm();
        }

        //////////////////////////////////////////////////////////////////////
        // UI benachrichtigen
        FmNavRemovedHint aRemovedHint( pEntry );
        Broadcast( aRemovedHint );

        // Eintrag loeschen
        delete pEntry;

        m_pPropChangeList->UnLock();
        StartListening(*m_pFormModel);
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::RemoveForm(FmFormData* pFormData)
    {
        //////////////////////////////////////////////////////////////////////
        // Form und Parent holen
        if (!pFormData || !m_pFormModel)
            return;

        FmEntryDataList*    pChildList = pFormData->GetChildList();
        for ( size_t i = pChildList->size(); i > 0; )
        {
            FmEntryData* pEntryData = pChildList->at( --i );

            //////////////////////////////////////////////////////////////////////
            // Child ist Form -> rekursiver Aufruf
            if( pEntryData->ISA(FmFormData) )
                RemoveForm( (FmFormData*)pEntryData);
            else if( pEntryData->ISA(FmControlData) )
                RemoveFormComponent((FmControlData*) pEntryData);
        }

        //////////////////////////////////////////////////////////////////////
        // Als PropertyChangeListener abmelden
        Reference< XPropertySet > xSet( pFormData->GetPropertySet() );
        if ( xSet.is() )
            xSet->removePropertyChangeListener( FM_PROP_NAME, m_pPropChangeList );

        Reference< XContainer >  xContainer( pFormData->GetContainer() );
        if (xContainer.is())
            xContainer->removeContainerListener((XContainerListener*)m_pPropChangeList);
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::RemoveFormComponent(FmControlData* pControlData)
    {
        //////////////////////////////////////////////////////////////////////
        // Control und Parent holen
        if (!pControlData)
            return;

        //////////////////////////////////////////////////////////////////////
        // Als PropertyChangeListener abmelden
        Reference< XPropertySet >  xSet( pControlData->GetPropertySet() );
        if (xSet.is())
            xSet->removePropertyChangeListener( FM_PROP_NAME, m_pPropChangeList);
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::ClearBranch( FmFormData* pParentData )
    {
        //////////////////////////////////////////////////////////////////////
        // Alle Eintraege dieses Zweiges loeschen
        FmEntryDataList* pChildList = pParentData->GetChildList();

        for( size_t i = pChildList->size(); i > 0; )
        {
            FmEntryData* pChildData = pChildList->at( --i );
            if( pChildData->ISA(FmFormData) )
                ClearBranch( (FmFormData*)pChildData );

            pChildList->remove( pChildData );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::FillBranch( FmFormData* pFormData )
    {
        //////////////////////////////////////////////////////////////
        // Forms aus der Root einfuegen
        if( pFormData == NULL )
        {
            Reference< XIndexContainer >   xForms(GetForms(), UNO_QUERY);
            if (!xForms.is())
                return;

            Reference< XForm >     xSubForm;
            FmFormData* pSubFormData;
            for (sal_Int32 i=0; i<xForms->getCount(); ++i)
            {
                DBG_ASSERT( xForms->getByIndex(i).getValueType() == ::getCppuType((const Reference< XForm>*)NULL),
                    "NavigatorTreeModel::FillBranch : the root container should supply only elements of type XForm");

                xForms->getByIndex(i) >>= xSubForm;
                pSubFormData = new FmFormData( xSubForm, m_aNormalImages, pFormData );
                Insert( pSubFormData, LIST_APPEND );

                //////////////////////////////////////////////////////////////
                // Neuer Branch, wenn SubForm wiederum Subforms enthaelt
                FillBranch( pSubFormData );
            }
        }

        //////////////////////////////////////////////////////////////
        // Componenten einfuegen
        else
        {
            Reference< XIndexContainer >  xComponents( GetFormComponents(pFormData));
            if( !xComponents.is() ) return;

            Reference< XInterface >  xInterface;
            Reference< XPropertySet >  xSet;
            FmControlData* pNewControlData;
            FmFormData* pSubFormData;

            Reference< XFormComponent >  xCurrentComponent;
            for (sal_Int32 j=0; j<xComponents->getCount(); ++j)
            {
                xComponents->getByIndex(j) >>= xCurrentComponent;
                Reference< XForm >  xSubForm(xCurrentComponent, UNO_QUERY);

                if (xSubForm.is())
                {   // die aktuelle Component ist eine Form
                    pSubFormData = new FmFormData(xSubForm, m_aNormalImages, pFormData);
                    Insert(pSubFormData, LIST_APPEND);

                    //////////////////////////////////////////////////////////////
                    // Neuer Branch, wenn SubForm wiederum Subforms enthaelt
                    FillBranch(pSubFormData);
                }
                else
                {
                    pNewControlData = new FmControlData(xCurrentComponent, m_aNormalImages, pFormData);
                    Insert(pNewControlData, LIST_APPEND);
                }
            }
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::InsertForm(const Reference< XForm > & xForm, sal_uInt32 nRelPos)
    {
        FmFormData* pFormData = (FmFormData*)FindData( xForm, GetRootList() );
        if (pFormData)
            return;

        //////////////////////////////////////////////////////////
        // ParentData setzen
        Reference< XInterface >  xIFace( xForm->getParent());
        Reference< XForm >  xParentForm(xIFace, UNO_QUERY);
        FmFormData* pParentData = NULL;
        if (xParentForm.is())
            pParentData = (FmFormData*)FindData( xParentForm, GetRootList() );

        pFormData = new FmFormData( xForm, m_aNormalImages, pParentData );
        Insert( pFormData, nRelPos );
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::InsertFormComponent(const Reference< XFormComponent > & xComp, sal_uInt32 nRelPos)
    {
        //////////////////////////////////////////////////////////
        // ParentData setzen
        Reference< XInterface >  xIFace( xComp->getParent());
        Reference< XForm >  xForm(xIFace, UNO_QUERY);
        if (!xForm.is())
            return;

        FmFormData* pParentData = (FmFormData*)FindData( xForm, GetRootList() );
        if( !pParentData )
        {
            pParentData = new FmFormData( xForm, m_aNormalImages, NULL );
            Insert( pParentData, LIST_APPEND );
        }

        if (!FindData(xComp, pParentData->GetChildList(),sal_False))
        {
            //////////////////////////////////////////////////////////
            // Neue EntryData setzen
            FmEntryData* pNewEntryData = new FmControlData( xComp, m_aNormalImages, pParentData );

            //////////////////////////////////////////////////////////
            // Neue EntryData einfuegen
            Insert( pNewEntryData, nRelPos );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::ReplaceFormComponent(
        const Reference< XFormComponent > & xOld,
        const Reference< XFormComponent > & xNew
    )
    {
        FmEntryData* pData = FindData(xOld, GetRootList(), sal_True);
        DBG_ASSERT(pData && pData->ISA(FmControlData), "NavigatorTreeModel::ReplaceFormComponent : invalid argument !");
        ((FmControlData*)pData)->ModelReplaced( xNew, m_aNormalImages );

        FmNavModelReplacedHint aReplacedHint( pData );
        Broadcast( aReplacedHint );
    }

    //------------------------------------------------------------------------
    FmEntryData* NavigatorTreeModel::FindData(const Reference< XInterface > & xElement, FmEntryDataList* pDataList, sal_Bool bRecurs)
    {
        // normalize
        Reference< XInterface > xIFace( xElement, UNO_QUERY );

        for ( size_t i = 0; i < pDataList->size(); i++ )
        {
            FmEntryData* pEntryData = pDataList->at( i );
            if ( pEntryData->GetElement().get() == xIFace.get() )
                return pEntryData;
            else if (bRecurs)
            {
                pEntryData = FindData( xElement, pEntryData->GetChildList() );
                if (pEntryData)
                    return pEntryData;
            }
        }
        return NULL;
    }

    //------------------------------------------------------------------------
    FmEntryData* NavigatorTreeModel::FindData( const OUString& rText, FmFormData* pParentData, sal_Bool bRecurs )
    {
        FmEntryDataList* pDataList;
        if( !pParentData )
            pDataList = GetRootList();
        else
            pDataList = pParentData->GetChildList();

        OUString aEntryText;
        FmEntryData* pEntryData;
        FmEntryData* pChildData;

        for( size_t i = 0; i < pDataList->size(); i++ )
        {
            pEntryData = pDataList->at( i );
            aEntryText = pEntryData->GetText();

            if (rText == aEntryText)
                return pEntryData;

            if( bRecurs && pEntryData->ISA(FmFormData) )
            {
                pChildData = FindData( rText, (FmFormData*)pEntryData );
                if( pChildData )
                    return pChildData;
            }
        }

        return NULL;
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
    {
        if( rHint.ISA(SdrHint) )
        {
            SdrHint* pSdrHint = (SdrHint*)&rHint;
            switch( pSdrHint->GetKind() )
            {
                case HINT_OBJINSERTED:
                    InsertSdrObj(pSdrHint->GetObject());
                    break;
                case HINT_OBJREMOVED:
                    RemoveSdrObj(pSdrHint->GetObject());
                    break;
                default:
                    break;
            }
        }
        // hat sich die shell verabschiedet?
        else if ( rHint.ISA(SfxSimpleHint) && ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING)
            UpdateContent((FmFormShell*)NULL);

        // hat sich die Markierung der Controls veraendert ?
        else if (rHint.ISA(FmNavViewMarksChanged))
        {
            FmNavViewMarksChanged* pvmcHint = (FmNavViewMarksChanged*)&rHint;
            BroadcastMarkedObjects( pvmcHint->GetAffectedView()->GetMarkedObjectList() );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::InsertSdrObj( const SdrObject* pObj )
    {
        const FmFormObj* pFormObject = FmFormObj::GetFormObject( pObj );
        if ( pFormObject )
        {
            try
            {
                Reference< XFormComponent > xFormComponent( pFormObject->GetUnoControlModel(), UNO_QUERY_THROW );
                Reference< XIndexAccess > xContainer( xFormComponent->getParent(), UNO_QUERY_THROW );

                sal_Int32 nPos = getElementPos( xContainer, xFormComponent );
                InsertFormComponent( xFormComponent, nPos );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        else if ( pObj->IsGroupObject() )
        {
            SdrObjListIter aIter( *pObj->GetSubList() );
            while ( aIter.IsMore() )
                InsertSdrObj( aIter.Next() );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::RemoveSdrObj( const SdrObject* pObj )
    {
        const FmFormObj* pFormObject = FmFormObj::GetFormObject( pObj );
        if ( pFormObject )
        {
            try
            {
                Reference< XFormComponent > xFormComponent( pFormObject->GetUnoControlModel(), UNO_QUERY_THROW );
                FmEntryData* pEntryData = FindData( xFormComponent, GetRootList(), sal_True );
                if ( pEntryData )
                    Remove( pEntryData );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
        else if ( pObj->IsGroupObject() )
        {
            SdrObjListIter aIter( *pObj->GetSubList() );
            while ( aIter.IsMore() )
                RemoveSdrObj( aIter.Next() );
        }
    }

    sal_Bool NavigatorTreeModel::InsertFormComponent(FmNavRequestSelectHint& rHint, SdrObject* pObject)
    {
        if ( pObject->ISA(SdrObjGroup) )
        {   // rekursiv absteigen
            const SdrObjList *pChildren = ((SdrObjGroup*)pObject)->GetSubList();
            for ( sal_uInt16 i=0; i<pChildren->GetObjCount(); ++i )
            {
                SdrObject* pCurrent = pChildren->GetObj(i);
                if (!InsertFormComponent(rHint, pCurrent))
                    return sal_False;
            }
        }
        else
        {
            FmFormObj* pFormObject = FmFormObj::GetFormObject( pObject );
            if ( !pFormObject )
                return sal_False;

            try
            {
                Reference< XFormComponent > xFormViewControl( pFormObject->GetUnoControlModel(), UNO_QUERY_THROW );
                FmEntryData* pControlData = FindData( xFormViewControl, GetRootList() );
                if ( !pControlData )
                    return sal_False;

                rHint.AddItem( pControlData );
                return sal_True;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
                return sal_False;
            }
        }

        return sal_True;
    }

    void NavigatorTreeModel::BroadcastMarkedObjects(const SdrMarkList& mlMarked)
    {
        // gehen wir durch alle markierten Objekte und suchen wir die raus, mit denen ich was anfangen kann
        FmNavRequestSelectHint rshRequestSelection;
        sal_Bool bIsMixedSelection = sal_False;

        for (sal_uLong i=0; (i<mlMarked.GetMarkCount()) && !bIsMixedSelection; i++)
        {
            SdrObject* pobjCurrent = mlMarked.GetMark(i)->GetMarkedSdrObj();
            bIsMixedSelection |= !InsertFormComponent(rshRequestSelection, pobjCurrent);
                // bei einem Nicht-Form-Control liefert InsertFormComponent sal_False !
        }

        rshRequestSelection.SetMixedSelection(bIsMixedSelection);
        if (bIsMixedSelection)
            rshRequestSelection.ClearItems();

        Broadcast(rshRequestSelection);
            // eine leere Liste interpretiert der NavigatorTree so, dass er seine Selektion komplett rausnimmt
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::UpdateContent( const Reference< css::form::XForms > & xForms )
    {
        //////////////////////////////////////////////////////////////////////
        // Model von der Root aufwaerts neu fuellen
        Clear();
        if (xForms.is())
        {
            xForms->addContainerListener((XContainerListener*)m_pPropChangeList);

            FillBranch(NULL);

            // jetzt in meinem Tree genau die das in meiner View markierte Control selektieren
            // (bzw alle solchen), falls es eines gibt ...
            if(!m_pFormShell) return;       // keine Shell -> wech

            FmFormView* pFormView = m_pFormShell->GetFormView();
            DBG_ASSERT(pFormView != NULL, "NavigatorTreeModel::UpdateContent : keine FormView");
            BroadcastMarkedObjects(pFormView->GetMarkedObjectList());
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::UpdateContent( FmFormShell* pShell )
    {
        //////////////////////////////////////////////////////////////////////
        // Wenn Shell sich nicht veraendert hat, nichts machen
        FmFormPage* pNewPage = pShell ? pShell->GetCurPage() : NULL;
        if ((pShell == m_pFormShell) && (m_pFormPage == pNewPage))
            return;

        //////////////////////////////////////////////////////////////////////
        // Als Listener abmelden
        if( m_pFormShell )
        {
            if (m_pFormModel)
                EndListening( *m_pFormModel );
            m_pFormModel = NULL;
            EndListening( *m_pFormShell );
            Clear();
        }

        //////////////////////////////////////////////////////////////////////
        // Vollupdate
        m_pFormShell = pShell;
        if (m_pFormShell)
        {
            m_pFormPage = pNewPage;
            UpdateContent(m_pFormPage->GetForms());
        } else
            m_pFormPage = NULL;

        //////////////////////////////////////////////////////////////////////
        // Als Listener neu anmelden
        if( m_pFormShell )
        {
            StartListening( *m_pFormShell );
            m_pFormModel = m_pFormShell->GetFormModel();
            if( m_pFormModel )
                StartListening( *m_pFormModel );
        }
    }

    //------------------------------------------------------------------------
    Reference< XIndexContainer >  NavigatorTreeModel::GetFormComponents( FmFormData* pFormData )
    {
        //////////////////////////////////////////////////////////////////////
        // Von der Form Components holen
        if (pFormData)
            return Reference< XIndexContainer > (pFormData->GetFormIface(), UNO_QUERY);

        return Reference< XIndexContainer > ();
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTreeModel::Rename( FmEntryData* pEntryData, const OUString& rNewText )
    {
        //////////////////////////////////////////////////////////////////////
        // Wenn Name schon vorhanden, Fehlermeldung
        pEntryData->SetText( rNewText );

        //////////////////////////////////////////////////////////////////////
        // PropertySet besorgen
        Reference< XFormComponent >  xFormComponent;

        if( pEntryData->ISA(FmFormData) )
        {
            FmFormData* pFormData = (FmFormData*)pEntryData;
            Reference< XForm >  xForm( pFormData->GetFormIface());
            xFormComponent = xForm;
        }

        if( pEntryData->ISA(FmControlData) )
        {
            FmControlData* pControlData = (FmControlData*)pEntryData;
            xFormComponent = pControlData->GetFormComponent();
        }

        if( !xFormComponent.is() ) return sal_False;
        Reference< XPropertySet >  xSet(xFormComponent, UNO_QUERY);
        if( !xSet.is() ) return sal_False;

        //////////////////////////////////////////////////////////////////////
        // Namen setzen
        xSet->setPropertyValue( FM_PROP_NAME, makeAny(rNewText) );

        return sal_True;
    }

    //------------------------------------------------------------------
    SdrObject* NavigatorTreeModel::Search(SdrObjListIter& rIter, const Reference< XFormComponent > & xComp)
    {
        while (rIter.IsMore())
        {
            SdrObject* pObj = rIter.Next();
            FmFormObj* pFormObject = FmFormObj::GetFormObject( pObj );
            if ( pFormObject )
            {
                Reference< XFormComponent > xFormViewControl( pFormObject->GetUnoControlModel(), UNO_QUERY );
                if ( xFormViewControl == xComp )
                    return pObj;
            }
            else if ( pObj->IsGroupObject() )
            {
                SdrObjListIter aIter( *pObj->GetSubList() );
                pObj = Search( aIter, xComp );
                if ( pObj )
                    return pObj;
            }
        }
        return NULL;
    }

//............................................................................
}   // namespace svxform
//............................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
