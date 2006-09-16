/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: logotextstatusbarcontroller.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 14:21:33 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_UIELEMENT_LOGOTEXTSTATUSBARCONTROLLER_HXX_
#include <uielement/logotextstatusbarcontroller.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_FWKRESID_HXX_
#include <classes/fwkresid.hxx>
#endif
#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif
#include <classes/resource.hrc>

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_STATUS_HXX
#include <vcl/status.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   LogoTextStatusbarController             ,
                                            OWeakObject                             ,
                                            SERVICENAME_STATUSBARCONTROLLER         ,
                                            IMPLEMENTATIONNAME_LOGOTEXTSTATUSBARCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   LogoTextStatusbarController, {} )

LogoTextStatusbarController::LogoTextStatusbarController()
{
    m_aLogoText = String( FwkResId( STR_STATUSBAR_LOGOTEXT ));
}

LogoTextStatusbarController::LogoTextStatusbarController( const uno::Reference< lang::XMultiServiceFactory >& xServiceManager ) :
    svt::StatusbarController( xServiceManager, uno::Reference< frame::XFrame >(), rtl::OUString(), 0 )
{
    m_aLogoText = String( FwkResId( STR_STATUSBAR_LOGOTEXT ));
}

LogoTextStatusbarController::~LogoTextStatusbarController()
{
}

// XInterface
Any SAL_CALL LogoTextStatusbarController::queryInterface( const Type& rType )
throw ( RuntimeException )
{
    return svt::StatusbarController::queryInterface( rType );
}

void SAL_CALL LogoTextStatusbarController::acquire() throw ()
{
    svt::StatusbarController::acquire();
}

void SAL_CALL LogoTextStatusbarController::release() throw ()
{
    svt::StatusbarController::release();
}

void SAL_CALL LogoTextStatusbarController::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    svt::StatusbarController::initialize( aArguments );

    if ( m_xParentWindow.is() && m_nID > 0 )
    {
        Window* pWindow = VCLUnoHelper::GetWindow( m_xParentWindow );
        if ( pWindow && ( pWindow->GetType() == WINDOW_STATUSBAR ))
        {
            StatusBar* pStatusBar = (StatusBar *)pWindow;
            pStatusBar->SetItemText( m_nID, m_aLogoText );
        }
    }
}

// XComponent
void SAL_CALL LogoTextStatusbarController::dispose()
throw (::com::sun::star::uno::RuntimeException)
{
    svt::StatusbarController::dispose();
}

// XEventListener
void SAL_CALL LogoTextStatusbarController::disposing( const EventObject& Source )
throw ( RuntimeException )
{
    svt::StatusbarController::disposing( Source );
}

// XStatusListener
void SAL_CALL LogoTextStatusbarController::statusChanged( const FeatureStateEvent& )
throw ( RuntimeException )
{
}

// XStatusbarController
::sal_Bool SAL_CALL LogoTextStatusbarController::mouseButtonDown(
    const ::com::sun::star::awt::MouseEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

::sal_Bool SAL_CALL LogoTextStatusbarController::mouseMove(
    const ::com::sun::star::awt::MouseEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

::sal_Bool SAL_CALL LogoTextStatusbarController::mouseButtonUp(
    const ::com::sun::star::awt::MouseEvent& )
throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL LogoTextStatusbarController::command(
    const ::com::sun::star::awt::Point& aPos,
    ::sal_Int32 nCommand,
    ::sal_Bool bMouseEvent,
    const ::com::sun::star::uno::Any& aData )
throw (::com::sun::star::uno::RuntimeException)
{
    svt::StatusbarController::command( aPos, nCommand, bMouseEvent, aData );
}

void SAL_CALL LogoTextStatusbarController::paint(
    const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics,
    const ::com::sun::star::awt::Rectangle& rOutputRectangle,
    ::sal_Int32 nItemId,
    ::sal_Int32 nStyle )
throw (::com::sun::star::uno::RuntimeException)
{
    svt::StatusbarController::paint( xGraphics, rOutputRectangle, nItemId, nStyle );
}

void SAL_CALL LogoTextStatusbarController::click()
throw (::com::sun::star::uno::RuntimeException)
{
    svt::StatusbarController::click();
}

void SAL_CALL LogoTextStatusbarController::doubleClick() throw (::com::sun::star::uno::RuntimeException)
{
    svt::StatusbarController::doubleClick();
}

}
