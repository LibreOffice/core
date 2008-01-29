/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propbrw.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:52:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include "precompiled_reportdesign.hxx"

#ifndef _REPORT_PROPBRW_HXX
#include "propbrw.hxx"
#endif
#ifndef _REPORT_RPTUIOBJ_HXX
#include "RptObject.hxx"
#endif
#ifndef RPTUI_REPORTCONTROLLER_HXX
#include "ReportController.hxx"
#endif
#ifndef _CPPUHELPER_COMPONENT_CONTEXT_HXX_
#include <cppuhelper/component_context.hxx>
#endif
#ifndef _REPORT_DLGRESID_HRC
#include <RptResId.hrc>
#endif
#ifndef _RPTUI_SLOTID_HRC_
#include "rptui_slotid.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_OBJECTINSPECTOR_HPP_
#include <com/sun/star/inspection/ObjectInspector.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_DEFAULTHELPPROVIDER_HPP_
#include <com/sun/star/inspection/DefaultHelpProvider.hpp>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif

#ifndef _SVDVIEW_HXX //autogen
#include <svx/svdview.hxx>
#endif

#ifndef _SVDOGRP_HXX
#include <svx/svdogrp.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_NAMECONTAINER_HXX_
#include <comphelper/namecontainer.hxx>
#endif
#ifndef _COMPHELPER_COMPOSEDPROPS_HXX_
#include <comphelper/composedprops.hxx>
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#include <comphelper/sequence.hxx>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_INSPECTION_DEFAULTCOMPONENTINSPECTORMODEL_HPP_
#include <com/sun/star/report/inspection/DefaultComponentInspectorModel.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _REPORT_SECTIONVIEW_HXX
#include "SectionView.hxx"
#endif
#ifndef REPORT_REPORTSECTION_HXX
#include "ReportSection.hxx"
#endif
#ifndef REPORTDESIGN_SHARED_UISTRINGS_HRC
#include "uistrings.hrc"
#endif
#ifndef RPTUI_DESIGNVIEW_HXX
#include "DesignView.hxx"
#endif
#include "ViewsWindow.hxx"
#include "UITools.hxx"
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif

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
            m_xMeAsFrame->setName(::rtl::OUString::createFromAscii("report property browser"));  // change name!
        }
    }
    catch (Exception&)
    {
        DBG_ERROR("PropBrw::PropBrw: could not create/initialize my frame!");
        m_xMeAsFrame.clear();
    }

    if (m_xMeAsFrame.is())
    {
        Reference< XComponentContext > xOwnContext;
        try
        {
            // our own component context
            Reference< XPropertySet > xFactoryProperties( m_xORB, UNO_QUERY_THROW );
            xOwnContext.set(
                xFactoryProperties->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ),
                UNO_QUERY_THROW );

            /*uno::Reference< XComponent> xModel = new OContextHelper(m_xORB,uno::Reference< XComponent>(m_pDesignView->getController()->getModel(),uno::UNO_QUERY) );
            uno::Reference< XComponent> xDialogParentWindow = new OContextHelper(m_xORB,uno::Reference< XComponent>(VCLUnoHelper::GetInterface ( this ),uno::UNO_QUERY) );
            uno::Reference< XComponent> xConnection = new OContextHelper(m_xORB,uno::Reference< XComponent>(m_pDesignView->getController()->getConnection(),uno::UNO_QUERY) );*/
            // a ComponentContext for the
            ::cppu::ContextEntry_Init aHandlerContextInfo[] =
            {
                ::cppu::ContextEntry_Init( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ContextDocument" ) ), makeAny( m_pDesignView->getController()->getModel() )),
                ::cppu::ContextEntry_Init( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogParentWindow" ) ), makeAny( VCLUnoHelper::GetInterface ( this ) )),
                ::cppu::ContextEntry_Init( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ActiveConnection" ) ), makeAny( m_pDesignView->getController()->getConnection() ) ),
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
            xFactoryProperties->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ,makeAny(xOwnContext));
        }
        catch (Exception&)
        {
            DBG_ERROR("PropBrw::PropBrw: could not create/initialize the browser controller!");
            try
            {
                ::comphelper::disposeComponent(m_xBrowserController);
                ::comphelper::disposeComponent(m_xBrowserComponentWindow);
                Reference< XPropertySet > xFactoryProperties( m_xORB, UNO_QUERY_THROW );
                if ( xOwnContext.is() )
                    xFactoryProperties->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) ,makeAny(xOwnContext));
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

        if ( !sCurrentPage.getLength() )
            sCurrentPage = m_sLastActivePage;
    }
    catch( const Exception& )
    {
        OSL_ENSURE( sal_False, "PropBrw::getCurrentPage: caught an exception while retrieving the current page!" );
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
            OSL_ENSURE( sal_False, "FmPropBrw::Close: caught an exception while asking the controller!" );
        }
    }
    implDetachController();

    if( IsRollUp() )
        RollDown();

    m_pDesignView->getController()->executeUnChecked(SID_PROPERTYBROWSER_LAST_PAGE,uno::Sequence< beans::PropertyValue>());

    return TRUE;
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
        m_xBrowserController->inspect(uno::Sequence< Reference<uno::XInterface> >());
        m_xBrowserController->inspect(_aObjects);

        //Resize();
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
        sal_uInt16 nResId = 0;
        aName = String(ModuleRes(RID_STR_BRWTITLE_PROPERTIES));

        uno::Reference< container::XNameContainer > xNameCont(_aObjects[0],uno::UNO_QUERY);
        Reference< lang::XServiceInfo > xServiceInfo( xNameCont->getByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReportComponent"))), UNO_QUERY );
        if ( xServiceInfo.is() )
        {
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
                OSL_ENSURE(0,"Unknown service name!");
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
            ,uno::makeAny(uno::Reference< uno::XInterface>(m_pDesignView->getController()->getRowSet())));

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
            if ( m_sLastActivePage.getLength() && m_xBrowserController.is() )
            {
                try
                {
                    m_xBrowserController->restoreViewData( makeAny( m_sLastActivePage ) );
                }
                catch( const Exception& )
                {
                    OSL_ENSURE( sal_False, "FmPropBrw::StateChanged: caught an exception while setting the initial page!" );
                }
            }
        }

        if ( !pNewView )
            return;
        else
            m_pView = pNewView;

        uno::Sequence< Reference<uno::XInterface> > aMarkedObjects;
        OViewsWindow* pViews = m_pView->getSectionWindow()->getViewsWindow();
        const USHORT nSectionCount = pViews->getSectionCount();
        for (USHORT i = 0; i < nSectionCount; ++i)
        {
            ::boost::shared_ptr<OReportSection> pReportSection = pViews->getSection(i);
            if ( pReportSection )
            {
                const SdrMarkList& rMarkList = pReportSection->getView()->GetMarkedObjectList();
                aMarkedObjects = ::comphelper::concatSequences(aMarkedObjects,CreateCompPropSet( rMarkList ));
            }
        }

  //      if ( nMarkCount == 1 )
  //      {
  //          m_xLastSection.clear();
        //  SdrObject* pDlgEdObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
        //  OObjectBase* pObj = dynamic_cast<OObjectBase*>(pDlgEdObj);
        //  if ( pObj )
        //  {
        //      if ( pDlgEdObj && pDlgEdObj->IsGroupObject() )
        //      {
        //          implSetNewObject( CreateCompPropSet( rMarkList ) );
        //      }
        //      else // single selection
        //      {
  //                  uno::Reference< uno::XInterface > xTemp = CreateComponentPair(pObj);
        //          implSetNewObject( uno::Sequence< uno::Reference< uno::XInterface> >(&xTemp,1) );
        //      }
        //  }
  //          else
  //          {
  //              implSetNewObject( );
  //          }
        //}
  //      else
        if ( aMarkedObjects.getLength() ) // multiple selection
        {
            m_xLastSection.clear();
            implSetNewObject( aMarkedObjects );
        }
        else if ( m_xLastSection != m_pView->getSectionWindow()->getSection() )
        {
            uno::Reference< uno::XInterface> xTemp(m_pView->getSectionWindow()->getSection());
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
        DBG_ERROR( "PropBrw::Update: Exception occured!" );
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
            } // if ( m_pView )

            uno::Reference< uno::XInterface> xTemp(CreateComponentPair(_xReportComponent,_xReportComponent));
            implSetNewObject( uno::Sequence< uno::Reference< uno::XInterface> >(&xTemp,1) );
        }
        catch ( Exception& )
        {
            DBG_ERROR( "PropBrw::Update: Exception occured!" );
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
    m_pDesignView->getController()->InvalidateAll();
}
//----------------------------------------------------------------------------
}
