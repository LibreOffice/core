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

#include "services/taskcreatorsrv.hxx"

#include <helper/persistentwindowstate.hxx>
#include <helper/tagwindowasmodified.hxx>
#include <helper/titlebarupdate.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <loadenv/targethelper.hxx>
#include <services.h>

#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>

#include <svtools/colorcfg.hxx>
#include <vcl/svapp.hxx>

#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>

//_______________________________________________
// namespaces

namespace framework
{
//-----------------------------------------------
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE_2(TaskCreatorService                ,
                                       ::cppu::OWeakObject               ,
                                       "com.sun.star.frame.TaskCreator",
                                       IMPLEMENTATIONNAME_FWK_TASKCREATOR)

//-----------------------------------------------
DEFINE_INIT_SERVICE(
                    TaskCreatorService,
                    {
                        /*Attention
                            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                            to create a new instance of this class by our own supported service factory.
                            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further information!
                        */
                    }
                   )

//-----------------------------------------------
TaskCreatorService::TaskCreatorService(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : ThreadHelpBase     (&Application::GetSolarMutex())
    , m_xContext         (xContext                     )
{
}

//-----------------------------------------------
TaskCreatorService::~TaskCreatorService()
{
}

//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > SAL_CALL TaskCreatorService::createInstance()
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    return createInstanceWithArguments(css::uno::Sequence< css::uno::Any >());
}

//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > SAL_CALL TaskCreatorService::createInstanceWithArguments(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    ::comphelper::SequenceAsHashMap lArgs(lArguments);

    css::uno::Reference< css::frame::XFrame > xParentFrame                  = lArgs.getUnpackedValueOrDefault(OUString(ARGUMENT_PARENTFRAME)                  , css::uno::Reference< css::frame::XFrame >());
    OUString                           sFrameName                    = lArgs.getUnpackedValueOrDefault(OUString(ARGUMENT_FRAMENAME)                    , OUString()                          );
    sal_Bool                                  bVisible                      = lArgs.getUnpackedValueOrDefault(OUString(ARGUMENT_MAKEVISIBLE)                  , sal_False                                  );
    sal_Bool                                  bCreateTopWindow              = lArgs.getUnpackedValueOrDefault(OUString(ARGUMENT_CREATETOPWINDOW)              , sal_True                                   );
    // only possize=[0,0,0,0] triggers default handling of vcl !
    css::awt::Rectangle                       aPosSize                      = lArgs.getUnpackedValueOrDefault(OUString(ARGUMENT_POSSIZE)                      , css::awt::Rectangle(0, 0, 0, 0)            );
    css::uno::Reference< css::awt::XWindow >  xContainerWindow              = lArgs.getUnpackedValueOrDefault(OUString(ARGUMENT_CONTAINERWINDOW)              , css::uno::Reference< css::awt::XWindow >() );
    sal_Bool                                  bSupportPersistentWindowState = lArgs.getUnpackedValueOrDefault(OUString(ARGUMENT_SUPPORTPERSISTENTWINDOWSTATE) , sal_False                                  );
    sal_Bool                                  bEnableTitleBarUpdate         = lArgs.getUnpackedValueOrDefault(OUString(ARGUMENT_ENABLE_TITLEBARUPDATE)        , sal_True                                   );

    // We use FrameName property to set it as API name of the new created frame later.
    // But those frame names must be different from the set of special target names as e.g. _blank, _self etcpp !
    OUString sRightName = impl_filterNames(sFrameName);

    // if no external frame window was given ... create a new one.
    if ( ! xContainerWindow.is())
    {
        css::uno::Reference< css::awt::XWindow > xParentWindow;
        if (xParentFrame.is())
            xParentWindow = xParentFrame->getContainerWindow();

        // Parent has no own window ...
        // So we have to create a top level window always !
        if ( ! xParentWindow.is())
            bCreateTopWindow = sal_True;

        xContainerWindow = implts_createContainerWindow(xParentWindow, aPosSize, bCreateTopWindow);
    }

    // #i53630#
    // Mark all document windows as "special ones", so VCL can bind
    // special features to it. Because VCL doesnt know anything about documents ...
    // Note: Doing so it's no longer supported, that e.g. our wizards can use findFrame(_blank)
    // to create it's previes frames. They must do it manually by using WindowDescriptor+Toolkit!
    css::uno::Reference< css::frame::XDesktop > xDesktop(xParentFrame, css::uno::UNO_QUERY);
    ::sal_Bool bTopLevelDocumentWindow = (
                                            sRightName.isEmpty() &&
                                            (
                                                (! xParentFrame.is() )    ||
                                                (  xDesktop.is()     )
                                            )
                                         );
    if (bTopLevelDocumentWindow)
        implts_applyDocStyleToWindow(xContainerWindow);
    //------------------->

    // create the new frame
    css::uno::Reference< css::frame::XFrame2 > xFrame = implts_createFrame(xParentFrame, xContainerWindow, sRightName);

    // special freature:
    // A special listener will restore pos/size states in case
    // a component was loaded into the frame first time.
    if (bSupportPersistentWindowState)
        implts_establishWindowStateListener(xFrame);

    // special feature: On Mac we need tagging the window in case
    // the underlying model was modified.
    // VCL will ignore our calls in case different platform then Mac
    // is used ...
    if (bTopLevelDocumentWindow)
        implts_establishDocModifyListener (xFrame);

    // special freature:
    // A special listener will update title bar (text and icon)
    // if component of frame will be changed.
    if (bEnableTitleBarUpdate)
        implts_establishTitleBarUpdate(xFrame);

    // Make it visible directly here ...
    // if its required from outside.
    if (bVisible)
        xContainerWindow->setVisible(bVisible);

    return css::uno::Reference< css::uno::XInterface >(xFrame, css::uno::UNO_QUERY_THROW);
}

//-----------------------------------------------
void TaskCreatorService::implts_applyDocStyleToWindow(const css::uno::Reference< css::awt::XWindow >& xWindow) const
{
    // SYNCHRONIZED ->
    SolarMutexGuard aSolarGuard;
    Window* pVCLWindow = VCLUnoHelper::GetWindow(xWindow);
    if (pVCLWindow)
        pVCLWindow->SetExtendedStyle(WB_EXT_DOCUMENT);
    // <- SYNCHRONIZED
}

//-----------------------------------------------
css::uno::Reference< css::awt::XWindow > TaskCreatorService::implts_createContainerWindow( const css::uno::Reference< css::awt::XWindow >& xParentWindow ,
                                                                                           const css::awt::Rectangle&                      aPosSize      ,
                                                                                                 sal_Bool                                  bTopWindow    )
{
    // SAFE  ->
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    // <- SAFE

    // get toolkit to create task container window
    css::uno::Reference< css::awt::XToolkit2 > xToolkit = css::awt::Toolkit::create( xContext );

    // Check if child frames can be created realy. We need at least a valid window at the parent frame ...
    css::uno::Reference< css::awt::XWindowPeer > xParentWindowPeer;
    if ( ! bTopWindow)
    {
        if ( ! xParentWindow.is())
            bTopWindow = sal_False;
        else
            xParentWindowPeer = css::uno::Reference< css::awt::XWindowPeer >(xParentWindow, css::uno::UNO_QUERY_THROW);
    }

    // describe window properties.
    css::awt::WindowDescriptor aDescriptor;
    if (bTopWindow)
    {
        aDescriptor.Type                =   css::awt::WindowClass_TOP                       ;
        aDescriptor.WindowServiceName   =   OUString("window");
        aDescriptor.ParentIndex         =   -1                                              ;
        aDescriptor.Parent              =   css::uno::Reference< css::awt::XWindowPeer >()  ;
        aDescriptor.Bounds              =   aPosSize                                        ;
        aDescriptor.WindowAttributes    =   css::awt::WindowAttribute::BORDER               |
                                            css::awt::WindowAttribute::MOVEABLE             |
                                            css::awt::WindowAttribute::SIZEABLE             |
                                            css::awt::WindowAttribute::CLOSEABLE            |
                                            css::awt::VclWindowPeerAttribute::CLIPCHILDREN  ;
    }
    else
    {
        aDescriptor.Type                =   css::awt::WindowClass_TOP                       ;
        aDescriptor.WindowServiceName   =   OUString("dockingwindow");
        aDescriptor.ParentIndex         =   1                                               ;
        aDescriptor.Parent              =   xParentWindowPeer                               ;
        aDescriptor.Bounds              =   aPosSize                                        ;
        aDescriptor.WindowAttributes    =   css::awt::VclWindowPeerAttribute::CLIPCHILDREN  ;
    }

    // create a new blank container window and get access to parent container to append new created task.
    css::uno::Reference< css::awt::XWindowPeer > xPeer      = xToolkit->createWindow( aDescriptor );
    css::uno::Reference< css::awt::XWindow >     xWindow    ( xPeer, css::uno::UNO_QUERY );
    if ( ! xWindow.is())
        throw css::uno::Exception("TaskCreator service was not able to create suitable frame window.",
                                  static_cast< ::cppu::OWeakObject* >(this));
    if (bTopWindow)
        xPeer->setBackground(::svtools::ColorConfig().GetColorValue(::svtools::APPBACKGROUND).nColor);
    else
        xPeer->setBackground(0xffffffff);

    return xWindow;
}

//-----------------------------------------------
css::uno::Reference< css::frame::XFrame2 > TaskCreatorService::implts_createFrame( const css::uno::Reference< css::frame::XFrame >& xParentFrame    ,
                                                                                  const css::uno::Reference< css::awt::XWindow >&  xContainerWindow,
                                                                                  const OUString&                           sName           )
{
    // SAFE  ->
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    // <- SAFE

    // create new frame.
    css::uno::Reference< css::frame::XFrame2 > xNewFrame = css::frame::Frame::create( xContext );

    // Set window on frame.
    // Do it before calling any other interface methods ...
    // The new created frame must be initialized before you can do anything else there.
    xNewFrame->initialize( xContainerWindow );

    // Put frame to the frame tree.
    // Note: The property creator/parent will be set on the new putted frame automaticly ... by the parent container.
    if (xParentFrame.is())
    {
        css::uno::Reference< css::frame::XFramesSupplier > xSupplier  (xParentFrame, css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::frame::XFrames >         xContainer = xSupplier->getFrames();
        xContainer->append( css::uno::Reference<css::frame::XFrame>(xNewFrame, css::uno::UNO_QUERY_THROW) );
    }

    // Set it's API name (if there is one from outside)
    if (!sName.isEmpty())
        xNewFrame->setName( sName );

    return xNewFrame;
}

//-----------------------------------------------
void TaskCreatorService::implts_establishWindowStateListener( const css::uno::Reference< css::frame::XFrame2 >& xFrame )
{
    // SAFE  ->
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    // <- SAFE

    // Special feature: It's allowed for frames using a top level window only!
    // We must create a special listener service and couple it with the new created task frame.
    // He will restore or save the window state of it ...
    // See used classes for further information too.
    PersistentWindowState* pPersistentStateHandler = new PersistentWindowState( xContext );
    css::uno::Reference< css::lang::XInitialization > xInit(static_cast< ::cppu::OWeakObject* >(pPersistentStateHandler), css::uno::UNO_QUERY_THROW);

    css::uno::Sequence< css::uno::Any > lInitData(1);
    lInitData[0] <<= xFrame;
    xInit->initialize(lInitData);
}

//-----------------------------------------------
void TaskCreatorService::implts_establishDocModifyListener( const css::uno::Reference< css::frame::XFrame2 >& xFrame )
{
    // Special feature: It's allowed for frames using a top level window only!
    // We must create a special listener service and couple it with the new created task frame.
    // It will tag the window as modified if the underlying model was modified ...
    TagWindowAsModified* pTag = new TagWindowAsModified();
    css::uno::Reference< css::lang::XInitialization > xInit(static_cast< ::cppu::OWeakObject* >(pTag), css::uno::UNO_QUERY_THROW);

    css::uno::Sequence< css::uno::Any > lInitData(1);
    lInitData[0] <<= xFrame;
    xInit->initialize(lInitData);
}

//-----------------------------------------------
void TaskCreatorService::implts_establishTitleBarUpdate( const css::uno::Reference< css::frame::XFrame2 >& xFrame )
{
    // SAFE  ->
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    // <- SAFE

    TitleBarUpdate* pHelper = new TitleBarUpdate (xContext);
    css::uno::Reference< css::lang::XInitialization > xInit(static_cast< ::cppu::OWeakObject* >(pHelper), css::uno::UNO_QUERY_THROW);

    css::uno::Sequence< css::uno::Any > lInitData(1);
    lInitData[0] <<= xFrame;
    xInit->initialize(lInitData);
}

//-----------------------------------------------
OUString TaskCreatorService::impl_filterNames( const OUString& sName )
{
    OUString sFiltered;
    if (TargetHelper::isValidNameForFrame(sName))
        sFiltered = sName;
    return sFiltered;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
