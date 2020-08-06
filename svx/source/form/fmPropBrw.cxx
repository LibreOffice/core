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


#include <sal/macros.h>

#include <fmprop.hxx>
#include <fmPropBrw.hxx>
#include <svx/strings.hrc>
#include <fmservs.hxx>
#include <fmshimp.hxx>
#include <fmpgeimp.hxx>

#include <svx/dialmgr.hxx>
#include <svx/fmpage.hxx>
#include <svx/fmshell.hxx>
#include <svx/fmview.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>

#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/form/XForm.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/form/inspection/DefaultFormComponentInspectorModel.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/inspection/ObjectInspector.hpp>
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#include <com/sun/star/inspection/DefaultHelpProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/VetoException.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/component_context.hxx>
#include <o3tl/deleter.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/objitem.hxx>
#include <sfx2/objsh.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/confignode.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::form::inspection;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::inspection;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::svxform;
using ::com::sun::star::awt::XWindow;

//= FmPropBrwMgr
SFX_IMPL_MODELESSDIALOGCONTOLLER(FmPropBrwMgr, SID_FM_SHOW_PROPERTIES)

FmPropBrwMgr::FmPropBrwMgr( vcl::Window* _pParent, sal_uInt16 _nId,
                            SfxBindings* _pBindings, SfxChildWinInfo* _pInfo)
              :SfxChildWindow(_pParent, _nId)
{
    std::shared_ptr<FmPropBrw> xControl(new FmPropBrw(::comphelper::getProcessComponentContext(), _pBindings,
                                                         this, _pParent->GetFrameWeld(), _pInfo), o3tl::default_delete<FmPropBrw>());
    SetController(std::move(xControl));
    static_cast<FmPropBrw*>(GetController().get())->Initialize( _pInfo );
}

static OUString GetUIHeadlineName(sal_Int16 nClassId, const Any& aUnoObj)
{
    const char* pClassNameResourceId = nullptr;

    switch ( nClassId )
    {
        case FormComponentType::TEXTFIELD:
        {
            Reference< XInterface >  xIFace;
            aUnoObj >>= xIFace;
            pClassNameResourceId = RID_STR_PROPTITLE_EDIT;
            if (xIFace.is())
            {   // we have a chance to check if it's a formatted field model
                Reference< XServiceInfo >  xInfo(xIFace, UNO_QUERY);
                if (xInfo.is() && (xInfo->supportsService(FM_SUN_COMPONENT_FORMATTEDFIELD)))
                    pClassNameResourceId = RID_STR_PROPTITLE_FORMATTED;
                else if (!xInfo.is())
                {
                    // couldn't distinguish between formatted and edit with the service name, so try with the properties
                    Reference< XPropertySet >  xProps(xIFace, UNO_QUERY);
                    if (xProps.is())
                    {
                        Reference< XPropertySetInfo >  xPropsInfo = xProps->getPropertySetInfo();
                        if (xPropsInfo.is() && xPropsInfo->hasPropertyByName(FM_PROP_FORMATSSUPPLIER))
                            pClassNameResourceId = RID_STR_PROPTITLE_FORMATTED;
                    }
                }
            }
        }
        break;

        case FormComponentType::COMMANDBUTTON:
            pClassNameResourceId = RID_STR_PROPTITLE_PUSHBUTTON; break;
        case FormComponentType::RADIOBUTTON:
            pClassNameResourceId = RID_STR_PROPTITLE_RADIOBUTTON; break;
        case FormComponentType::CHECKBOX:
            pClassNameResourceId = RID_STR_PROPTITLE_CHECKBOX; break;
        case FormComponentType::LISTBOX:
            pClassNameResourceId = RID_STR_PROPTITLE_LISTBOX; break;
        case FormComponentType::COMBOBOX:
            pClassNameResourceId = RID_STR_PROPTITLE_COMBOBOX; break;
        case FormComponentType::GROUPBOX:
            pClassNameResourceId = RID_STR_PROPTITLE_GROUPBOX; break;
        case FormComponentType::IMAGEBUTTON:
            pClassNameResourceId = RID_STR_PROPTITLE_IMAGEBUTTON; break;
        case FormComponentType::FIXEDTEXT:
            pClassNameResourceId = RID_STR_PROPTITLE_FIXEDTEXT; break;
        case FormComponentType::GRIDCONTROL:
            pClassNameResourceId = RID_STR_PROPTITLE_DBGRID; break;
        case FormComponentType::FILECONTROL:
            pClassNameResourceId = RID_STR_PROPTITLE_FILECONTROL; break;
        case FormComponentType::DATEFIELD:
            pClassNameResourceId = RID_STR_PROPTITLE_DATEFIELD; break;
        case FormComponentType::TIMEFIELD:
            pClassNameResourceId = RID_STR_PROPTITLE_TIMEFIELD; break;
        case FormComponentType::NUMERICFIELD:
            pClassNameResourceId = RID_STR_PROPTITLE_NUMERICFIELD; break;
        case FormComponentType::CURRENCYFIELD:
            pClassNameResourceId = RID_STR_PROPTITLE_CURRENCYFIELD; break;
        case FormComponentType::PATTERNFIELD:
            pClassNameResourceId = RID_STR_PROPTITLE_PATTERNFIELD; break;
        case FormComponentType::IMAGECONTROL:
            pClassNameResourceId = RID_STR_PROPTITLE_IMAGECONTROL; break;
        case FormComponentType::HIDDENCONTROL:
            pClassNameResourceId = RID_STR_PROPTITLE_HIDDEN; break;
        case FormComponentType::SCROLLBAR:
            pClassNameResourceId = RID_STR_PROPTITLE_SCROLLBAR; break;
        case FormComponentType::SPINBUTTON:
            pClassNameResourceId = RID_STR_PROPTITLE_SPINBUTTON; break;
        case FormComponentType::NAVIGATIONBAR:
            pClassNameResourceId = RID_STR_PROPTITLE_NAVBAR; break;
        case FormComponentType::CONTROL:
        default:
            pClassNameResourceId = RID_STR_CONTROL; break;
    }

    return SvxResId(pClassNameResourceId);
}

FmPropBrw::FmPropBrw(const Reference< XComponentContext >& _xORB, SfxBindings* _pBindings,
                     SfxChildWindow* _pMgr, weld::Window* _pParent, const SfxChildWinInfo* _pInfo)
    : SfxModelessDialogController(_pBindings, _pMgr, _pParent, "svx/ui/formpropertydialog.ui", "FormPropertyDialog")
    , SfxControllerItem(SID_FM_PROPERTY_CONTROL, *_pBindings)
    , m_bInitialStateChange(true)
    , m_pParent(_pParent)
    , m_nAsyncGetFocusId(nullptr)
    , m_xContainer(m_xBuilder->weld_container("container"))
    , m_xORB(_xORB)
{
    m_xContainer->set_size_request(m_xContainer->get_approximate_digit_width() * 72, m_xContainer->get_text_height() * 20);

    try
    {
        // create a frame wrapper for myself
        m_xMeAsFrame = Frame::create(m_xORB);

        // transport the container area of this dialog to be the container window of the frame
        css::uno::Reference<css::awt::XWindow> xFrameContainerWindow(new weld::TransportAsXWindow(m_xContainer.get()));
        m_xMeAsFrame->initialize(xFrameContainerWindow);
        m_xMeAsFrame->setName("form property browser");
    }
    catch (const Exception&)
    {
        OSL_FAIL("FmPropBrw::FmPropBrw: could not create/initialize my frame!");
        m_xMeAsFrame.clear();
    }

    if ( _pInfo )
        m_sLastActivePage = _pInfo->aExtraString;
}

FmPropBrw::~FmPropBrw()
{
    if (m_nAsyncGetFocusId)
    {
        Application::RemoveUserEvent(m_nAsyncGetFocusId);
        m_nAsyncGetFocusId = nullptr;
    }

    if (m_xBrowserController.is())
        implDetachController();
    try
    {
        // remove our own properties from the component context. We cannot ensure that the component context
        // is freed (there might be refcount problems :-\), so at least ensure the context itself
        // does hold the objects anymore
        Reference<XNameContainer> xName(m_xInspectorContext,uno::UNO_QUERY);
        if ( xName.is() )
        {
            const OUString pProps[] = { OUString( "ContextDocument" )
                                             , OUString( "DialogParentWindow" )
                                             , OUString( "ControlContext" )
                                             , OUString( "ControlShapeAccess" ) };
            for (const auto & i : pProps)
                xName->removeByName( i );
        }
    }
    catch (const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
    ::SfxControllerItem::dispose();
}

OUString FmPropBrw::getCurrentPage() const
{
    OUString sCurrentPage;
    try
    {
        if ( m_xBrowserController.is() )
        {
            OSL_VERIFY( m_xBrowserController->getViewData() >>= sCurrentPage );
        }

        if ( sCurrentPage.isEmpty() )
            sCurrentPage = m_sLastActivePage;
    }
    catch( const Exception& )
    {
        OSL_FAIL( "FmPropBrw::getCurrentPage: caught an exception while retrieving the current page!" );
    }
    return sCurrentPage;
}

void FmPropBrw::implDetachController()
{
    m_sLastActivePage = getCurrentPage();

    implSetNewSelection( InterfaceBag() );

    if ( m_xMeAsFrame.is() )
    {
        try
        {
            m_xMeAsFrame->setComponent(nullptr, nullptr);
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmPropBrw::implDetachController: caught an exception while resetting the component!" );
        }
    }

    // we attached a frame to the controller manually, so we need to manually tell it that it's detached, too
    if ( m_xBrowserController.is() )
    {
        m_xBrowserController->attachFrame( nullptr );
    }

    m_xBrowserController.clear();
    m_xInspectorModel.clear();
    m_xMeAsFrame.clear();
}

void FmPropBrw::Close()
{
    // suspend the controller (it is allowed to veto)
    if ( m_xMeAsFrame.is() )
    {
        try
        {
            Reference< XController > xController( m_xMeAsFrame->getController() );
            if ( xController.is() && !xController->suspend( true ) )
                return;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmPropBrw::Close: caught an exception while asking the controller!" );
        }
    }

    implDetachController();

    // remember our bindings: while we're closed, we're deleted, too, so accessing the bindings after this
    // would be deadly
    // 10/19/00 - 79321 - FS
    SfxBindings& rBindings = SfxControllerItem::GetBindings();

    SfxModelessDialogController::Close();

    rBindings.Invalidate(SID_FM_CTL_PROPERTIES);
    rBindings.Invalidate(SID_FM_PROPERTIES);
}

bool FmPropBrw::implIsReadOnlyModel() const
{
    try
    {
        if ( m_xInspectorModel.is() )
            return m_xInspectorModel->getIsReadOnly();
        return false;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
    return true;
}


void FmPropBrw::implSetNewSelection( const InterfaceBag& _rSelection )
{
    if ( !m_xBrowserController.is() )
        return;

    try
    {
        Reference< XObjectInspector > xInspector( m_xBrowserController, UNO_QUERY_THROW );

        // tell it the objects to inspect
        xInspector->inspect( comphelper::containerToSequence(_rSelection) );
    }
    catch( const VetoException& )
    {
        return;
    }
    catch( const Exception& )
    {
        OSL_FAIL( "FmPropBrw::implSetNewSelection: caught an unexpected exception!" );
        return;
    }

    // set the new title according to the selected object
    OUString sTitle;

    if ( _rSelection.empty() )
    {
        sTitle = SvxResId(RID_STR_NO_PROPERTIES);
    }
    else if ( _rSelection.size() > 1 )
    {
        // no form component and (no form or no name) -> Multiselection
        sTitle = SvxResId(RID_STR_PROPERTIES_CONTROL) +
            SvxResId(RID_STR_PROPTITLE_MULTISELECT);
    }
    else
    {
        Reference< XPropertySet > xSingleSelection( *_rSelection.begin(), UNO_QUERY);
        if  ( ::comphelper::hasProperty( FM_PROP_CLASSID, xSingleSelection ) )
        {
            sal_Int16 nClassID = FormComponentType::CONTROL;
            xSingleSelection->getPropertyValue( FM_PROP_CLASSID ) >>= nClassID;

            sTitle = SvxResId(RID_STR_PROPERTIES_CONTROL) +
                GetUIHeadlineName(nClassID, makeAny(xSingleSelection));
        }
        else if ( Reference< XForm >( xSingleSelection, UNO_QUERY ).is() )
            sTitle = SvxResId(RID_STR_PROPERTIES_FORM);
    }

    if ( implIsReadOnlyModel() )
        sTitle += SvxResId(RID_STR_READONLY_VIEW);

    m_xDialog->set_title(sTitle);
}

void FmPropBrw::FillInfo( SfxChildWinInfo& rInfo ) const
{
    rInfo.bVisible = false;
    rInfo.aExtraString = getCurrentPage();
}

IMPL_LINK_NOARG( FmPropBrw, OnAsyncGetFocus, void*, void )
{
    m_nAsyncGetFocusId = nullptr;
}

namespace
{
    bool lcl_shouldEnableHelpSection( const Reference< XComponentContext >& _rxContext )
    {
        ::utl::OConfigurationTreeRoot aConfiguration(
            ::utl::OConfigurationTreeRoot::createWithComponentContext(
                _rxContext,
                "/org.openoffice.Office.Common/Forms/PropertyBrowser/" ) );

        bool bEnabled = false;
        OSL_VERIFY( aConfiguration.getNodeValue( "DirectHelp" ) >>= bEnabled );
        return bEnabled;
    }
}

void FmPropBrw::impl_createPropertyBrowser_throw( FmFormShell* _pFormShell )
{
    // the document in which we live
    Reference< XInterface > xDocument;
    if ( _pFormShell && _pFormShell->GetObjectShell() )
        xDocument = _pFormShell->GetObjectShell()->GetModel();

    // the context of the controls in our document
    Reference< awt::XControlContainer > xControlContext;
    if ( _pFormShell && _pFormShell->GetFormView() )
    {
        SdrPageView* pPageView = _pFormShell->GetFormView()->GetSdrPageView();

        if(pPageView)
        {
            SdrPageWindow* pPageWindow = pPageView->GetPageWindow(0);

            if(pPageWindow)
            {
                xControlContext = pPageWindow->GetControlContainer();
            }
        }
    }

    // the default parent window for message boxes
    Reference< XWindow > xParentWindow(m_xDialog->GetXWindow());

    // the mapping from control models to control shapes
    Reference< XMap > xControlMap;
    FmFormPage* pFormPage = _pFormShell ? _pFormShell->GetCurPage() : nullptr;
    if ( pFormPage )
        xControlMap = pFormPage->GetImpl().getControlToShapeMap();

    // our own component context

    // a ComponentContext for the
    ::cppu::ContextEntry_Init aHandlerContextInfo[] =
    {
        ::cppu::ContextEntry_Init( "ContextDocument", makeAny( xDocument ) ),
        ::cppu::ContextEntry_Init( "DialogParentWindow", makeAny( xParentWindow ) ),
        ::cppu::ContextEntry_Init( "ControlContext", makeAny( xControlContext ) ),
        ::cppu::ContextEntry_Init( "ControlShapeAccess", makeAny( xControlMap ) )
    };
    m_xInspectorContext.set(
        ::cppu::createComponentContext( aHandlerContextInfo, SAL_N_ELEMENTS( aHandlerContextInfo ),
        m_xORB ) );

    bool bEnableHelpSection = lcl_shouldEnableHelpSection( m_xORB );

    // an object inspector model
    m_xInspectorModel =
            bEnableHelpSection
        ?   DefaultFormComponentInspectorModel::createWithHelpSection( m_xInspectorContext, 3, 5 )
        :   DefaultFormComponentInspectorModel::createDefault( m_xInspectorContext );

    // an object inspector
    m_xBrowserController =
        ObjectInspector::createWithModel(
            m_xInspectorContext, m_xInspectorModel
        );

    if ( !m_xBrowserController.is() )
    {
        ShowServiceNotAvailableError(m_pParent, "com.sun.star.inspection.ObjectInspector", true);
    }
    else
    {
        m_xBrowserController->attachFrame( Reference<XFrame>(m_xMeAsFrame,UNO_QUERY_THROW) );
    }

    if ( bEnableHelpSection )
    {
        Reference< XObjectInspector > xInspector( m_xBrowserController, UNO_QUERY_THROW );
        Reference< XObjectInspectorUI > xInspectorUI( xInspector->getInspectorUI() );
        DefaultHelpProvider::create( m_xInspectorContext, xInspectorUI );
    }
}


void FmPropBrw::impl_ensurePropertyBrowser_nothrow( FmFormShell* _pFormShell )
{
    // the document in which we live
    Reference< XInterface > xDocument;
    SfxObjectShell* pObjectShell = _pFormShell ? _pFormShell->GetObjectShell() : nullptr;
    if ( pObjectShell )
        xDocument = pObjectShell->GetModel();
    if ( ( xDocument == m_xLastKnownDocument ) && m_xBrowserController.is() )
        // nothing to do
        return;

    try
    {
        // clean up any previous instances of the object inspector
        if ( m_xMeAsFrame.is() )
            m_xMeAsFrame->setComponent( nullptr, nullptr );
        else
            ::comphelper::disposeComponent( m_xBrowserController );
        m_xBrowserController.clear();
        m_xInspectorModel.clear();

        // and create a new one
        impl_createPropertyBrowser_throw( _pFormShell );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }
    m_xLastKnownDocument = xDocument;
}


void FmPropBrw::StateChanged(sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    if (!pState  || SID_FM_PROPERTY_CONTROL != nSID)
        return;

    try
    {
        if (eState >= SfxItemState::DEFAULT)
        {
            FmFormShell* pShell = dynamic_cast<FmFormShell*>( static_cast<const SfxObjectItem*>(pState)->GetShell() );
            InterfaceBag aSelection;
            if ( pShell )
                pShell->GetImpl()->getCurrentSelection_Lock(aSelection);

            impl_ensurePropertyBrowser_nothrow( pShell );

            // set the new object to inspect
            implSetNewSelection( aSelection );

            // if this is the first time we're here, some additional things need to be done ...
            if ( m_bInitialStateChange )
            {
                // if we're just newly created, we want to have the focus
                m_nAsyncGetFocusId = Application::PostUserEvent(LINK(this, FmPropBrw, OnAsyncGetFocus));

                // and additionally, we want to show the page which was active during
                // our previous incarnation
                if ( !m_sLastActivePage.isEmpty() )
                {
                    try
                    {
                        if ( m_xBrowserController.is() )
                            m_xBrowserController->restoreViewData( makeAny( m_sLastActivePage ) );
                    }
                    catch( const Exception& )
                    {
                        OSL_FAIL( "FmPropBrw::StateChanged: caught an exception while setting the initial page!" );
                    }
                }

                m_bInitialStateChange = false;
            }

        }
        else
        {
            implSetNewSelection( InterfaceBag() );
        }
    }
    catch (Exception&)
    {
        OSL_FAIL("FmPropBrw::StateChanged: Exception occurred!");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
