/*************************************************************************
 *
 *  $RCSfile: fmview.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:45:00 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#pragma hdrstop
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifdef REFERENCE
#undef REFERENCE
#endif
#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif

#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif

#ifndef _SVX_FMVWIMP_HXX
#include "fmvwimp.hxx"
#endif

#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SB_SBUNO_HXX
#include <basic/sbuno.hxx>
#endif
#ifndef _SFX_MACROCONF_HXX //autogen
#include <sfx2/macrconf.hxx>
#endif

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif

#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
#endif

#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
#endif
#ifndef _SVDITER_HXX //autogen
#include "svditer.hxx"
#endif
#ifndef _SVDPAGV_HXX //autogen
#include "svdpagv.hxx"
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svdogrp.hxx>
#endif
#ifndef _FM_FMVIEW_HXX
#include "fmview.hxx"
#endif
#ifndef _FM_FMMODEL_HXX
#include "fmmodel.hxx"
#endif
#ifndef _FM_FMPAGE_HXX
#include "fmpage.hxx"
#endif
#ifndef _SVX_FMSHELL_HXX
#include "fmshell.hxx"
#endif
#ifndef _SVX_FMPGEIMP_HXX
#include "fmpgeimp.hxx"
#endif
#ifndef _SVX_FMTOOLS_HXX
#include "fmtools.hxx"
#endif
#ifndef _SVX_FMSHIMP_HXX
#include "fmshimp.hxx"
#endif
#ifndef _SVX_FMSERVS_HXX
#include "fmservs.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _SVX_FMUNDO_HXX
#include "fmundo.hxx"
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include "dataaccessdescriptor.hxx"
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#include "fmglob.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::util;
using namespace ::svxform;
using namespace ::svx;

//========================================================================
//------------------------------------------------------------------------
TYPEINIT1(FmFormView, E3dView);

//------------------------------------------------------------------------
FmFormView::FmFormView( FmFormModel* pModel, OutputDevice* pOut )
    :E3dView(pModel,pOut)
{
    Init();
}

//------------------------------------------------------------------------
FmFormView::FmFormView( FmFormModel* pModel, ExtOutputDevice* pXOut )
    :E3dView( pModel, pXOut )
{
    Init();
}

//------------------------------------------------------------------------
FmFormView::FmFormView( FmFormModel* pModel )
    :E3dView( pModel )
{
    Init();
}

//------------------------------------------------------------------------
void FmFormView::Init()
{
    pFormShell = NULL;
    pImpl = new FmXFormView(::comphelper::getProcessServiceFactory(),this);
    pImpl->acquire();

    //////////////////////////////////////////////////////////////////////
    // Model setzen
    SdrModel* pModel = GetModel();

    DBG_ASSERT( pModel->ISA(FmFormModel), "Falsches Model" );
    if( !pModel->ISA(FmFormModel) ) return;
    FmFormModel* pFormModel = (FmFormModel*)pModel;

    //////////////////////////////////////////////////////////////////////
    // DesignMode vom Model holen
    sal_Bool bInitDesignMode = pFormModel->GetOpenInDesignMode();
    if ( pFormModel->OpenInDesignModeIsDefaulted( ) )
    {   // this means that nobody ever explicitly set this on the model, and the model has never
        // been loaded from a stream.
        // This means this is a newly created document. This means, we want to have it in design
        // mode by default (though a newly created model returns true for GetOpenInDesignMode).
        // We _want_ to have this because it makes a lot of hacks following the original fix
        // for #94595# unnecessary
        // #96399# - 2002-10-11 - fs@openoffice.org
        DBG_ASSERT( !bInitDesignMode, "FmFormView::Init: doesn't the model default to FALSE anymore?" );
            // if this asserts, either the on-contruction default in the model has changed (then this here
            // may not be necessary anymore), or we're not dealing with a new document ....
        bInitDesignMode = sal_True;
    }

    SfxObjectShell* pObjShell = pFormModel->GetObjectShell();
    if ( pObjShell && pObjShell->GetMedium() )
    {
        const SfxPoolItem *pItem=0;
        if ( pObjShell->GetMedium()->GetItemSet()->GetItemState( SID_COMPONENTDATA, sal_False, &pItem ) == SFX_ITEM_SET )
        {
            Sequence< PropertyValue> aSeq;
            ( ((SfxUnoAnyItem*)pItem)->GetValue() ) >>= aSeq;
            const PropertyValue* pIter  = aSeq.getConstArray();
            const PropertyValue* pEnd   = pIter + aSeq.getLength();
            for( ; pIter != pEnd ; ++pIter)
            {
                if ( pIter->Name.equalsAscii("ApplyFormDesignMode") )
                {
                    pIter->Value >>= bInitDesignMode;
                    break;
                }
            }
        }
    }

    sal_Bool bReadOnly = sal_False;
    if( pObjShell )
        bReadOnly = pObjShell->IsReadOnly();
    if( bReadOnly )
        bInitDesignMode = sal_False;

    // dieses wird in der Shell vorgenommen
    // bDesignMode = !bInitDesignMode;  // erzwingt, dass SetDesignMode ausgefuehrt wird
    SetDesignMode( bInitDesignMode );
}

//------------------------------------------------------------------------
FmFormView::~FmFormView()
{
    pImpl->notifyViewDying();
    pImpl->release();

    // Bei der Shell abmelden
    if( pFormShell )
        pFormShell->SetView( NULL );
}

//------------------------------------------------------------------------
void FmFormView::MarkListHasChanged()
{
    E3dView::MarkListHasChanged();

    if ( pFormShell && IsDesignMode() )
    {
        FmFormObj* pObj = getMarkedGrid();
        if ( pImpl->m_pMarkedGrid && pImpl->m_pMarkedGrid != pObj )
        {
            pImpl->m_pMarkedGrid = NULL;
            if ( pImpl->m_xWindow.is() )
            {
                pImpl->m_xWindow->removeFocusListener(pImpl);
                pImpl->m_xWindow = NULL;
            }
            SetMoveOutside(FALSE);
            RefreshAllIAOManagers();
        }

        pFormShell->GetImpl()->SetSelectionDelayed(this);
    }
}

//------------------------------------------------------------------------
void FmFormView::AddWin(OutputDevice* pWin1)
{
    E3dView::AddWin(pWin1);
}

//------------------------------------------------------------------------
void FmFormView::DelWin(OutputDevice* pWin1)
{
    E3dView::DelWin(pWin1);
}

//------------------------------------------------------------------------
void FmFormView::ChangeDesignMode(sal_Bool bDesign)
{
    if (bDesign == IsDesignMode())
        return;

    FmFormModel* pModel = PTR_CAST(FmFormModel, GetModel());
    if (pModel)
    {   // fuer die Zeit des Uebergangs das Undo-Environment ausschalten, das sichert, dass man dort auch nicht-transiente
        // Properties mal eben aendern kann (sollte allerdings mit Vorsicht genossen und beim Rueckschalten des Modes
        // auch immer wieder rueckgaegig gemacht werden. Ein Beispiel ist das Setzen der maximalen Text-Laenge durch das
        // FmXEditModel an seinem Control.)
        pModel->GetUndoEnv().Lock();
    }

    // Reihenfolge beim umsetzen !Designmode
    // a.) Datenbankforms laden
    // b.) Designmode an die ::com::sun::star::sdbcx::View weitergeben
    // c.) Controls aktivieren

    SdrPageView* pCurPageView = GetPageViewPvNum(0);
    FmFormPage*  pCurPage = pCurPageView ? PTR_CAST(FmFormPage,pCurPageView->GetPage()) : NULL;

    if (pCurPage && bDesign)
    {
        DeactivateControls(pCurPageView);
        DBG_ASSERT( pFormShell && pFormShell->GetImpl(), "FmFormView::ChangeDesignMode: no valid shell!" );
            // Since fixing 101864, 96009, et.al., we route the deactivation through the FormShell. I _suppose_
            // the shell is always existent here, but I am not sure if there is a valid scenario where it isn't.
            // Thus this assertion.

        if ( pFormShell && pFormShell->GetImpl() )
            pFormShell->GetImpl()->viewDeactivated( this );
        else
            pImpl->Deactivate( sal_True );
    }

    // über all angemeldeten Pages iterieren
    // nur die aktive wird umgeschaltet
    sal_uInt16 nCount = GetPageViewCount();
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        FmFormPage* pPage = PTR_CAST(FmFormPage,GetPageViewPvNum(i)->GetPage());
        if (pPage)
        {
            // during load the environment covers the error handling
            if (!bDesign)
                ActivateControls(pCurPageView);

            DBG_ASSERT( pFormShell && pFormShell->GetImpl(), "FmFormView::ChangeDesignMode: no valid shell!" );
            if ( pFormShell && pFormShell->GetImpl() )
                pFormShell->GetImpl()->loadForms( pPage, ( bDesign ? FORMS_UNLOAD : FORMS_LOAD ) );
        }
    }

    SetDesignMode(bDesign);
    if (pCurPage)
    {
        if (bDesign)
        {
            if ( GetActualOutDev() && GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW )
            {
                const Window* pWindow = static_cast< const Window* >( GetActualOutDev() );
                const_cast< Window* >( pWindow )->GrabFocus();
            }

            // redraw UNO objects
            if (pCurPageView)
            {
                SdrObjListIter aIter(*pCurPage);
                while( aIter.IsMore() )
                {
                    SdrObject* pObj = aIter.Next();
                    if (pObj && pObj->IsUnoObj())
                    {
                        // For redraw just use ActionChanged()
                        // pObj->BroadcastObjectChange();
                        pObj->ActionChanged();
                    }
                }
            }
        }
        else
        {
            // notify our shell that we have been activated
            if ( pFormShell && pFormShell->GetImpl() )
                pFormShell->GetImpl()->viewActivated( this );
            else
                pImpl->Activate();

            // set the auto focus to the first control (if indicated by the model to do so)
            sal_Bool bForceControlFocus = pModel ? pModel->GetAutoControlFocus() : sal_False;
            if (bForceControlFocus)
                pImpl->AutoFocus();
        }
    }

    // und mein Undo-Environment wieder an
    if (pModel)
        pModel->GetUndoEnv().UnLock();
}

//------------------------------------------------------------------------
void FmFormView::GrabFirstControlFocus( sal_Bool _bForceSync )
{
    if ( !IsDesignMode() )
        pImpl->AutoFocus( _bForceSync );
}

//------------------------------------------------------------------------
SdrPageView* FmFormView::ShowPage(SdrPage* pPage, const Point& rOffs)
{
    SdrPageView* pPV = E3dView::ShowPage(pPage, rOffs);

    if (pPage)
    {
        if (!IsDesignMode())
        {
            // creating the controllers
            ActivateControls(pPV);

            // Alles deselektieren
            UnmarkAll();

            // notify our shell that we have been activated
            if ( pFormShell && pFormShell->GetImpl() )
                pFormShell->GetImpl()->viewActivated( this );
            else
                pImpl->Activate( );
        }   // nur wenn die Shell bereits im DesignMode ist
        else if (pFormShell && pFormShell->IsDesignMode())
        {
            FmXFormShell* pFormShellImpl = pFormShell->GetImpl();
            Reference< ::com::sun::star::container::XIndexAccess >  xForms(((FmFormPage*)pPage)->GetForms(), UNO_QUERY);
            pFormShellImpl->ResetForms(xForms, sal_True);

            // damit der Formular-Navigator auf den Seitenwechsel reagieren kann
            pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_FMEXPLORER_CONTROL , sal_True, sal_False);

            pFormShellImpl->SetSelection(GetMarkedObjectList());
        }
    }
    return pPV;
}

//------------------------------------------------------------------------
void FmFormView::HidePage(SdrPageView* pPV)
{
    if (!IsDesignMode())
    {
        // Controls wieder deaktivieren
        DeactivateControls(pPV);
        if ( pFormShell && pFormShell->GetImpl() )
            pFormShell->GetImpl()->viewDeactivated( this );
        else
            pImpl->Deactivate( sal_True );
    }
    E3dView::HidePage(pPV);
}

//------------------------------------------------------------------------
SdrModel* FmFormView::GetMarkedObjModel() const
{
    return E3dView::GetMarkedObjModel();
}

//------------------------------------------------------------------------
sal_Bool FmFormView::Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    return E3dView::Paste(rMod, rPos, pLst, nOptions);
}

//------------------------------------------------------------------------
void FmFormView::ActivateControls(SdrPageView* pPageView)
{
    if (!pPageView) return;

    //const SdrPageViewWinList& rWinList = pPageView->GetWinList();
    //const SdrPageViewWindows& rPageViewWindows = pPageView->GetPageViewWindows();

    for (sal_uInt32 i = 0L; i < pPageView->WindowCount(); i++)
    {
        const SdrPageViewWindow& rPageViewWindow = *pPageView->GetWindow(i);

        if (rPageViewWindow.GetControlList().GetCount())
        {
            // pImpl->addWindow(&rWinList[i]);
            pImpl->addWindow(rPageViewWindow);
        }
    }
}

//------------------------------------------------------------------------
void FmFormView::DeactivateControls(SdrPageView* pPageView)
{
    if( !pPageView ) return;

    // const SdrPageViewWinList& rWinList = pPageView->GetWinList();
    // const SdrPageViewWindows& rPageViewWindows = pPageView->GetPageViewWindows();

    for (sal_uInt32 i = 0L; i < pPageView->WindowCount(); i++)
    {
        const SdrPageViewWindow& rPageViewWindow = *pPageView->GetWindow(i);

        if (rPageViewWindow.GetControlList().GetCount())
        {
            pImpl->removeWindow(rPageViewWindow.GetControlContainerRef() );
        }
    }
}

//------------------------------------------------------------------------
void FmFormView::ObjectCreated(FmFormObj* pObj)
{
    Reference< XPropertySet > xSet( pObj->GetUnoControlModel(), UNO_QUERY );
    if ( !xSet.is() )
        return;

    // some initial property defaults
    sal_Int16 nClassId = pImpl->implInitializeNewControlModel( xSet, pObj );

    if ( !pFormShell->GetImpl()->GetWizardUsing() )
        return;

    Reference< XChild >  xChild(xSet, UNO_QUERY);
    Reference< XRowSet >  xForm(xChild->getParent(), UNO_QUERY);
    String sWizardName;
    Any aObj;

    switch (nClassId)
    {
        case FormComponentType::GRIDCONTROL:
            sWizardName.AssignAscii("com.sun.star.sdb.GridControlAutoPilot");
            aObj <<= xChild;
            break;
        case FormComponentType::LISTBOX:
        case FormComponentType::COMBOBOX:
            sWizardName.AssignAscii("com.sun.star.sdb.ListComboBoxAutoPilot");
            aObj <<= xChild;
            break;
        case FormComponentType::GROUPBOX:
            sWizardName.AssignAscii("com.sun.star.sdb.GroupBoxAutoPilot");
            aObj <<= xChild;
            break;
    }

    if (sWizardName.Len() != 0)
    {
        // build the argument list
        Sequence< Any > aWizardArgs(1);
        // the object affected
        aWizardArgs[0] = makeAny(PropertyValue(
            ::rtl::OUString::createFromAscii("ObjectModel"),
            0,
            makeAny(xChild),
            PropertyState_DIRECT_VALUE
        ));

        // create the wizard object
        Reference< XExecutableDialog > xWizard;
        try
        {
            Reference< XMultiServiceFactory > xORB = ::comphelper::getProcessServiceFactory();
            xWizard = Reference< XExecutableDialog >(
                ::comphelper::getProcessServiceFactory()->createInstanceWithArguments(sWizardName, aWizardArgs),
                UNO_QUERY);
        }
        catch(Exception&)
        {
        }
        if (!xWizard.is())
        {
            ShowServiceNotAvailableError(NULL, sWizardName, sal_True);
            return;
        }

        // execute the wizard
        try
        {
            xWizard->execute();
        }
        catch(Exception&)
        {
            DBG_ERROR("FmFormView::ObjectCreated: could not execute the AutoPilot!");
            // TODO: real error handling
        }
    }
}

//------------------------------------------------------------------------
SdrObject* FmFormView::CreateFieldControl( const ODataAccessDescriptor& _rColumnDescriptor )
{
    return pImpl->implCreateFieldControl( _rColumnDescriptor );
}

//------------------------------------------------------------------------
SdrObject* FmFormView::CreateFieldControl(const UniString& rFieldDesc) const
{
    // SBA_FIELDEXCHANGE_FORMAT
    // "Datenbankname";"Tabellen/QueryName";1/0(fuer Tabelle/Abfrage);"Feldname"
    ::rtl::OUString sDataSource     = rFieldDesc.GetToken(0,sal_Unicode(11));
    ::rtl::OUString sObjectName     = rFieldDesc.GetToken(1,sal_Unicode(11));
    sal_uInt16 nObjectType          = (sal_uInt16)rFieldDesc.GetToken(2,sal_Unicode(11)).ToInt32();
    ::rtl::OUString sFieldName      = rFieldDesc.GetToken(3,sal_Unicode(11));

    if (!sFieldName.getLength() || !sObjectName.getLength() || !sDataSource.getLength())
        return NULL;

    ODataAccessDescriptor aColumnDescriptor;
    aColumnDescriptor.setDataSource(sDataSource);
    aColumnDescriptor[ daCommand ]          <<= sObjectName;
    aColumnDescriptor[ daCommandType ]      <<= nObjectType;
    aColumnDescriptor[ daColumnName ]       <<= sFieldName;

    return pImpl->implCreateFieldControl( aColumnDescriptor );
}

//------------------------------------------------------------------------
void FmFormView::InsertControlContainer(const Reference< ::com::sun::star::awt::XControlContainer > & xCC)
{
    if( !IsDesignMode() )
    {
        SdrPageView* pPageView = GetPageViewPvNum(0);
        if( pPageView )
        {
            // const SdrPageViewWinList& rWinList = pPageView->GetWinList();
            // const SdrPageViewWindows& rPageViewWindows = pPageView->GetPageViewWindows();

            for( sal_uInt32 i = 0L; i < pPageView->WindowCount(); i++ )
            {
                const SdrPageViewWindow& rPageViewWindow = *pPageView->GetWindow(i);

                if( rPageViewWindow.GetControlContainerRef() == xCC )
                {
                    pImpl->addWindow(rPageViewWindow);
                    break;
                }
            }
        }
    }
}

//------------------------------------------------------------------------
void FmFormView::RemoveControlContainer(const Reference< ::com::sun::star::awt::XControlContainer > & xCC)
{
    if( !IsDesignMode() )
    {
        pImpl->removeWindow( xCC );
    }
}
// -----------------------------------------------------------------------------
BOOL FmFormView::KeyInput(const KeyEvent& rKEvt, Window* pWin)
{
    BOOL bDone = FALSE;
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    if (    IsDesignMode()
        &&  pWin
        &&  !rKeyCode.IsShift()
        &&  !rKeyCode.IsMod1()
        &&  !rKeyCode.IsMod2()
        &&  rKeyCode.GetCode() == KEY_RETURN )
    {
        FmFormObj* pObj = getMarkedGrid();
        if ( pObj )
        {
            Reference< ::com::sun::star::awt::XWindow> xWindow(pObj->GetUnoControl( pWin ),UNO_QUERY);
            if ( xWindow.is() )
            {
                pImpl->m_pMarkedGrid = pObj;
                pImpl->m_xWindow = xWindow;
                // add as listener to get notified when ESC will be pressed inside the grid
                pImpl->m_xWindow->addFocusListener(pImpl);
                SetMoveOutside(TRUE);
                RefreshAllIAOManagers();
                xWindow->setFocus();
                bDone = TRUE;
            }
        }
    }
    if ( !bDone )
        bDone = E3dView::KeyInput(rKEvt,pWin);
    return bDone;
}
// -----------------------------------------------------------------------------
sal_Bool FmFormView::checkUnMarkAll(const Reference< XInterface >& _xSource)
{
    sal_Bool bRet = sal_False;
    Reference< ::com::sun::star::awt::XControl> xControl(pImpl->m_xWindow,UNO_QUERY);
    if ( bRet = ( !xControl.is() || !_xSource.is() || _xSource != xControl->getModel() ) )
        UnmarkAll();

    return bRet;
}
// -----------------------------------------------------------------------------
FmFormObj* FmFormView::getMarkedGrid() const
{
    FmFormObj* pObj = NULL;
    const SdrMarkList& rMarkList = GetMarkedObjectList();
    if ( 1 == rMarkList.GetMarkCount() )
    {
        SdrMark* pMark = rMarkList.GetMark(0);
        if ( pMark )
        {
            pObj = PTR_CAST(FmFormObj,pMark->GetObj());
            if ( pObj )
            {
                Reference<XServiceInfo> xServInfo(pObj->GetUnoControlModel(),UNO_QUERY);
                if ( !xServInfo.is() || !xServInfo->supportsService(FM_SUN_COMPONENT_GRIDCONTROL) )
                    pObj = NULL;
            }
        }
    }
    return pObj;
}
// -----------------------------------------------------------------------------
