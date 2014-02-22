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


namespace svxform
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::sdb;

    
    
    
    
    OFormComponentObserver::OFormComponentObserver(NavigatorTreeModel* _pModel)
        :m_pNavModel(_pModel)
        ,m_nLocks(0)
        ,m_bCanUndo(sal_True)
    {
    }

    
    
    void SAL_CALL OFormComponentObserver::disposing(const EventObject& Source) throw( RuntimeException )
    {
        Remove( Source.Source );
    }

    
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

    
    
    void SAL_CALL OFormComponentObserver::elementInserted(const ContainerEvent& evt) throw(RuntimeException)
    {
        if (IsLocked() || !m_pNavModel)
            return;

        
        m_bCanUndo = sal_False;

        Reference< XInterface > xTemp;
        evt.Element >>= xTemp;
        Insert(xTemp, ::comphelper::getINT32(evt.Accessor));

        m_bCanUndo = sal_True;
    }

    
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

    
    void SAL_CALL OFormComponentObserver::elementReplaced(const ContainerEvent& evt) throw(RuntimeException)
    {
        if (IsLocked() || !m_pNavModel)
            return;

        m_bCanUndo = sal_False;

        
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
                    
                m_pNavModel->ReplaceFormComponent(xReplaced, xComp);
            }
            else if (pEntryData->ISA(FmFormData))
            {
                OSL_FAIL("replacing forms not implemented yet !");
            }
        }

        m_bCanUndo = sal_True;
    }

    
    void OFormComponentObserver::Remove( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement )
    {
        if (IsLocked() || !m_pNavModel)
            return;

        m_bCanUndo = sal_False;

        
        
        FmEntryData* pEntryData = m_pNavModel->FindData( _rxElement, m_pNavModel->GetRootList(), sal_True );
        if (pEntryData)
            m_pNavModel->Remove(pEntryData);

        m_bCanUndo = sal_True;
    }

    
    void SAL_CALL OFormComponentObserver::elementRemoved(const ContainerEvent& evt) throw(RuntimeException)
    {
        Reference< XInterface > xElement;
        evt.Element >>= xElement;
        Remove( xElement );
    }

    
    
    

    
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

    
    NavigatorTreeModel::~NavigatorTreeModel()
    {
        
        
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


    
    void NavigatorTreeModel::SetModified( sal_Bool bMod )
    {
        if( !m_pFormShell ) return;
        SfxObjectShell* pObjShell = m_pFormShell->GetFormModel()->GetObjectShell();
        if( !pObjShell ) return;
        pObjShell->SetModified( bMod );
    }

    
    void NavigatorTreeModel::Clear()
    {
        Reference< css::form::XForms >  xForms( GetForms());
        if(xForms.is())
            xForms->removeContainerListener((XContainerListener*)m_pPropChangeList);

        
        
        GetRootList()->clear();

        
        
        FmNavClearedHint aClearedHint;
        Broadcast( aClearedHint );
    }

    
    Reference< css::form::XForms >  NavigatorTreeModel::GetForms() const
    {
        if( !m_pFormShell || !m_pFormShell->GetCurPage())
            return NULL;
        else
            return m_pFormShell->GetCurPage()->GetForms();
    }

    
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

            
            if ( bUndo && m_pPropChangeList->CanUndo())
            {
                m_pFormModel->AddUndo(new FmUndoContainerAction(*m_pFormModel,
                                                         FmUndoContainerAction::Inserted,
                                                         xContainer,
                                                         xElement,
                                                         nRelPos));
            }

            
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

        
        
        Reference< XPropertySet >  xSet(xElement, UNO_QUERY);
        if( xSet.is() )
            xSet->addPropertyChangeListener( FM_PROP_NAME, m_pPropChangeList );

        
        
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

        
        
        FmNavInsertedHint aInsertedHint( pEntry, nRelPos );
        Broadcast( aInsertedHint );

        m_pPropChangeList->UnLock();
        if (IsListening(*m_pFormModel))
            StartListening(*m_pFormModel);
    }

    
    void NavigatorTreeModel::Remove(FmEntryData* pEntry, sal_Bool bAlterModel)
    {
        
        
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

        
        if (pEntry->ISA(FmFormData))
            RemoveForm((FmFormData*)pEntry);
        else
            RemoveFormComponent((FmControlData*)pEntry);


        if (bAlterModel)
        {
            Reference< XIndexContainer >  xContainer(xElement->getParent(), UNO_QUERY);
            
            sal_Int32 nContainerIndex = getElementPos(xContainer.get(), xElement);
            
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

        
        if (pFolder)
            pFolder->GetChildList()->remove( pEntry );
        else
        {
            GetRootList()->remove( pEntry );
            
            
            if ( !GetRootList()->size() )
                m_pFormShell->GetImpl()->forgetCurrentForm();
        }

        
        
        FmNavRemovedHint aRemovedHint( pEntry );
        Broadcast( aRemovedHint );

        
        delete pEntry;

        m_pPropChangeList->UnLock();
        StartListening(*m_pFormModel);
    }

    
    void NavigatorTreeModel::RemoveForm(FmFormData* pFormData)
    {
        
        
        if (!pFormData || !m_pFormModel)
            return;

        FmEntryDataList*    pChildList = pFormData->GetChildList();
        for ( size_t i = pChildList->size(); i > 0; )
        {
            FmEntryData* pEntryData = pChildList->at( --i );

            
            
            if( pEntryData->ISA(FmFormData) )
                RemoveForm( (FmFormData*)pEntryData);
            else if( pEntryData->ISA(FmControlData) )
                RemoveFormComponent((FmControlData*) pEntryData);
        }

        
        
        Reference< XPropertySet > xSet( pFormData->GetPropertySet() );
        if ( xSet.is() )
            xSet->removePropertyChangeListener( FM_PROP_NAME, m_pPropChangeList );

        Reference< XContainer >  xContainer( pFormData->GetContainer() );
        if (xContainer.is())
            xContainer->removeContainerListener((XContainerListener*)m_pPropChangeList);
    }

    
    void NavigatorTreeModel::RemoveFormComponent(FmControlData* pControlData)
    {
        
        
        if (!pControlData)
            return;

        
        
        Reference< XPropertySet >  xSet( pControlData->GetPropertySet() );
        if (xSet.is())
            xSet->removePropertyChangeListener( FM_PROP_NAME, m_pPropChangeList);
    }

    
    void NavigatorTreeModel::ClearBranch( FmFormData* pParentData )
    {
        
        
        FmEntryDataList* pChildList = pParentData->GetChildList();

        for( size_t i = pChildList->size(); i > 0; )
        {
            FmEntryData* pChildData = pChildList->at( --i );
            if( pChildData->ISA(FmFormData) )
                ClearBranch( (FmFormData*)pChildData );

            pChildList->remove( pChildData );
        }
    }

    
    void NavigatorTreeModel::FillBranch( FmFormData* pFormData )
    {
        
        
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

                
                
                FillBranch( pSubFormData );
            }
        }

        
        
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
                {   
                    pSubFormData = new FmFormData(xSubForm, m_aNormalImages, pFormData);
                    Insert(pSubFormData, LIST_APPEND);

                    
                    
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

    
    void NavigatorTreeModel::InsertForm(const Reference< XForm > & xForm, sal_uInt32 nRelPos)
    {
        FmFormData* pFormData = (FmFormData*)FindData( xForm, GetRootList() );
        if (pFormData)
            return;

        
        
        Reference< XInterface >  xIFace( xForm->getParent());
        Reference< XForm >  xParentForm(xIFace, UNO_QUERY);
        FmFormData* pParentData = NULL;
        if (xParentForm.is())
            pParentData = (FmFormData*)FindData( xParentForm, GetRootList() );

        pFormData = new FmFormData( xForm, m_aNormalImages, pParentData );
        Insert( pFormData, nRelPos );
    }

    
    void NavigatorTreeModel::InsertFormComponent(const Reference< XFormComponent > & xComp, sal_uInt32 nRelPos)
    {
        
        
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
            
            
            FmEntryData* pNewEntryData = new FmControlData( xComp, m_aNormalImages, pParentData );

            
            
            Insert( pNewEntryData, nRelPos );
        }
    }

    
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

    
    FmEntryData* NavigatorTreeModel::FindData(const Reference< XInterface > & xElement, FmEntryDataList* pDataList, sal_Bool bRecurs)
    {
        
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
        
        else if ( rHint.ISA(SfxSimpleHint) && ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING)
            UpdateContent((FmFormShell*)NULL);

        
        else if (rHint.ISA(FmNavViewMarksChanged))
        {
            FmNavViewMarksChanged* pvmcHint = (FmNavViewMarksChanged*)&rHint;
            BroadcastMarkedObjects( pvmcHint->GetAffectedView()->GetMarkedObjectList() );
        }
    }

    
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
        {   
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
        
        FmNavRequestSelectHint rshRequestSelection;
        bool bIsMixedSelection = false;

        for (sal_uLong i=0; (i<mlMarked.GetMarkCount()) && !bIsMixedSelection; i++)
        {
            SdrObject* pobjCurrent = mlMarked.GetMark(i)->GetMarkedSdrObj();
            bIsMixedSelection |= !InsertFormComponent(rshRequestSelection, pobjCurrent);
                
        }

        rshRequestSelection.SetMixedSelection(bIsMixedSelection);
        if (bIsMixedSelection)
            rshRequestSelection.ClearItems();

        Broadcast(rshRequestSelection);
            
    }

    
    void NavigatorTreeModel::UpdateContent( const Reference< css::form::XForms > & xForms )
    {
        
        
        Clear();
        if (xForms.is())
        {
            xForms->addContainerListener((XContainerListener*)m_pPropChangeList);

            FillBranch(NULL);

            
            
            if(!m_pFormShell) return;       

            FmFormView* pFormView = m_pFormShell->GetFormView();
            DBG_ASSERT(pFormView != NULL, "NavigatorTreeModel::UpdateContent : keine FormView");
            BroadcastMarkedObjects(pFormView->GetMarkedObjectList());
        }
    }

    
    void NavigatorTreeModel::UpdateContent( FmFormShell* pShell )
    {
        
        
        FmFormPage* pNewPage = pShell ? pShell->GetCurPage() : NULL;
        if ((pShell == m_pFormShell) && (m_pFormPage == pNewPage))
            return;

        
        
        if( m_pFormShell )
        {
            if (m_pFormModel)
                EndListening( *m_pFormModel );
            m_pFormModel = NULL;
            EndListening( *m_pFormShell );
            Clear();
        }

        
        
        m_pFormShell = pShell;
        if (m_pFormShell)
        {
            m_pFormPage = pNewPage;
            UpdateContent(m_pFormPage->GetForms());
        } else
            m_pFormPage = NULL;

        
        
        if( m_pFormShell )
        {
            StartListening( *m_pFormShell );
            m_pFormModel = m_pFormShell->GetFormModel();
            if( m_pFormModel )
                StartListening( *m_pFormModel );
        }
    }

    
    Reference< XIndexContainer >  NavigatorTreeModel::GetFormComponents( FmFormData* pFormData )
    {
        
        
        if (pFormData)
            return Reference< XIndexContainer > (pFormData->GetFormIface(), UNO_QUERY);

        return Reference< XIndexContainer > ();
    }

    
    sal_Bool NavigatorTreeModel::Rename( FmEntryData* pEntryData, const OUString& rNewText )
    {
        
        
        pEntryData->SetText( rNewText );

        
        
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

        
        
        xSet->setPropertyValue( FM_PROP_NAME, makeAny(rNewText) );

        return sal_True;
    }

    
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


}   



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
