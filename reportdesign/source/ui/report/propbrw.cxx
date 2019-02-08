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
#include <memory>
#include <propbrw.hxx>
#include <RptObject.hxx>
#include <ReportController.hxx>
#include <cppuhelper/component_context.hxx>
#include <strings.hrc>
#include <rptui_slotid.hrc>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/inspection/ObjectInspector.hpp>
#include <com/sun/star/inspection/DefaultHelpProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/report/inspection/DefaultComponentInspectorModel.hpp>
#include <svx/svxids.hrc>
#include <vcl/stdtext.hxx>
#include <vcl/weld.hxx>
#include <svx/svdview.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/namecontainer.hxx>
#include <comphelper/types.hxx>
#include <comphelper/sequence.hxx>
#include <core_resource.hxx>
#include <SectionView.hxx>
#include <ReportSection.hxx>
#include <strings.hxx>
#include <DesignView.hxx>
#include <ViewsWindow.hxx>
#include <UITools.hxx>
#include <unotools/confignode.hxx>

namespace rptui
{
#define STD_WIN_SIZE_X  300
#define STD_WIN_SIZE_Y  350

using namespace ::com::sun::star;
using namespace uno;
using namespace lang;
using namespace frame;
using namespace beans;
using namespace container;
using namespace ::comphelper;


namespace
{
    bool lcl_shouldEnableHelpSection( const Reference< XComponentContext >& _rxContext )
    {
        ::utl::OConfigurationTreeRoot aConfiguration(
            ::utl::OConfigurationTreeRoot::createWithComponentContext(
                _rxContext, "/org.openoffice.Office.ReportDesign/PropertyBrowser/" ) );

        bool bEnabled = false;
        OSL_VERIFY( aConfiguration.getNodeValue( "DirectHelp"  ) >>= bEnabled );
        return bEnabled;
    }
}


// PropBrw


PropBrw::PropBrw(const Reference< XComponentContext >& _xORB, vcl::Window* pParent, ODesignView*  _pDesignView)
          :DockingWindow(pParent,WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_3DLOOK|WB_ROLLABLE))
          ,m_xORB(_xORB)
          ,m_pDesignView(_pDesignView)
          ,m_pView( nullptr )
          ,m_bInitialStateChange(true)
{

    Size aPropWinSize(STD_WIN_SIZE_X,STD_WIN_SIZE_Y);
    SetOutputSizePixel(aPropWinSize);

    try
    {
        // create a frame wrapper for myself
        m_xMeAsFrame = Frame::create( m_xORB );
        m_xMeAsFrame->initialize( VCLUnoHelper::GetInterface ( this ) );
        m_xMeAsFrame->setName("report property browser");  // change name!
    }
    catch (Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("reportdesign");
        OSL_FAIL("PropBrw::PropBrw: could not create/initialize my frame!");
        m_xMeAsFrame.clear();
    }

    if (m_xMeAsFrame.is())
    {
        try
        {
            ::cppu::ContextEntry_Init aHandlerContextInfo[] =
            {
                ::cppu::ContextEntry_Init( "ContextDocument", makeAny( m_pDesignView->getController().getModel() )),
                ::cppu::ContextEntry_Init( "DialogParentWindow", makeAny( VCLUnoHelper::GetInterface ( this ) )),
                ::cppu::ContextEntry_Init( "ActiveConnection", makeAny( m_pDesignView->getController().getConnection() ) ),
            };
            m_xInspectorContext.set(
                ::cppu::createComponentContext( aHandlerContextInfo, SAL_N_ELEMENTS( aHandlerContextInfo ),
                m_xORB ) );
            // create a property browser controller
            bool bEnableHelpSection = lcl_shouldEnableHelpSection( m_xORB );
            Reference< inspection::XObjectInspectorModel> xInspectorModel( bEnableHelpSection
                ?   report::inspection::DefaultComponentInspectorModel::createWithHelpSection( m_xInspectorContext, 3, 8 )
                :   report::inspection::DefaultComponentInspectorModel::createDefault( m_xInspectorContext ) );

            m_xBrowserController = inspection::ObjectInspector::createWithModel(m_xInspectorContext, xInspectorModel);
            if ( !m_xBrowserController.is() )
            {
                const OUString sServiceName( "com.sun.star.inspection.ObjectInspector" );
                ShowServiceNotAvailableError(pParent ? pParent->GetFrameWeld() : nullptr, sServiceName, true);
            }
            else
            {
                m_xBrowserController->attachFrame( Reference<XFrame>(m_xMeAsFrame, UNO_QUERY_THROW));
                m_xBrowserComponentWindow = m_xMeAsFrame->getComponentWindow();
                OSL_ENSURE(m_xBrowserComponentWindow.is(), "PropBrw::PropBrw: attached the controller, but have no component window!");
                if ( bEnableHelpSection )
                {
                    uno::Reference< inspection::XObjectInspector > xInspector( m_xBrowserController, uno::UNO_QUERY_THROW );
                    uno::Reference< inspection::XObjectInspectorUI > xInspectorUI( xInspector->getInspectorUI() );
                    uno::Reference< uno::XInterface > xDefaultHelpProvider( inspection::DefaultHelpProvider::create( m_xInspectorContext, xInspectorUI ) );
                }
            }
        }
        catch (Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("reportdesign");
            OSL_FAIL("PropBrw::PropBrw: could not create/initialize the browser controller!");
            try
            {
                ::comphelper::disposeComponent(m_xBrowserController);
                ::comphelper::disposeComponent(m_xBrowserComponentWindow);
            }
            catch(Exception&) { }
            m_xBrowserController.clear();
            m_xBrowserComponentWindow.clear();
        }
    }

    if (m_xBrowserComponentWindow.is())
    {

        m_xBrowserComponentWindow->setPosSize(0, 0, aPropWinSize.Width(), aPropWinSize.Height(),
            awt::PosSize::WIDTH | awt::PosSize::HEIGHT | awt::PosSize::X | awt::PosSize::Y);
        Resize();
        m_xBrowserComponentWindow->setVisible(true);
    }
    ::rptui::notifySystemWindow(pParent,this,::comphelper::mem_fun(&TaskPaneList::AddWindow));
}


PropBrw::~PropBrw()
{
    disposeOnce();
}

void PropBrw::dispose()
{
    if (m_xBrowserController.is())
        implDetachController();

    try
    {
        uno::Reference<container::XNameContainer> xName(m_xInspectorContext,uno::UNO_QUERY);
        if ( xName.is() )
        {
            const OUString pProps[] = { OUString( "ContextDocument" )
                                            ,  OUString( "DialogParentWindow" )
                                            , OUString( "ActiveConnection" )};
            for (const auto & i : pProps)
                xName->removeByName(i);
        }
    }
    catch(Exception&)
    {}

    ::rptui::notifySystemWindow(this,this,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));
    m_pDesignView.clear();
    DockingWindow::dispose();
}

void PropBrw::setCurrentPage(const OUString& _sLastActivePage)
{
    m_sLastActivePage = _sLastActivePage;
}


void PropBrw::implDetachController()
{
    m_sLastActivePage = getCurrentPage();
    implSetNewObject(  );

    if ( m_xMeAsFrame.is() )
        m_xMeAsFrame->setComponent( nullptr, nullptr );

    if ( m_xBrowserController.is() )
        m_xBrowserController->attachFrame( nullptr );

    m_xMeAsFrame.clear();
    m_xBrowserController.clear();
    m_xBrowserComponentWindow.clear();
}

OUString PropBrw::getCurrentPage() const
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
        OSL_FAIL( "PropBrw::getCurrentPage: caught an exception while retrieving the current page!" );
    }
    return sCurrentPage;
}


bool PropBrw::Close()
{
    m_xLastSection.clear();
    // suspend the controller (it is allowed to veto)
    if ( m_xMeAsFrame.is() )
    {
        try
        {
            Reference< XController > xController( m_xMeAsFrame->getController() );
            if ( xController.is() && !xController->suspend( true ) )
                return false;
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmPropBrw::Close: caught an exception while asking the controller!" );
        }
    }
    implDetachController();

    if( IsRollUp() )
        RollDown();

    m_pDesignView->getController().executeUnChecked(SID_PROPERTYBROWSER_LAST_PAGE,uno::Sequence< beans::PropertyValue>());

    return true;
}


uno::Sequence< Reference<uno::XInterface> > PropBrw::CreateCompPropSet(const SdrMarkList& _rMarkList)
{
    const size_t nMarkCount = _rMarkList.GetMarkCount();
    ::std::vector< uno::Reference< uno::XInterface> > aSets;
    aSets.reserve(nMarkCount);

    for(size_t i=0; i<nMarkCount; ++i)
    {
        SdrObject* pCurrent = _rMarkList.GetMark(i)->GetMarkedSdrObj();

        ::std::unique_ptr<SdrObjListIter> pGroupIterator;
        if (pCurrent->IsGroupObject())
        {
            pGroupIterator.reset(new SdrObjListIter(pCurrent->GetSubList()));
            pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : nullptr;
        }

        while (pCurrent)
        {
            OObjectBase* pObj = dynamic_cast<OObjectBase*>(pCurrent);
            if ( pObj )
                aSets.push_back(CreateComponentPair(pObj));

            // next element
            pCurrent = pGroupIterator.get() && pGroupIterator->IsMore() ? pGroupIterator->Next() : nullptr;
        }
    }
    return uno::Sequence< Reference<uno::XInterface> >(aSets.data(), aSets.size());
}

void PropBrw::implSetNewObject( const uno::Sequence< Reference<uno::XInterface> >& _aObjects )
{
    if ( m_xBrowserController.is() )
    {
        try
        {
            m_xBrowserController->inspect(uno::Sequence< Reference<uno::XInterface> >());
            m_xBrowserController->inspect(_aObjects);
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FmPropBrw::StateChanged: caught an exception while setting the initial page!" );
        }
    }
    SetText( GetHeadlineName(_aObjects) );
}


OUString PropBrw::GetHeadlineName( const uno::Sequence< Reference<uno::XInterface> >& _aObjects )
{
    OUString aName;
    if ( !_aObjects.getLength() )
    {
        aName = RptResId(RID_STR_BRWTITLE_NO_PROPERTIES);
    }
    else if ( _aObjects.getLength() == 1 )    // single selection
    {
        aName = RptResId(RID_STR_BRWTITLE_PROPERTIES);

        uno::Reference< container::XNameContainer > xNameCont(_aObjects[0],uno::UNO_QUERY);
        Reference< lang::XServiceInfo > xServiceInfo( xNameCont->getByName("ReportComponent"), UNO_QUERY );
        if ( xServiceInfo.is() )
        {
            const char* pResId;
            if ( xServiceInfo->supportsService( SERVICE_FIXEDTEXT ) )
            {
                pResId = RID_STR_PROPTITLE_FIXEDTEXT;
            }
            else if ( xServiceInfo->supportsService( SERVICE_IMAGECONTROL ) )
            {
                pResId = RID_STR_PROPTITLE_IMAGECONTROL;
            }
            else if ( xServiceInfo->supportsService( SERVICE_FORMATTEDFIELD ) )
            {
                pResId = RID_STR_PROPTITLE_FORMATTED;
            }
            else if ( xServiceInfo->supportsService( SERVICE_SHAPE ) )
            {
                pResId = RID_STR_PROPTITLE_SHAPE;
            }
            else if ( xServiceInfo->supportsService( SERVICE_REPORTDEFINITION ) )
            {
                pResId = RID_STR_PROPTITLE_REPORT;
            }
            else if ( xServiceInfo->supportsService( SERVICE_SECTION ) )
            {
                pResId = RID_STR_PROPTITLE_SECTION;
            }
            else if ( xServiceInfo->supportsService( SERVICE_FUNCTION ) )
            {
                pResId = RID_STR_PROPTITLE_FUNCTION;
            }
            else if ( xServiceInfo->supportsService( SERVICE_GROUP ) )
            {
                pResId = RID_STR_PROPTITLE_GROUP;
            }
            else if ( xServiceInfo->supportsService( SERVICE_FIXEDLINE ) )
            {
                pResId = RID_STR_PROPTITLE_FIXEDLINE;
            }
            else
            {
                OSL_FAIL("Unknown service name!");
                aName += RID_STR_CLASS_FORMATTEDFIELD;
                return aName;
            }

            aName += RptResId(pResId);
        }
    }
    else    // multiselection
    {
        aName = RptResId(RID_STR_BRWTITLE_PROPERTIES);
        aName += RptResId(RID_STR_BRWTITLE_MULTISELECT);
    }

    return aName;
}

uno::Reference< uno::XInterface> PropBrw::CreateComponentPair(OObjectBase* _pObj)
{
    _pObj->initializeOle();
    return CreateComponentPair(_pObj->getAwtComponent(),_pObj->getReportComponent());
}

uno::Reference< uno::XInterface> PropBrw::CreateComponentPair(const uno::Reference< uno::XInterface>& _xFormComponent
                                                              ,const uno::Reference< uno::XInterface>& _xReportComponent)
{
    uno::Reference< container::XNameContainer > xNameCont = ::comphelper::NameContainer_createInstance(cppu::UnoType<XInterface>::get());
    xNameCont->insertByName("FormComponent",uno::makeAny(_xFormComponent));
    xNameCont->insertByName("ReportComponent",uno::makeAny(_xReportComponent));
    xNameCont->insertByName("RowSet",uno::makeAny(uno::Reference< uno::XInterface>(m_pDesignView->getController().getRowSet())));

    return xNameCont.get();
}

::Size PropBrw::getMinimumSize() const
{
    ::Size aSize;
    Reference< awt::XLayoutConstrains > xLayoutConstrains( m_xBrowserController, UNO_QUERY );
    if( xLayoutConstrains.is() )
    {
        awt::Size aMinSize = xLayoutConstrains->getMinimumSize();
        aMinSize.Height += 4;
        aMinSize.Width += 4;
        aSize.setHeight( aMinSize.Height );
        aSize.setWidth( aMinSize.Width );
    }
    return aSize;
}

void PropBrw::Resize()
{
    Window::Resize();

    Reference< awt::XLayoutConstrains > xLayoutConstrains( m_xBrowserController, UNO_QUERY );
    if( xLayoutConstrains.is() )
    {
        ::Size aMinSize = getMinimumSize();
        SetMinOutputSizePixel( aMinSize );
        ::Size aSize = GetOutputSizePixel();
        bool bResize = false;
        if( aSize.Width() < aMinSize.Width() )
        {
            aSize.setWidth( aMinSize.Width() );
            bResize = true;
        }
        if( aSize.Height() < aMinSize.Height() )
        {
            aSize.setHeight( aMinSize.Height() );
            bResize = true;
        }
        if( bResize )
            SetOutputSizePixel( aSize );
    }
    // adjust size
    if (m_xBrowserComponentWindow.is())
    {
        Size  aSize = GetOutputSizePixel();
        m_xBrowserComponentWindow->setPosSize(0, 0, aSize.Width(), aSize.Height(),
            awt::PosSize::WIDTH | awt::PosSize::HEIGHT);
    }
}

void PropBrw::Update( OSectionView* pNewView )
{
    try
    {
        if ( m_pView )
        {
            EndListening( *(m_pView->GetModel()) );
            m_pView = nullptr;
        }

        // set focus on initialization
        if ( m_bInitialStateChange )
        {
            // if we're just newly created, we want to have the focus
            PostUserEvent( LINK( this, PropBrw, OnAsyncGetFocus ), nullptr, true );
            m_bInitialStateChange = false;
            // and additionally, we want to show the page which was active during
            // our previous incarnation
            if ( !m_sLastActivePage.isEmpty() && m_xBrowserController.is() )
            {
                try
                {
                    m_xBrowserController->restoreViewData( makeAny( m_sLastActivePage ) );
                }
                catch( const Exception& )
                {
                    OSL_FAIL( "FmPropBrw::StateChanged: caught an exception while setting the initial page!" );
                }
            }
        }

        if ( !pNewView )
            return;
        else
            m_pView = pNewView;

        uno::Sequence< Reference<uno::XInterface> > aMarkedObjects;
        OViewsWindow* pViews = m_pView->getReportSection()->getSectionWindow()->getViewsWindow();
        const sal_uInt16 nSectionCount = pViews->getSectionCount();
        for (sal_uInt16 i = 0; i < nSectionCount; ++i)
        {
            OSectionWindow* pSectionWindow = pViews->getSectionWindow(i);
            if ( pSectionWindow )
            {
                const SdrMarkList& rMarkList = pSectionWindow->getReportSection().getSectionView().GetMarkedObjectList();
                aMarkedObjects = ::comphelper::concatSequences(aMarkedObjects,CreateCompPropSet( rMarkList ));
            }
        }

        if ( aMarkedObjects.getLength() ) // multiple selection
        {
            m_xLastSection.clear();
            implSetNewObject( aMarkedObjects );
        }
        else if ( m_xLastSection != m_pView->getReportSection()->getSection() )
        {
            uno::Reference< uno::XInterface> xTemp(m_pView->getReportSection()->getSection());
            m_xLastSection = xTemp;
            uno::Reference< container::XNameContainer > xNameCont = ::comphelper::NameContainer_createInstance(cppu::UnoType<XInterface>::get() );
            xNameCont->insertByName("ReportComponent",uno::makeAny(xTemp));
            xTemp = xNameCont;

            implSetNewObject( uno::Sequence< uno::Reference< uno::XInterface> >(&xTemp,1) );
        }

        StartListening( *(m_pView->GetModel()) );
    }
    catch ( Exception& )
    {
        OSL_FAIL( "PropBrw::Update: Exception occurred!" );
    }
}

void PropBrw::Update( const uno::Reference< uno::XInterface>& _xReportComponent)
{
    if ( m_xLastSection != _xReportComponent )
    {
        m_xLastSection = _xReportComponent;
        try
        {
            if ( m_pView )
            {
                EndListening( *(m_pView->GetModel()) );
                m_pView = nullptr;
            }

            uno::Reference< uno::XInterface> xTemp(CreateComponentPair(_xReportComponent,_xReportComponent));
            implSetNewObject( uno::Sequence< uno::Reference< uno::XInterface> >(&xTemp,1) );
        }
        catch ( Exception& )
        {
            OSL_FAIL( "PropBrw::Update: Exception occurred!" );
        }
    }
}

IMPL_LINK_NOARG( PropBrw, OnAsyncGetFocus, void*, void )
{
    if (m_xBrowserComponentWindow.is())
        m_xBrowserComponentWindow->setFocus();
}

void PropBrw::LoseFocus()
{
    DockingWindow::LoseFocus();
    if (m_pDesignView)
        m_pDesignView->getController().InvalidateAll();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
