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
#include <vcl/svapp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/msgbox.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/servicehelper.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/Desktop.hpp>

namespace chart
{
using namespace ::com::sun::star;

CreationWizardUnoDlg::CreationWizardUnoDlg( const uno::Reference< uno::XComponentContext >& xContext )
                    : OComponentHelper( m_aMutex )
                    , m_xChartModel( 0 )
                    , m_xCC( xContext )
                    , m_xParentWindow( 0 )
                    , m_pDialog( 0 )
                    , m_bUnlockControllersOnExecute(false)
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(m_xCC);
    uno::Reference< frame::XTerminateListener > xListener( this );
    xDesktop->addTerminateListener( xListener );
}
CreationWizardUnoDlg::~CreationWizardUnoDlg()
{
    SolarMutexGuard aSolarGuard;
    m_pDialog.disposeAndClear();
}
// lang::XServiceInfo
OUString SAL_CALL CreationWizardUnoDlg::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString CreationWizardUnoDlg::getImplementationName_Static()
{
    return OUString(CHART_WIZARD_DIALOG_SERVICE_IMPLEMENTATION_NAME);
}

sal_Bool SAL_CALL CreationWizardUnoDlg::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL CreationWizardUnoDlg::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > CreationWizardUnoDlg::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSNS( 1 );
    aSNS.getArray()[ 0 ] = CHART_WIZARD_DIALOG_SERVICE_NAME;
    return aSNS;
}

// XInterface
uno::Any SAL_CALL CreationWizardUnoDlg::queryInterface( const uno::Type& aType ) throw (uno::RuntimeException, std::exception)
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
uno::Any SAL_CALL CreationWizardUnoDlg::queryAggregation( uno::Type const & rType ) throw (uno::RuntimeException, std::exception)
{
    if (rType == cppu::UnoType<ui::dialogs::XExecutableDialog>::get())
    {
        void * p = static_cast< ui::dialogs::XExecutableDialog * >( this );
        return uno::Any( &p, rType );
    }
    else if (rType == cppu::UnoType<lang::XServiceInfo>::get())
    {
        void * p = static_cast< lang::XTypeProvider * >( this );
        return uno::Any( &p, rType );
    }
    else if (rType == cppu::UnoType<lang::XInitialization>::get())
    {
        void * p = static_cast< lang::XInitialization * >( this );
        return uno::Any( &p, rType );
    }
    else if (rType == cppu::UnoType<frame::XTerminateListener>::get())
    {
        void * p = static_cast< frame::XTerminateListener * >( this );
        return uno::Any( &p, rType );
    }
    else if (rType == cppu::UnoType<beans::XPropertySet>::get())
    {
        void * p = static_cast< beans::XPropertySet * >( this );
        return uno::Any( &p, rType );
    }
    return OComponentHelper::queryAggregation( rType );
}

uno::Sequence< uno::Type > CreationWizardUnoDlg::getTypes() throw(uno::RuntimeException, std::exception)
{
    static uno::Sequence< uno::Type > aTypeList;

    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if( !aTypeList.getLength() )
    {
        ::std::vector< uno::Type > aTypes;
        aTypes.push_back( cppu::UnoType<lang::XComponent>::get() );
        aTypes.push_back( cppu::UnoType<lang::XTypeProvider>::get() );
        aTypes.push_back( cppu::UnoType<uno::XAggregation>::get() );
        aTypes.push_back( cppu::UnoType<uno::XWeak>::get() );
        aTypes.push_back( cppu::UnoType<lang::XServiceInfo>::get() );
        aTypes.push_back( cppu::UnoType<lang::XInitialization>::get() );
        aTypes.push_back( cppu::UnoType<frame::XTerminateListener>::get() );
        aTypes.push_back( cppu::UnoType<ui::dialogs::XExecutableDialog>::get() );
        aTypes.push_back( cppu::UnoType<beans::XPropertySet>::get() );
        aTypeList = ::chart::ContainerHelper::ContainerToSequence( aTypes );
    }

    return aTypeList;
}

uno::Sequence< sal_Int8 > SAL_CALL CreationWizardUnoDlg::getImplementationId() throw( uno::RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

// XTerminateListener
void SAL_CALL CreationWizardUnoDlg::queryTermination( const lang::EventObject& /*Event*/ ) throw( frame::TerminationVetoException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;

    // we will never give a veto here
    if( m_pDialog && !m_pDialog->isClosable() )
    {
        m_pDialog->ToTop();
        throw frame::TerminationVetoException();
    }
}

void SAL_CALL CreationWizardUnoDlg::notifyTermination( const lang::EventObject& /*Event*/ ) throw (uno::RuntimeException, std::exception)
{
    // we are going down, so dispose us!
    dispose();
}

void SAL_CALL CreationWizardUnoDlg::disposing( const lang::EventObject& /*Source*/ ) throw (uno::RuntimeException, std::exception)
{
    //Listener should deregister himself and release all references to the closing object.
}

void SAL_CALL CreationWizardUnoDlg::setTitle( const OUString& /*rTitle*/ ) throw(uno::RuntimeException, std::exception)
{
}
void CreationWizardUnoDlg::createDialogOnDemand()
{
    SolarMutexGuard aSolarGuard;
    if( !m_pDialog )
    {
        vcl::Window* pParent = NULL;
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
            m_pDialog = VclPtr<CreationWizard>::Create( pParent, m_xChartModel, m_xCC );
            m_pDialog->AddEventListener( LINK( this, CreationWizardUnoDlg, DialogEventHdl ) );
        }
    }
}
IMPL_LINK_TYPED( CreationWizardUnoDlg, DialogEventHdl, VclWindowEvent&, rEvent, void )
{
    if(rEvent.GetId() == VCLEVENT_OBJECT_DYING)
        m_pDialog = 0;//avoid duplicate destruction of m_pDialog
}

sal_Int16 SAL_CALL CreationWizardUnoDlg::execute(  ) throw(uno::RuntimeException, std::exception)
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

void SAL_CALL CreationWizardUnoDlg::initialize( const uno::Sequence< uno::Any >& aArguments ) throw(uno::Exception, uno::RuntimeException, std::exception)
{
    const uno::Any* pArguments = aArguments.getConstArray();
    for(sal_Int32 i=0; i<aArguments.getLength(); ++i, ++pArguments)
    {
        beans::PropertyValue aProperty;
        if(*pArguments >>= aProperty)
        {
            if( aProperty.Name == "ParentWindow" )
            {
                aProperty.Value >>= m_xParentWindow;
            }
            else if( aProperty.Name == "ChartModel" )
            {
                aProperty.Value >>= m_xChartModel;
            }
        }
    }
}

// ____ OComponentHelper ____
/// Called in dispose method after the listeners were notified.
void SAL_CALL CreationWizardUnoDlg::disposing()
{
    m_xChartModel.clear();
    m_xParentWindow.clear();

    SolarMutexGuard aSolarGuard;
    m_pDialog.disposeAndClear();

    try
    {
        uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(m_xCC);
        uno::Reference< frame::XTerminateListener > xListener( this );
        xDesktop->removeTerminateListener( xListener );
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

//XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL CreationWizardUnoDlg::getPropertySetInfo()
    throw (uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
    return 0;
}

void SAL_CALL CreationWizardUnoDlg::setPropertyValue( const OUString& rPropertyName
                                                     , const uno::Any& rValue )
    throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException
          , lang::WrappedTargetException, uno::RuntimeException, std::exception)
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
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
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
        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}
void SAL_CALL CreationWizardUnoDlg::removePropertyChangeListener(
    const OUString& /* aPropertyName */, const uno::Reference< beans::XPropertyChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL CreationWizardUnoDlg::addVetoableChangeListener( const OUString& /* PropertyName */, const uno::Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL CreationWizardUnoDlg::removeVetoableChangeListener( const OUString& /* PropertyName */, const uno::Reference< beans::XVetoableChangeListener >& /* aListener */ )
    throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    OSL_FAIL("not implemented");
}

} //namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_chart2_WizardDialog_get_implementation(css::uno::XComponentContext *context,
                                                         css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::CreationWizardUnoDlg(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
