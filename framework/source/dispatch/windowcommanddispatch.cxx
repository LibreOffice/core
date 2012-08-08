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

#include <dispatch/windowcommanddispatch.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <targets.h>
#include <services.h>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <comphelper/componentcontext.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/cmdevt.hxx>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <rtl/logfile.hxx>

//_______________________________________________
// namespace

namespace framework{

namespace css = ::com::sun::star;

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
    impl_stopListening();
    m_xSMGR.clear();
}

//-----------------------------------------------
void WindowCommandDispatch::impl_startListening()
{
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::awt::XWindow > xWindow( m_xWindow.get(), css::uno::UNO_QUERY );
    aReadLock.unlock();

    if ( ! xWindow.is())
        return;

    {
        SolarMutexGuard aSolarLock;

        Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
        if ( ! pWindow)
            return;

        pWindow->AddEventListener( LINK(this, WindowCommandDispatch, impl_notifyCommand) );
    }
}

void WindowCommandDispatch::impl_stopListening()
{
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::awt::XWindow > xWindow( m_xWindow.get(), css::uno::UNO_QUERY );
    aReadLock.unlock();

    if (!xWindow.is())
        return;

    {
        SolarMutexGuard aSolarLock;

        Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
        if (!pWindow)
            return;

        pWindow->RemoveEventListener( LINK(this, WindowCommandDispatch, impl_notifyCommand) );

        m_xWindow.clear();
    }
}


//-----------------------------------------------
IMPL_LINK(WindowCommandDispatch, impl_notifyCommand, void*, pParam)
{
    if ( ! pParam)
        return 0L;

    const VclWindowEvent* pEvent = (VclWindowEvent*)pParam;
    if (pEvent->GetId() == VCLEVENT_OBJECT_DYING)
    {
        impl_stopListening();
        return 0L;
    }
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
                sCommand = rtl::OUString(".uno:OptionsTreeDialog");
                break;

        case SHOWDIALOG_ID_ABOUT :
                sCommand = rtl::OUString(".uno:About");
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

        css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(::comphelper::ComponentContext(xSMGR).getUNOContext()));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
