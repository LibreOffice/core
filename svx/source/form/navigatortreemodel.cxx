/*************************************************************************
 *
 *  $RCSfile: navigatortreemodel.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 14:38:18 $
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
#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif
#ifndef _SVDITER_HXX
#include "svditer.hxx"
#endif
#ifndef _SVDOGRP_HXX
#include "svdogrp.hxx"
#endif
#ifndef _SVDPAGV_HXX //autogen
#include "svdpagv.hxx"
#endif


#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
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

#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif

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
                              ,m_bCanUndo(sal_True)
                              ,m_nLocks(0)
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

        FmEntryData* pEntryData;
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
                DBG_ERROR("replacing forms not implemented yet !");
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
    NavigatorTreeModel::NavigatorTreeModel( const ImageList& _rNormalImages, const ImageList& _rHCImages )
                    :m_pFormShell(NULL)
                    ,m_pFormPage(NULL)
                    ,m_pFormModel(NULL)
                    ,m_aNormalImages( _rNormalImages )
                    ,m_aHCImages( _rHCImages )
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
        if( !m_pFormShell || !m_pFormShell->GetCurPage())
            return NULL;
        else
            return m_pFormShell->GetCurPage()->GetForms();
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::Insert(FmEntryData* pEntry, sal_uInt32 nRelPos, sal_Bool bAlterModel)
    {
        if (IsListening(*m_pFormModel))
            EndListening(*m_pFormModel);

        m_pPropChangeList->Lock();
        FmFormData* pFolder     = (FmFormData*) pEntry->GetParent();
        Reference< XChild > xElement( pEntry->GetChildIFace() );
        if (bAlterModel)
        {
            XubString aStr;
            if (pEntry->ISA(FmFormData))
                aStr = SVX_RES(RID_STR_FORM);
            else
                aStr = SVX_RES(RID_STR_CONTROL);

            Reference< XIndexContainer >  xContainer;
            if (pFolder)
                xContainer = Reference< XIndexContainer > (pFolder->GetFormIface(), UNO_QUERY);
            else
                xContainer = Reference< XIndexContainer > (GetForms(), UNO_QUERY);

            XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_INSERT));
            aUndoStr.SearchAndReplace('#', aStr);
            m_pFormModel->BegUndo(aUndoStr);

            if (nRelPos >= (sal_uInt32)xContainer->getCount())
                nRelPos = (sal_uInt32)xContainer->getCount();

            // UndoAction
            if (m_pPropChangeList->CanUndo())
                m_pFormModel->AddUndo(new FmUndoContainerAction(*m_pFormModel,
                                                         FmUndoContainerAction::Inserted,
                                                         xContainer,
                                                         xElement,
                                                         nRelPos));

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
        //////////////////////////////////////////////////////////////////////
        // Form und Parent holen
        if (!pEntry || !m_pFormModel)
            return;

        if (IsListening(*m_pFormModel))
            EndListening(*m_pFormModel);

        m_pPropChangeList->Lock();
        FmFormData*     pFolder     = (FmFormData*) pEntry->GetParent();
        Reference< XChild > xElement ( pEntry->GetChildIFace() );
        if (bAlterModel)
        {
            XubString        aStr;
            if (pEntry->ISA(FmFormData))
                aStr = SVX_RES(RID_STR_FORM);
            else
                aStr = SVX_RES(RID_STR_CONTROL);

            XubString aUndoStr(SVX_RES(RID_STR_UNDO_CONTAINER_REMOVE));
            aUndoStr.SearchAndReplace('#', aStr);
            m_pFormModel->BegUndo(aUndoStr);
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
                if (m_pPropChangeList->CanUndo())
                    m_pFormModel->AddUndo(new FmUndoContainerAction(*m_pFormModel,
                                                          FmUndoContainerAction::Removed,
                                                          xContainer,
                                                          xElement, nContainerIndex ));
                xContainer->removeByIndex(nContainerIndex );
            }
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
                m_pFormShell->GetImpl()->setCurForm( Reference< XForm > () );
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
        sal_uInt32 nCount = pChildList->Count();
        for (sal_uInt32 i = nCount; i > 0; i--)
        {
            FmEntryData* pEntryData = pChildList->GetObject(i - 1);

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
        FmEntryData* pChildData;

        for( sal_uInt32 i=pChildList->Count(); i>0; i-- )
        {
            pChildData = pChildList->GetObject(i-1);
            if( pChildData->ISA(FmFormData) )
                ClearBranch( (FmFormData*)pChildData );

            pChildList->Remove( pChildData );
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
        FmEntryData* pData = FindData(xOld, GetRootList(), sal_True);
        DBG_ASSERT(pData && pData->ISA(FmControlData), "NavigatorTreeModel::ReplaceFormComponent : invalid argument !");
        ((FmControlData*)pData)->ModelReplaced( xNew, m_aNormalImages, m_aHCImages );

        FmNavModelReplacedHint aReplacedHint( pData );
        Broadcast( aReplacedHint );
    }

    //------------------------------------------------------------------------
    FmEntryData* NavigatorTreeModel::FindData(const Reference< XInterface > & xElement, FmEntryDataList* pDataList, sal_Bool bRecurs)
    {
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
    void NavigatorTreeModel::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
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
            }
        }
        // hat sich die shell verabschiedet?
        else if ( rHint.ISA(SfxSimpleHint) && ((SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING)
            Update((FmFormShell*)NULL);

        // hat sich die Markierung der Controls veraendert ?
        else if (rHint.ISA(FmNavViewMarksChanged))
        {
            FmNavViewMarksChanged* pvmcHint = (FmNavViewMarksChanged*)&rHint;
            BroadcastMarkedObjects( pvmcHint->GetAffectedView()->GetMarkedObjectList() );
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::InsertSdrObj(const SdrObject* pObj)
    {
        if (pObj->GetObjInventor() == FmFormInventor)
        {                                           //////////////////////////////////////////////////////////////////////
            // Ist dieses Objekt ein XFormComponent?
            Reference< XFormComponent >  xFormComponent(((SdrUnoObj*)pObj)->GetUnoControlModel(), UNO_QUERY);
            if (xFormComponent.is())
            {
                Reference< XIndexContainer >  xContainer(xFormComponent->getParent(), UNO_QUERY);
                if (xContainer.is())
                {
                    sal_Int32 nPos = getElementPos(Reference< XIndexAccess > (xContainer, UNO_QUERY), xFormComponent);
                    InsertFormComponent(xFormComponent, nPos);
                }
            }
        }
        else if (pObj->IsGroupObject())
        {
            SdrObjListIter aIter(*pObj->GetSubList());
            while (aIter.IsMore())
                InsertSdrObj(aIter.Next());
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::RemoveSdrObj(const SdrObject* pObj)
    {
        if (pObj->GetObjInventor() == FmFormInventor)
        {                                           //////////////////////////////////////////////////////////////////////
            // Ist dieses Objekt ein XFormComponent?
            Reference< XFormComponent >  xFormComponent(((SdrUnoObj*)pObj)->GetUnoControlModel(), UNO_QUERY);
            if (xFormComponent.is())
            {
                FmEntryData* pEntryData = FindData(xFormComponent, GetRootList(), sal_True);
                if (pEntryData)
                    Remove(pEntryData);
            }
        }
        else if (pObj->IsGroupObject())
        {
            SdrObjListIter aIter(*pObj->GetSubList());
            while (aIter.IsMore())
                RemoveSdrObj(aIter.Next());
        }
    }

    sal_Bool NavigatorTreeModel::InsertFormComponent(FmNavRequestSelectHint& rHint, SdrObject* pObject)
    {
        if ( pObject->ISA(SdrObjGroup) )
        {   // rekursiv absteigen
            const SdrObjList *pChilds = ((SdrObjGroup*)pObject)->GetSubList();
            for ( sal_uInt16 i=0; i<pChilds->GetObjCount(); ++i )
            {
                SdrObject* pCurrent = pChilds->GetObj(i);
                if (!InsertFormComponent(rHint, pCurrent))
                    return sal_False;
            }
        } else
            if (pObject->IsUnoObj())
            {
                Reference< XInterface >  xControlModel( ((SdrUnoObj*)pObject)->GetUnoControlModel());
                // Ist dieses Objekt ein XFormComponent?
                Reference< XFormComponent >  xFormViewControl(xControlModel, UNO_QUERY);
                if (xFormViewControl.is())
                {   // es ist ein Form-Control -> selektieren lassen
                    FmEntryData* pControlData = FindData( xFormViewControl, GetRootList() );
                    if (pControlData)
                        rHint.AddItem( pControlData );
                } else
                {   // es ist kein Form-Control -> im Baum ueberhaupt nix selektieren lassen
                    return sal_False;
                }
            } else
                return sal_False;

        return sal_True;
    }

    void NavigatorTreeModel::BroadcastMarkedObjects(const SdrMarkList& mlMarked)
    {
        // gehen wir durch alle markierten Objekte und suchen wir die raus, mit denen ich was anfangen kann
        FmNavRequestSelectHint rshRequestSelection;
        sal_Bool bIsMixedSelection = sal_False;

        for (ULONG i=0; (i<mlMarked.GetMarkCount()) && !bIsMixedSelection; i++)
        {
            SdrObject* pobjCurrent = mlMarked.GetMark(i)->GetObj();
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
    void NavigatorTreeModel::Update( const Reference< XNameContainer > & xForms )
    {
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
            DBG_ASSERT(pFormView != NULL, "NavigatorTreeModel::Update : keine FormView");
            BroadcastMarkedObjects(pFormView->GetMarkedObjectList());
        }
    }

    //------------------------------------------------------------------------
    void NavigatorTreeModel::Update( FmFormShell* pShell )
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
            Update(m_pFormPage->GetForms());
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
    sal_Bool NavigatorTreeModel::CheckEntry( FmEntryData* pEntryData )
    {
        //////////////////////////////////////////////////////////////////////
        // Nur Forms duerfen auf Doppeldeutigkeit untersucht werden
        if( !pEntryData->ISA(FmFormData) ) return sal_True;

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
            xFormComponent = Reference< XFormComponent > (xForm, UNO_QUERY);
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

    //------------------------------------------------------------------------
    sal_Bool NavigatorTreeModel::IsNameAlreadyDefined( const ::rtl::OUString& rName, FmFormData* pParentData )
    {
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
        if (!pControlData || !m_pFormShell)
            return NULL;

        //////////////////////////////////////////////////////////////////////
        // In der Page das entsprechende SdrObj finden und selektieren
        Reference< XFormComponent >  xFormComponent( pControlData->GetFormComponent());
        if (!xFormComponent.is())
            return NULL;

        FmFormView*     pFormView       = m_pFormShell->GetFormView();
        SdrPageView*    pPageView       = pFormView->GetPageViewPvNum(0);
        SdrPage*        pPage           = pPageView->GetPage();

        SdrObjListIter  aIter( *pPage );
        return Search(aIter, xFormComponent);
    }

    //------------------------------------------------------------------
    SdrObject* NavigatorTreeModel::Search(SdrObjListIter& rIter, const Reference< XFormComponent > & xComp)
    {
        while (rIter.IsMore())
        {
            SdrObject* pObj = rIter.Next();
            //////////////////////////////////////////////////////////////////////
            // Es interessieren nur Uno-Objekte
            if (pObj->GetObjInventor() == FmFormInventor)
            {                                           //////////////////////////////////////////////////////////////////////
                // Ist dieses Objekt ein XFormComponent?
                Reference< XFormComponent >  xFormViewControl(((SdrUnoObj*)pObj)->GetUnoControlModel(), UNO_QUERY);
                if (xFormViewControl == xComp)
                    return pObj;
            }
            else if (pObj->IsGroupObject())
            {
                SdrObjListIter aIter(*pObj->GetSubList());
                pObj = Search(aIter, xComp);
                if (pObj)
                    return pObj;
            }
        }
        return NULL;
    }

//............................................................................
}   // namespace svxform
//............................................................................


