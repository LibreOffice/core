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

#include <helper/tagwindowasmodified.hxx>
#include <pattern/window.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <macros/generic.hxx>
#include <services.h>

#include <com/sun/star/awt/XWindow.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/FrameAction.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/wintypes.hxx>


namespace framework{


//*****************************************************************************************************************
TagWindowAsModified::TagWindowAsModified()
    : ThreadHelpBase          (&Application::GetSolarMutex())
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
    SolarMutexGuard aSolarGuard;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
