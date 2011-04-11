/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "dlg_CreationWizard_UNO.hxx"
#include "dlg_CreationWizard.hxx"
#include "macros.hxx"
#include "servicenames.hxx"
#include "ContainerHelper.hxx"
#include "TimerTriggeredControllerLock.hxx"
#include <osl/mutex.hxx>
#include <osl/mutex.hxx>
// header for class Application
#include <vcl/svapp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
// header for define RET_CANCEL
#include <vcl/msgbox.hxx>
// header for class OImplementationId
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDesktop.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

CreationWizardUnoDlg::CreationWizardUnoDlg( const uno::Reference< uno::XComponentContext >& xContext )
                    : OComponentHelper( m_aMutex )
                    , m_xChartModel( 0 )
                    , m_xCC( xContext )
                    , m_xParentWindow( 0 )
                    , m_pDialog( 0 )
                    , m_bUnlockControllersOnExecute(false)
{
    uno::Reference< frame::XDesktop > xDesktop(
        m_xCC->getServiceManager()->createInstanceWithContext(
            C2U( "com.sun.star.frame.Desktop" ), m_xCC ), uno::UNO_QUERY );
    if( xDesktop.is() )
    {
        uno::Reference< frame::XTerminateListener > xListener( this );
        xDesktop->addTerminateListener( xListener );
    }
}
CreationWizardUnoDlg::~CreationWizardUnoDlg()
{
    SolarMutexGuard aSolarGuard;
    if( m_pDialog )
    {
        delete m_pDialog;
        m_pDialog = 0;
    }
}
//-------------------------------------------------------------------------
// lang::XServiceInfo
APPHELPER_XSERVICEINFO_IMPL(CreationWizardUnoDlg,CHART_WIZARD_DIALOG_SERVICE_IMPLEMENTATION_NAME)

    uno::Sequence< rtl::OUString > CreationWizardUnoDlg
::getSupportedServiceNames_Static()
{
    uno::Sequence< rtl::OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART_WIZARD_DIALOG_SERVICE_NAME;
    return aSNS;
}

//-------------------------------------------------------------------------
// XInterface
uno::Any SAL_CALL CreationWizardUnoDlg::queryInterface( const uno::Type& aType ) throw (uno::RuntimeException)
{
    return OComponentHelper::queryInterface( aType );
}
void SAL_CALL CreationWizardUnoDlg::acquire() throw ()
{
    OComponentHelper::acquire();
}
void SAL_CALL CreationWizardUnoDlg::release() throw ()
{
    OComponentHelper::release();
}
uno::Any SAL_CALL CreationWizardUnoDlg::queryAggregation( uno::Type const & rType ) throw (uno::RuntimeException)
{
    if (rType == ::getCppuType( (uno::Reference< ui::dialogs::XExecutableDialog > const *)0 ))
    {
        void * p = static_cast< ui::dialogs::XExecutableDialog * >( this );
        return uno::Any( &p, rType );
    }
    else if (rType == ::getCppuType( (uno::Reference< lang::XServiceInfo > const *)0 ))
    {
        void * p = static_cast< lang::XTypeProvider * >( this );
        return uno::Any( &p, rType );
    }
    else if (rType == ::getCppuType( (uno::Reference< lang::XInitialization > const *)0 ))
    {
        void * p = static_cast< lang::XInitialization * >( this );
        return uno::Any( &p, rType );
    }
    else if (rType == ::getCppuType( (uno::Reference< frame::XTerminateListener > const *)0 ))
    {
        void * p = static_cast< frame::XTerminateListener * >( this );
        return uno::Any( &p, rType );
    }
    else if (rType == ::getCppuType( (uno::Reference< beans::XPropertySet > const *)0 ))
    {
        void * p = static_cast< beans::XPropertySet * >( this );
        return uno::Any( &p, rType );
    }
    return OComponentHelper::queryAggregation( rType );
}

//-------------------------------------------------------------------------
#define LCL_CPPUTYPE(t) (::getCppuType( reinterpret_cast< const uno::Reference<t> *>(0)))

uno::Sequence< uno::Type > CreationWizardUnoDlg::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence< uno::Type > aTypeList;

    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !aTypeList.getLength() )
    {
        ::std::vector< uno::Type > aTypes;
        aTypes.push_back( LCL_CPPUTYPE( lang::XComponent ));
        aTypes.push_back( LCL_CPPUTYPE( lang::XTypeProvider ));
        aTypes.push_back( LCL_CPPUTYPE( uno::XAggregation ));
        aTypes.push_back( LCL_CPPUTYPE( uno::XWeak ));
        aTypes.push_back( LCL_CPPUTYPE( lang::XServiceInfo ));
        aTypes.push_back( LCL_CPPUTYPE( lang::XInitialization ));
        aTypes.push_back( LCL_CPPUTYPE( frame::XTerminateListener ));
        aTypes.push_back( LCL_CPPUTYPE( ui::dialogs::XExecutableDialog ));
        aTypes.push_back( LCL_CPPUTYPE( beans::XPropertySet ));
        aTypeList = ::chart::ContainerHelper::ContainerToSequence( aTypes );
    }

    return aTypeList;
}
uno::Sequence< sal_Int8 > SAL_CALL CreationWizardUnoDlg::getImplementationId( void ) throw( uno::RuntimeException )
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}

//-------------------------------------------------------------------------

// XTerminateListener
void SAL_CALL CreationWizardUnoDlg::queryTermination( const lang::EventObject& /*Event*/ ) throw( frame::TerminationVetoException, uno::RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    // we will never give a veto here
    if( m_pDialog && !m_pDialog->isClosable() )
    {
        m_pDialog->ToTop();
        throw frame::TerminationVetoException();
    }
}

//-------------------------------------------------------------------------

void SAL_CALL CreationWizardUnoDlg::notifyTermination( const lang::EventObject& /*Event*/ ) throw (uno::RuntimeException)
{
    // we are going down, so dispose us!
    dispose();
}

void SAL_CALL CreationWizardUnoDlg::disposing( const lang::EventObject& /*Source*/ ) throw (uno::RuntimeException)
{
    //Listener should deregister himself and relaese all references to the closing object.
}

//-------------------------------------------------------------------------
void SAL_CALL CreationWizardUnoDlg::setTitle( const ::rtl::OUString& /*rTitle*/ ) throw(uno::RuntimeException)
{
}
//-------------------------------------------------------------------------
void CreationWizardUnoDlg::createDialogOnDemand()
{
    SolarMutexGuard aSolarGuard;
    if( !m_pDialog )
    {
        Window* pParent = NULL;
        if( !m_xParentWindow.is() && m_xChartModel.is() )
        {
            uno::Reference< frame::XController > xController(
                m_xChartModel->getCurrentController() );
            if( xController.is() )
            {
                uno::Reference< frame::XFrame > xFrame(
                    xController->getFrame() );
                if(xFrame.is())
                    m_xParentWindow = xFrame->getContainerWindow();
            }
        }
        if( m_xParentWindow.is() )
        {
            VCLXWindow* pImplementation = VCLXWindow::GetImplementation(m_xParentWindow);
            if (pImplementation)
                pParent = pImplementation->GetWindow();
        }
        uno::Reference< XComponent > xComp( this );
        if( m_xChartModel.is() )
        {
            m_pDialog = new CreationWizard( pParent, m_xChartModel, m_xCC );
            m_pDialog->AddEventListener( LINK( this, CreationWizardUnoDlg, DialogEventHdl ) );
        }
    }
}
//-------------------------------------------------------------------------
IMPL_LINK( CreationWizardUnoDlg, DialogEventHdl, VclWindowEvent*, pEvent )
{
    if(pEvent && (pEvent->GetId() == VCLEVENT_OBJECT_DYING) )
        m_pDialog = 0;//avoid duplicate destruction of m_pDialog
    return 0;
}

//-------------------------------------------------------------------------
sal_Int16 SAL_CALL CreationWizardUnoDlg::execute(  ) throw(uno::RuntimeException)
{
    sal_Int16 nRet = RET_CANCEL;
    {
        SolarMutexGuard aSolarGuard;
        createDialogOnDemand();
        if( !m_pDialog )
            return nRet;
        TimerTriggeredControllerLock aTimerTriggeredControllerLock( m_xChartModel );
        if( m_bUnlockControllersOnExecute && m_xChartModel.is() )
            m_xChartModel->unlockControllers();
        nRet = m_pDialog->Execute();
    }
    return nRet;
}

//-------------------------------------------------------------------------
void SAL_CALL CreationWizardUnoDlg::initialize( const uno::Sequence< uno::Any >& aArguments ) throw(uno::Exception, uno::RuntimeException)
{
    const uno::Any* pArguments = aArguments.getConstArray();
    for(sal_Int32 i=0; i<aArguments.getLength(); ++i, ++pArguments)
    {
        beans::PropertyValue aProperty;
        if(*pArguments >>= aProperty)
        {
            if( aProperty.Name.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "ParentWindow" ) ) == 0 )
            {
                aProperty.Value >>= m_xParentWindow;
            }
            else if( aProperty.Name.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "ChartModel" ) ) == 0 )
            {
                aProperty.Value >>= m_xChartModel;
            }
        }
    }
}

//-------------------------------------------------------------------------
// ____ OComponentHelper ____
/// Called in dispose method after the listeners were notified.
void SAL_CALL CreationWizardUnoDlg::disposing()
{
    m_xChartModel.clear();
    m_xParentWindow.clear();

    SolarMutexGuard aSolarGuard;
    if( m_pDialog )
    {
        delete m_pDialog;
        m_pDialog = 0;
    }

    try
    {
        uno::Reference< frame::XDesktop > xDesktop(
            m_xCC->getServiceManager()->createInstanceWithContext(
                C2U( "com.sun.star.frame.Desktop" ), m_xCC ), uno::UNO_QUERY );
        if( xDesktop.is() )
        {
            uno::Reference< frame::XTerminateListener > xListener( this );
            xDesktop->removeTerminateListener( xListener );
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

//XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL CreationWizardUnoDlg::getPropertySetInfo()
    throw (uno::RuntimeException)
{
    OSL_FAIL("not implemented");
    return 0;
}

void SAL_CALL CreationWizardUnoDlg::setPropertyValue( const ::rtl::OUString& rPropertyName
                                                     , const uno::Any& rValue )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException
          , lang::WrappedTargetException, uno::RuntimeException)
{
    if( rPropertyName.equals(C2U("Position")) )
    {
        awt::Point aPos;
        if( ! (rValue >>= aPos) )
            throw lang::IllegalArgumentException( C2U("Property 'Position' requires value of type awt::Point"), 0, 0 );

        //set left upper outer corner relative to screen
        //pixels, screen position
        SolarMutexGuard aSolarGuard;
        createDialogOnDemand();
        if( m_pDialog )
        {
            m_pDialog->SetPosPixel( Point(0,0) );
            Rectangle aRect( m_pDialog->GetWindowExtentsRelative( 0 ) );

            Point aNewOuterPos = Point( aPos.X - aRect.Left(), aPos.Y - aRect.Top() );
            m_pDialog->SetPosPixel( aNewOuterPos );
        }
    }
    else if( rPropertyName.equals(C2U("Size")) )
    {
        //read only property, do nothing
    }
    else if( rPropertyName.equals(C2U("UnlockControllersOnExecute")) )
    {
        if( ! (rValue >>= m_bUnlockControllersOnExecute) )
            throw lang::IllegalArgumentException( C2U("Property 'UnlockControllers' requires value of type boolean"), 0, 0 );
    }
    else
        throw beans::UnknownPropertyException( C2U("unknown property was tried to set to chart wizard"), 0 );
}

uno::Any SAL_CALL CreationWizardUnoDlg::getPropertyValue( const ::rtl::OUString& rPropertyName )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;
    if( rPropertyName.equals(C2U("Position")) )
    {
        //get left upper outer corner relative to screen
        //pixels, screen position
        SolarMutexGuard aSolarGuard;
        createDialogOnDemand();
        if( m_pDialog )
        {
            Rectangle aRect( m_pDialog->GetWindowExtentsRelative( 0 ) );
            awt::Point aPoint(aRect.Left(),aRect.Top());
            aRet = uno::makeAny( aPoint );
        }
    }
    else if( rPropertyName.equals(C2U("Size")) )
    {
        //get outer size inclusive decoration
        //pixels, screen position
        SolarMutexGuard aSolarGuard;
        createDialogOnDemand();
        if( m_pDialog )
        {
            Rectangle aRect( m_pDialog->GetWindowExtentsRelative( 0 ) );
            awt::Size aSize(aRect.GetWidth(),aRect.GetHeight());
            aRet = uno::makeAny( aSize );
        }
    }
    else if( rPropertyName.equals(C2U("UnlockControllersOnExecute")) )
    {
        aRet = uno::makeAny( m_bUnlockControllersOnExecute );
    }
    else
        throw beans::UnknownPropertyException( C2U("unknown property was tried to get from chart wizard"), 0 );
    return aRet;
}

void SAL_CALL CreationWizardUnoDlg::addPropertyChangeListener(
        const ::rtl::OUString& /* aPropertyName */, const uno::Reference< beans::XPropertyChangeListener >& /* xListener */ )
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL CreationWizardUnoDlg::removePropertyChangeListener(
    const ::rtl::OUString& /* aPropertyName */, const uno::Reference< beans::XPropertyChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL CreationWizardUnoDlg::addVetoableChangeListener( const ::rtl::OUString& /* PropertyName */, const uno::Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL CreationWizardUnoDlg::removeVetoableChangeListener( const ::rtl::OUString& /* PropertyName */, const uno::Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
