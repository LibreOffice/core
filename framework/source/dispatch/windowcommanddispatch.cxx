/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: windowcommanddispatch.cxx,v $
 * $Revision: 1.3 $
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
#include "precompiled_framework.hxx"

//_______________________________________________
// my own includes

#include <dispatch/windowcommanddispatch.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <targets.h>
#include <services.h>

//_______________________________________________
// interface includes

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

//_______________________________________________
// includes of other projects

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/cmdevt.hxx>
#include <vos/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>

//_______________________________________________
// namespace

namespace framework{

namespace css = ::com::sun::star;

//_______________________________________________
// declarations

const ::rtl::OUString WindowCommandDispatch::COMMAND_PREFERENCES = ::rtl::OUString::createFromAscii(".uno:OptionsTreeDialog");
const ::rtl::OUString WindowCommandDispatch::COMMAND_ABOUTBOX    = ::rtl::OUString::createFromAscii(".uno:About");

//-----------------------------------------------
WindowCommandDispatch::WindowCommandDispatch(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                         const css::uno::Reference< css::frame::XFrame >&              xFrame)
    : ThreadHelpBase(                            )
    , m_xSMGR       (xSMGR                       )
    , m_xFrame      (xFrame                      )
    , m_xWindow     (xFrame->getContainerWindow())
{
    impl_startListening();
}

//-----------------------------------------------
WindowCommandDispatch::~WindowCommandDispatch()
{
    m_xSMGR.clear();
}

//-----------------------------------------------
void SAL_CALL WindowCommandDispatch::disposing(const css::lang::EventObject& /*aSource*/)
    throw (css::uno::RuntimeException)
{
    // We hold our window weak ... so there is no need to clear it's reference here.
    // The window and we will die by ref count automatically.
}

//-----------------------------------------------
void WindowCommandDispatch::impl_startListening()
{
    // SYNCHRONIZED ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::awt::XWindow > xWindow( m_xWindow.get(), css::uno::UNO_QUERY );
    aReadLock.unlock();
    // <- SYNCHRONIZED

    if ( ! xWindow.is())
        return;

    // SYNCHRONIZED ->
    ::vos::OClearableGuard aSolarLock(Application::GetSolarMutex());

    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    if ( ! pWindow)
        return;

    pWindow->AddEventListener( LINK(this, WindowCommandDispatch, impl_notifyCommand) );

    aSolarLock.clear();
    // <- SYNCHRONIZED
}

//-----------------------------------------------
IMPL_LINK(WindowCommandDispatch, impl_notifyCommand, void*, pParam)
{
    if ( ! pParam)
        return 0L;

    const VclWindowEvent* pEvent = (VclWindowEvent*)pParam;
    if (pEvent->GetId() != VCLEVENT_WINDOW_COMMAND)
        return 0L;

    const CommandEvent* pCommand = (CommandEvent*)pEvent->GetData();
    if (pCommand->GetCommand() != COMMAND_SHOWDIALOG)
        return 0L;

    const CommandDialogData* pData = pCommand->GetDialogData();
    if ( ! pData)
        return 0L;

    const int nCommand = pData->GetDialogId();
          ::rtl::OUString sCommand;

    switch (nCommand)
    {
        case SHOWDIALOG_ID_PREFERENCES :
                sCommand = WindowCommandDispatch::COMMAND_PREFERENCES;
                break;

        case SHOWDIALOG_ID_ABOUT :
                sCommand = WindowCommandDispatch::COMMAND_ABOUTBOX;
                break;

        default :
                return 0L;
    }

    impl_dispatchCommand(sCommand);

    return 0L;
}

//-----------------------------------------------
void WindowCommandDispatch::impl_dispatchCommand(const ::rtl::OUString& sCommand)
{
    // ignore all errors here. It's clicking a menu entry only ...
    // The user will try it again, in case nothing happens .-)
    try
    {
        // SYNCHRONIZED ->
        ReadGuard aReadLock(m_aLock);
        css::uno::Reference< css::frame::XDispatchProvider >   xProvider(m_xFrame.get(), css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR    = m_xSMGR;
        aReadLock.unlock();
        // <- SYNCHRONIZED

        // check provider ... we know it's weak reference only
        if ( ! xProvider.is())
            return;

        css::uno::Reference< css::util::XURLTransformer > xParser(xSMGR->createInstance(SERVICENAME_URLTRANSFORMER), css::uno::UNO_QUERY_THROW);
        css::util::URL aCommand;
        aCommand.Complete = sCommand;
        xParser->parseStrict(aCommand);

        css::uno::Reference< css::frame::XDispatch > xDispatch = xProvider->queryDispatch(aCommand, SPECIALTARGET_SELF, 0);
        if (xDispatch.is())
            xDispatch->dispatch(aCommand, css::uno::Sequence< css::beans::PropertyValue >());
    }
    catch(const css::uno::Exception&)
    {}
}

} // namespace framework
