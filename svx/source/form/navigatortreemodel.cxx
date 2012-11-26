/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/dialmgr.hxx>
#include <svx/fmshell.hxx>
#include <svx/fmmodel.hxx>
#include <svx/fmpage.hxx>
#include <svx/fmglob.hxx>
#include <svx/svditer.hxx>
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
#include <rtl/logfile.hxx>
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
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "OFormComponentObserver::OFormComponentObserver" );
    }

    // XPropertyChangeListener
    //------------------------------------------------------------------------
    void SAL_CALL OFormComponentObserver::disposing(const EventObject& Source) throw( RuntimeException )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "OFormComponentObserver::disposing" );
        Remove( Source.Source );
    }

    //------------------------------------------------------------------------
    void SAL_CALL OFormComponentObserver::propertyChange(const PropertyChangeEvent& evt) throw(RuntimeException)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "OFormComponentObserver::propertyChange" );
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
            ::rtl::OUString aNewName =  ::comphelper::getString(evt.NewValue);
            pEntryData->SetText( aNewName );
            FmNavNameChangedHint aNameChangedHint( pEntryData, aNewName );
            m_pNavModel->Broadcast( aNameChangedHint );
        }
    }

    // XContainerListener
    //------------------------------------------------------------------------------
    void SAL_CALL OFormComponentObserver::elementInserted(const ContainerEvent& evt) throw(RuntimeException)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "OFormComponentObserver::elementInserted" );
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
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "OFormComponentObserver::Insert" );
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
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "OFormComponentObserver::elementReplaced" );
        if (IsLocked() || !m_pNavModel)
            return;

        m_bCanUndo = sal_False;

        // EntryData loeschen
        Reference< XFormComponent >  xReplaced;
        evt.ReplacedElement >>= xReplaced;
        FmEntryData* pEntryData = m_pNavModel->FindData(xReplaced, m_pNavModel->GetRootList(), sal_True);
        if (pEntryData)
        {
            if (dynamic_cast< FmControlData* >(pEntryData))
            {
                Reference< XFormComponent >  xComp;
                evt.Element >>= xComp;
                DBG_ASSERT(xComp.is(), "OFormComponentObserver::elementReplaced : invalid argument !");
                    // an einer FmControlData sollte eine XFormComponent haengen
                m_pNavModel->ReplaceFormComponent(xReplaced, xComp);
            }
            else if (dynamic_cast< FmFormData* >(pEntryData))
            {
                DBG_ERROR("replacing forms not implemented yet !");
            }
        }

        m_bCanUndo = sal_True;
    }

    //------------------------------------------------------------------------------
    void OFormComponentObserver::Remove( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "OFormComponentObserver::Remove" );
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
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "OFormComponentObserver::elementRemoved" );
        Reference< XInterface > xElement;
        evt.Element >>= xElement;
        Remove( xElement );
    }

    //========================================================================
    // class NavigatorTreeModel
    //========================================================================

    //------------------------------------------------------------------------
    NavigatorTreeModel::NavigatorTreeModel( const ImageList& _rNormalImages, const ImageList& _rHCImages )
                    :m_pFormShell(NULL)
                    ,m_pFormPage(NULL)
                    ,m_pFormModel(NULL)
                    ,m_aNormalImages( _rNormalImages )
                    ,m_aHCImages( _rHCImages )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::NavigatorTreeModel" );
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
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::SetModified" );
        if( !m_pFormShell ) return;
        SfxObjectShell* pObjShell = m_pFormShell->GetFormModel()->GetObjectShell();
        if( !pObjShell ) return;
        pObjShell->SetModified( bMod );
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::Clear()
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::Clear" );
        Reference< XNameContainer >  xForms( GetForms());
        Reference< XContainer >  xContainer(xForms, UNO_QUERY);
        if (xContainer.is())
            xContainer->removeContainerListener((XContainerListener*)m_pPropChangeList);

        //////////////////////////////////////////////////////////////////////
        // RootList loeschen
        FmEntryData* pChildData;
        FmEntryDataList* pRootList = GetRootList();

        for( sal_uInt32 i=pRootList->Count(); i>0; i-- )
        {
            pChildData = pRootList->GetObject(i-1);
            pRootList->Remove( pChildData );
            delete pChildData;
        }

        //////////////////////////////////////////////////////////////////////
        // UI benachrichtigen
        FmNavClearedHint aClearedHint;
        Broadcast( aClearedHint );
    }

    //------------------------------------------------------------------------
    Reference< XNameContainer >  NavigatorTreeModel::GetForms() const
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::GetForms" );
        if( !m_pFormShell || !m_pFormShell->GetCurPage())
            return NULL;
        else
            return m_pFormShell->GetCurPage()->GetForms();
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::Insert(FmEntryData* pEntry, sal_uLong nRelPos, sal_Bool bAlterModel)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::Insert" );
        if (IsListening(*m_pFormModel))
            EndListening(*m_pFormModel);

        m_pPropChangeList->Lock();
        FmFormData* pFolder     = (FmFormData*) pEntry->GetParent();
        Reference< XChild > xElement( pEntry->GetChildIFace() );
        if (bAlterModel)
        {
            XubString aStr;
            if (dynamic_cast< FmFormData* >(pEntry))
                aStr = SVX_RES(RID_STR_FORM);
            else
                aStr = SVX_RES(RID_STR_CONTROL);

            Reference< XIndexContainer >  xContainer;
            if (pFolder)
                xContainer = Reference< XIndexContainer > (pFolder->GetFormIface(), UNO_QUERY);
            else
                xContainer = Reference< XIndexContainer > (GetForms(), UNO_QUERY);

            bool bUndo = m_pFormModel->IsUndoEnabled();

            if( bUndo )
            {
                XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
                aUndoStr.SearchAndReplace('#', aStr);
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
                DBG_ERROR("NavigatorTreeModel::Insert : the parent container needs an elementtype I don't know !");
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
        if (dynamic_cast< FmFormData* >(pEntry))
        {
            Reference< XContainer >  xContainer(xElement, UNO_QUERY);
            if (xContainer.is())
                xContainer->addContainerListener((XContainerListener*)m_pPropChangeList);
        }

        if (pFolder)
            pFolder->GetChildList()->Insert( pEntry, nRelPos );
        else
            GetRootList()->Insert( pEntry, nRelPos );

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
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::Remove" );
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
            XubString        aStr;
            if (dynamic_cast< FmFormData* >(pEntry))
                aStr = SVX_RES(RID_STR_FORM);
            else
                aStr = SVX_RES(RID_STR_CONTROL);

            if( bUndo )
            {
                XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_REMOVE));
                aUndoStr.SearchAndReplace('#', aStr);
                m_pFormModel->BegUndo(aUndoStr);
            }
        }

        // jetzt die eigentliche Entfernung der Daten aus dem Model
        FmFormData* pFmFormData = dynamic_cast< FmFormData* >(pEntry);

        if (pFmFormData)
        {
            RemoveForm( pFmFormData );
        }
        else
        {
            FmControlData* pFmControlData = dynamic_cast< FmControlData* >(pEntry);

            if(pFmControlData)
            {
                RemoveFormComponent( pFmControlData );
            }
        }

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
            pFolder->GetChildList()->Remove(pEntry);
        else
        {
            GetRootList()->Remove(pEntry);
            //////////////////////////////////////////////////////////////////////
            // Wenn keine Form mehr in der Root, an der Shell CurForm zuruecksetzen
            if (!GetRootList()->Count())
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
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::RemoveForm" );
        //////////////////////////////////////////////////////////////////////
        // Form und Parent holen
        if (!pFormData || !m_pFormModel)
            return;

        FmEntryDataList*    pChildList = pFormData->GetChildList();
        sal_uInt32 nCount = pChildList->Count();
        for (sal_uInt32 i = nCount; i > 0; i--)
        {
            FmEntryData* pEntryData = pChildList->GetObject(i - 1);

            //////////////////////////////////////////////////////////////////////
            // Child ist Form -> rekursiver Aufruf
            FmFormData* pFmFormData = dynamic_cast< FmFormData* >(pEntryData);

            if( pFmFormData )
            {
                RemoveForm( pFmFormData );
            }
            else
            {
                FmControlData* pFmControlData = dynamic_cast< FmControlData* >(pEntryData);

                if( pFmControlData )
                {
                    RemoveFormComponent( pFmControlData );
                }
            }
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
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::RemoveFormComponent" );
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
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::ClearBranch" );
        //////////////////////////////////////////////////////////////////////
        // Alle Eintraege dieses Zweiges loeschen
        FmEntryDataList* pChildList = pParentData->GetChildList();
        FmEntryData* pChildData;

        for( sal_uInt32 i=pChildList->Count(); i>0; i-- )
        {
            pChildData = pChildList->GetObject(i-1);
            FmFormData* pFmFormData = dynamic_cast< FmFormData* >(pChildData);

            if( pFmFormData )
                ClearBranch( pFmFormData );

            pChildList->Remove( pChildData );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::FillBranch( FmFormData* pFormData )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::FillBranch" );
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
                pSubFormData = new FmFormData( xSubForm, m_aNormalImages, m_aHCImages, pFormData );
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

            ::rtl::OUString aControlName;
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
                    pSubFormData = new FmFormData(xSubForm, m_aNormalImages, m_aHCImages, pFormData);
                    Insert(pSubFormData, LIST_APPEND);

                    //////////////////////////////////////////////////////////////
                    // Neuer Branch, wenn SubForm wiederum Subforms enthaelt
                    FillBranch(pSubFormData);
                }
                else
                {
                    pNewControlData = new FmControlData(xCurrentComponent, m_aNormalImages, m_aHCImages, pFormData);
                    Insert(pNewControlData, LIST_APPEND);
                }
            }
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::InsertForm(const Reference< XForm > & xForm, sal_uInt32 nRelPos)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::InsertForm" );
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

        pFormData = new FmFormData( xForm, m_aNormalImages, m_aHCImages, pParentData );
        Insert( pFormData, nRelPos );
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::InsertFormComponent(const Reference< XFormComponent > & xComp, sal_uInt32 nRelPos)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::InsertFormComponent" );
        //////////////////////////////////////////////////////////
        // ParentData setzen
        Reference< XInterface >  xIFace( xComp->getParent());
        Reference< XForm >  xForm(xIFace, UNO_QUERY);
        if (!xForm.is())
            return;

        FmFormData* pParentData = (FmFormData*)FindData( xForm, GetRootList() );
        if( !pParentData )
        {
            pParentData = new FmFormData( xForm, m_aNormalImages, m_aHCImages, NULL );
            Insert( pParentData, LIST_APPEND );
        }

        if (!FindData(xComp, pParentData->GetChildList(),sal_False))
        {
            //////////////////////////////////////////////////////////
            // Neue EntryData setzen
            FmEntryData* pNewEntryData = new FmControlData( xComp, m_aNormalImages, m_aHCImages, pParentData );

            //////////////////////////////////////////////////////////
            // Neue EntryData einfuegen
            Insert( pNewEntryData, nRelPos );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::ReplaceFormComponent(const Reference< XFormComponent > & xOld, const Reference< XFormComponent > & xNew)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::ReplaceFormComponent" );
        FmEntryData* pData = FindData(xOld, GetRootList(), sal_True);
        DBG_ASSERT(pData && dynamic_cast< FmControlData* >(pData), "NavigatorTreeModel::ReplaceFormComponent : invalid argument !");
        ((FmControlData*)pData)->ModelReplaced( xNew, m_aNormalImages, m_aHCImages );

        FmNavModelReplacedHint aReplacedHint( pData );
        Broadcast( aReplacedHint );
    }

    //------------------------------------------------------------------------
    FmEntryData* NavigatorTreeModel::FindData(const Reference< XInterface > & xElement, FmEntryDataList* pDataList, sal_Bool bRecurs)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::FindData" );
        // normalize
        Reference< XInterface > xIFace( xElement, UNO_QUERY );

        for (sal_uInt16 i=0; i < pDataList->Count(); i++)
        {
            FmEntryData* pEntryData = pDataList->GetObject(i);
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
    FmEntryData* NavigatorTreeModel::FindData( const ::rtl::OUString& rText, FmFormData* pParentData, sal_Bool bRecurs )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::FindData" );
        FmEntryDataList* pDataList;
        if( !pParentData )
            pDataList = GetRootList();
        else
            pDataList = pParentData->GetChildList();

        ::rtl::OUString aEntryText;
        FmEntryData* pEntryData;
        FmEntryData* pChildData;

        for( sal_uInt16 i=0; i<pDataList->Count(); i++ )
        {
            pEntryData = pDataList->GetObject(i);
            aEntryText = pEntryData->GetText();

            if (rText == aEntryText)
                return pEntryData;

            FmFormData* pFmFormData = dynamic_cast< FmFormData* >(pEntryData);

            if( bRecurs && pFmFormData )
            {
                pChildData = FindData( rText, pFmFormData );
                if( pChildData )
                    return pChildData;
            }
        }

        return NULL;
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::Notify( SfxBroadcaster& /*rBC*/, const SfxHint& rHint )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::Notify" );
        const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);
        const SfxSimpleHint* pSfxSimpleHint = dynamic_cast< const SfxSimpleHint* >(&rHint);
        const FmNavViewMarksChanged* pFmNavViewMarksChanged = dynamic_cast< const FmNavViewMarksChanged* >(&rHint);

        if(pSdrHint)
        {
            switch( pSdrHint->GetSdrHintKind() )
            {
                case HINT_OBJINSERTED:
                    InsertSdrObj(pSdrHint->GetSdrHintObject());
                    break;
                case HINT_OBJREMOVED:
                    RemoveSdrObj(pSdrHint->GetSdrHintObject());
                    break;
                default:
                    break;
            }
        }
        // hat sich die shell verabschiedet?
        else if ( pSfxSimpleHint && SFX_HINT_DYING == pSfxSimpleHint->GetId() )
        {
            UpdateContent((FmFormShell*)NULL);
        }
        // hat sich die Markierung der Controls veraendert ?
        else if (pFmNavViewMarksChanged)
        {
            BroadcastMarkedObjects( pFmNavViewMarksChanged->GetAffectedView()->getSelectedSdrObjectVectorFromSdrMarkView() );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::InsertSdrObj( const SdrObject* pObj )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::InsertSdrObj" );
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
        else if ( pObj->getChildrenOfSdrObject() )
        {
            SdrObjListIter aIter( *pObj->getChildrenOfSdrObject() );
            while ( aIter.IsMore() )
                InsertSdrObj( aIter.Next() );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::RemoveSdrObj( const SdrObject* pObj )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::RemoveSdrObj" );
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
        else if ( pObj->getChildrenOfSdrObject() )
        {
            SdrObjListIter aIter( *pObj->getChildrenOfSdrObject() );
            while ( aIter.IsMore() )
                RemoveSdrObj( aIter.Next() );
        }
    }

    sal_Bool NavigatorTreeModel::InsertFormComponent(FmNavRequestSelectHint& rHint, SdrObject* pObject)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::InsertFormComponent" );
        SdrObjGroup* pSdrObjGroup = dynamic_cast< SdrObjGroup* >(pObject);

        if ( pSdrObjGroup )
        {   // rekursiv absteigen
            const SdrObjList *pChilds = pSdrObjGroup->getChildrenOfSdrObject();
            for ( sal_uInt16 i=0; i<pChilds->GetObjCount(); ++i )
            {
                SdrObject* pCurrent = pChilds->GetObj(i);
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

    void NavigatorTreeModel::BroadcastMarkedObjects(const SdrObjectVector& mlMarked)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::BroadcastMarkedObjects" );
        // gehen wir durch alle markierten Objekte und suchen wir die raus, mit denen ich was anfangen kann
        FmNavRequestSelectHint rshRequestSelection;
        sal_Bool bIsMixedSelection = sal_False;

        for (sal_uInt32 i(0); (i < mlMarked.size()) && !bIsMixedSelection; i++)
        {
            SdrObject* pobjCurrent = mlMarked[i];
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
    void NavigatorTreeModel::UpdateContent( const Reference< XNameContainer > & xForms )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::UpdateContent" );
        //////////////////////////////////////////////////////////////////////
        // Model von der Root aufwaerts neu fuellen
        Clear();
        if (xForms.is())
        {
            Reference< XContainer >  xFormContainer(xForms, UNO_QUERY);
            if (xFormContainer.is())
                xFormContainer->addContainerListener((XContainerListener*)m_pPropChangeList);

            FillBranch(NULL);

            // jetzt in meinem Tree genau die das in meiner View markierte Control selektieren
            // (bzw alle solchen), falls es eines gibt ...
            if(!m_pFormShell) return;       // keine Shell -> wech

            FmFormView* pFormView = m_pFormShell->GetFormView();
            DBG_ASSERT(pFormView != NULL, "NavigatorTreeModel::UpdateContent : keine FormView");
            BroadcastMarkedObjects(pFormView->getSelectedSdrObjectVectorFromSdrMarkView());
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::UpdateContent( FmFormShell* pShell )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::UpdateContent" );
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
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::GetFormComponents" );
        //////////////////////////////////////////////////////////////////////
        // Von der Form Components holen
        if (pFormData)
            return Reference< XIndexContainer > (pFormData->GetFormIface(), UNO_QUERY);

        return Reference< XIndexContainer > ();
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTreeModel::CheckEntry( FmEntryData* pEntryData )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::CheckEntry" );
        //////////////////////////////////////////////////////////////////////
        // Nur Forms duerfen auf Doppeldeutigkeit untersucht werden
        if( !dynamic_cast< FmFormData* >(pEntryData) )
            return sal_True;

        //////////////////////////////////////////////////////////////////////
        // ChildListe des Parents holen
        FmFormData* pParentData = (FmFormData*)pEntryData->GetParent();
        FmEntryDataList* pChildList;
        if( !pParentData )
            pChildList = GetRootList();
        else
            pChildList = pParentData->GetChildList();

        //////////////////////////////////////////////////////////////////////
        // In ChildListe nach doppelten Namen suchen
        ::rtl::OUString aChildText;
        FmEntryData* pChildData;

        for( sal_uInt16 i=0; i<pChildList->Count(); i++ )
        {
            pChildData = pChildList->GetObject(i);
            aChildText = pChildData->GetText();

            //////////////////////////////////////////////////////////////////////
            // Gleichen Eintrag gefunden
            if  (   (aChildText == pEntryData->GetText())
                &&  (pEntryData!=pChildData)
                )
            {


                SQLContext aError;
                aError.Message = String(SVX_RES(RID_ERR_CONTEXT_ADDFORM));
                aError.Details = String(SVX_RES(RID_ERR_DUPLICATE_NAME));
                displayException(aError);

                return sal_False;
            }
        }

        return sal_True;
    }

    //------------------------------------------------------------------------
    sal_Bool NavigatorTreeModel::Rename( FmEntryData* pEntryData, const ::rtl::OUString& rNewText )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::Rename" );
        //////////////////////////////////////////////////////////////////////
        // Wenn Name schon vorhanden, Fehlermeldung
        pEntryData->SetText( rNewText );

        //////////////////////////////////////////////////////////////////////
        // PropertySet besorgen
        Reference< XFormComponent >  xFormComponent;
        FmFormData* pFormData = dynamic_cast< FmFormData* >(pEntryData);

        if( pFormData )
        {
            Reference< XForm >  xForm( pFormData->GetFormIface());
            xFormComponent = Reference< XFormComponent > (xForm, UNO_QUERY);
        }

        FmControlData* pControlData = dynamic_cast< FmControlData* >(pEntryData);

        if( pControlData )
        {
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

    //------------------------------------------------------------------------
    sal_Bool NavigatorTreeModel::IsNameAlreadyDefined( const ::rtl::OUString& rName, FmFormData* pParentData )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::IsNameAlreadyDefined" );
        //////////////////////////////////////////////////////////////////////
        // Form in der Root
        if( !pParentData )
        {
            if (GetForms()->hasByName(rName))
                return sal_True;
        }

        //////////////////////////////////////////////////////////////////////
        // Restliche Components
        else
        {
            Reference< XNameContainer >  xFormComponents(GetFormComponents(pParentData), UNO_QUERY);
            if (xFormComponents.is() && xFormComponents->hasByName(rName))
                return sal_True;
        }

        return sal_False;
    }

    //------------------------------------------------------------------------
    SdrObject* NavigatorTreeModel::GetSdrObj( FmControlData* pControlData )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::GetSdrObj" );
        if (!pControlData || !m_pFormShell)
            return NULL;

        //////////////////////////////////////////////////////////////////////
        // In der Page das entsprechende SdrObj finden und selektieren
        Reference< XFormComponent >  xFormComponent( pControlData->GetFormComponent());
        if (!xFormComponent.is())
            return NULL;

        FmFormView*     pFormView       = m_pFormShell->GetFormView();
        SdrPageView*    pPageView       = pFormView->GetSdrPageView();

        if(pPageView)
        {
            SdrPage& rPage = pPageView->getSdrPageFromSdrPageView();
            SdrObjListIter aIter( rPage );
        return Search(aIter, xFormComponent);
    }

        return 0;
    }

    //------------------------------------------------------------------
    SdrObject* NavigatorTreeModel::Search(SdrObjListIter& rIter, const Reference< XFormComponent > & xComp)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "svx", "Ocke.Janssen@sun.com", "NavigatorTreeModel::Search" );
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
            else if ( pObj->getChildrenOfSdrObject() )
            {
                SdrObjListIter aIter( *pObj->getChildrenOfSdrObject() );
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


