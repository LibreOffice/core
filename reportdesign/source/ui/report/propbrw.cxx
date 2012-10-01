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
#include "propbrw.hxx"
#include "RptObject.hxx"
#include "ReportController.hxx"
#include <cppuhelper/component_context.hxx>
#include <RptResId.hrc>
#include "rptui_slotid.hrc"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#include <com/sun/star/inspection/ObjectInspector.hpp>
#include <com/sun/star/inspection/DefaultHelpProvider.hpp>
#include <svx/svxids.hrc>
#include <vcl/stdtext.hxx>
#include <svx/svdview.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>

#include <toolkit/unohlp.hxx>
#include <comphelper/property.hxx>
#include <comphelper/namecontainer.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/types.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/report/inspection/DefaultComponentInspectorModel.hpp>
#include <comphelper/processfactory.hxx>
#include "SectionView.hxx"
#include "ReportSection.hxx"
#include "uistrings.hrc"
#include "DesignView.hxx"
#include "ViewsWindow.hxx"
#include "UITools.hxx"
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

//----------------------------------------------------------------------------
//-----------------------------------------------------------------------
namespace
{
    static bool lcl_shouldEnableHelpSection( const Reference< XMultiServiceFactory >& _rxFactory )
    {
        const ::rtl::OUString sConfigName( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.ReportDesign/PropertyBrowser/" ) );
        const ::rtl::OUString sPropertyName( RTL_CONSTASCII_USTRINGPARAM( "DirectHelp" ) );

        ::utl::OConfigurationTreeRoot aConfiguration(
            ::utl::OConfigurationTreeRoot::createWithServiceFactory( _rxFactory, sConfigName ) );

        bool bEnabled = false;
        OSL_VERIFY( aConfiguration.getNodeValue( sPropertyName ) >>= bEnabled );
        return bEnabled;
    }
}
//-----------------------------------------------------------------------
//============================================================================
// PropBrw
//============================================================================

DBG_NAME( rpt_PropBrw )

//----------------------------------------------------------------------------

PropBrw::PropBrw(const Reference< XMultiServiceFactory >&   _xORB,Window* pParent,ODesignView*  _pDesignView)
          :DockingWindow(pParent,WinBits(WB_STDMODELESS|WB_SIZEABLE|WB_3DLOOK|WB_ROLLABLE))
          ,m_xORB(_xORB)
          ,m_pDesignView(_pDesignView)
          ,m_pView( NULL )
          ,m_bInitialStateChange(sal_True)
{
    DBG_CTOR( rpt_PropBrw,NULL);

    Size aPropWinSize(STD_WIN_SIZE_X,STD_WIN_SIZE_Y);
    SetOutputSizePixel(aPropWinSize);

    try
    {
        // create a frame wrapper for myself
        m_xMeAsFrame = Reference< XFrame >(m_xORB->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Frame"))), UNO_QUERY);
        if (m_xMeAsFrame.is())
        {
            m_xMeAsFrame->initialize( VCLUnoHelper::GetInterface ( this ) );
            m_xMeAsFrame->setName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("report property browser")));  // change name!
        }
    }
    catch (Exception&)
    {
        OSL_FAIL("PropBrw::PropBrw: could not create/initialize my frame!");
        DBG_UNHANDLED_EXCEPTION();
        m_xMeAsFrame.clear();
    }

    if (m_xMeAsFrame.is())
    {
        Reference< XComponentContext > xOwnContext(
            comphelper::getComponentContext( m_xORB ) );
        try
        {
            ::cppu::ContextEntry_Init aHandlerContextInfo[] =
            {
                ::cppu::ContextEntry_Init( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContextDocument" ) ), makeAny( m_pDesignView->getController().getModel() )),
                ::cppu::ContextEntry_Init( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogParentWindow" ) ), makeAny( VCLUnoHelper::GetInterface ( this ) )),
                ::cppu::ContextEntry_Init( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ActiveConnection" ) ), makeAny( m_pDesignView->getController().getConnection() ) ),
            };
            m_xInspectorContext.set(
                ::cppu::createComponentContext( aHandlerContextInfo, sizeof( aHandlerContextInfo ) / sizeof( aHandlerContextInfo[0] ),
                xOwnContext ) );
            // create a property browser controller
            bool bEnableHelpSection = lcl_shouldEnableHelpSection( m_xORB );
            Reference< inspection::XObjectInspectorModel> xInspectorModel( bEnableHelpSection
                ?   report::inspection::DefaultComponentInspectorModel::createWithHelpSection( m_xInspectorContext, 3, 8 )
                :   report::inspection::DefaultComponentInspectorModel::createDefault( m_xInspectorContext ) );

            m_xBrowserController = inspection::ObjectInspector::createWithModel(m_xInspectorContext, xInspectorModel);
            if ( !m_xBrowserController.is() )
            {
                const ::rtl::OUString sServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.inspection.ObjectInspector" ) );
                ShowServiceNotAvailableError(pParent, sServiceName, sal_True);
            }
            else
            {
                m_xBrowserController->attachFrame(m_xMeAsFrame);
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
            OSL_FAIL("PropBrw::PropBrw: could not create/initialize the browser controller!");
            DBG_UNHANDLED_EXCEPTION();
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
        m_xBrowserComponentWindow->setVisible(sal_True);
    }
    ::rptui::notifySystemWindow(pParent,this,::comphelper::mem_fun(&TaskPaneList::AddWindow));
}

//----------------------------------------------------------------------------

PropBrw::~PropBrw()
{
    if (m_xBrowserController.is())
        implDetachController();

    try
    {
        uno::Reference<container::XNameContainer> xName(m_xInspectorContext,uno::UNO_QUERY);
        if ( xName.is() )
        {
            const ::rtl::OUString pProps[] = { ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContextDocument" ) )
                                            ,  ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogParentWindow" ) )
                                            , ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ActiveConnection" ) )};
            for (size_t i = 0; i < sizeof(pProps)/sizeof(pProps[0]); ++i)
                xName->removeByName(pProps[i]);
        }
    }
    catch(Exception&)
    {}

    ::rptui::notifySystemWindow(this,this,::comphelper::mem_fun(&TaskPaneList::RemoveWindow));

    DBG_DTOR( rpt_PropBrw,NULL);
}
// -----------------------------------------------------------------------------
void PropBrw::setCurrentPage(const ::rtl::OUString& _sLastActivePage)
{
    m_sLastActivePage = _sLastActivePage;
}
//----------------------------------------------------------------------------

void PropBrw::implDetachController()
{
    m_sLastActivePage = getCurrentPage();
    implSetNewObject(  );

    if ( m_xMeAsFrame.is() )
        m_xMeAsFrame->setComponent( NULL, NULL );

    if ( m_xBrowserController.is() )
        m_xBrowserController->attachFrame( NULL );

    m_xMeAsFrame.clear();
    m_xBrowserController.clear();
    m_xBrowserComponentWindow.clear();
}
//-----------------------------------------------------------------------
::rtl::OUString PropBrw::getCurrentPage() const
{
    ::rtl::OUString sCurrentPage;
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
//----------------------------------------------------------------------------

sal_Bool PropBrw::Close()
{
    m_xLastSection.clear();
    // suspend the controller (it is allowed to veto)
    if ( m_xMeAsFrame.is() )
    {
        try
        {
            Reference< XController > xController( m_xMeAsFrame->getController() );
            if ( xController.is() && !xController->suspend( sal_True ) )
                return sal_False;
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

    return sal_True;
}

//----------------------------------------------------------------------------

uno::Sequence< Reference<uno::XInterface> > PropBrw::CreateCompPropSet(const SdrMarkList& _rMarkList)
{
    sal_uInt32 nMarkCount = _rMarkList.GetMarkCount();
    ::std::vector< uno::Reference< uno::XInterface> > aSets;
    aSets.reserve(nMarkCount);

    for(sal_uInt32 i=0;i<nMarkCount;++i)
    {
        SdrObject* pCurrent = _rMarkList.GetMark(i)->GetMarkedSdrObj();

        ::std::auto_ptr<SdrObjListIter> pGroupIterator;
        if (pCurrent->IsGroupObject())
        {
            pGroupIterator.reset(new SdrObjListIter(*pCurrent->GetSubList()));
            pCurrent = pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
        }

        while (pCurrent)
        {
            OObjectBase* pObj = dynamic_cast<OObjectBase*>(pCurrent);
            if ( pObj )
                aSets.push_back(CreateComponentPair(pObj));

            // next element
            pCurrent = pGroupIterator.get() && pGroupIterator->IsMore() ? pGroupIterator->Next() : NULL;
        }
    }
    Reference<uno::XInterface> *pSets = aSets.empty() ? NULL : &aSets[0];
    return uno::Sequence< Reference<uno::XInterface> >(pSets, aSets.size());
}
//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------

::rtl::OUString PropBrw::GetHeadlineName( const uno::Sequence< Reference<uno::XInterface> >& _aObjects )
{
    ::rtl::OUString aName;
    if ( !_aObjects.getLength() )
    {
        aName = String(ModuleRes(RID_STR_BRWTITLE_NO_PROPERTIES));
    }
    else if ( _aObjects.getLength() == 1 )    // single selection
    {
        aName = String(ModuleRes(RID_STR_BRWTITLE_PROPERTIES));

        uno::Reference< container::XNameContainer > xNameCont(_aObjects[0],uno::UNO_QUERY);
        Reference< lang::XServiceInfo > xServiceInfo( xNameCont->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReportComponent"))), UNO_QUERY );
        if ( xServiceInfo.is() )
        {
            sal_uInt16 nResId = 0;
            if ( xServiceInfo->supportsService( SERVICE_FIXEDTEXT ) )
            {
                nResId = RID_STR_PROPTITLE_FIXEDTEXT;
            }
            else if ( xServiceInfo->supportsService( SERVICE_IMAGECONTROL ) )
            {
                nResId = RID_STR_PROPTITLE_IMAGECONTROL;
            }
            else if ( xServiceInfo->supportsService( SERVICE_FORMATTEDFIELD ) )
            {
                nResId = RID_STR_PROPTITLE_FORMATTED;
            }
            else if ( xServiceInfo->supportsService( SERVICE_SHAPE ) )
            {
                nResId = RID_STR_PROPTITLE_SHAPE;
            }
            else if ( xServiceInfo->supportsService( SERVICE_REPORTDEFINITION ) )
            {
                nResId = RID_STR_PROPTITLE_REPORT;
            }
            else if ( xServiceInfo->supportsService( SERVICE_SECTION ) )
            {
                nResId = RID_STR_PROPTITLE_SECTION;
            }
            else if ( xServiceInfo->supportsService( SERVICE_FUNCTION ) )
            {
                nResId = RID_STR_PROPTITLE_FUNCTION;
            }
            else if ( xServiceInfo->supportsService( SERVICE_GROUP ) )
            {
                nResId = RID_STR_PROPTITLE_GROUP;
            }
            else if ( xServiceInfo->supportsService( SERVICE_FIXEDLINE ) )
            {
                nResId = RID_STR_PROPTITLE_FIXEDLINE;
            }
            else
            {
                OSL_FAIL("Unknown service name!");
                nResId = RID_STR_CLASS_FORMATTEDFIELD;
            }

            if (nResId)
            {
                aName += String(ModuleRes(nResId));
            }
        }
    }
    else    // multiselection
    {
        aName = String(ModuleRes(RID_STR_BRWTITLE_PROPERTIES));
        aName += String(ModuleRes(RID_STR_BRWTITLE_MULTISELECT));
    }

    return aName;
}
// -----------------------------------------------------------------------------
uno::Reference< uno::XInterface> PropBrw::CreateComponentPair(OObjectBase* _pObj)
{
    _pObj->initializeOle();
    return CreateComponentPair(_pObj->getAwtComponent(),_pObj->getReportComponent());
}
//----------------------------------------------------------------------------
uno::Reference< uno::XInterface> PropBrw::CreateComponentPair(const uno::Reference< uno::XInterface>& _xFormComponent
                                                              ,const uno::Reference< uno::XInterface>& _xReportComponent)
{
    uno::Reference< container::XNameContainer > xNameCont = ::comphelper::NameContainer_createInstance(::getCppuType(static_cast<Reference<XInterface> * >(NULL)));
    xNameCont->insertByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FormComponent")),uno::makeAny(_xFormComponent));
    xNameCont->insertByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReportComponent")),uno::makeAny(_xReportComponent));
    xNameCont->insertByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("RowSet"))
            ,uno::makeAny(uno::Reference< uno::XInterface>(m_pDesignView->getController().getRowSet())));

    return xNameCont.get();
}
// -----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
void PropBrw::Resize()
{
    Window::Resize();

    Reference< awt::XLayoutConstrains > xLayoutConstrains( m_xBrowserController, UNO_QUERY );
    if( xLayoutConstrains.is() )
    {
        ::Size aMinSize = getMinimumSize();
        SetMinOutputSizePixel( aMinSize );
        ::Size aSize = GetOutputSizePixel();
        sal_Bool bResize = sal_False;
        if( aSize.Width() < aMinSize.Width() )
        {
            aSize.setWidth( aMinSize.Width() );
            bResize = sal_True;
        }
        if( aSize.Height() < aMinSize.Height() )
        {
            aSize.setHeight( aMinSize.Height() );
            bResize = sal_True;
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
//----------------------------------------------------------------------------
void PropBrw::Update( OSectionView* pNewView )
{
    try
    {
        if ( m_pView )
        {
            EndListening( *(m_pView->GetModel()) );
            m_pView = NULL;
        }

        // set focus on initialization
        if ( m_bInitialStateChange )
        {
            // if we're just newly created, we want to have the focus
            PostUserEvent( LINK( this, PropBrw, OnAsyncGetFocus ) );
            m_bInitialStateChange = sal_False;
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
            ::boost::shared_ptr<OSectionWindow> pSectionWindow = pViews->getSectionWindow(i);
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
            uno::Reference< container::XNameContainer > xNameCont = ::comphelper::NameContainer_createInstance(::getCppuType(static_cast<Reference<XInterface> * >(NULL)));
            xNameCont->insertByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReportComponent")),uno::makeAny(xTemp));
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
//----------------------------------------------------------------------------
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
                m_pView = NULL;
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
//-----------------------------------------------------------------------
IMPL_LINK( PropBrw, OnAsyncGetFocus, void*,  )
{
    if (m_xBrowserComponentWindow.is())
        m_xBrowserComponentWindow->setFocus();
    return 0L;
}
//----------------------------------------------------------------------------
void PropBrw::LoseFocus()
{
    DockingWindow::LoseFocus();
    m_pDesignView->getController().InvalidateAll();
}
//----------------------------------------------------------------------------
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
