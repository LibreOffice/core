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

#include <dlg_CreationWizard_UNO.hxx>
#include <dlg_CreationWizard.hxx>
#include <servicenames.hxx>
#include <TimerTriggeredControllerLock.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <tools/diagnose_ex.h>
#include <comphelper/lok.hxx>
#include <sfx2/viewsh.hxx>

namespace chart
{
using namespace ::com::sun::star;

CreationWizardUnoDlg::CreationWizardUnoDlg(const uno::Reference<uno::XComponentContext>& xContext)
    : OComponentHelper(m_aMutex)
    , m_xCC(xContext)
    , m_bUnlockControllersOnExecute(false)
{
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(m_xCC);
    uno::Reference< frame::XTerminateListener > xListener( this );
    xDesktop->addTerminateListener( xListener );
}

CreationWizardUnoDlg::~CreationWizardUnoDlg()
{
    SolarMutexGuard aSolarGuard;
    m_xDialog.reset();
}

// lang::XServiceInfo
OUString SAL_CALL CreationWizardUnoDlg::getImplementationName()
{
    return CHART_WIZARD_DIALOG_SERVICE_IMPLEMENTATION_NAME;
}

sal_Bool SAL_CALL CreationWizardUnoDlg::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL CreationWizardUnoDlg::getSupportedServiceNames()
{
    return { CHART_WIZARD_DIALOG_SERVICE_NAME };
}

// XInterface
uno::Any SAL_CALL CreationWizardUnoDlg::queryInterface( const uno::Type& aType )
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
uno::Any SAL_CALL CreationWizardUnoDlg::queryAggregation( uno::Type const & rType )
{
    if (rType == cppu::UnoType<ui::dialogs::XAsynchronousExecutableDialog>::get())
    {
        void * p = static_cast< ui::dialogs::XAsynchronousExecutableDialog * >( this );
        return uno::Any( &p, rType );
    }
    else if (rType == cppu::UnoType<lang::XServiceInfo>::get())
    {
        void * p = static_cast< lang::XServiceInfo * >( this );
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

uno::Sequence< uno::Type > CreationWizardUnoDlg::getTypes()
{
    static uno::Sequence<uno::Type> aTypeList{ cppu::UnoType<lang::XComponent>::get(),
                                               cppu::UnoType<lang::XTypeProvider>::get(),
                                               cppu::UnoType<uno::XAggregation>::get(),
                                               cppu::UnoType<uno::XWeak>::get(),
                                               cppu::UnoType<lang::XServiceInfo>::get(),
                                               cppu::UnoType<lang::XInitialization>::get(),
                                               cppu::UnoType<frame::XTerminateListener>::get(),
                                               cppu::UnoType<ui::dialogs::XAsynchronousExecutableDialog>::get(),
                                               cppu::UnoType<beans::XPropertySet>::get() };
    return aTypeList;
}

uno::Sequence< sal_Int8 > SAL_CALL CreationWizardUnoDlg::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XTerminateListener
void SAL_CALL CreationWizardUnoDlg::queryTermination( const lang::EventObject& /*Event*/ )
{
}

void SAL_CALL CreationWizardUnoDlg::notifyTermination( const lang::EventObject& /*Event*/ )
{
    // we are going down, so dispose us!
    dispose();
}

void SAL_CALL CreationWizardUnoDlg::disposing( const lang::EventObject& /*Source*/ )
{
    //Listener should deregister himself and release all references to the closing object.
}

void SAL_CALL CreationWizardUnoDlg::setDialogTitle( const OUString& /*rTitle*/ )
{
}
void CreationWizardUnoDlg::createDialogOnDemand()
{
    SolarMutexGuard aSolarGuard;
    if (m_xDialog)
        return;

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
    uno::Reference< XComponent > xKeepAlive( this );
    if( m_xChartModel.is() )
    {
        m_xDialog = std::make_shared<CreationWizard>(Application::GetFrameWeld(m_xParentWindow), m_xChartModel, m_xCC);
    }
}

IMPL_STATIC_LINK_NOARG(CreationWizardUnoDlg, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*)
{
    return SfxViewShell::Current();
}

void SAL_CALL CreationWizardUnoDlg::startExecuteModal( const css::uno::Reference<css::ui::dialogs::XDialogClosedListener>& xListener )
{
    SolarMutexGuard aSolarGuard;
    createDialogOnDemand();

    if( !m_xDialog )
        return;

    m_xDialog->getDialog()->SetInstallLOKNotifierHdl(
                                LINK(this, CreationWizardUnoDlg, InstallLOKNotifierHdl));

    TimerTriggeredControllerLock aTimerTriggeredControllerLock( m_xChartModel );
    if( m_bUnlockControllersOnExecute && m_xChartModel.is() )
        m_xChartModel->unlockControllers();

    CreationWizardUnoDlg* xThat = this;
    weld::DialogController::runAsync(m_xDialog, [xListener, xThat](sal_Int32 nResult){
            if( xListener.is() )
            {
                ::css::uno::Reference< ::css::uno::XInterface > xSource;
                // Notify UNO listener to perform correct action depending on the result
                css::ui::dialogs::DialogClosedEvent aEvent( xSource, nResult );
                xListener->dialogClosed( aEvent );
            }
            xThat->m_xDialog.reset();
        });
}

void SAL_CALL CreationWizardUnoDlg::initialize( const uno::Sequence< uno::Any >& aArguments )
{
    for(const uno::Any& rArgument : aArguments)
    {
        beans::PropertyValue aProperty;
        if(rArgument >>= aProperty)
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
    m_xDialog.reset();

    try
    {
        uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(m_xCC);
        uno::Reference< frame::XTerminateListener > xListener( this );
        xDesktop->removeTerminateListener( xListener );
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

//XPropertySet
uno::Reference< beans::XPropertySetInfo > SAL_CALL CreationWizardUnoDlg::getPropertySetInfo()
{
    OSL_FAIL("not implemented");
    return nullptr;
}

void SAL_CALL CreationWizardUnoDlg::setPropertyValue(const OUString& rPropertyName,
                                                     const uno::Any& rValue)
{
    if( rPropertyName == "Position" )
    {
        SolarMutexGuard aSolarGuard;
        createDialogOnDemand();

        //read only property, do nothing else
    }
    else if( rPropertyName == "Size")
    {
        //read only property, do nothing
    }
    else if( rPropertyName == "UnlockControllersOnExecute" )
    {
        if( ! (rValue >>= m_bUnlockControllersOnExecute) )
            throw lang::IllegalArgumentException( "Property 'UnlockControllers' requires value of type boolean" , nullptr, 0 );
    }
    else
        throw beans::UnknownPropertyException( "unknown property was tried to set to chart wizard" , nullptr );
}

uno::Any SAL_CALL CreationWizardUnoDlg::getPropertyValue( const OUString& rPropertyName )
{
    uno::Any aRet;
    if( rPropertyName == "Position" )
    {
        //get left upper outer corner relative to screen
        //pixels, screen position
        SolarMutexGuard aSolarGuard;
        createDialogOnDemand();
        if (m_xDialog)
        {
            Point aPos(m_xDialog->getDialog()->get_position());
            awt::Point aPoint(aPos.X(), aPos.Y());
            aRet <<= aPoint;
        }
    }
    else if( rPropertyName == "Size" )
    {
        //get outer size inclusive decoration
        //pixels, screen position
        SolarMutexGuard aSolarGuard;
        createDialogOnDemand();
        if (m_xDialog)
        {
            Size aRect(m_xDialog->getDialog()->get_size());
            awt::Size aSize(aRect.Width(), aRect.Height());
            aRet <<= aSize;
        }
    }
    else if( rPropertyName == "UnlockControllersOnExecute" )
    {
        aRet <<= m_bUnlockControllersOnExecute;
    }
    else
        throw beans::UnknownPropertyException( "unknown property was tried to get from chart wizard" , nullptr );
    return aRet;
}

void SAL_CALL CreationWizardUnoDlg::addPropertyChangeListener(
        const OUString& /* aPropertyName */, const uno::Reference< beans::XPropertyChangeListener >& /* xListener */ )
{
    OSL_FAIL("not implemented");
}
void SAL_CALL CreationWizardUnoDlg::removePropertyChangeListener(
    const OUString& /* aPropertyName */, const uno::Reference< beans::XPropertyChangeListener >& /* aListener */ )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL CreationWizardUnoDlg::addVetoableChangeListener( const OUString& /* PropertyName */, const uno::Reference< beans::XVetoableChangeListener >& /* aListener */ )
{
    OSL_FAIL("not implemented");
}

void SAL_CALL CreationWizardUnoDlg::removeVetoableChangeListener( const OUString& /* PropertyName */, const uno::Reference< beans::XVetoableChangeListener >& /* aListener */ )
{
    OSL_FAIL("not implemented");
}

} //namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_chart2_WizardDialog_get_implementation(css::uno::XComponentContext *context,
                                                         css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::CreationWizardUnoDlg(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
