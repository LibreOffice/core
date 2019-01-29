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

#include <statusbarcontroller.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <comphelper/types.hxx>
#include <svx/zoomsliderctrl.hxx>
#include <svx/zoomctrl.hxx>
#include <svx/svxids.hrc>
#include <sfx2/zoomitem.hxx>
#include <svx/zoomslideritem.hxx>

#include <vcl/svapp.hxx>
#include <vcl/status.hxx>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace rptui
{
    using namespace svt;
    using namespace com::sun::star::uno;
    using namespace com::sun::star::beans;
    using namespace com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;

OUString SAL_CALL OStatusbarController::getImplementationName()
{
    return getImplementationName_Static();
}

OUString OStatusbarController::getImplementationName_Static()
{
    return OUString("com.sun.star.report.comp.StatusbarController");
}

Sequence< OUString> OStatusbarController::getSupportedServiceNames_Static()
{
    Sequence<OUString> aSupported { "com.sun.star.frame.StatusbarController" };
    return aSupported;
}

sal_Bool SAL_CALL OStatusbarController::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString> SAL_CALL OStatusbarController::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

Reference< XInterface > OStatusbarController::create(Reference< XComponentContext > const & xContext)
{
    return *(new OStatusbarController(xContext));
}
IMPLEMENT_FORWARD_XINTERFACE2(OStatusbarController, ::svt::StatusbarController,OStatusbarController_BASE)

OStatusbarController::OStatusbarController(const Reference< XComponentContext >& rxContext)
: ::svt::StatusbarController(rxContext, Reference< XFrame >(), OUString(), 0)
,m_nSlotId(0)
,m_nId(1)
{
}

void SAL_CALL OStatusbarController::initialize( const Sequence< Any >& _rArguments )
{
    StatusbarController::initialize(_rArguments);
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard(m_aMutex);

    VclPtr< StatusBar > pStatusBar = static_cast<StatusBar*>(VCLUnoHelper::GetWindow(m_xParentWindow).get());
    if ( pStatusBar )
    {
        const sal_uInt16 nCount = pStatusBar->GetItemCount();
        for (sal_uInt16 nPos = 0; nPos < nCount; ++nPos)
        {
            const sal_uInt16 nItemId = pStatusBar->GetItemId(nPos);
            if ( pStatusBar->GetItemCommand(nItemId) == m_aCommandURL )
            {
                m_nId = nItemId;
                break;
            }
        }

        SfxStatusBarControl *pController = nullptr;
        if ( m_aCommandURL == ".uno:ZoomSlider" )
        {
            m_nSlotId = SID_ATTR_ZOOMSLIDER;
            pController = new SvxZoomSliderControl(m_nSlotId,m_nId,*pStatusBar);
        }
        else if ( m_aCommandURL == ".uno:Zoom" )
        {
            m_nSlotId = SID_ATTR_ZOOM;
            pController = new SvxZoomStatusBarControl(m_nSlotId,m_nId,*pStatusBar);
        }

        if ( pController )
        {
            m_rController.set( pController );
            if ( m_rController.is() )
            {
                m_rController->initialize(_rArguments);
                m_rController->update();
            }
        }

        addStatusListener(m_aCommandURL);
        update();
    }
}
// XStatusListener
void SAL_CALL OStatusbarController::statusChanged( const FeatureStateEvent& _aEvent)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( m_rController.is() )
    {
        if ( m_aCommandURL == ".uno:ZoomSlider" )
        {
            Sequence< PropertyValue > aSeq;
            if ( (_aEvent.State >>= aSeq) && aSeq.getLength() == 2 )
            {
                SvxZoomSliderItem aZoomSlider(100,20,400);
                aZoomSlider.PutValue(_aEvent.State, 0);
                static_cast<SvxZoomSliderControl*>(m_rController.get())->StateChanged(m_nSlotId,SfxItemState::DEFAULT,&aZoomSlider);
            }
        }
        else if ( m_aCommandURL == ".uno:Zoom" )
        {
            Sequence< PropertyValue > aSeq;
            if ( (_aEvent.State >>= aSeq) && aSeq.getLength() == 3 )
            {
                SvxZoomItem aZoom;
                aZoom.PutValue(_aEvent.State, 0 );
                static_cast<SvxZoomStatusBarControl*>(m_rController.get())->StateChanged(m_nSlotId,SfxItemState::DEFAULT,&aZoom);
            }
        }
    }
}

// XStatusbarController
sal_Bool SAL_CALL OStatusbarController::mouseButtonDown(const css::awt::MouseEvent& _aEvent)
{
    return m_rController.is() && m_rController->mouseButtonDown(_aEvent);
}

sal_Bool SAL_CALL OStatusbarController::mouseMove(    const css::awt::MouseEvent& _aEvent)
{
    return m_rController.is() && m_rController->mouseMove(_aEvent);
}

sal_Bool SAL_CALL OStatusbarController::mouseButtonUp(    const css::awt::MouseEvent& _aEvent)
{
    return m_rController.is() && m_rController->mouseButtonUp(_aEvent);
}

void SAL_CALL OStatusbarController::command(
    const css::awt::Point& aPos,
    ::sal_Int32 nCommand,
    sal_Bool bMouseEvent,
    const css::uno::Any& aData )
{
    if ( m_rController.is() )
        m_rController->command( aPos, nCommand, bMouseEvent, aData );
}

void SAL_CALL OStatusbarController::paint(
    const css::uno::Reference< css::awt::XGraphics >& xGraphics,
    const css::awt::Rectangle& rOutputRectangle,
    ::sal_Int32 nStyle )
{
    if ( m_rController.is() )
        m_rController->paint( xGraphics, rOutputRectangle, nStyle );
}

void SAL_CALL OStatusbarController::click(
    const css::awt::Point& aPos )
{
    if ( m_rController.is() )
        m_rController->click( aPos );
}

void SAL_CALL OStatusbarController::doubleClick(
    const css::awt::Point& aPos )
{
    if ( m_rController.is() )
        m_rController->doubleClick( aPos );
}

void SAL_CALL OStatusbarController::update()
{
    ::svt::StatusbarController::update();
    if ( m_rController.is() )
        m_rController->update();
}

// XComponent
void SAL_CALL OStatusbarController::dispose()
{
    if ( m_rController.is() )
        ::comphelper::disposeComponent( m_rController );

    svt::StatusbarController::dispose();
}

} // rptui


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
