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

#include <fmvwimp.hxx>
#include <svx/fmshell.hxx>
#include <svx/fmtools.hxx>
#include <fmservs.hxx>
#include <fmprop.hxx>
#include <fmpgeimp.hxx>
#include <fmundo.hxx>
#include <vcl/waitobj.hxx>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/sdbcx/Privilege.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <sfx2/viewfrm.hxx>
#include <vcl/weld.hxx>
#include <vcl/wrkwin.hxx>
#include <svl/whiter.hxx>
#include <sfx2/app.hxx>
#include <svl/intitem.hxx>
#include <svl/visitem.hxx>
#include <unotools/moduleoptions.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svx/svdobj.hxx>
#include <svx/fmpage.hxx>
#include <svx/svditer.hxx>
#include <fmobj.hxx>

#include <svx/svxids.hrc>

#include <fmexch.hxx>
#include <svx/fmglob.hxx>
#include <svl/eitem.hxx>
#include <tools/diagnose_ex.h>
#include <svx/svdpage.hxx>
#include <svx/fmmodel.hxx>
#include <fmshimp.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/viewsh.hxx>
#include <fmexpl.hxx>
#include <formcontrolling.hxx>
#include <svl/numuno.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/types.hxx>
#include <fmdocumentclassification.hxx>
#include <formtoolbars.hxx>

#include <svx/svxdlg.hxx>

#include <svx/sdrobjectfilter.hxx>

#define ShellClass_FmFormShell
#include <svxslots.hxx>

#include <tbxform.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <memory>

// is used for Invalidate -> maintain it as well
// sort ascending !!!!!!
sal_uInt16 const ControllerSlotMap[] =    // slots of the controller
{
    SID_FM_CONFIG,
    SID_FM_PUSHBUTTON,
    SID_FM_RADIOBUTTON,
    SID_FM_CHECKBOX,
    SID_FM_FIXEDTEXT,
    SID_FM_GROUPBOX,
    SID_FM_EDIT,
    SID_FM_LISTBOX,
    SID_FM_COMBOBOX,
    SID_FM_DBGRID,
    SID_FM_IMAGEBUTTON,
    SID_FM_FILECONTROL,
    SID_FM_NAVIGATIONBAR,
    SID_FM_CTL_PROPERTIES,
    SID_FM_PROPERTIES,
    SID_FM_TAB_DIALOG,
    SID_FM_ADD_FIELD,
    SID_FM_DESIGN_MODE,
    SID_FM_SHOW_FMEXPLORER,
    SID_FM_SHOW_PROPERTIES,
    SID_FM_FMEXPLORER_CONTROL,
    SID_FM_DATEFIELD,
    SID_FM_TIMEFIELD,
    SID_FM_NUMERICFIELD,
    SID_FM_CURRENCYFIELD,
    SID_FM_PATTERNFIELD,
    SID_FM_OPEN_READONLY,
    SID_FM_IMAGECONTROL,
    SID_FM_USE_WIZARDS,
    SID_FM_FORMATTEDFIELD,
    SID_FM_FILTER_NAVIGATOR,
    SID_FM_AUTOCONTROLFOCUS,
    SID_FM_SCROLLBAR,
    SID_FM_SPINBUTTON,
    SID_FM_SHOW_DATANAVIGATOR,
    SID_FM_DATANAVIGATOR_CONTROL,

    0
};

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::form::runtime;
using namespace ::svxform;

FmDesignModeChangedHint::FmDesignModeChangedHint( bool bDesMode )
    :m_bDesignMode( bDesMode )
{
}


FmDesignModeChangedHint::~FmDesignModeChangedHint()
{
}

SFX_IMPL_INTERFACE(FmFormShell, SfxShell)

void FmFormShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_NAVIGATION, SfxVisibilityFlags::Standard|SfxVisibilityFlags::ReadonlyDoc,
                                            ToolbarId::SvxTbx_Form_Navigation,
                                            SfxShellFeature::FormShowDatabaseBar);

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_NAVIGATION, SfxVisibilityFlags::Standard|SfxVisibilityFlags::ReadonlyDoc,
                                            ToolbarId::SvxTbx_Form_Filter,
                                            SfxShellFeature::FormShowFilterBar);

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Standard | SfxVisibilityFlags::ReadonlyDoc,
                                            ToolbarId::SvxTbx_Text_Control_Attributes,
                                            SfxShellFeature::FormShowTextControlBar);

    GetStaticInterface()->RegisterChildWindow(SID_FM_ADD_FIELD, false, SfxShellFeature::FormShowField);
    GetStaticInterface()->RegisterChildWindow(SID_FM_SHOW_PROPERTIES, false, SfxShellFeature::FormShowProperies);
    GetStaticInterface()->RegisterChildWindow(SID_FM_SHOW_FMEXPLORER, false, SfxShellFeature::FormShowExplorer);
    GetStaticInterface()->RegisterChildWindow(SID_FM_FILTER_NAVIGATOR, false, SfxShellFeature::FormShowFilterNavigator);
    GetStaticInterface()->RegisterChildWindow(SID_FM_SHOW_DATANAVIGATOR, false, SfxShellFeature::FormShowDataNavigator);

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Standard,
                                            ToolbarId::SvxTbx_Controls,
                                            SfxShellFeature::FormTBControls);

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Standard,
                                            ToolbarId::SvxTbx_FormDesign,
                                            SfxShellFeature::FormTBDesign);
}


FmFormShell::FmFormShell( SfxViewShell* _pParent, FmFormView* pView )
            :SfxShell(_pParent)
            ,m_pImpl(new FmXFormShell(*this, _pParent->GetViewFrame()))
            ,m_pFormView( pView )
            ,m_pFormModel( nullptr )
            ,m_nLastSlot( 0 )
            ,m_bDesignMode( true )
            ,m_bHasForms(false)
{
    SetPool( &SfxGetpApp()->GetPool() );
    SetName( "Form" );

    SetView(m_pFormView);
}


FmFormShell::~FmFormShell()
{
    if ( m_pFormView )
        SetView( nullptr );

    m_pImpl->dispose();
}


void FmFormShell::NotifyMarkListChanged(FmFormView* pWhichView)
{
    FmNavViewMarksChanged aChangeNotification(pWhichView);
    Broadcast(aChangeNotification);
}


bool FmFormShell::PrepareClose(bool bUI)
{
    if (GetImpl()->didPrepareClose_Lock())
        // we already did a PrepareClose for the current modifications of the current form
        return true;

    bool bResult = true;
    // Save the data records, not in DesignMode and FilterMode
    if (!m_bDesignMode && !GetImpl()->isInFilterMode_Lock() &&
        m_pFormView && m_pFormView->GetActualOutDev() &&
        m_pFormView->GetActualOutDev()->GetOutDevType() == OUTDEV_WINDOW)
    {
        SdrPageView* pCurPageView = m_pFormView->GetSdrPageView();

        // sal_uInt16 nPos = pCurPageView ? pCurPageView->GetWinList().Find((OutputDevice*)m_pFormView->GetActualOutDev()) : SDRPAGEVIEWWIN_NOTFOUND;
        SdrPageWindow* pWindow = pCurPageView ? pCurPageView->FindPageWindow(*const_cast<OutputDevice*>(m_pFormView->GetActualOutDev())) : nullptr;

        if(pWindow)
        {
            // First, the current contents of the controls are stored.
            // If everything has gone smoothly, the modified records are stored.
            if (GetImpl()->getActiveController_Lock().is())
            {
                const svx::ControllerFeatures& rController = GetImpl()->getActiveControllerFeatures_Lock();
                if ( rController->commitCurrentControl() )
                {
                    const bool bModified = rController->isModifiedRow();

                    if ( bModified && bUI )
                    {
                        SfxViewShell* pShell = GetViewShell();
                        vcl::Window* pShellWnd = pShell ? pShell->GetWindow() : nullptr;
                        weld::Widget* pFrameWeld = pShellWnd ? pShellWnd->GetFrameWeld() : nullptr;
                        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pFrameWeld, "svx/ui/savemodifieddialog.ui"));
                        std::unique_ptr<weld::MessageDialog> xQry(xBuilder->weld_message_dialog("SaveModifiedDialog"));
                        switch (xQry->run())
                        {
                            case RET_YES:
                                bResult = rController->commitCurrentRecord( );
                                [[fallthrough]];
                            case RET_NO:
                                GetImpl()->didPrepareClose_Lock(true);
                                break;

                            case RET_CANCEL:
                                return false;
                        }
                    }
                }
            }
        }
    }
    return bResult;
}


void FmFormShell::impl_setDesignMode(bool bDesign)
{
    if (m_pFormView)
    {
        if (!bDesign)
            m_nLastSlot = SID_FM_DESIGN_MODE;

        GetImpl()->SetDesignMode_Lock(bDesign);
        // my m_bDesignMode is also set by the Impl ...
    }
    else
    {
        m_bHasForms = false;
        m_bDesignMode = bDesign;
        UIFeatureChanged();
    }

    GetViewShell()->GetViewFrame()->GetBindings().Invalidate(ControllerSlotMap);
}


bool FmFormShell::HasUIFeature(SfxShellFeature nFeature) const
{
    assert((nFeature & ~SfxShellFeature::FormMask) == SfxShellFeature::NONE);
    bool bResult = false;
    if (nFeature & SfxShellFeature::FormShowDatabaseBar)
    {
        // only if forms are also available
        bResult = !m_bDesignMode && GetImpl()->hasDatabaseBar_Lock() && !GetImpl()->isInFilterMode_Lock();
    }
    else if (nFeature & SfxShellFeature::FormShowFilterBar)
    {
        // only if forms are also available
        bResult = !m_bDesignMode && GetImpl()->hasDatabaseBar_Lock() && GetImpl()->isInFilterMode_Lock();
    }
    else if (nFeature & SfxShellFeature::FormShowFilterNavigator)
    {
        bResult = !m_bDesignMode && GetImpl()->hasDatabaseBar_Lock() && GetImpl()->isInFilterMode_Lock();
    }
    else if (nFeature & SfxShellFeature::FormShowField)
    {
        bResult = m_bDesignMode && m_pFormView && m_bHasForms;
    }
    else if (nFeature & SfxShellFeature::FormShowProperies)
    {
        bResult = m_bDesignMode && m_pFormView && m_bHasForms;
    }
    else if (nFeature & SfxShellFeature::FormShowExplorer)
    {
        bResult = m_bDesignMode; // OJ #101593# && m_pFormView && m_bHasForms;
    }
    else if (nFeature & SfxShellFeature::FormShowTextControlBar)
    {
        bResult = !GetImpl()->IsReadonlyDoc_Lock() && m_pImpl->IsActiveControl_Lock(true);
    }
    else if (nFeature & SfxShellFeature::FormShowDataNavigator)
    {
        bResult = GetImpl()->isEnhancedForm_Lock();
    }
    else if (  (nFeature & SfxShellFeature::FormTBControls)
            || (nFeature & SfxShellFeature::FormTBDesign)
            )
    {
        bResult = true;
    }

    return bResult;
}


void FmFormShell::Execute(SfxRequest &rReq)
{
    sal_uInt16 nSlot = rReq.GetSlot();


    // set MasterSlot
    switch( nSlot )
    {
        case SID_FM_PUSHBUTTON:
        case SID_FM_RADIOBUTTON:
        case SID_FM_CHECKBOX:
        case SID_FM_FIXEDTEXT:
        case SID_FM_GROUPBOX:
        case SID_FM_LISTBOX:
        case SID_FM_COMBOBOX:
        case SID_FM_NAVIGATIONBAR:
        case SID_FM_EDIT:
        case SID_FM_DBGRID:
        case SID_FM_IMAGEBUTTON:
        case SID_FM_IMAGECONTROL:
        case SID_FM_FILECONTROL:
        case SID_FM_DATEFIELD:
        case SID_FM_TIMEFIELD:
        case SID_FM_NUMERICFIELD:
        case SID_FM_CURRENCYFIELD:
        case SID_FM_PATTERNFIELD:
        case SID_FM_FORMATTEDFIELD:
        case SID_FM_SCROLLBAR:
        case SID_FM_SPINBUTTON:
            m_nLastSlot = nSlot;
            break;
    }


    // set the Identifier and Inventor of the Uno control
    sal_uInt16 nIdentifier = 0;
    switch( nSlot )
    {
        case SID_FM_CHECKBOX:
            nIdentifier = OBJ_FM_CHECKBOX;
            break;
        case SID_FM_PUSHBUTTON:
            nIdentifier = OBJ_FM_BUTTON;
            break;
        case SID_FM_FIXEDTEXT:
            nIdentifier = OBJ_FM_FIXEDTEXT;
            break;
        case SID_FM_LISTBOX:
            nIdentifier = OBJ_FM_LISTBOX;
            break;
        case SID_FM_EDIT:
            nIdentifier = OBJ_FM_EDIT;
            break;
        case SID_FM_RADIOBUTTON:
            nIdentifier = OBJ_FM_RADIOBUTTON;
            break;
        case SID_FM_GROUPBOX:
            nIdentifier = OBJ_FM_GROUPBOX;
            break;
        case SID_FM_COMBOBOX:
            nIdentifier = OBJ_FM_COMBOBOX;
            break;
        case SID_FM_NAVIGATIONBAR:
            nIdentifier = OBJ_FM_NAVIGATIONBAR;
            break;
        case SID_FM_DBGRID:
            nIdentifier = OBJ_FM_GRID;
            break;
        case SID_FM_IMAGEBUTTON:
            nIdentifier = OBJ_FM_IMAGEBUTTON;
            break;
        case SID_FM_IMAGECONTROL:
            nIdentifier = OBJ_FM_IMAGECONTROL;
            break;
        case SID_FM_FILECONTROL:
            nIdentifier = OBJ_FM_FILECONTROL;
            break;
        case SID_FM_DATEFIELD:
            nIdentifier = OBJ_FM_DATEFIELD;
            break;
        case SID_FM_TIMEFIELD:
            nIdentifier = OBJ_FM_TIMEFIELD;
            break;
        case SID_FM_NUMERICFIELD:
            nIdentifier = OBJ_FM_NUMERICFIELD;
            break;
        case SID_FM_CURRENCYFIELD:
            nIdentifier = OBJ_FM_CURRENCYFIELD;
            break;
        case SID_FM_PATTERNFIELD:
            nIdentifier = OBJ_FM_PATTERNFIELD;
            break;
        case SID_FM_FORMATTEDFIELD:
            nIdentifier = OBJ_FM_FORMATTEDFIELD;
            break;
        case SID_FM_SCROLLBAR:
            nIdentifier = OBJ_FM_SCROLLBAR;
            break;
        case SID_FM_SPINBUTTON:
            nIdentifier = OBJ_FM_SPINBUTTON;
            break;
    }

    switch ( nSlot )
    {
        case SID_FM_CHECKBOX:
        case SID_FM_PUSHBUTTON:
        case SID_FM_FIXEDTEXT:
        case SID_FM_LISTBOX:
        case SID_FM_EDIT:
        case SID_FM_RADIOBUTTON:
        case SID_FM_COMBOBOX:
        case SID_FM_NAVIGATIONBAR:
        case SID_FM_GROUPBOX:
        case SID_FM_DBGRID:
        case SID_FM_IMAGEBUTTON:
        case SID_FM_IMAGECONTROL:
        case SID_FM_FILECONTROL:
        case SID_FM_DATEFIELD:
        case SID_FM_TIMEFIELD:
        case SID_FM_NUMERICFIELD:
        case SID_FM_CURRENCYFIELD:
        case SID_FM_PATTERNFIELD:
        case SID_FM_FORMATTEDFIELD:
        case SID_FM_SCROLLBAR:
        case SID_FM_SPINBUTTON:
        {
            const SfxBoolItem* pGrabFocusItem = rReq.GetArg<SfxBoolItem>(SID_FM_TOGGLECONTROLFOCUS);
            if ( pGrabFocusItem && pGrabFocusItem->GetValue() )
            {   // see below
                SfxViewShell* pShell = GetViewShell();
                vcl::Window* pShellWnd = pShell ? pShell->GetWindow() : nullptr;
                if ( pShellWnd )
                    pShellWnd->GrabFocus();
                break;
            }

            SfxUInt16Item aIdentifierItem( SID_FM_CONTROL_IDENTIFIER, nIdentifier );
            SfxUInt32Item aInventorItem( SID_FM_CONTROL_INVENTOR, sal_uInt32(SdrInventor::FmForm) );
            const SfxPoolItem* pArgs[] =
            {
                &aIdentifierItem, &aInventorItem, nullptr
            };
            const SfxPoolItem* pInternalArgs[] =
            {
                nullptr
            };

            GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( SID_FM_CREATE_CONTROL, SfxCallMode::ASYNCHRON,
                                      pArgs, rReq.GetModifier(), pInternalArgs );

            if ( rReq.GetModifier() & KEY_MOD1 )
            {
                //  #99013# if selected with control key, return focus to current view
                // do this asynchron, so that the creation can be finished first
                // reusing the SID_FM_TOGGLECONTROLFOCUS is somewhat hacky ... which it wouldn't if it would have another
                // name, so I do not really have a big problem with this ....
                SfxBoolItem aGrabFocusIndicatorItem( SID_FM_TOGGLECONTROLFOCUS, true );
                GetViewShell()->GetViewFrame()->GetDispatcher()->ExecuteList(
                        nSlot, SfxCallMode::ASYNCHRON,
                        { &aGrabFocusIndicatorItem });
            }

            rReq.Done();
        }   break;
    }

    // individual actions
    switch( nSlot )
    {
        case SID_FM_FORM_DESIGN_TOOLS:
        {
            FormToolboxes aToolboxAccess(GetImpl()->getHostFrame_Lock());
            aToolboxAccess.toggleToolbox( nSlot );
            rReq.Done();
        }
        break;

        case SID_FM_TOGGLECONTROLFOCUS:
        {
            FmFormView* pFormView = GetFormView();
            if ( !pFormView )
                break;

            // if we execute this ourself, then either the application does not implement an own handling for this,
            // of we're on the top of the dispatcher stack, which means a control has the focus.
            // In the latter case, we put the focus to the document window, otherwise, we focus the first control
            const bool bHasControlFocus = GetImpl()->HasControlFocus_Lock();
            if ( bHasControlFocus )
            {
                if (m_pFormView)
                {
                    const OutputDevice* pDevice = m_pFormView->GetActualOutDev();
                    vcl::Window* pWindow = dynamic_cast< vcl::Window* >( const_cast< OutputDevice* >( pDevice ) );
                    if ( pWindow )
                        pWindow->GrabFocus();
                }
            }
            else
            {
                pFormView->GrabFirstControlFocus( );
            }
        }
        break;

        case SID_FM_VIEW_AS_GRID:
            GetImpl()->CreateExternalView_Lock();
            break;
        case SID_FM_CONVERTTO_EDIT          :
        case SID_FM_CONVERTTO_BUTTON            :
        case SID_FM_CONVERTTO_FIXEDTEXT     :
        case SID_FM_CONVERTTO_LISTBOX       :
        case SID_FM_CONVERTTO_CHECKBOX      :
        case SID_FM_CONVERTTO_RADIOBUTTON   :
        case SID_FM_CONVERTTO_GROUPBOX      :
        case SID_FM_CONVERTTO_COMBOBOX      :
        case SID_FM_CONVERTTO_IMAGEBUTTON   :
        case SID_FM_CONVERTTO_FILECONTROL   :
        case SID_FM_CONVERTTO_DATE          :
        case SID_FM_CONVERTTO_TIME          :
        case SID_FM_CONVERTTO_NUMERIC       :
        case SID_FM_CONVERTTO_CURRENCY      :
        case SID_FM_CONVERTTO_PATTERN       :
        case SID_FM_CONVERTTO_IMAGECONTROL  :
        case SID_FM_CONVERTTO_FORMATTED     :
        case SID_FM_CONVERTTO_SCROLLBAR     :
        case SID_FM_CONVERTTO_SPINBUTTON    :
        case SID_FM_CONVERTTO_NAVIGATIONBAR :
            GetImpl()->executeControlConversionSlot_Lock(FmXFormShell::SlotToIdent(nSlot));
            // after the conversion, re-determine the selection, since the
            // selected object has changed
            GetImpl()->SetSelection_Lock(GetFormView()->GetMarkedObjectList());
            break;
        case SID_FM_LEAVE_CREATE:
            m_nLastSlot = 0;
            rReq.Done();
            break;
        case SID_FM_SHOW_PROPERTY_BROWSER:
        {
            const SfxBoolItem* pShowItem = rReq.GetArg<SfxBoolItem>(SID_FM_SHOW_PROPERTIES);
            bool bShow = true;
            if ( pShowItem )
                bShow = pShowItem->GetValue();
            GetImpl()->ShowSelectionProperties_Lock(bShow);

            rReq.Done();
        } break;

        case SID_FM_PROPERTIES:
        {
            // display the PropertyBrowser
            const SfxBoolItem* pShowItem = rReq.GetArg<SfxBoolItem>(nSlot);
            bool bShow = pShowItem == nullptr || pShowItem->GetValue();

            InterfaceBag aOnlyTheForm;
            aOnlyTheForm.insert(Reference<XInterface>(GetImpl()->getCurrentForm_Lock(), UNO_QUERY));
            GetImpl()->setCurrentSelection_Lock(aOnlyTheForm);

            GetImpl()->ShowSelectionProperties_Lock(bShow);

            rReq.Done();
        }   break;

        case SID_FM_CTL_PROPERTIES:
        {
            const SfxBoolItem* pShowItem = rReq.GetArg<SfxBoolItem>(nSlot);
            bool bShow = pShowItem == nullptr || pShowItem->GetValue();

            OSL_ENSURE( GetImpl()->onlyControlsAreMarked_Lock(), "FmFormShell::Execute: ControlProperties should be disabled!" );
            if ( bShow )
                GetImpl()->selectLastMarkedControls_Lock();
            GetImpl()->ShowSelectionProperties_Lock(bShow);

            rReq.Done();
        }   break;
        case SID_FM_SHOW_PROPERTIES:
        case SID_FM_ADD_FIELD:
        case SID_FM_FILTER_NAVIGATOR:
        case SID_FM_SHOW_DATANAVIGATOR :
        {
            GetViewShell()->GetViewFrame()->ChildWindowExecute( rReq );
            rReq.Done();
        }   break;
        case SID_FM_SHOW_FMEXPLORER:
        {
            if (!m_pFormView)   // force setting the view
                GetViewShell()->GetViewFrame()->GetDispatcher()->Execute(SID_CREATE_SW_DRAWVIEW);

            GetViewShell()->GetViewFrame()->ChildWindowExecute(rReq);
            rReq.Done();
        }
        break;

        case SID_FM_TAB_DIALOG:
        {
            GetImpl()->ExecuteTabOrderDialog_Lock(
                Reference<XTabControllerModel>(GetImpl()->getCurrentForm_Lock(), UNO_QUERY));
            rReq.Done();
        }
        break;

        case SID_FM_DESIGN_MODE:
        {
            const SfxBoolItem* pDesignItem = rReq.GetArg<SfxBoolItem>(nSlot);
            bool bDesignMode = pDesignItem ? pDesignItem->GetValue() : !m_bDesignMode;
            SetDesignMode( bDesignMode );
            if ( m_bDesignMode == bDesignMode )
                rReq.Done();

            m_nLastSlot = SID_FM_DESIGN_MODE;
        }
        break;

        case SID_FM_AUTOCONTROLFOCUS:
        {
            FmFormModel* pModel = GetFormModel();
            DBG_ASSERT(pModel, "FmFormShell::Execute : invalid call !");
                // should have been disabled in GetState if we don't have a FormModel
            pModel->SetAutoControlFocus( !pModel->GetAutoControlFocus() );
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_AUTOCONTROLFOCUS);
        }
        break;
        case SID_FM_OPEN_READONLY:
        {
            FmFormModel* pModel = GetFormModel();
            DBG_ASSERT(pModel, "FmFormShell::Execute : invalid call !");
                // should have been disabled in GetState if we don't have a FormModel
            pModel->SetOpenInDesignMode( !pModel->GetOpenInDesignMode() );
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_OPEN_READONLY);
        }
        break;
        case SID_FM_USE_WIZARDS:
        {
            GetImpl()->SetWizardUsing_Lock(!GetImpl()->GetWizardUsing_Lock());
            GetViewShell()->GetViewFrame()->GetBindings().Invalidate(SID_FM_USE_WIZARDS);
        }
        break;
        case SID_FM_SEARCH:
        {
            const svx::ControllerFeatures& rController = GetImpl()->getActiveControllerFeatures_Lock();
            if ( rController->commitCurrentControl() && rController->commitCurrentRecord() )
                GetImpl()->ExecuteSearch_Lock();
            rReq.Done();
        }
        break;

        case SID_FM_RECORD_FIRST:
        case SID_FM_RECORD_PREV:
        case SID_FM_RECORD_NEXT:
        case SID_FM_RECORD_LAST:
        case SID_FM_RECORD_NEW:
        case SID_FM_REFRESH:
        case SID_FM_REFRESH_FORM_CONTROL:
        case SID_FM_RECORD_DELETE:
        case SID_FM_RECORD_UNDO:
        case SID_FM_RECORD_SAVE:
        case SID_FM_REMOVE_FILTER_SORT:
        case SID_FM_SORTDOWN:
        case SID_FM_SORTUP:
        case SID_FM_AUTOFILTER:
        case SID_FM_ORDERCRIT:
        case SID_FM_FORM_FILTERED:
        {
            GetImpl()->ExecuteFormSlot_Lock(nSlot);
            rReq.Done();
        }
        break;

        case SID_FM_RECORD_ABSOLUTE:
        {
            const svx::ControllerFeatures& rController = GetImpl()->getNavControllerFeatures_Lock();
            sal_Int32 nRecord = -1;

            const SfxItemSet* pArgs = rReq.GetArgs();
            if ( pArgs )
            {
                const SfxPoolItem* pItem;
                if ( ( pArgs->GetItemState( FN_PARAM_1, true, &pItem ) ) == SfxItemState::SET )
                {
                    const SfxInt32Item* pTypedItem = dynamic_cast<const SfxInt32Item* >( pItem );
                    if ( pTypedItem )
                        nRecord = std::max( pTypedItem->GetValue(), sal_Int32(0) );
                }
            }
            else
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ScopedVclPtr<AbstractFmInputRecordNoDialog> dlg(pFact->CreateFmInputRecordNoDialog(rReq.GetFrameWeld()));
                dlg->SetValue( rController->getCursor()->getRow() );
                if ( dlg->Execute() == RET_OK )
                    nRecord = dlg->GetValue();

                rReq.AppendItem( SfxInt32Item( FN_PARAM_1, nRecord ) );
            }

            if ( nRecord != -1 )
                rController->execute( nSlot, "Position", makeAny( nRecord ) );

            rReq.Done();
        }   break;
        case SID_FM_FILTER_EXECUTE:
        case SID_FM_FILTER_EXIT:
        {
            bool bCancelled = ( SID_FM_FILTER_EXIT == nSlot );
            bool bReopenNavigator = false;

            if ( !bCancelled )
            {
                // if the filter navigator is still open, we need to close it, so it can possibly
                // commit it's most recent changes
                if ( GetViewShell() && GetViewShell()->GetViewFrame() )
                    if ( GetViewShell()->GetViewFrame()->HasChildWindow( SID_FM_FILTER_NAVIGATOR ) )
                    {
                        GetViewShell()->GetViewFrame()->ToggleChildWindow( SID_FM_FILTER_NAVIGATOR );
                        bReopenNavigator = true;
                    }

                Reference<runtime::XFormController> const xController(GetImpl()->getActiveController_Lock());

                if  (   GetViewShell()->GetViewFrame()->HasChildWindow( SID_FM_FILTER_NAVIGATOR )
                        // closing the window was denied, for instance because of a invalid criterion

                    ||  (   xController.is()
                        &&  !GetImpl()->getActiveControllerFeatures_Lock()->commitCurrentControl()
                        )
                        // committing the controller was denied
                    )
                {
                    rReq.Done();
                    break;
                }
            }

            GetImpl()->stopFiltering_Lock(!bCancelled);
            rReq.Done();

            if ( bReopenNavigator )
                // we closed the navigator only to implicitly commit it (as we do not have another
                // direct wire to it), but to the user, it should look as it was always open
                GetViewShell()->GetViewFrame()->ToggleChildWindow( SID_FM_FILTER_NAVIGATOR );
        }
        break;

        case SID_FM_FILTER_START:
        {
            GetImpl()->startFiltering_Lock();
            rReq.Done();

            // initially open the filter navigator, the whole form based filter is pretty useless without it
            SfxBoolItem aIdentifierItem( SID_FM_FILTER_NAVIGATOR, true );
            GetViewShell()->GetViewFrame()->GetDispatcher()->ExecuteList(
                    SID_FM_FILTER_NAVIGATOR, SfxCallMode::ASYNCHRON,
                    { &aIdentifierItem });
        }   break;
    }
}


void FmFormShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();
    while ( nWhich )
    {
        switch( nWhich )
        {
            case SID_FM_FORM_DESIGN_TOOLS:
            {
                FormToolboxes aToolboxAccess(GetImpl()->getHostFrame_Lock());
                rSet.Put( SfxBoolItem( nWhich, aToolboxAccess.isToolboxVisible( nWhich ) ) );
            }
            break;

            case SID_FM_FILTER_EXECUTE:
            case SID_FM_FILTER_EXIT:
                if (!GetImpl()->isInFilterMode_Lock())
                    rSet.DisableItem( nWhich );
                break;

            case SID_FM_USE_WIZARDS:
                if  ( !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::EModule::DATABASE ) )
                    rSet.Put( SfxVisibilityItem( nWhich, false ) );
                else if (!GetFormModel())
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put(SfxBoolItem(nWhich, GetImpl()->GetWizardUsing_Lock()));
                break;
            case SID_FM_AUTOCONTROLFOCUS:
                if (!GetFormModel())
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem(nWhich, GetFormModel()->GetAutoControlFocus() ) );
                break;
            case SID_FM_OPEN_READONLY:
                if (!GetFormModel())
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem(nWhich, GetFormModel()->GetOpenInDesignMode() ) );
                break;

            case SID_FM_NAVIGATIONBAR:
            case SID_FM_DBGRID:
                if ( !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::EModule::DATABASE ) )
                {
                    rSet.Put( SfxVisibilityItem( nWhich, false ) );
                    break;
                }
                [[fallthrough]];

            case SID_FM_SCROLLBAR:
            case SID_FM_IMAGECONTROL:
            case SID_FM_FILECONTROL:
            case SID_FM_CURRENCYFIELD:
            case SID_FM_PATTERNFIELD:
            case SID_FM_IMAGEBUTTON:
            case SID_FM_RADIOBUTTON:
            case SID_FM_COMBOBOX:
            case SID_FM_GROUPBOX:
            case SID_FM_CHECKBOX:
            case SID_FM_PUSHBUTTON:
            case SID_FM_FIXEDTEXT:
            case SID_FM_LISTBOX:
            case SID_FM_EDIT:
            case SID_FM_DATEFIELD:
            case SID_FM_TIMEFIELD:
            case SID_FM_NUMERICFIELD:
            case SID_FM_FORMATTEDFIELD:
            case SID_FM_SPINBUTTON:
                if (!m_bDesignMode)
                    rSet.DisableItem( nWhich );
                else
                {
                    bool bLayerLocked = false;
                    if (m_pFormView)
                    {
                        // If the css::drawing::Layer is locked, the slots must be disabled. #36897
                        SdrPageView* pPV = m_pFormView->GetSdrPageView();
                        if (pPV != nullptr)
                            bLayerLocked = pPV->IsLayerLocked(m_pFormView->GetActiveLayer());
                    }
                    if (bLayerLocked)
                        rSet.DisableItem( nWhich );
                    else
                        rSet.Put( SfxBoolItem(nWhich, (nWhich==m_nLastSlot)) );
                }
                break;
            case SID_FM_FILTER_NAVIGATOR_CONTROL:
            {
                if (GetImpl()->isInFilterMode_Lock())
                    rSet.Put(SfxObjectItem(nWhich, this));
                else
                    rSet.Put(SfxObjectItem(nWhich));
            }   break;
            case SID_FM_FIELDS_CONTROL:
            case SID_FM_PROPERTY_CONTROL:
            {
                if (!m_bDesignMode || !m_pFormView || !m_bHasForms)
                    rSet.Put(SfxObjectItem(nWhich));
                else
                    rSet.Put(SfxObjectItem(nWhich, this));

            }   break;
            case SID_FM_FMEXPLORER_CONTROL:
            case SID_FM_DATANAVIGATOR_CONTROL :
            {
                if (!m_bDesignMode || !m_pFormView)
                    rSet.Put(SfxObjectItem(nWhich));
                else
                    rSet.Put(SfxObjectItem(nWhich, this));

            }   break;
            case SID_FM_ADD_FIELD:
            case SID_FM_SHOW_FMEXPLORER:
            case SID_FM_SHOW_PROPERTIES:
            case SID_FM_FILTER_NAVIGATOR:
            case SID_FM_SHOW_DATANAVIGATOR:
            {
                if ( GetViewShell()->GetViewFrame()->KnowsChildWindow(nWhich) )
                    rSet.Put( SfxBoolItem( nWhich, GetViewShell()->GetViewFrame()->HasChildWindow(nWhich)) );
                else
                    rSet.DisableItem(nWhich);
            }   break;

            case SID_FM_SHOW_PROPERTY_BROWSER:
            {
                rSet.Put(SfxBoolItem(nWhich, GetImpl()->IsPropBrwOpen_Lock()));
            }
            break;

            case SID_FM_CTL_PROPERTIES:
            {
                // potentially, give the Impl the opportunity to update its
                // current objects which are aligned with the current MarkList
                if (GetImpl()->IsSelectionUpdatePending_Lock())
                    GetImpl()->ForceUpdateSelection_Lock();

                if (!m_pFormView || !m_bDesignMode || !GetImpl()->onlyControlsAreMarked_Lock())
                    rSet.DisableItem( nWhich );
                else
                {
                    bool const bChecked = GetImpl()->IsPropBrwOpen_Lock() && !GetImpl()->isSolelySelected_Lock(GetImpl()->getCurrentForm_Lock());
                        // if the property browser is open, and only controls are marked, and the current selection
                        // does not consist of only the current form, then the current selection is the (composition of)
                        // the currently marked controls
                    rSet.Put( SfxBoolItem( nWhich, bChecked ) );
                }
            }   break;

            case SID_FM_PROPERTIES:
            {
                // potentially, give the Impl the opportunity to update its
                // current objects which are aligned with the current MarkList
                if (GetImpl()->IsSelectionUpdatePending_Lock())
                    GetImpl()->ForceUpdateSelection_Lock();

                if (!m_pFormView || !m_bDesignMode || !GetImpl()->getCurrentForm_Lock().is())
                    rSet.DisableItem( nWhich );
                else
                {
                    bool const bChecked = GetImpl()->IsPropBrwOpen_Lock() && GetImpl()->isSolelySelected_Lock(GetImpl()->getCurrentForm_Lock());
                    rSet.Put(SfxBoolItem(nWhich, bChecked));
                }
            }   break;
            case SID_FM_TAB_DIALOG:
                // potentially, give the Impl the opportunity to update its
                // current objects which are aligned with the current MarkList
                if (GetImpl()->IsSelectionUpdatePending_Lock())
                    GetImpl()->ForceUpdateSelection_Lock();

                if (!m_pFormView || !m_bDesignMode || !GetImpl()->getCurrentForm_Lock().is() )
                    rSet.DisableItem( nWhich );
                break;
            case SID_FM_DESIGN_MODE:
                if (!m_pFormView || GetImpl()->IsReadonlyDoc_Lock())
                    rSet.DisableItem( nWhich );
                else
                    rSet.Put( SfxBoolItem(nWhich, m_bDesignMode) );
                break;
            case SID_FM_SEARCH:
            case SID_FM_RECORD_FIRST:
            case SID_FM_RECORD_NEXT:
            case SID_FM_RECORD_PREV:
            case SID_FM_RECORD_LAST:
            case SID_FM_RECORD_NEW:
            case SID_FM_RECORD_DELETE:
            case SID_FM_RECORD_ABSOLUTE:
            case SID_FM_RECORD_TOTAL:
            case SID_FM_RECORD_SAVE:
            case SID_FM_RECORD_UNDO:
            case SID_FM_FORM_FILTERED:
            case SID_FM_REMOVE_FILTER_SORT:
            case SID_FM_SORTUP:
            case SID_FM_SORTDOWN:
            case SID_FM_ORDERCRIT:
            case SID_FM_FILTER_START:
            case SID_FM_AUTOFILTER:
            case SID_FM_REFRESH:
            case SID_FM_REFRESH_FORM_CONTROL:
            case SID_FM_VIEW_AS_GRID:
                GetFormState(rSet,nWhich);
                break;

            case SID_FM_CHANGECONTROLTYPE:
            {
                if ( !m_pFormView || !m_bDesignMode )
                    rSet.DisableItem( nWhich );
                else
                {
                    if (!GetImpl()->canConvertCurrentSelectionToControl_Lock("ConvertToFixed"))
                        // if it cannot be converted to a fixed text, it is no single control
                        rSet.DisableItem( nWhich );
                }
            } break;

            case SID_FM_CONVERTTO_FILECONTROL   :
            case SID_FM_CONVERTTO_CURRENCY      :
            case SID_FM_CONVERTTO_PATTERN       :
            case SID_FM_CONVERTTO_IMAGECONTROL  :
            case SID_FM_CONVERTTO_SCROLLBAR     :
            case SID_FM_CONVERTTO_NAVIGATIONBAR :
            case SID_FM_CONVERTTO_IMAGEBUTTON   :
            case SID_FM_CONVERTTO_EDIT          :
            case SID_FM_CONVERTTO_BUTTON        :
            case SID_FM_CONVERTTO_FIXEDTEXT     :
            case SID_FM_CONVERTTO_LISTBOX       :
            case SID_FM_CONVERTTO_CHECKBOX      :
            case SID_FM_CONVERTTO_RADIOBUTTON   :
            case SID_FM_CONVERTTO_GROUPBOX      :
            case SID_FM_CONVERTTO_COMBOBOX      :
            case SID_FM_CONVERTTO_DATE          :
            case SID_FM_CONVERTTO_TIME          :
            case SID_FM_CONVERTTO_NUMERIC       :
            case SID_FM_CONVERTTO_FORMATTED     :
            case SID_FM_CONVERTTO_SPINBUTTON    :
            {
                if (!m_pFormView || !m_bDesignMode || !GetImpl()->canConvertCurrentSelectionToControl_Lock(FmXFormShell::SlotToIdent(nWhich)))
                    rSet.DisableItem( nWhich );
                else
                {
                    rSet.Put( SfxBoolItem( nWhich, false ) );
                    // just to have a defined state (available and not checked)
                }
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}


void FmFormShell::GetFormState(SfxItemSet &rSet, sal_uInt16 nWhich)
{
    if  (   !GetImpl()->getNavController_Lock().is()
        ||  !isRowSetAlive(GetImpl()->getNavController_Lock()->getModel())
        ||  !m_pFormView
        ||  m_bDesignMode
        ||  !GetImpl()->getActiveForm_Lock().is()
        ||  GetImpl()->isInFilterMode_Lock()
        )
        rSet.DisableItem(nWhich);
    else
    {
        bool bEnable = false;
        try
        {
            switch (nWhich)
            {
            case SID_FM_VIEW_AS_GRID:
                if (GetImpl()->getHostFrame_Lock().is() && GetImpl()->getNavController_Lock().is())
                {
                    bEnable = true;
                    bool bDisplayingCurrent =
                        GetImpl()->getInternalForm_Lock(
                            Reference<XForm>(GetImpl()->getNavController_Lock()->getModel(), UNO_QUERY)
                        ) == GetImpl()->getExternallyDisplayedForm_Lock();
                    rSet.Put(SfxBoolItem(nWhich, bDisplayingCurrent));
                }
                break;

            case SID_FM_SEARCH:
            {
                Reference<css::beans::XPropertySet> const xNavSet(GetImpl()->getActiveForm_Lock(), UNO_QUERY);
                sal_Int32 nCount = ::comphelper::getINT32(xNavSet->getPropertyValue(FM_PROP_ROWCOUNT));
                bEnable = nCount != 0;
            }   break;
            case SID_FM_RECORD_ABSOLUTE:
            case SID_FM_RECORD_TOTAL:
            {
                FeatureState aState;
                GetImpl()->getNavControllerFeatures_Lock()->getState( nWhich, aState );
                if ( SID_FM_RECORD_ABSOLUTE == nWhich )
                {
                    sal_Int32 nPosition = 0;
                    aState.State >>= nPosition;
                    rSet.Put( SfxInt32Item( nWhich, nPosition ) );
                }
                else if ( SID_FM_RECORD_TOTAL == nWhich )
                {
                    OUString sTotalCount;
                    aState.State >>= sTotalCount;
                    rSet.Put( SfxStringItem( nWhich, sTotalCount ) );
                }
                bEnable = aState.Enabled;
            }
            break;

            // first, prev, next, last, and absolute affect the nav controller, not the
            // active controller
            case SID_FM_RECORD_FIRST:
            case SID_FM_RECORD_PREV:
            case SID_FM_RECORD_NEXT:
            case SID_FM_RECORD_LAST:
            case SID_FM_RECORD_NEW:
            case SID_FM_RECORD_SAVE:
            case SID_FM_RECORD_UNDO:
            case SID_FM_RECORD_DELETE:
            case SID_FM_REFRESH:
            case SID_FM_REFRESH_FORM_CONTROL:
            case SID_FM_REMOVE_FILTER_SORT:
            case SID_FM_SORTUP:
            case SID_FM_SORTDOWN:
            case SID_FM_AUTOFILTER:
            case SID_FM_ORDERCRIT:
                bEnable = GetImpl()->IsFormSlotEnabled( nWhich, nullptr );
                break;

            case SID_FM_FORM_FILTERED:
            {
                FeatureState aState;
                bEnable = GetImpl()->IsFormSlotEnabled( nWhich, &aState );

                rSet.Put( SfxBoolItem( nWhich, ::comphelper::getBOOL( aState.State ) ) );
            }
            break;

            case SID_FM_FILTER_START:
                bEnable = GetImpl()->getActiveControllerFeatures_Lock()->canDoFormFilter();
                break;
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmFormShell::GetFormState: caught an exception while determining the state!" );
        }
        if (!bEnable)
            rSet.DisableItem(nWhich);
    }
}


FmFormPage* FmFormShell::GetCurPage() const
{
    FmFormPage* pP = nullptr;
    if (m_pFormView && m_pFormView->GetSdrPageView())
        pP = dynamic_cast<FmFormPage*>( m_pFormView->GetSdrPageView()->GetPage() );
    return pP;
}


void FmFormShell::SetView( FmFormView* _pView )
{
    if ( m_pFormView )
    {
        if ( IsActive() )
            GetImpl()->viewDeactivated_Lock(*m_pFormView);

        m_pFormView->SetFormShell( nullptr, FmFormView::FormShellAccess() );
        m_pFormView = nullptr;
        m_pFormModel = nullptr;
    }

    if ( !_pView )
        return;

    m_pFormView = _pView;
    m_pFormView->SetFormShell( this, FmFormView::FormShellAccess() );
    m_pFormModel = static_cast<FmFormModel*>(m_pFormView->GetModel());

    impl_setDesignMode( m_pFormView->IsDesignMode() );

    // We activate our view if we are activated ourself, but sometimes the Activate precedes the SetView.
    // But here we know both the view and our activation state so we at least are able to pass the latter
    // to the former.
    // FS - 30.06.99 - 67308
    if ( IsActive() )
        GetImpl()->viewActivated_Lock(*m_pFormView);
}


void FmFormShell::DetermineForms(bool bInvalidate)
{
    // are there forms on the current page
    bool bForms = GetImpl()->hasForms_Lock();
    if (bForms != m_bHasForms)
    {
        m_bHasForms = bForms;
        if (bInvalidate)
            UIFeatureChanged();
    }
}


bool FmFormShell::GetY2KState(sal_uInt16& nReturn)
{
    return GetImpl()->GetY2KState_Lock(nReturn);
}


void FmFormShell::SetY2KState(sal_uInt16 n)
{
    GetImpl()->SetY2KState_Lock(n);
}


void FmFormShell::Activate(bool bMDI)
{
    SfxShell::Activate(bMDI);

    if ( m_pFormView )
        GetImpl()->viewActivated_Lock(*m_pFormView, true);
}


void FmFormShell::Deactivate(bool bMDI)
{
    SfxShell::Deactivate(bMDI);

    if ( m_pFormView )
        GetImpl()->viewDeactivated_Lock(*m_pFormView, false);
}


void FmFormShell::ExecuteTextAttribute( SfxRequest& _rReq )
{
    m_pImpl->ExecuteTextAttribute_Lock(_rReq);
}


void FmFormShell::GetTextAttributeState( SfxItemSet& _rSet )
{
    m_pImpl->GetTextAttributeState_Lock(_rSet);
}


bool FmFormShell::IsActiveControl() const
{
    return m_pImpl->IsActiveControl_Lock(false);
}


void FmFormShell::ForgetActiveControl()
{
    m_pImpl->ForgetActiveControl_Lock();
}


void FmFormShell::SetControlActivationHandler( const Link<LinkParamNone*,void>& _rHdl )
{
    m_pImpl->SetControlActivationHandler_Lock(_rHdl);
}


namespace
{
    SdrUnoObj* lcl_findUnoObject( const SdrObjList& _rObjList, const Reference< XControlModel >& _rxModel )
    {
        SdrObjListIter aIter( &_rObjList );
        while ( aIter.IsMore() )
        {
            SdrObject* pObject = aIter.Next();
            SdrUnoObj* pUnoObject = dynamic_cast<SdrUnoObj*>( pObject  );
            if ( !pUnoObject )
                continue;

            Reference< XControlModel > xControlModel = pUnoObject->GetUnoControlModel();
            if ( !xControlModel.is() )
                continue;

            if ( _rxModel == xControlModel )
                return pUnoObject;
        }
        return nullptr;
    }
}


void FmFormShell::ToggleControlFocus( const SdrUnoObj& i_rUnoObject, const SdrView& i_rView, OutputDevice& i_rDevice ) const
{
    try
    {
        // check if the focus currently is in a control
        // Well, okay, do it the other way 'round: Check whether the current control of the active controller
        // actually has the focus. This should be equivalent.
        const bool bHasControlFocus = GetImpl()->HasControlFocus_Lock();

        if ( bHasControlFocus )
        {
            vcl::Window* pWindow( dynamic_cast< vcl::Window* >( &i_rDevice ) );
            OSL_ENSURE( pWindow, "FmFormShell::ToggleControlFocus: I need a Window, really!" );
            if ( pWindow )
                pWindow->GrabFocus();
        }
        else
        {
            Reference< XControl > xControl;
            GetFormControl( i_rUnoObject.GetUnoControlModel(), i_rView, i_rDevice, xControl );
            Reference< XWindow > xControlWindow( xControl, UNO_QUERY );
            if ( xControlWindow.is() )
                xControlWindow->setFocus();
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
}


namespace
{
    class FocusableControlsFilter : public svx::ISdrObjectFilter
    {
    public:
        FocusableControlsFilter( const SdrView& i_rView, const OutputDevice& i_rDevice )
            :m_rView( i_rView )
            ,m_rDevice( i_rDevice )
        {
        }

    public:
        virtual bool    includeObject( const SdrObject& i_rObject ) const override
        {
            const SdrUnoObj* pUnoObj = dynamic_cast< const SdrUnoObj* >( &i_rObject );
            if ( !pUnoObj )
                return false;

            Reference< XControl > xControl = pUnoObj->GetUnoControl( m_rView, m_rDevice );
            return FmXFormView::isFocusable( xControl );
        }

    private:
        const SdrView&      m_rView;
        const OutputDevice& m_rDevice;
    };
}


::std::unique_ptr< svx::ISdrObjectFilter > FmFormShell::CreateFocusableControlFilter( const SdrView& i_rView, const OutputDevice& i_rDevice )
{
    ::std::unique_ptr< svx::ISdrObjectFilter > pFilter;

    if ( !i_rView.IsDesignMode() )
        pFilter.reset( new FocusableControlsFilter( i_rView, i_rDevice ) );

    return pFilter;
}


SdrUnoObj* FmFormShell::GetFormControl( const Reference< XControlModel >& _rxModel, const SdrView& _rView, const OutputDevice& _rDevice, Reference< XControl >& _out_rxControl ) const
{
    if ( !_rxModel.is() )
        return nullptr;

    // we can only retrieve controls for SdrObjects which belong to page which is actually displayed in the given view
    SdrPageView* pPageView = _rView.GetSdrPageView();
    SdrPage* pPage = pPageView ? pPageView->GetPage() : nullptr;
    OSL_ENSURE( pPage, "FmFormShell::GetFormControl: no page displayed in the given view!" );
    if ( !pPage )
        return nullptr;

    SdrUnoObj* pUnoObject = lcl_findUnoObject( *pPage, _rxModel );
    if ( pUnoObject )
    {
        _out_rxControl = pUnoObject->GetUnoControl( _rView, _rDevice );
        return pUnoObject;
    }

#if OSL_DEBUG_LEVEL > 0
    // perhaps we are fed with a control model which lives on a page other than the one displayed
    // in the given view. This is worth being reported as error, in non-product builds.
    FmFormModel* pModel = GetFormModel();
    if ( pModel )
    {
        sal_uInt16 pageCount = pModel->GetPageCount();
        for ( sal_uInt16 page = 0; page < pageCount; ++page )
        {
            pPage = pModel->GetPage( page );
            OSL_ENSURE( pPage, "FmFormShell::GetFormControl: NULL page encountered!" );
            if  ( !pPage )
                continue;

            pUnoObject = lcl_findUnoObject( *pPage, _rxModel );
            OSL_ENSURE( !pUnoObject, "FmFormShell::GetFormControl: the given control model belongs to a wrong page (displayed elsewhere)!" );
        }
    }
#else
    (void) this; // avoid loplugin:staticmethods
#endif

    return nullptr;
}


Reference< runtime::XFormController > FmFormShell::GetFormController( const Reference< XForm >& _rxForm, const SdrView& _rView, const OutputDevice& _rDevice )
{
    const FmFormView* pFormView = dynamic_cast< const FmFormView* >( &_rView );
    if ( !pFormView )
        return nullptr;

    return pFormView->GetFormController( _rxForm, _rDevice );
}


void FmFormShell::SetDesignMode( bool _bDesignMode )
{
    if ( _bDesignMode == m_bDesignMode )
        return;

    FmFormModel* pModel = GetFormModel();
    if (pModel)
        // Switch off the undo environment for the time of the transition. This ensures that
        // one can also change non-transient properties there. (It should be done with
        // caution, however, and it should always be reversed when one switches the mode back.
        // An example is the setting of the maximum text length by the OEditModel on its control.)
        pModel->GetUndoEnv().Lock();

    // then the actual switch
    if ( m_bDesignMode || PrepareClose() )
        impl_setDesignMode(!m_bDesignMode );

    // and my undo environment back on
    if ( pModel )
        pModel->GetUndoEnv().UnLock();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
