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

#include <dispatch/windowcommanddispatch.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <targets.h>
#include <services.h>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <comphelper/processfactory.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/cmdevt.hxx>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>


namespace framework{

//-----------------------------------------------
WindowCommandDispatch::WindowCommandDispatch(const css::uno::Reference< css::uno::XComponentContext >& xContext ,
                         const css::uno::Reference< css::frame::XFrame >&              xFrame)
    : ThreadHelpBase(                            )
    , m_xContext    (xContext                    )
    , m_xFrame      (xFrame                      )
    , m_xWindow     (xFrame->getContainerWindow())
{
    impl_startListening();
}

//-----------------------------------------------
WindowCommandDispatch::~WindowCommandDispatch()
{
    impl_stopListening();
    m_xContext.clear();
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
    OUString sCommand;

    switch (nCommand)
    {
        case SHOWDIALOG_ID_PREFERENCES :
                sCommand = OUString(".uno:OptionsTreeDialog");
                break;

        case SHOWDIALOG_ID_ABOUT :
                sCommand = OUString(".uno:About");
                break;

        default :
                return 0L;
    }

    impl_dispatchCommand(sCommand);

    return 0L;
}

//-----------------------------------------------
void WindowCommandDispatch::impl_dispatchCommand(const OUString& sCommand)
{
    // ignore all errors here. It's clicking a menu entry only ...
    // The user will try it again, in case nothing happens .-)
    try
    {
        // SYNCHRONIZED ->
        ReadGuard aReadLock(m_aLock);
        css::uno::Reference< css::frame::XDispatchProvider >   xProvider(m_xFrame.get(), css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::uno::XComponentContext >     xContext    = m_xContext;
        aReadLock.unlock();
        // <- SYNCHRONIZED

        // check provider ... we know it's weak reference only
        if ( ! xProvider.is())
            return;

        css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(xContext));
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
