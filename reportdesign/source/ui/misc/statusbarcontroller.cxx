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
#include "precompiled_reportdesign.hxx"
#include "statusbarcontroller.hxx"

#include <svx/zoomsliderctrl.hxx>
#include <svx/zoomctrl.hxx>
#include <svx/svxids.hrc>
#include <svx/zoomitem.hxx>
#include <svx/zoomslideritem.hxx>

#include <vcl/svapp.hxx>
#include <vcl/status.hxx>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>

namespace rptui
{
    using namespace svt;
    using namespace com::sun::star::uno;
    using namespace com::sun::star::beans;
    using namespace com::sun::star::lang;
    using namespace ::com::sun::star::frame;
    using namespace ::com::sun::star::util;

::rtl::OUString SAL_CALL OStatusbarController::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
::rtl::OUString OStatusbarController::getImplementationName_Static() throw( RuntimeException )
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.comp.StatusbarController"));
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> OStatusbarController::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.StatusbarController"));
    return aSupported;
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OStatusbarController::supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException)
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
//-------------------------------------------------------------------------
Sequence< ::rtl::OUString> SAL_CALL OStatusbarController::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XInterface > OStatusbarController::create(Reference< XComponentContext > const & xContext)
{
    return *(new OStatusbarController(Reference< XMultiServiceFactory >(xContext->getServiceManager(),UNO_QUERY)));
}
IMPLEMENT_FORWARD_XINTERFACE2(OStatusbarController, ::svt::StatusbarController,OStatusbarController_BASE)

OStatusbarController::OStatusbarController(const Reference< XMultiServiceFactory >& _rxORB)
: m_nSlotId(0)
,m_nId(1)
{
    m_xServiceManager = _rxORB;
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatusbarController::initialize( const Sequence< Any >& _rArguments ) throw (Exception, RuntimeException)
{
    StatusbarController::initialize(_rArguments);
    SolarMutexGuard aSolarMutexGuard;
    ::osl::MutexGuard aGuard(m_aMutex);

    StatusBar* pStatusBar = static_cast<StatusBar*>(VCLUnoHelper::GetWindow(m_xParentWindow));
    if ( pStatusBar )
    {
        const sal_uInt16 nCount = pStatusBar->GetItemCount();
        for (sal_uInt16 nPos = 0; nPos < nCount; ++nPos)
        {
            const sal_uInt16 nItemId = pStatusBar->GetItemId(nPos);
            if ( pStatusBar->GetItemCommand(nItemId) == String(m_aCommandURL) )
            {
                m_nId = nItemId;
                break;
            }
        }
        if ( m_aCommandURL.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(".uno:ZoomSlider")) )
        {
            m_pController = TStatusbarHelper::createFromQuery(new SvxZoomSliderControl(m_nSlotId = SID_ATTR_ZOOMSLIDER,m_nId,*pStatusBar));
        }
        else if ( m_aCommandURL.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(".uno:Zoom")) )
        {
            m_pController = TStatusbarHelper::createFromQuery(new SvxZoomStatusBarControl(m_nSlotId = SID_ATTR_ZOOM,m_nId,*pStatusBar));
        }

        if ( m_pController.is() )
        {
            m_pController->initialize(_rArguments);
            m_pController->update();
        }

        addStatusListener(m_aCommandURL);
        update();
    }
}
// XStatusListener
void SAL_CALL OStatusbarController::statusChanged( const FeatureStateEvent& _aEvent)throw ( RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( m_pController.is() )
    {
        if ( m_aCommandURL.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(".uno:ZoomSlider")) )
        {
            Sequence< PropertyValue > aSeq;
            if ( (_aEvent.State >>= aSeq) && aSeq.getLength() == 2 )
            {
                SvxZoomSliderItem aZoomSlider(100,20,400);
                aZoomSlider.PutValue(_aEvent.State);
                static_cast<SvxZoomSliderControl*>(m_pController.get())->StateChanged(m_nSlotId,SFX_ITEM_AVAILABLE,&aZoomSlider);
            }
        }
        else if ( m_aCommandURL.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(".uno:Zoom")) )
        {
            Sequence< PropertyValue > aSeq;
            if ( (_aEvent.State >>= aSeq) && aSeq.getLength() == 3 )
            {
                SvxZoomItem aZoom;
                aZoom.PutValue(_aEvent.State);
                static_cast<SvxZoomStatusBarControl*>(m_pController.get())->StateChanged(m_nSlotId,SFX_ITEM_AVAILABLE,&aZoom);
            }
        }
    }
}

// XStatusbarController
::sal_Bool SAL_CALL OStatusbarController::mouseButtonDown(const ::com::sun::star::awt::MouseEvent& _aEvent)throw (::com::sun::star::uno::RuntimeException)
{
    return m_pController.is() && m_pController.getRef()->mouseButtonDown(_aEvent);
}

::sal_Bool SAL_CALL OStatusbarController::mouseMove(    const ::com::sun::star::awt::MouseEvent& _aEvent)throw (::com::sun::star::uno::RuntimeException)
{
    return m_pController.is() && m_pController.getRef()->mouseMove(_aEvent);
}

::sal_Bool SAL_CALL OStatusbarController::mouseButtonUp(    const ::com::sun::star::awt::MouseEvent& _aEvent)throw (::com::sun::star::uno::RuntimeException)
{
    return m_pController.is() && m_pController.getRef()->mouseButtonUp(_aEvent);
}

void SAL_CALL OStatusbarController::command(
    const ::com::sun::star::awt::Point& aPos,
    ::sal_Int32 nCommand,
    ::sal_Bool bMouseEvent,
    const ::com::sun::star::uno::Any& aData )
throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_pController.is() )
        m_pController.getRef()->command( aPos, nCommand, bMouseEvent, aData );
}

void SAL_CALL OStatusbarController::paint(
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics,
    const ::com::sun::star::awt::Rectangle& rOutputRectangle,
    ::sal_Int32 nItemId,
    ::sal_Int32 nStyle )
throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_pController.is() )
        m_pController.getRef()->paint( xGraphics, rOutputRectangle, nItemId, nStyle );
}

void SAL_CALL OStatusbarController::click() throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_pController.is() )
        m_pController.getRef()->click();
}

void SAL_CALL OStatusbarController::doubleClick() throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_pController.is() )
        m_pController.getRef()->doubleClick();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatusbarController::update() throw ( RuntimeException )
{
    ::svt::StatusbarController::update();
    Reference< XUpdatable > xUp(m_pController.getRef(),UNO_QUERY);
    if ( xUp.is() )
        xUp->update();
}
// -----------------------------------------------------------------------------
// XComponent
void SAL_CALL OStatusbarController::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XComponent > xComp( m_pController.getRef(), UNO_QUERY );
    ::comphelper::disposeComponent(xComp);
    m_pController.dispose();
    svt::StatusbarController::dispose();
}
// =============================================================================
} // rptui
// =============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
