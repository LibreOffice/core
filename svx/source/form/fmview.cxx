/*************************************************************************
 *
 *  $RCSfile: fmview.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-07 13:16:50 $
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

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#ifndef _EHDL_HXX
#include <svtools/ehdl.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDDB_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDATABASEACCESS_HPP_
#include <com/sun/star/sdb/XDatabaseAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XDATABASEENVIRONMENT_HPP_
#include <com/sun/star/sdb/XDatabaseEnvironment.hpp>
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
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATS_HPP_
#include <com/sun/star/util/XNumberFormats.hpp>
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

#ifndef _SVX_DIALMGR_HXX //autogen
#include "dialmgr.hxx"
#endif

#ifndef _SVX_FMITEMS_HXX
#include "fmitems.hxx"
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
#ifndef _SVX_FMGLOB_HXX
#include "fmglob.hxx"
#endif
#ifndef _SVX_FMOBJ_HXX
#include "fmobj.hxx"
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
#endif // _SVX_FMUNDO_HXX

#ifndef _MULTIPRO_HXX
#include "multipro.hxx"
#endif

#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XLOCALIZEDALIASES_HXX_
#include <com/sun/star/util/XLocalizedAliases.hpp>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

void getConnectionSpecs(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn, ::rtl::OUString& rURL, ::rtl::OUString& _rRegisteredTitle)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xURLSupplier(_rxConn, ::com::sun::star::uno::UNO_QUERY);
    if (!::comphelper::hasProperty(FM_PROP_URL, xURLSupplier))
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild > xChild(_rxConn, ::com::sun::star::uno::UNO_QUERY);
        if (xChild.is())
            xURLSupplier = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >(xChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
    }
    if (::comphelper::hasProperty(FM_PROP_URL, xURLSupplier))
    {
        try { rURL = ::comphelper::getString(xURLSupplier->getPropertyValue(FM_PROP_URL)); } catch(...) { }
    }

    if (rURL.getLength())
        _rRegisteredTitle = rURL;
}

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
    SfxObjectShell* pObjShell = pFormModel->GetObjectShell();
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
    pImpl->release();

    // Bei der Shell abmelden
    if( pFormShell )
        pFormShell->SetView( NULL );
}

//------------------------------------------------------------------------
void FmFormView::MarkListHasChanged()
{
    E3dView::MarkListHasChanged();

    if (pFormShell && IsDesignMode())
        pFormShell->GetImpl()->SetSelectionDelayed(this);
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
        pImpl->Deactivate(pCurPageView);
    }

    // über all angemeldeten Pages iterieren
    // nur die aktive wird umgeschaltet
    sal_uInt16 nCount = GetPageViewCount();
    for (sal_uInt16 i = 0; i < nCount; i++)
    {
        FmFormPage* pPage = PTR_CAST(FmFormPage,GetPageViewPvNum(i)->GetPage());
        if (pPage)
        {
            // Un/Load all forms
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xForms(((FmFormPage*)pPage)->GetForms(), ::com::sun::star::uno::UNO_QUERY);

            // during load the environment covers the error handling
            if (!bDesign)
                ActivateControls(pCurPageView);

            ::com::sun::star::uno::Reference< ::com::sun::star::form::XReset >  xReset;
            for (sal_Int32 i = 0, nCount = xForms->getCount(); i < nCount; i++)
            {
                xForms->getByIndex(i) >>= xReset;
                ::com::sun::star::uno::Reference< ::com::sun::star::form::XLoadable >  xLoad(xReset, ::com::sun::star::uno::UNO_QUERY);

                if (bDesign)
                {
                    if (xLoad.is() && xLoad->isLoaded())
                        xLoad->unload();

                    if (xReset.is())
                        xReset->reset();
                }
                else
                {
                    if (::isLoadable(xLoad) && !xLoad->isLoaded())
                        xLoad->load();
                }
            }
        }
    }

    SetDesignMode(bDesign);
    if (pCurPage)
    {
        if (bDesign)
        {
            //////////////////////////////////////////////////////////////////
            // UnoObjekte neu zeichnen
            if (pCurPageView)
            {
                SdrObjListIter aIter(*pCurPage);
                while( aIter.IsMore() )
                {
                    SdrObject* pObj = aIter.Next();
                    if (pObj && pObj->IsUnoObj())
                        pObj->SendRepaintBroadcast();
                }
            }
        }
        else
        {
            UnmarkAll();

            // Erste ::com::sun::star::form aktivieren
            pImpl->Activate(NULL);
        }
    }

    // und mein Undo-Environment wieder an
    if (pModel)
        pModel->GetUndoEnv().UnLock();
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

            // Erste ::com::sun::star::form aktivieren
            pImpl->Activate(pPV);
        }   // nur wenn die Shell bereits im DesignMode ist
        else if (pFormShell && pFormShell->IsDesignMode())
        {
            FmXFormShell* pFormShellImpl = pFormShell->GetImpl();
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >  xForms(((FmFormPage*)pPage)->GetForms(), ::com::sun::star::uno::UNO_QUERY);
            pFormShellImpl->ResetForms(xForms, sal_True);

            // damit der Formular-Navigator auf den Seitenwechsel reagieren kann
            pFormShell->GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_FMEXPLORER_CONTROL , sal_True, sal_False);

            pFormShellImpl->SetSelection(GetMarkList());
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
        pImpl->Deactivate(pPV);
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
    const SdrPageViewWinList& rWinList = pPageView->GetWinList();
    for (sal_uInt16 i = 0; i < rWinList.GetCount(); i++)
    {
        if (rWinList[i].GetControlList().GetCount())
        {
            pImpl->addWindow(&rWinList[i]);
        }
    }
}

//------------------------------------------------------------------------
void FmFormView::DeactivateControls(SdrPageView* pPageView)
{
    if( !pPageView ) return;
    const SdrPageViewWinList& rWinList = pPageView->GetWinList();
    for (sal_uInt16 i = 0; i < rWinList.GetCount(); i++)
    {
        if (rWinList[i].GetControlList().GetCount())
        {
            pImpl->removeWindow(rWinList[i].GetControlContainerRef() );
        }
    }
}

//------------------------------------------------------------------------
void FmFormView::ObjectCreated(FmFormObj* pObj)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xSet(pObj->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
    if (!xSet.is())
        return;

    if (!pFormShell->GetImpl()->GetWizardUsing())
        return;

    if (sal_True)
        return;
    // no wizards at the moment. The wizards need a complete rewriting, that's why this feature is disable at the
    // moment

    ::com::sun::star::uno::Any aValue = xSet->getPropertyValue(FM_PROP_CLASSID);

    sal_Int16 nClassId;
    if(!(aValue >>= nClassId))
        nClassId = ::com::sun::star::form::FormComponentType::CONTROL;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild >  xChild(xSet, ::com::sun::star::uno::UNO_QUERY);
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  xForm(xChild->getParent(), ::com::sun::star::uno::UNO_QUERY);
    String aWizardName;
    ::com::sun::star::uno::Any aObj;

    switch (nClassId)
    {
        case ::com::sun::star::form::FormComponentType::GRIDCONTROL:
            aWizardName.AssignAscii("GridWizard.GridWizard.MainWithDefault");
            aObj <<= xChild;
            break;
        case ::com::sun::star::form::FormComponentType::LISTBOX:
        case ::com::sun::star::form::FormComponentType::COMBOBOX:
            // Hat die ::com::sun::star::form eine Verbindung zur Datenbank?
        {
            sal_Bool bDataForm = sal_False;
            try
            {
                bDataForm = xForm.is() && ::dbtools::calcConnection(xForm, ::comphelper::getProcessServiceFactory()).is();
            }
            catch(...)
            {
            }

            if (bDataForm)
            {
                aWizardName.AssignAscii("ComboWizard.ComboWizard.MainWithDefault");
                aObj <<= xChild;
            }
        }   break;
        case ::com::sun::star::form::FormComponentType::GROUPBOX:
            // Hat die ::com::sun::star::form eine Verbindung zur Datenbank?
            aWizardName.AssignAscii("GroupWizard.GroupWizard.MainWithDefault");
            aObj <<= xChild;
            break;
    }

    if (aWizardName.Len() != 0)
    {
        SfxApplication* pApp = SFX_APP();
        SbxArrayRef xArray   = new SbxArray();
        SbxVariableRef xReturn  = new SbxVariable();
        SbxVariableRef xParam= new SbxVariable();
        xParam->PutBool(sal_True);

        SbxObjectRef xObj    = ::GetSbUnoObject(String(), aObj);
        xArray->Put(xObj,1);
        xArray->Put(xParam,2);

        pApp->EnterBasicCall();
        ErrCode aResult = pApp->GetMacroConfig()->Call(NULL,aWizardName,pApp->GetBasicManager(),xArray,xReturn);
        pApp->LeaveBasicCall();

        if (ERRCODE_NONE != aResult)
        {
            sal_uInt16 nContextId(0);
            switch (nClassId)
            {
                case ::com::sun::star::form::FormComponentType::GRIDCONTROL:
                    nContextId = RID_SUB_GRIDCONTROL_WIZARD;
                    break;
                case ::com::sun::star::form::FormComponentType::LISTBOX:
                    nContextId = RID_SUB_LISTBOX_WIZARD;
                    break;
                case ::com::sun::star::form::FormComponentType::COMBOBOX:
                    nContextId = RID_SUB_COMBOBOX_WIZARD;
                    break;
                case ::com::sun::star::form::FormComponentType::GROUPBOX:
                    nContextId = RID_SUB_GROUPBOX_WIZARD;
                    break;
            }
            SfxErrorContext aContext(nContextId, NULL, RID_RES_CONTROL_WIZARDS_ERROR_CONTEXTS, DIALOG_MGR());
            ErrorHandler::HandleError(aResult);
        }
    }
}

//------------------------------------------------------------------------
void FmFormView::CreateControlWithLabel(OutputDevice* pOutDev, sal_Int32 nYOffsetMM, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xField,
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats >  xNumberFormats, sal_uInt16 nObjID, const ::rtl::OUString& rFieldPostfix,
    FmFormObj*& pLabel, FmFormObj*& pControl) const
{
    sal_Int32 nDataType = ::comphelper::getINT32(xField->getPropertyValue(FM_PROP_FIELDTYPE));
    sal_Int32 nFormatKey = ::comphelper::getINT32(xField->getPropertyValue(FM_PROP_FORMATKEY));

    ::com::sun::star::uno::Any aFieldName(xField->getPropertyValue(FM_PROP_NAME));
    ::rtl::OUString sFieldName;
    aFieldName >>= sFieldName;

    // das Label
    pLabel = (FmFormObj*)SdrObjFactory::MakeNewObject( FmFormInventor, OBJ_FM_FIXEDTEXT, NULL, NULL );
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xLabelSet(pLabel->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
    xLabelSet->setPropertyValue(FM_PROP_LABEL, ::com::sun::star::uno::makeAny(sFieldName + rFieldPostfix));

    // positionieren unter Beachtung der Einstellungen des Ziel-Output-Devices
    Size aTextSize(pOutDev->GetTextWidth(sFieldName + rFieldPostfix), pOutDev->GetTextHeight());

    SdrModel* pModel    = GetModel();
    MapMode   eTargetMode(pOutDev->GetMapMode()),
              eSourceMode(MAP_100TH_MM);

    // Textbreite ist mindestens 5cm
    // Texthoehe immer halber cm
    Size aDefTxtSize(3000, 500);
    Size aDefSize(4000, 500);
    Size aDefImageSize(4000, 4000);
    // Abstand zwischen Text und Control
    Size aDelta(500, 0);

    Size aRealSize = pOutDev->LogicToLogic(aTextSize, eTargetMode, eSourceMode);
    aRealSize.Width() = max(aRealSize.Width(), aDefTxtSize.Width()) + aDelta.Width();
    aRealSize.Height()= aDefSize.Height();

    // je nach Skalierung des Zieldevices muss die Groesse noch normiert werden (#53523#)
    aRealSize.Width() = sal_Int32(Fraction(aRealSize.Width(), 1) * eTargetMode.GetScaleX());
    aRealSize.Height() = sal_Int32(Fraction(aRealSize.Height(), 1) * eTargetMode.GetScaleY());
    pLabel->SetLogicRect(
        Rectangle(  pOutDev->LogicToLogic(Point(0, nYOffsetMM), eSourceMode, eTargetMode),
                    pOutDev->LogicToLogic(aRealSize, eSourceMode, eTargetMode)
        ));

    // jetzt das Control
    pControl = (FmFormObj*)SdrObjFactory::MakeNewObject( FmFormInventor, nObjID, NULL, NULL );

    // positionieren
    Size szControlSize;
    if (::com::sun::star::sdbc::DataType::BIT == nDataType)
        szControlSize = aDefSize;
    else if (OBJ_FM_IMAGECONTROL == nObjID || ::com::sun::star::sdbc::DataType::LONGVARCHAR == nDataType)
        szControlSize = aDefImageSize;
    else
        szControlSize = aDefSize;

    // normieren wie oben
    szControlSize.Width() = sal_Int32(Fraction(szControlSize.Width(), 1) * eTargetMode.GetScaleX());
    szControlSize.Height() = sal_Int32(Fraction(szControlSize.Height(), 1) * eTargetMode.GetScaleY());
    pControl->SetLogicRect(
        Rectangle(  pOutDev->LogicToLogic(Point(aRealSize.Width(), nYOffsetMM), eSourceMode, eTargetMode),
                    pOutDev->LogicToLogic(szControlSize, eSourceMode, eTargetMode)
        ));

    // ein paar initiale Einstellungen am ControlModel
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xControlSet = ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (pControl->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
    if (xControlSet.is())
    {
        // ein paar numersiche Eigenschaften durchschleifen
        if (::comphelper::hasProperty(FM_PROP_DECIMAL_ACCURACY, xControlSet))
        {
            // Number braucht eine Scale
            ::com::sun::star::uno::Any aScaleVal(::comphelper::getNumberFormatDecimals(xNumberFormats, nFormatKey));
            xControlSet->setPropertyValue(FM_PROP_DECIMAL_ACCURACY, aScaleVal);
        }
        if (::comphelper::hasProperty(FM_PROP_VALUEMIN, xControlSet) && ::comphelper::hasProperty(FM_PROP_VALUEMAX, xControlSet))
        {
            // die minimale/maximale Zahl in diesem Feld
            sal_Int32 nMinValue = -1000000000, nMaxValue = 1000000000;
            switch (nDataType)
            {
                case ::com::sun::star::sdbc::DataType::TINYINT  : nMinValue = 0; nMaxValue = 255; break;
                case ::com::sun::star::sdbc::DataType::SMALLINT : nMinValue = -32768; nMaxValue = 32767; break;
                case ::com::sun::star::sdbc::DataType::INTEGER  : nMinValue = 0x80000000; nMaxValue = 0x7FFFFFFF; break;
                    // um die doubles/singles kuemmere ich mich nicht, da es ein wenig sinnlos ist
            }
            ::com::sun::star::uno::Any aVal;
            aVal <<= nMinValue;
            xControlSet->setPropertyValue(FM_PROP_VALUEMIN,aVal);
            aVal <<= nMaxValue;
            xControlSet->setPropertyValue(FM_PROP_VALUEMAX,aVal);
        }

        if (::comphelper::hasProperty(FM_PROP_STRICTFORMAT, xControlSet))
        {   // Formatueberpruefung fue numeric fields standardmaessig sal_True
            sal_Bool bB(sal_True);
            ::com::sun::star::uno::Any aVal(&bB,getBooleanCppuType());
            xControlSet->setPropertyValue(FM_PROP_STRICTFORMAT, aVal);
        }

        xControlSet->setPropertyValue(FM_PROP_CONTROLSOURCE, aFieldName);
        xControlSet->setPropertyValue(FM_PROP_NAME, aFieldName);

        if (nDataType == ::com::sun::star::sdbc::DataType::LONGVARCHAR)
        {
            sal_Bool bB(sal_True);
            xControlSet->setPropertyValue(FM_PROP_MULTILINE,::com::sun::star::uno::Any(&bB,getBooleanCppuType()));
        }

        if (nObjID == OBJ_FM_CHECKBOX)
            xControlSet->setPropertyValue(FM_PROP_TRISTATE,
                ::com::sun::star::uno::makeAny(xField->getPropertyValue(FM_PROP_ISNULLABLE))
            );
    }

    // announce the label to the control
    if (::comphelper::hasProperty(FM_PROP_CONTROLLABEL, xControlSet))
    {
        // (try-catch as the control may refuse a model without the right service name - which we don't know
        // usually a fixed text we use as label should be accepted, but to be sure ....)
        try
        {
            xControlSet->setPropertyValue(FM_PROP_CONTROLLABEL, ::com::sun::star::uno::makeAny(xLabelSet));
        }
        catch(...)
        {
            DBG_ERROR("FmFormView::CreateControlWithLabel : could not marry the control and the label !");
        }
        ;
    }
}

//------------------------------------------------------------------------
SdrObject* FmFormView::CreateFieldControl(const UniString& rFieldDesc) const
{
    if (!IsDesignMode())
        return NULL;

    FmFormPage& rPage = *(FmFormPage*)GetPageViewPvNum(0)->GetPage();

    // SBA_FIELDEXCHANGE_FORMAT
    // "Datenbankname";"Tabellen/QueryName";1/0(fuer Tabelle/Abfrage);"Feldname"
    ::rtl::OUString aDatabaseName = rFieldDesc.GetToken(0,char(11));
    ::rtl::OUString aObjectName  = rFieldDesc.GetToken(1,char(11));
    sal_uInt16 nObjectType = rFieldDesc.GetToken(2,char(11)).ToInt32();
    ::rtl::OUString aFieldName   = rFieldDesc.GetToken(3,char(11));

    if (!aFieldName.getLength() || !aObjectName.getLength() || !aDatabaseName.getLength())
        return NULL;


    // Datenbank, Tabelle/Abfrage und Feld bestimmen
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseEnvironment >  xEnv(::comphelper::getProcessServiceFactory()->createInstance(SRV_SDB_DATABASE_ENVIRONMENT), ::com::sun::star::uno::UNO_QUERY);
    if (!xEnv.is())
        return NULL;

    // Einlesen des default workspace
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >  xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseAccess >  xDatabase;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement >  xStatement;

    try
    {
        ::rtl::OUString sDatabaseName = aDatabaseName;
        try
        {
            xDatabase = xEnv->getDatabaseAccess(sDatabaseName);
        }
        catch(::com::sun::star::sdbc::SQLException e)
        {   // allowed, the env may throw an exception in case of an invalid name
            e; // make compiler happy
        }

        if (!xDatabase.is())
        {   // aDatabaseName isn't a database path. maybe a favorite name ?
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XNamingService >  xDatabaseAccesses(::comphelper::getProcessServiceFactory()->createInstance(SRV_SDB_DATABASE_CONTEXT), ::com::sun::star::uno::UNO_QUERY);
            if (xDatabaseAccesses.is())
            {
                try
                {
                    xDatabase = ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XDatabaseAccess > (xDatabaseAccesses->getRegisteredObject(sDatabaseName), ::com::sun::star::uno::UNO_QUERY);
                }
                catch( ::com::sun::star::container::NoSuchElementException&)
                {   // allowed, means aDatabaseName isn't a valid favorite name ....
                }
            }
        }
        if (!xDatabase.is())
        {
            DBG_ERROR("FmGridHeader::FmFormView::CreateFieldControl : could not retrieve the database access object !");
            return NULL;
        }
        xConnection = xDatabase->getConnection(::rtl::OUString(), ::rtl::OUString());
        if (!xConnection.is())
            return NULL;

        // check if the document is able to handle forms with the given data source
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > xDocParentConn = findConnection(rPage.GetForms());
        if (xDocParentConn.is())
        {   // there is a connection which restricts the allowed data sources for the forms

            // check if the new to-be-set data source (connection) complies to this restriction
            ::rtl::OUString sDocParentConnURL, sDocParentConnTitle;
            getConnectionSpecs(xDocParentConn, sDocParentConnURL, sDocParentConnTitle);

            ::rtl::OUString sNewConnURL, sNewConnTitle;
            getConnectionSpecs(xConnection, sNewConnURL, sNewConnTitle);

            sal_Bool bCompliant = sal_True;
            if (sDocParentConnTitle.getLength() && sNewConnTitle.getLength())
            {
                // both databases are registered
                if (!sDocParentConnTitle.equals(sNewConnTitle))
                    // and they're registered under different names -> not allowed
                    bCompliant = sal_False;
            }
            else if (sDocParentConnTitle.getLength() + sNewConnTitle.getLength() != 0)
            {   // exactly one database is registered -> they're not equal -> not allowed
                bCompliant = sal_False;
            }
            else
            {   // none of the data sources is registered -> compare the URLs
                INetURLObject aNewConnURL(sNewConnURL);
                INetURLObject aDocParentConnURL(sDocParentConnURL);
                if (aNewConnURL != aDocParentConnURL)
                    bCompliant = sal_False;
            }

            if (!bCompliant)
            {
                pImpl->m_sErrorMessage = String(SVX_RES(RID_STR_CREATECONTROLS_INVALIDSOURCE));
                pImpl->m_sErrorMessage.SearchAndReplaceAscii("$docname$", sDocParentConnTitle.getLength() ? sDocParentConnTitle : sDocParentConnURL);
                pImpl->m_sErrorMessage.SearchAndReplaceAscii("$dropname$", sNewConnTitle.getLength() ? sNewConnTitle : sNewConnURL);

                // no message boxes while DnD
                DBG_ASSERT(pImpl->m_nErrorMessageEvent == 0, "FmFormView::CreateFieldControl : two error events : you can't be that fast !");
                pImpl->m_nErrorMessageEvent = Application::PostUserEvent(LINK(pImpl, FmXFormView, OnDelayedErrorMessage));
                return NULL;
            }
        }

        // Festellen des Feldes
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >  xFields;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >  xField;
        switch (nObjectType)
        {
            case 0: // old : DataSelectionType_TABLE:
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier >  xSupplyTables(xConnection, ::com::sun::star::uno::UNO_QUERY);
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyColumns;
                xSupplyTables->getTables()->getByName(aObjectName) >>= xSupplyColumns;
                xFields = xSupplyColumns->getColumns();
            }
            break;
            case 1: // old : DataSelectionType_QUERY:
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XQueriesSupplier >  xSupplyQueries(xConnection, ::com::sun::star::uno::UNO_QUERY);
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyColumns;
                xSupplyQueries->getQueries()->getByName(aObjectName) >>= xSupplyColumns;
                xFields  = xSupplyColumns->getColumns();
            }
            break;
            default:
            {
                xStatement = xConnection->prepareStatement(aObjectName);
                // not interested in any results
                ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > (xStatement,::com::sun::star::uno::UNO_QUERY)->setPropertyValue(::rtl::OUString::createFromAscii("MaxRows"),::com::sun::star::uno::makeAny(sal_Int32(0)));
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyCols(xStatement->executeQuery(), ::com::sun::star::uno::UNO_QUERY);
                if (xSupplyCols.is())
                    xFields = xSupplyCols->getColumns();
            }
        }

        if (xFields.is() && xFields->hasByName(aFieldName))
            xFields->getByName(aFieldName) >>= xField;

        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier = ::dbtools::getNumberFormats(xConnection, sal_False);
        if (!xSupplier.is())
            return NULL;

        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats >  xNumberFormats(xSupplier->getNumberFormats());
        if (!xNumberFormats.is())
            return NULL;

        // Vom Feld werden nun zwei Informationen benoetigt:
        // a.) Name des Feldes fuer Label und ControlSource
        // b.) FormatKey, um festzustellen, welches Feld erzeugt werden soll
        sal_Int32 nDataType = ::comphelper::getINT32(xField->getPropertyValue(FM_PROP_FIELDTYPE));
        sal_Int32 nFormatKey = ::comphelper::getINT32(xField->getPropertyValue(FM_PROP_FORMATKEY));

        ::rtl::OUString sLabelPostfix;

        ////////////////////////////////////////////////////////////////
        // nur fuer Textgroesse
        OutputDevice* pOutDev = NULL;
        if (pActualOutDev && pActualOutDev->GetOutDevType() == OUTDEV_WINDOW)
            pOutDev = (OutputDevice*)pActualOutDev;
        else
        {// OutDev suchen
            SdrPageView* pPageView = GetPageViewPvNum(0);
            if( pPageView && !pOutDev )
            {
                const SdrPageViewWinList& rWinList = pPageView->GetWinList();
                for( sal_uInt16 i = 0; i < rWinList.GetCount(); i++ )
                {
                    if( rWinList[i].GetOutputDevice()->GetOutDevType() == OUTDEV_WINDOW)
                    {
                        pOutDev = rWinList[i].GetOutputDevice();
                        break;
                    }
                }
            }
        }

        if (!pOutDev)
            return NULL;

        if ((::com::sun::star::sdbc::DataType::BINARY == nDataType) || (::com::sun::star::sdbc::DataType::VARBINARY == nDataType))
            return NULL;
        //////////////////////////////////////////////////////////////////////
        // Anhand des FormatKeys wird festgestellt, welches Feld benoetigt wird
        sal_uInt16 nOBJID = 0;
        sal_Bool bDateNTimeField = sal_False;

        sal_Bool bIsCurrency = sal_False;
        if (::comphelper::hasProperty(FM_PROP_ISCURRENCY, xField))
            bIsCurrency = ::comphelper::getBOOL(xField->getPropertyValue(FM_PROP_ISCURRENCY));

        if (bIsCurrency)
            nOBJID = OBJ_FM_CURRENCYFIELD;
        else
            switch (nDataType)
            {
                case ::com::sun::star::sdbc::DataType::LONGVARBINARY:
                    nOBJID = OBJ_FM_IMAGECONTROL;
                    break;
                case ::com::sun::star::sdbc::DataType::LONGVARCHAR:
                    nOBJID = OBJ_FM_EDIT;
                    break;
                case ::com::sun::star::sdbc::DataType::BINARY:
                case ::com::sun::star::sdbc::DataType::VARBINARY:
                    return NULL;
                case ::com::sun::star::sdbc::DataType::BIT:
                    nOBJID = OBJ_FM_CHECKBOX;
                    break;
                case ::com::sun::star::sdbc::DataType::TINYINT:
                case ::com::sun::star::sdbc::DataType::SMALLINT:
                case ::com::sun::star::sdbc::DataType::INTEGER:
                    nOBJID = OBJ_FM_NUMERICFIELD;
                    break;
                case ::com::sun::star::sdbc::DataType::REAL:
                case ::com::sun::star::sdbc::DataType::DOUBLE:
                case ::com::sun::star::sdbc::DataType::NUMERIC:
                case ::com::sun::star::sdbc::DataType::DECIMAL:
                    nOBJID = OBJ_FM_FORMATTEDFIELD;
                    break;
                case ::com::sun::star::sdbc::DataType::TIMESTAMP:
                    bDateNTimeField = sal_True;
                    sLabelPostfix = UniString(SVX_RES(RID_STR_DATETIME_LABELPOSTFIX)).GetToken(0, ';');
                    // DON'T break !
                case ::com::sun::star::sdbc::DataType::DATE:
                    nOBJID = OBJ_FM_DATEFIELD;
                    break;
                case ::com::sun::star::sdbc::DataType::TIME:
                    nOBJID = OBJ_FM_TIMEFIELD;
                    break;
                case ::com::sun::star::sdbc::DataType::CHAR:
                case ::com::sun::star::sdbc::DataType::VARCHAR:
                default:
                    nOBJID = OBJ_FM_EDIT;
                    break;
            }
        if (!nOBJID)
            return NULL;

        FmFormObj* pLabel;
        FmFormObj* pControl;
        CreateControlWithLabel(pOutDev, 0, xField, xNumberFormats, nOBJID, sLabelPostfix, pLabel, pControl);
        if (!pLabel || !pControl)
        {
            delete pLabel;
            delete pControl;
            return NULL;
        }

        //////////////////////////////////////////////////////////////////////
        // Feststellen ob eine ::com::sun::star::form erzeugt werden muss
        // Dieses erledigt die Page fuer uns bzw. die PageImpl
        ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent >  xContent(pLabel->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >  xContainer(rPage.GetImpl()->SetDefaults(xContent, xDatabase, aDatabaseName, aObjectName, nObjectType), ::com::sun::star::uno::UNO_QUERY);
        if (xContainer.is())
            xContainer->insertByIndex(xContainer->getCount(), ::com::sun::star::uno::makeAny(xContent));

        xContent = ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > (pControl->GetUnoControlModel(), ::com::sun::star::uno::UNO_QUERY);
        xContainer = ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > (rPage.GetImpl()->SetDefaults(xContent, xDatabase, aDatabaseName, aObjectName, nObjectType), ::com::sun::star::uno::UNO_QUERY);
        if (xContainer.is())
            xContainer->insertByIndex(xContainer->getCount(), ::com::sun::star::uno::makeAny(xContent));

        //////////////////////////////////////////////////////////////////////
        // Objekte gruppieren
        SdrObjGroup* pGroup  = new SdrObjGroup();
        SdrObjList* pObjList = pGroup->GetSubList();
        pObjList->InsertObject(pLabel);
        pObjList->InsertObject(pControl);


        if (bDateNTimeField)
        {   // wir haben bis jetzt nur ein Datums-Feld eingefuegt, brauchen aber noch ein extra Feld fuer
            // die Zeit-Komponente
            pLabel = pControl = NULL;
            CreateControlWithLabel(pOutDev, 1000, xField, xNumberFormats, OBJ_FM_TIMEFIELD,
                UniString(SVX_RES(RID_STR_DATETIME_LABELPOSTFIX)).GetToken(1, ';'),
                pLabel, pControl);

            if (pLabel && pControl)
            {
                pObjList->InsertObject(pLabel);
                pObjList->InsertObject(pControl);
            }
            else
            {
                delete pLabel;
                delete pControl;
            }
        }

        return pGroup; // und fertig
    }
    catch(...)
    {
        DBG_ERROR("FmFormView::CreateFieldControl : catched an exception while creating the control !");
        ::comphelper::disposeComponent(xStatement);
    }


    return NULL;
}

//------------------------------------------------------------------------
void FmFormView::InsertControlContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & xCC)
{
    if( !IsDesignMode() )
    {
        SdrPageView* pPageView = GetPageViewPvNum(0);
        if( pPageView )
        {
            const SdrPageViewWinList& rWinList = pPageView->GetWinList();
            for( sal_uInt16 i = 0; i < rWinList.GetCount(); i++ )
            {
                if( rWinList[i].GetControlContainerRef() == xCC )
                {
                    pImpl->addWindow(&rWinList[i]);
                    break;
                }
            }
        }
    }
}

//------------------------------------------------------------------------
void FmFormView::RemoveControlContainer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > & xCC)
{
    if( !IsDesignMode() )
    {
        pImpl->removeWindow( xCC );
    }
}


