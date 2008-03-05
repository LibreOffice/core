/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tagwindowasmodified.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:22:42 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_HELPER_TAGWINDOWASMODIFIED_HXX_
#include <helper/tagwindowasmodified.hxx>
#endif

#ifndef __FRAMEWORK_PATTERN_WINDOW_HXX_
#include <pattern/window.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICXEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_FRAMEACTION_HPP_
#include <com/sun/star/frame/FrameAction.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#ifndef _SV_SYSWIN_HXX
#include <vcl/syswin.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_WINTYPES_HXX
#include <vcl/wintypes.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace

namespace framework{

//_________________________________________________________________________________________________________________
//  definitions

//*****************************************************************************************************************
//  XInterface, XTypeProvider

DEFINE_XINTERFACE_4(TagWindowAsModified                                                     ,
                    OWeakObject                                                             ,
                    DIRECT_INTERFACE (css::lang::XTypeProvider                             ),
                    DIRECT_INTERFACE (css::lang::XInitialization                           ),
                    DIRECT_INTERFACE (css::util::XModifyListener                           ),
                    DERIVED_INTERFACE(css::lang::XEventListener, css::util::XModifyListener))

DEFINE_XTYPEPROVIDER_4(TagWindowAsModified        ,
                       css::lang::XTypeProvider   ,
                       css::lang::XInitialization ,
                       css::util::XModifyListener ,
                       css::lang::XEventListener  )

//*****************************************************************************************************************
TagWindowAsModified::TagWindowAsModified(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
    : ThreadHelpBase          (&Application::GetSolarMutex())
    , m_xSMGR                 (xSMGR                        )
{
}

//*****************************************************************************************************************
TagWindowAsModified::~TagWindowAsModified()
{
}

//*****************************************************************************************************************
void SAL_CALL TagWindowAsModified::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    css::uno::Reference< css::frame::XFrame > xFrame;

    if (lArguments.getLength() > 0)
        lArguments[0] >>= xFrame;

    if ( ! xFrame.is ())
        return;

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_xFrame = xFrame ;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    xFrame->addFrameActionListener(this);
    impl_update (xFrame);
}

//*****************************************************************************************************************
void SAL_CALL TagWindowAsModified::modified(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    css::uno::Reference< css::util::XModifiable > xModel (m_xModel.get (), css::uno::UNO_QUERY);
    css::uno::Reference< css::awt::XWindow >      xWindow(m_xWindow.get(), css::uno::UNO_QUERY);
    if (
        ( ! xModel.is  ()       ) ||
        ( ! xWindow.is ()       ) ||
        (aEvent.Source != xModel)
       )
        return;

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    ::sal_Bool bModified = xModel->isModified ();

    // SYNCHRONIZED ->
    ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());

    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    if ( ! pWindow)
        return;

    sal_Bool bSystemWindow = pWindow->IsSystemWindow();
    sal_Bool bWorkWindow   = (pWindow->GetType() == WINDOW_WORKWINDOW);
    if (!bSystemWindow && !bWorkWindow)
        return;

    if (bModified)
        pWindow->SetExtendedStyle(WB_EXT_DOCMODIFIED);
    else
        pWindow->SetExtendedStyle( ! WB_EXT_DOCMODIFIED);

    aSolarGuard.clear();
    // <- SYNCHRONIZED
}

//*****************************************************************************************************************
void SAL_CALL TagWindowAsModified::frameAction(const css::frame::FrameActionEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    if (
        (aEvent.Action != css::frame::FrameAction_COMPONENT_REATTACHED) &&
        (aEvent.Action != css::frame::FrameAction_COMPONENT_ATTACHED  )
       )
        return;

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    css::uno::Reference< css::frame::XFrame > xFrame(m_xFrame.get(), css::uno::UNO_QUERY);
    if (
        ( ! xFrame.is ()        ) ||
        (aEvent.Source != xFrame)
       )
        return;

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    impl_update (xFrame);
}

//*****************************************************************************************************************
void SAL_CALL TagWindowAsModified::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    css::uno::Reference< css::frame::XFrame > xFrame(m_xFrame.get(), css::uno::UNO_QUERY);
    if (
        (xFrame.is ()           ) &&
        (aEvent.Source == xFrame)
       )
    {
        m_xFrame = css::uno::Reference< css::frame::XFrame >();
        return;
    }

    css::uno::Reference< css::frame::XModel > xModel(m_xModel.get(), css::uno::UNO_QUERY);
    if (
        (xModel.is ()           ) &&
        (aEvent.Source == xModel)
       )
    {
        m_xModel = css::uno::Reference< css::frame::XModel >();
        return;
    }

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//*****************************************************************************************************************
void TagWindowAsModified::impl_update (const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    if ( ! xFrame.is ())
        return;

    css::uno::Reference< css::awt::XWindow >       xWindow     = xFrame->getContainerWindow ();
    css::uno::Reference< css::frame::XController > xController = xFrame->getController ();
    css::uno::Reference< css::frame::XModel >      xModel ;
    if (xController.is ())
        xModel = xController->getModel ();

    if (
        ( ! xWindow.is ()) ||
        ( ! xModel.is  ())
       )
        return;

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    // Note: frame was set as member outside ! we have to refresh connections
    // regarding window and model only here.
    m_xWindow = xWindow;
    m_xModel  = xModel ;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::util::XModifyBroadcaster > xModifiable(xModel, css::uno::UNO_QUERY);
    if (xModifiable.is ())
        xModifiable->addModifyListener (this);
}

} // namespace framework
