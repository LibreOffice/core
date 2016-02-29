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
        ,m_bCanUndo(true)
    {
    }

    // XPropertyChangeListener

    void SAL_CALL OFormComponentObserver::disposing(const EventObject& Source) throw( RuntimeException, std::exception )
    {
        Remove( Source.Source );
    }


    void SAL_CALL OFormComponentObserver::propertyChange(const PropertyChangeEvent& evt) throw(RuntimeException, std::exception)
    {
        if( !m_pNavModel ) return;
        if( evt.PropertyName != FM_PROP_NAME ) return;

        Reference< XFormComponent >  xFormComponent(evt.Source, UNO_QUERY);
        Reference< XForm >  xForm(evt.Source, UNO_QUERY);

        FmEntryData* pEntryData( nullptr );
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

    void SAL_CALL OFormComponentObserver::elementInserted(const ContainerEvent& evt) throw(RuntimeException, std::exception)
    {
        if (IsLocked() || !m_pNavModel)
            return;

        // insert no Undoaction
        m_bCanUndo = false;

        Reference< XInterface > xTemp;
        evt.Element >>= xTemp;
        Insert(xTemp, ::comphelper::getINT32(evt.Accessor));

        m_bCanUndo = true;
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


    void SAL_CALL OFormComponentObserver::elementReplaced(const ContainerEvent& evt) throw(RuntimeException, std::exception)
    {
        if (IsLocked() || !m_pNavModel)
            return;

        m_bCanUndo = false;

        // delete EntryData
        Reference< XFormComponent >  xReplaced;
        evt.ReplacedElement >>= xReplaced;
        FmEntryData* pEntryData = m_pNavModel->FindData(xReplaced, m_pNavModel->GetRootList());
        if (pEntryData)
        {
            if (dynamic_cast<const FmControlData*>( pEntryData) !=  nullptr)
            {
                Reference< XFormComponent >  xComp;
                evt.Element >>= xComp;
                DBG_ASSERT(xComp.is(), "OFormComponentObserver::elementReplaced : invalid argument !");
                    // FmControlData should be coupled with XFormComponent
                m_pNavModel->ReplaceFormComponent(xReplaced, xComp);
            }
            else if (dynamic_cast<const FmFormData*>( pEntryData) !=  nullptr)
            {
                OSL_FAIL("replacing forms not implemented yet !");
            }
        }

        m_bCanUndo = true;
    }


    void OFormComponentObserver::Remove( const css::uno::Reference< css::uno::XInterface >& _rxElement )
    {
        if (IsLocked() || !m_pNavModel)
            return;

        m_bCanUndo = false;


        // delete EntryData
        FmEntryData* pEntryData = m_pNavModel->FindData( _rxElement, m_pNavModel->GetRootList() );
        if (pEntryData)
            m_pNavModel->Remove(pEntryData);

        m_bCanUndo = true;
    }


    void SAL_CALL OFormComponentObserver::elementRemoved(const ContainerEvent& evt) throw(RuntimeException, std::exception)
    {
        Reference< XInterface > xElement;
        evt.Element >>= xElement;
        Remove( xElement );
    }

    NavigatorTreeModel::NavigatorTreeModel( const ImageList& _rNormalImages )
                    :m_pFormShell(nullptr)
                    ,m_pFormPage(nullptr)
                    ,m_pFormModel(nullptr)
                    ,m_aNormalImages( _rNormalImages )
    {
        m_pPropChangeList = new OFormComponentObserver(this);
        m_pPropChangeList->acquire();
        m_pRootList = new FmEntryDataList();
    }


    NavigatorTreeModel::~NavigatorTreeModel()
    {

        // unregister Listener
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


    void NavigatorTreeModel::SetModified()
    {
        if( !m_pFormShell ) return;
        SfxObjectShell* pObjShell = m_pFormShell->GetFormModel()->GetObjectShell();
        if( !pObjShell ) return;
        pObjShell->SetModified();
    }


    void NavigatorTreeModel::Clear()
    {
        Reference< css::form::XForms >  xForms( GetForms());
        if(xForms.is())
            xForms->removeContainerListener(static_cast<XContainerListener*>(m_pPropChangeList));


        // delete RootList
        GetRootList()->clear();


        // notify UI
        FmNavClearedHint aClearedHint;
        Broadcast( aClearedHint );
    }


    Reference< css::form::XForms >  NavigatorTreeModel::GetForms() const
    {
        if( !m_pFormShell || !m_pFormShell->GetCurPage())
            return nullptr;
        else
            return m_pFormShell->GetCurPage()->GetForms();
    }


    void NavigatorTreeModel::Insert(FmEntryData* pEntry, sal_uLong nRelPos, bool bAlterModel)
    {
        if (IsListening(*m_pFormModel))
            EndListening(*m_pFormModel);

        m_pPropChangeList->Lock();
        FmFormData* pFolder     = static_cast<FmFormData*>( pEntry->GetParent() );
        Reference< XChild > xElement( pEntry->GetChildIFace() );
        if (bAlterModel)
        {
            OUString aStr;
            if (dynamic_cast<const FmFormData*>( pEntry) !=  nullptr)
                aStr = SVX_RESSTR(RID_STR_FORM);
            else
                aStr = SVX_RESSTR(RID_STR_CONTROL);

            Reference< XIndexContainer >  xContainer;
            if (pFolder)
                xContainer.set(pFolder->GetFormIface(), UNO_QUERY);
            else
                xContainer.set(GetForms(), UNO_QUERY);

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

            // Element has to be of the expected type by the container
            if (xContainer->getElementType() ==
                cppu::UnoType<XForm>::get())

            {
                Reference< XForm >  xElementAsForm(xElement, UNO_QUERY);
                xContainer->insertByIndex(nRelPos, makeAny(xElementAsForm));
            }
            else if (xContainer->getElementType() ==
                cppu::UnoType<XFormComponent>::get())

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

        // register as PropertyChangeListener
        Reference< XPropertySet >  xSet(xElement, UNO_QUERY);
        if( xSet.is() )
            xSet->addPropertyChangeListener( FM_PROP_NAME, m_pPropChangeList );


        // Remove data from model
        if (dynamic_cast<const FmFormData*>( pEntry) !=  nullptr)
        {
            Reference< XContainer >  xContainer(xElement, UNO_QUERY);
            if (xContainer.is())
                xContainer->addContainerListener(static_cast<XContainerListener*>(m_pPropChangeList));
        }

        if (pFolder)
            pFolder->GetChildList()->insert( pEntry, nRelPos );
        else
            GetRootList()->insert( pEntry, nRelPos );


        // notify UI
        FmNavInsertedHint aInsertedHint( pEntry, nRelPos );
        Broadcast( aInsertedHint );

        m_pPropChangeList->UnLock();
        if (IsListening(*m_pFormModel))
            StartListening(*m_pFormModel);
    }


    void NavigatorTreeModel::Remove(FmEntryData* pEntry, bool bAlterModel)
    {

        // get form and parent
        if (!pEntry || !m_pFormModel)
            return;

        if (IsListening(*m_pFormModel))
            EndListening(*m_pFormModel);

        const bool bUndo = m_pFormModel->IsUndoEnabled();

        m_pPropChangeList->Lock();
        FmFormData*     pFolder     = static_cast<FmFormData*>( pEntry->GetParent() );
        Reference< XChild > xElement ( pEntry->GetChildIFace() );
        if (bAlterModel)
        {
            OUString        aStr;
            if (dynamic_cast<const FmFormData*>( pEntry) !=  nullptr)
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

        // now real deletion of data form model
        if (dynamic_cast<const FmFormData*>( pEntry) !=  nullptr)
            RemoveForm(static_cast<FmFormData*>(pEntry));
        else
            RemoveFormComponent(static_cast<FmControlData*>(pEntry));


        if (bAlterModel)
        {
            Reference< XIndexContainer >  xContainer(xElement->getParent(), UNO_QUERY);
            // remove from Container
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

        // remove from parent
        if (pFolder)
            pFolder->GetChildList()->remove( pEntry );
        else
        {
            GetRootList()->remove( pEntry );

            // If root has no more form, reset CurForm at shell
            if ( !GetRootList()->size() )
                m_pFormShell->GetImpl()->forgetCurrentForm();
        }


        // notify UI
        FmNavRemovedHint aRemovedHint( pEntry );
        Broadcast( aRemovedHint );

        // delete entry
        delete pEntry;

        m_pPropChangeList->UnLock();
        StartListening(*m_pFormModel);
    }


    void NavigatorTreeModel::RemoveForm(FmFormData* pFormData)
    {

        // get form and parent
        if (!pFormData || !m_pFormModel)
            return;

        FmEntryDataList*    pChildList = pFormData->GetChildList();
        for ( size_t i = pChildList->size(); i > 0; )
        {
            FmEntryData* pEntryData = pChildList->at( --i );


            // Child is form -> recursive call
            if( dynamic_cast<const FmFormData*>( pEntryData) !=  nullptr )
                RemoveForm( static_cast<FmFormData*>(pEntryData));
            else if( dynamic_cast<const FmControlData*>( pEntryData) !=  nullptr )
                RemoveFormComponent(static_cast<FmControlData*>(pEntryData));
        }


        // unregister as PropertyChangeListener
        Reference< XPropertySet > xSet( pFormData->GetPropertySet() );
        if ( xSet.is() )
            xSet->removePropertyChangeListener( FM_PROP_NAME, m_pPropChangeList );

        Reference< XContainer >  xContainer( pFormData->GetContainer() );
        if (xContainer.is())
            xContainer->removeContainerListener(static_cast<XContainerListener*>(m_pPropChangeList));
    }


    void NavigatorTreeModel::RemoveFormComponent(FmControlData* pControlData)
    {

        // get control and parent
        if (!pControlData)
            return;


        // unregister as PropertyChangeListener
        Reference< XPropertySet >  xSet( pControlData->GetPropertySet() );
        if (xSet.is())
            xSet->removePropertyChangeListener( FM_PROP_NAME, m_pPropChangeList);
    }


    void NavigatorTreeModel::ClearBranch( FmFormData* pParentData )
    {

        // delete all entries of this branch
        FmEntryDataList* pChildList = pParentData->GetChildList();

        for( size_t i = pChildList->size(); i > 0; )
        {
            FmEntryData* pChildData = pChildList->at( --i );
            if( dynamic_cast<const FmFormData*>( pChildData) !=  nullptr )
                ClearBranch( static_cast<FmFormData*>(pChildData) );

            pChildList->remove( pChildData );
        }
    }


    void NavigatorTreeModel::FillBranch( FmFormData* pFormData )
    {

        // insert forms from root
        if( pFormData == nullptr )
        {
            Reference< XIndexContainer >   xForms(GetForms(), UNO_QUERY);
            if (!xForms.is())
                return;

            Reference< XForm >     xSubForm;
            for (sal_Int32 i=0; i<xForms->getCount(); ++i)
            {
                DBG_ASSERT( xForms->getByIndex(i).getValueType() == cppu::UnoType<XForm>::get(),
                    "NavigatorTreeModel::FillBranch : the root container should supply only elements of type XForm");

                xForms->getByIndex(i) >>= xSubForm;
                FmFormData* pSubFormData = new FmFormData( xSubForm, m_aNormalImages, pFormData );
                Insert( pSubFormData );


                // new branch, if SubForm contains Subforms itself
                FillBranch( pSubFormData );
            }
        }


        // insert components
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
                {   // actual component is a form
                    pSubFormData = new FmFormData(xSubForm, m_aNormalImages, pFormData);
                    Insert(pSubFormData);


                    // new branch, if SubForm contains Subforms itself
                    FillBranch(pSubFormData);
                }
                else
                {
                    pNewControlData = new FmControlData(xCurrentComponent, m_aNormalImages, pFormData);
                    Insert(pNewControlData);
                }
            }
        }
    }


    void NavigatorTreeModel::InsertForm(const Reference< XForm > & xForm, sal_uInt32 nRelPos)
    {
        FmFormData* pFormData = static_cast<FmFormData*>(FindData( xForm, GetRootList() ));
        if (pFormData)
            return;


        // set ParentData
        Reference< XInterface >  xIFace( xForm->getParent());
        Reference< XForm >  xParentForm(xIFace, UNO_QUERY);
        FmFormData* pParentData = nullptr;
        if (xParentForm.is())
            pParentData = static_cast<FmFormData*>(FindData( xParentForm, GetRootList() ));

        pFormData = new FmFormData( xForm, m_aNormalImages, pParentData );
        Insert( pFormData, nRelPos );
    }


    void NavigatorTreeModel::InsertFormComponent(const Reference< XFormComponent > & xComp, sal_uInt32 nRelPos)
    {

        // set ParentData
        Reference< XInterface >  xIFace( xComp->getParent());
        Reference< XForm >  xForm(xIFace, UNO_QUERY);
        if (!xForm.is())
            return;

        FmFormData* pParentData = static_cast<FmFormData*>(FindData( xForm, GetRootList() ));
        if( !pParentData )
        {
            pParentData = new FmFormData( xForm, m_aNormalImages, nullptr );
            Insert( pParentData );
        }

        if (!FindData(xComp, pParentData->GetChildList(),false))
        {

            // set new EntryData
            FmEntryData* pNewEntryData = new FmControlData( xComp, m_aNormalImages, pParentData );


            // insert new EntryData
            Insert( pNewEntryData, nRelPos );
        }
    }


    void NavigatorTreeModel::ReplaceFormComponent(
        const Reference< XFormComponent > & xOld,
        const Reference< XFormComponent > & xNew
    )
    {
        FmEntryData* pData = FindData(xOld, GetRootList());
        assert(pData && dynamic_cast<const FmControlData*>( pData) !=  nullptr); //NavigatorTreeModel::ReplaceFormComponent : invalid argument
        if (!pData || dynamic_cast<const FmControlData*>( pData) ==  nullptr)
            return;
        static_cast<FmControlData*>(pData)->ModelReplaced( xNew, m_aNormalImages );

        FmNavModelReplacedHint aReplacedHint( pData );
        Broadcast( aReplacedHint );
    }


    FmEntryData* NavigatorTreeModel::FindData(const Reference< XInterface > & xElement, FmEntryDataList* pDataList, bool bRecurs)
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
        return nullptr;
    }


    FmEntryData* NavigatorTreeModel::FindData( const OUString& rText, FmFormData* pParentData, bool bRecurs )
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

            if( bRecurs && dynamic_cast<const FmFormData*>( pEntryData) !=  nullptr )
            {
                pChildData = FindData( rText, static_cast<FmFormData*>(pEntryData) );
                if( pChildData )
                    return pChildData;
            }
        }

        return nullptr;
    }


    void NavigatorTreeModel::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
    {
        const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
        if( pSdrHint )
        {
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
        // is shell gone?
        else if ( dynamic_cast<const SfxSimpleHint*>(&rHint) && static_cast<const SfxSimpleHint*>(&rHint)->GetId() == SFX_HINT_DYING)
            UpdateContent(nullptr);

        // changed mark of controls?
        else if (dynamic_cast<const FmNavViewMarksChanged*>(&rHint))
        {
            const FmNavViewMarksChanged* pvmcHint = static_cast<const FmNavViewMarksChanged*>(&rHint);
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
                FmEntryData* pEntryData = FindData( xFormComponent, GetRootList() );
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

    bool NavigatorTreeModel::InsertFormComponent(FmNavRequestSelectHint& rHint, SdrObject* pObject)
    {
        if ( dynamic_cast<const SdrObjGroup*>( pObject) !=  nullptr )
        {   // descend recursively
            const SdrObjList *pChildren = static_cast<SdrObjGroup*>(pObject)->GetSubList();
            for ( size_t i=0; i<pChildren->GetObjCount(); ++i )
            {
                SdrObject* pCurrent = pChildren->GetObj(i);
                if (!InsertFormComponent(rHint, pCurrent))
                    return false;
            }
        }
        else
        {
            FmFormObj* pFormObject = FmFormObj::GetFormObject( pObject );
            if ( !pFormObject )
                return false;

            try
            {
                Reference< XFormComponent > xFormViewControl( pFormObject->GetUnoControlModel(), UNO_QUERY_THROW );
                FmEntryData* pControlData = FindData( xFormViewControl, GetRootList() );
                if ( !pControlData )
                    return false;

                rHint.AddItem( pControlData );
                return true;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
                return false;
            }
        }

        return true;
    }

    void NavigatorTreeModel::BroadcastMarkedObjects(const SdrMarkList& mlMarked)
    {
        // search all objects, which can be handled, out of marked objects
        FmNavRequestSelectHint rshRequestSelection;
        bool bIsMixedSelection = false;

        for (size_t i=0; (i<mlMarked.GetMarkCount()) && !bIsMixedSelection; ++i)
        {
            SdrObject* pobjCurrent = mlMarked.GetMark(i)->GetMarkedSdrObj();
            bIsMixedSelection |= !InsertFormComponent(rshRequestSelection, pobjCurrent);
                // if Not-Form-Control, InsertFormComponent returns sal_False !
        }

        rshRequestSelection.SetMixedSelection(bIsMixedSelection);
        if (bIsMixedSelection)
            rshRequestSelection.ClearItems();

        Broadcast(rshRequestSelection);
            // an empty list causes NavigatorTree to remove his selection
    }


    void NavigatorTreeModel::UpdateContent( const Reference< css::form::XForms > & xForms )
    {

        // refill model form root upward
        Clear();
        if (xForms.is())
        {
            xForms->addContainerListener(static_cast<XContainerListener*>(m_pPropChangeList));

            FillBranch(nullptr);

            // select same control in tree as in view
            // (or all of them), if there is one ...
            if(!m_pFormShell) return;       // no shell

            FmFormView* pFormView = m_pFormShell->GetFormView();
            DBG_ASSERT(pFormView != nullptr, "NavigatorTreeModel::UpdateContent : keine FormView");
            BroadcastMarkedObjects(pFormView->GetMarkedObjectList());
        }
    }


    void NavigatorTreeModel::UpdateContent( FmFormShell* pShell )
    {

        // If shell is unchanged, do nothing
        FmFormPage* pNewPage = pShell ? pShell->GetCurPage() : nullptr;
        if ((pShell == m_pFormShell) && (m_pFormPage == pNewPage))
            return;


        // unregister as Listener
        if( m_pFormShell )
        {
            if (m_pFormModel)
                EndListening( *m_pFormModel );
            m_pFormModel = nullptr;
            EndListening( *m_pFormShell );
            Clear();
        }


        // entire update
        m_pFormShell = pShell;
        if (m_pFormShell)
        {
            m_pFormPage = pNewPage;
            UpdateContent(m_pFormPage->GetForms());
        } else
            m_pFormPage = nullptr;


        // register as Listener again
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

        // get components from form
        if (pFormData)
            return Reference< XIndexContainer > (pFormData->GetFormIface(), UNO_QUERY);

        return Reference< XIndexContainer > ();
    }


    bool NavigatorTreeModel::Rename( FmEntryData* pEntryData, const OUString& rNewText )
    {

        // If name already exist, error message
        pEntryData->SetText( rNewText );


        // get PropertySet
        Reference< XFormComponent >  xFormComponent;

        if( dynamic_cast<const FmFormData*>( pEntryData) !=  nullptr )
        {
            FmFormData* pFormData = static_cast<FmFormData*>(pEntryData);
            Reference< XForm >  xForm( pFormData->GetFormIface());
            xFormComponent = xForm;
        }

        if( dynamic_cast<const FmControlData*>( pEntryData) !=  nullptr )
        {
            FmControlData* pControlData = static_cast<FmControlData*>(pEntryData);
            xFormComponent = pControlData->GetFormComponent();
        }

        if( !xFormComponent.is() ) return false;
        Reference< XPropertySet >  xSet(xFormComponent, UNO_QUERY);
        if( !xSet.is() ) return false;


        // set name
        xSet->setPropertyValue( FM_PROP_NAME, makeAny(rNewText) );

        return true;
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
        return nullptr;
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
