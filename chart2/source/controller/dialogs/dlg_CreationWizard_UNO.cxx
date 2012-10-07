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


#include "dlg_CreationWizard_UNO.hxx"
#include "dlg_CreationWizard.hxx"
#include "macros.hxx"
#include "servicenames.hxx"
#include "ContainerHelper.hxx"
#include "TimerTriggeredControllerLock.hxx"
#include <osl/mutex.hxx>
// header for class Application
#include <vcl/svapp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
// header for define RET_CANCEL
#include <vcl/msgbox.hxx>
// header for class OImplementationId
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/servicehelper.hxx>
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
            "com.sun.star.frame.Desktop" , m_xCC ), uno::UNO_QUERY );
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

    uno::Sequence< OUString > CreationWizardUnoDlg
::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSNS( 1 );
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

namespace
{
    class theCreationWizardUnoDlgImplementationId : public rtl::Static< UnoTunnelIdInit, theCreationWizardUnoDlgImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL CreationWizardUnoDlg::getImplementationId( void ) throw( uno::RuntimeException )
{
    return theCreationWizardUnoDlgImplementationId::get().getSeq();
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
void SAL_CALL CreationWizardUnoDlg::setTitle( const OUString& /*rTitle*/ ) throw(uno::RuntimeException)
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
                "com.sun.star.frame.Desktop" , m_xCC ), uno::UNO_QUERY );
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

void SAL_CALL CreationWizardUnoDlg::setPropertyValue( const OUString& rPropertyName
                                                     , const uno::Any& rValue )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException
          , lang::WrappedTargetException, uno::RuntimeException)
{
    if( rPropertyName == "Position" )
    {
        awt::Point aPos;
        if( ! (rValue >>= aPos) )
            throw lang::IllegalArgumentException( "Property 'Position' requires value of type awt::Point", 0, 0 );

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
    else if( rPropertyName == "Size")
    {
        //read only property, do nothing
    }
    else if( rPropertyName == "UnlockControllersOnExecute" )
    {
        if( ! (rValue >>= m_bUnlockControllersOnExecute) )
            throw lang::IllegalArgumentException( "Property 'UnlockControllers' requires value of type boolean" , 0, 0 );
    }
    else
        throw beans::UnknownPropertyException( "unknown property was tried to set to chart wizard" , 0 );
}

uno::Any SAL_CALL CreationWizardUnoDlg::getPropertyValue( const OUString& rPropertyName )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aRet;
    if( rPropertyName == "Position" )
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
    else if( rPropertyName == "Size" )
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
    else if( rPropertyName == "UnlockControllersOnExecute" )
    {
        aRet = uno::makeAny( m_bUnlockControllersOnExecute );
    }
    else
        throw beans::UnknownPropertyException( "unknown property was tried to get from chart wizard" , 0 );
    return aRet;
}

void SAL_CALL CreationWizardUnoDlg::addPropertyChangeListener(
        const OUString& /* aPropertyName */, const uno::Reference< beans::XPropertyChangeListener >& /* xListener */ )
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL CreationWizardUnoDlg::removePropertyChangeListener(
    const OUString& /* aPropertyName */, const uno::Reference< beans::XPropertyChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL CreationWizardUnoDlg::addVetoableChangeListener( const OUString& /* PropertyName */, const uno::Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL CreationWizardUnoDlg::removeVetoableChangeListener( const OUString& /* PropertyName */, const uno::Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
