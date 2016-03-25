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

#include <framework/titlehelper.hxx>
#include <classes/fwkresid.hxx>
#include <classes/resource.hrc>
#include <services.h>
#include <properties.h>

#include <com/sun/star/frame/UntitledNumbersConst.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>

#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#include <vcl/opengl/OpenGLWrapper.hxx>

namespace framework{

TitleHelper::TitleHelper(const css::uno::Reference< css::uno::XComponentContext >& rxContext)
    : ::cppu::BaseMutex ()
    , m_xContext        (rxContext)
    , m_xOwner          ()
    , m_xUntitledNumbers()
    , m_xSubTitle       ()
    , m_bExternalTitle  (false)
    , m_sTitle          ()
    , m_nLeasedNumber   (css::frame::UntitledNumbersConst::INVALID_NUMBER)
    , m_aListener       (m_aMutex)
{
}

TitleHelper::~TitleHelper()
{
}

void TitleHelper::setOwner(const css::uno::Reference< css::uno::XInterface >& xOwner)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_xOwner = xOwner;

    aLock.clear ();
    // <- SYNCHRONIZED

    css::uno::Reference< css::frame::XModel > xModel(xOwner, css::uno::UNO_QUERY);
    if (xModel.is ())
    {
        impl_startListeningForModel (xModel);
        return;
    }

    css::uno::Reference< css::frame::XController > xController(xOwner, css::uno::UNO_QUERY);
    if (xController.is ())
    {
        impl_startListeningForController (xController);
        return;
    }

    css::uno::Reference< css::frame::XFrame > xFrame(xOwner, css::uno::UNO_QUERY);
    if (xFrame.is ())
    {
        impl_startListeningForFrame (xFrame);
        return;
    }
}

OUString SAL_CALL TitleHelper::getTitle()
    throw (css::uno::RuntimeException, std::exception)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        // An external title will win always and disable all internal logic about
        // creating/using a title value.
        // Even an empty string will be accepted as valid title !
        if (m_bExternalTitle)
            return m_sTitle;

        // Title seems to be up-to-date. Return it directly.
        if (!m_sTitle.isEmpty())
            return m_sTitle;

        // Title seems to be unused till now ... do bootstraping
        impl_updateTitle (true);

        return m_sTitle;

    // <- SYNCHRONIZED
}

void TitleHelper::connectWithUntitledNumbers (const css::uno::Reference< css::frame::XUntitledNumbers >& xNumbers)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_xUntitledNumbers = xNumbers;

    // <- SYNCHRONIZED
}

void SAL_CALL TitleHelper::setTitle(const OUString& sTitle)
    throw (css::uno::RuntimeException, std::exception)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_bExternalTitle = true;
        m_sTitle         = sTitle;

    aLock.clear ();
    // <- SYNCHRONIZED

    impl_sendTitleChangedEvent ();
}

void SAL_CALL TitleHelper::addTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    // container is threadsafe by himself
    m_aListener.addInterface( cppu::UnoType<css::frame::XTitleChangeListener>::get(), xListener );
}

void SAL_CALL TitleHelper::removeTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
    throw (css::uno::RuntimeException, std::exception)
{
    // container is threadsafe by himself
    m_aListener.removeInterface( cppu::UnoType<css::frame::XTitleChangeListener>::get(), xListener );
}

void SAL_CALL TitleHelper::titleChanged(const css::frame::TitleChangedEvent& aEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::uno::Reference< css::frame::XTitle > xSubTitle(m_xSubTitle.get (), css::uno::UNO_QUERY);

    aLock.clear ();
    // <- SYNCHRONIZED

    if (aEvent.Source != xSubTitle)
        return;

    impl_updateTitle ();
}

void SAL_CALL TitleHelper::documentEventOccured(const css::document::DocumentEvent& aEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    if ( ! aEvent.EventName.equalsIgnoreAsciiCase("OnSaveAsDone")
      && ! aEvent.EventName.equalsIgnoreAsciiCase("OnModeChanged")
      && ! aEvent.EventName.equalsIgnoreAsciiCase("OnTitleChanged"))
        return;

    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::uno::Reference< css::frame::XModel > xOwner(m_xOwner.get (), css::uno::UNO_QUERY);

    aLock.clear ();
    // <- SYNCHRONIZED

    if (aEvent.Source != xOwner
        || ((aEvent.EventName.equalsIgnoreAsciiCase("OnModeChanged")
             || aEvent.EventName.equalsIgnoreAsciiCase("OnTitleChanged"))
            && !xOwner.is()))
    {
        return;
    }

    impl_updateTitle ();
}

void SAL_CALL TitleHelper::frameAction(const css::frame::FrameActionEvent& aEvent)
    throw(css::uno::RuntimeException, std::exception)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::uno::Reference< css::frame::XFrame > xOwner(m_xOwner.get (), css::uno::UNO_QUERY);

    aLock.clear ();
    // <- SYNCHRONIZED

    if (aEvent.Source != xOwner)
        return;

    // we are interested on events only, which must trigger a title bar update
    // because component was changed.
    if (
        (aEvent.Action == css::frame::FrameAction_COMPONENT_ATTACHED  ) ||
        (aEvent.Action == css::frame::FrameAction_COMPONENT_REATTACHED) ||
        (aEvent.Action == css::frame::FrameAction_COMPONENT_DETACHING )
       )
    {
        impl_updateListeningForFrame (xOwner);
        impl_updateTitle ();
    }
}

void SAL_CALL TitleHelper::disposing(const css::lang::EventObject& aEvent)
    throw (css::uno::RuntimeException, std::exception)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
        css::uno::Reference< css::uno::XInterface >         xOwner        (m_xOwner.get()          , css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XUntitledNumbers > xNumbers      (m_xUntitledNumbers.get(), css::uno::UNO_QUERY);
        ::sal_Int32                                         nLeasedNumber = m_nLeasedNumber;
    aLock.clear ();
    // <- SYNCHRONIZED

    if ( ! xOwner.is ())
        return;

    if (xOwner != aEvent.Source)
        return;

    if (
        (xNumbers.is ()                                                   ) &&
        (nLeasedNumber != css::frame::UntitledNumbersConst::INVALID_NUMBER)
       )
       xNumbers->releaseNumber (nLeasedNumber);

    // SYNCHRONIZED ->
    aLock.reset ();

         m_xOwner        = nullptr;
         m_sTitle        = OUString ();
         m_nLeasedNumber = css::frame::UntitledNumbersConst::INVALID_NUMBER;

    aLock.clear ();
    // <- SYNCHRONIZED

}

void TitleHelper::impl_sendTitleChangedEvent ()
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::frame::TitleChangedEvent aEvent(m_xOwner.get (), m_sTitle);

    aLock.clear ();
    // <- SYNCHRONIZED

    ::cppu::OInterfaceContainerHelper* pContainer = m_aListener.getContainer( cppu::UnoType<css::frame::XTitleChangeListener>::get());
    if ( ! pContainer)
        return;

    ::cppu::OInterfaceIteratorHelper pIt( *pContainer );
    while ( pIt.hasMoreElements() )
    {
        try
        {
            static_cast<css::frame::XTitleChangeListener*>(pIt.next())->titleChanged( aEvent );
        }
        catch(const css::uno::Exception&)
        {
            pIt.remove();
        }
    }
}

void TitleHelper::impl_updateTitle (bool init)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::uno::Reference< css::frame::XModel >      xModel     (m_xOwner.get(), css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XController > xController(m_xOwner.get(), css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XFrame >      xFrame     (m_xOwner.get(), css::uno::UNO_QUERY);

    aLock.clear ();
    // <- SYNCHRONIZED

    if (xModel.is ())
    {
        impl_updateTitleForModel (xModel, init);
    }
    else if (xController.is ())
    {
        impl_updateTitleForController (xController, init);
    }
    else if (xFrame.is ())
    {
        impl_updateTitleForFrame (xFrame, init);
    }
}

void TitleHelper::impl_updateTitleForModel (const css::uno::Reference< css::frame::XModel >& xModel, bool init)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        // external title won't be updated internally!
        // It has to be set from outside new.
        if (m_bExternalTitle)
            return;

        css::uno::Reference< css::uno::XInterface >         xOwner        (m_xOwner.get()          , css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XUntitledNumbers > xNumbers      (m_xUntitledNumbers.get(), css::uno::UNO_QUERY);
        ::sal_Int32                                         nLeasedNumber = m_nLeasedNumber;

    aLock.clear ();
    // <- SYNCHRONIZED

    if (
        ( ! xOwner.is    ()) ||
        ( ! xNumbers.is  ()) ||
        ( ! xModel.is    ())
       )
        return;

    OUString sTitle;
    OUString sURL;

    css::uno::Reference< css::frame::XStorable > xURLProvider(xModel , css::uno::UNO_QUERY);
    if (xURLProvider.is())
        sURL = xURLProvider->getLocation ();

    if (!sURL.isEmpty())
    {
        sTitle = impl_convertURL2Title(sURL);
        if (nLeasedNumber != css::frame::UntitledNumbersConst::INVALID_NUMBER)
            xNumbers->releaseNumber (nLeasedNumber);
        nLeasedNumber = css::frame::UntitledNumbersConst::INVALID_NUMBER;
    }
    else
    {
        if (nLeasedNumber == css::frame::UntitledNumbersConst::INVALID_NUMBER)
            nLeasedNumber = xNumbers->leaseNumber (xOwner);

        OUStringBuffer sNewTitle(256);
        sNewTitle.append (xNumbers->getUntitledPrefix ());
        if (nLeasedNumber != css::frame::UntitledNumbersConst::INVALID_NUMBER)
            sNewTitle.append ((::sal_Int32)nLeasedNumber);
        else
            sNewTitle.append("?");

        sTitle = sNewTitle.makeStringAndClear ();
    }

    // SYNCHRONIZED ->
    aLock.reset ();

    // WORKAROUND: the notification is currently sent always,
    //             can be changed after shared mode is supported per UNO API
    bool     bChanged        = !init; // && m_sTitle != sTitle

             m_sTitle        = sTitle;
             m_nLeasedNumber = nLeasedNumber;

    aLock.clear ();
    // <- SYNCHRONIZED

    if (bChanged)
        impl_sendTitleChangedEvent ();
}

void TitleHelper::impl_updateTitleForController (const css::uno::Reference< css::frame::XController >& xController, bool init)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        // external title won't be updated internally!
        // It has to be set from outside new.
        if (m_bExternalTitle)
            return;

        css::uno::Reference< css::uno::XInterface >         xOwner        (m_xOwner.get()          , css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XUntitledNumbers > xNumbers      (m_xUntitledNumbers.get(), css::uno::UNO_QUERY);
        ::sal_Int32                                         nLeasedNumber = m_nLeasedNumber;

    aLock.clear ();
    // <- SYNCHRONIZED

    if (
        ( ! xOwner.is      ()) ||
        ( ! xNumbers.is    ()) ||
        ( ! xController.is ())
       )
        return;

    OUStringBuffer sTitle(256);

    if (nLeasedNumber == css::frame::UntitledNumbersConst::INVALID_NUMBER)
        nLeasedNumber = xNumbers->leaseNumber (xOwner);

    css::uno::Reference< css::frame::XTitle > xModelTitle(xController->getModel (), css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XModel > xModel(xController->getModel (), css::uno::UNO_QUERY);
    if (!xModelTitle.is ())
        xModelTitle.set(xController, css::uno::UNO_QUERY);
    if (xModelTitle.is ())
    {
        sTitle.append      (xModelTitle->getTitle ());
        if ( nLeasedNumber > 1 )
        {
            sTitle.append (" : ");
            sTitle.append      ((::sal_Int32)nLeasedNumber);
        }
        if (xModel.is ())
        {
            INetURLObject aURL (xModel->getURL ());
            if (aURL.GetProtocol () != INetProtocol::File
                && aURL.GetProtocol () != INetProtocol::NotValid)
            {
                OUString sRemoteText (FwkResId (STR_REMOTE_TITLE));
                sTitle.append (sRemoteText);
            }
        }
    }
    else
    {
        sTitle.append (xNumbers->getUntitledPrefix ());
        if ( nLeasedNumber > 1 )
        {
            sTitle.append ((::sal_Int32)nLeasedNumber  );
        }
    }

    // SYNCHRONIZED ->
    aLock.reset ();

        OUString sNewTitle       = sTitle.makeStringAndClear ();
        bool     bChanged        = !init && m_sTitle != sNewTitle;
                 m_sTitle        = sNewTitle;
                 m_nLeasedNumber = nLeasedNumber;

    aLock.clear ();
    // <- SYNCHRONIZED

    if (bChanged)
        impl_sendTitleChangedEvent ();
}

void TitleHelper::impl_updateTitleForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame, bool init)
{
    if ( ! xFrame.is ())
        return;

    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        // external title won't be updated internally!
        // It has to be set from outside new.
        if (m_bExternalTitle)
            return;

    aLock.clear ();
    // <- SYNCHRONIZED

    css::uno::Reference< css::uno::XInterface > xComponent;
    xComponent = xFrame->getController ();
    if ( ! xComponent.is ())
        xComponent = xFrame->getComponentWindow ();

    OUStringBuffer sTitle (256);

    impl_appendComponentTitle   (sTitle, xComponent);
#ifndef MACOSX
    // fdo#70376: We want the window title to contain just the
    // document name (from the above "component title").
    impl_appendProductName      (sTitle);
    impl_appendModuleName       (sTitle);
    impl_appendDebugVersion     (sTitle);
#endif
    // SYNCHRONIZED ->
    aLock.reset ();

        OUString sNewTitle = sTitle.makeStringAndClear ();
        bool     bChanged  = !init && m_sTitle != sNewTitle;
                 m_sTitle  = sNewTitle;

    aLock.clear ();
    // <- SYNCHRONIZED

    if (bChanged)
        impl_sendTitleChangedEvent ();
}

void TitleHelper::impl_appendComponentTitle (      OUStringBuffer&                       sTitle    ,
                                             const css::uno::Reference< css::uno::XInterface >& xComponent)
{
    css::uno::Reference< css::frame::XTitle > xTitle(xComponent, css::uno::UNO_QUERY);

    // Note: Title has to be used (even if it's empty) if the right interface is supported.
    if (xTitle.is ())
        sTitle.append (xTitle->getTitle ());
}

void TitleHelper::impl_appendProductName (OUStringBuffer& sTitle)
{
    OUString name(utl::ConfigManager::getProductName());
    if (!name.isEmpty())
    {
        if (!sTitle.isEmpty())
            sTitle.append(" - ");
        sTitle.append(name);
    }
}

void TitleHelper::impl_appendModuleName (OUStringBuffer& sTitle)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::uno::Reference< css::uno::XInterface >        xOwner   = m_xOwner.get();
        css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;

    aLock.clear ();
    // <- SYNCHRONIZED

    try
    {
        css::uno::Reference< css::frame::XModuleManager2 > xModuleManager =
            css::frame::ModuleManager::create(xContext);

        const OUString                 sID     = xModuleManager->identify(xOwner);
              ::comphelper::SequenceAsHashMap lProps  = xModuleManager->getByName (sID);
        const OUString                 sUIName = lProps.getUnpackedValueOrDefault (OFFICEFACTORY_PROPNAME_ASCII_UINAME, OUString());

        // An UIname property is an optional value !
        // So please add it to the title in case it does really exists only.
        if (!sUIName.isEmpty())
        {
            sTitle.append (" "    );
            sTitle.append      (sUIName);
        }
    }
    catch(const css::uno::Exception&)
    {}
}

#ifdef DBG_UTIL
void TitleHelper::impl_appendDebugVersion (OUStringBuffer& sTitle)
{
    OUString version(utl::ConfigManager::getProductVersion());
    sTitle.append(' ');
    sTitle.append(version);
    OUString sDefault("development");
    OUString sVersion = ::utl::Bootstrap::getBuildIdData(sDefault);
    sTitle.append(" [");
    sTitle.append(sVersion);
    if (OpenGLWrapper::isVCLOpenGLEnabled())
        sTitle.append("-GL");
    sTitle.append("]");
}
#else
void TitleHelper::impl_appendDebugVersion (OUStringBuffer&)
{
}
#endif

void TitleHelper::impl_startListeningForModel (const css::uno::Reference< css::frame::XModel >& xModel)
{
    css::uno::Reference< css::document::XDocumentEventBroadcaster > xBroadcaster(xModel, css::uno::UNO_QUERY);
    if ( ! xBroadcaster.is ())
        return;

    xBroadcaster->addDocumentEventListener (static_cast< css::document::XDocumentEventListener* >(this));
}

void TitleHelper::impl_startListeningForController (const css::uno::Reference< css::frame::XController >& xController)
{
    css::uno::Reference< css::frame::XTitle > xSubTitle(xController->getModel (), css::uno::UNO_QUERY);
    impl_setSubTitle (xSubTitle);
}

void TitleHelper::impl_startListeningForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    xFrame->addFrameActionListener(this  );
    impl_updateListeningForFrame  (xFrame);
}

void TitleHelper::impl_updateListeningForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    css::uno::Reference< css::frame::XTitle > xSubTitle(xFrame->getController (), css::uno::UNO_QUERY);
    impl_setSubTitle (xSubTitle);
}

void TitleHelper::impl_setSubTitle (const css::uno::Reference< css::frame::XTitle >& xSubTitle)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        // ignore duplicate calls. Makes outside using of this helper more easy :-)
        css::uno::Reference< css::frame::XTitle > xOldSubTitle(m_xSubTitle.get(), css::uno::UNO_QUERY);
        if (xOldSubTitle == xSubTitle)
            return;

        m_xSubTitle = xSubTitle;

    aLock.clear ();
    // <- SYNCHRONIZED

    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xOldBroadcaster(xOldSubTitle                                          , css::uno::UNO_QUERY      );
    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xNewBroadcaster(xSubTitle                                             , css::uno::UNO_QUERY      );
    css::uno::Reference< css::frame::XTitleChangeListener >    xThis          (static_cast< css::frame::XTitleChangeListener* >(this), css::uno::UNO_QUERY_THROW);

    if (xOldBroadcaster.is())
        xOldBroadcaster->removeTitleChangeListener (xThis);

    if (xNewBroadcaster.is())
        xNewBroadcaster->addTitleChangeListener (xThis);
}

OUString TitleHelper::impl_convertURL2Title(const OUString& sURL)
{
    INetURLObject   aURL (sURL);
    OUString sTitle;

    if (aURL.GetProtocol() == INetProtocol::File)
    {
        if (aURL.HasMark())
            aURL = INetURLObject(aURL.GetURLNoMark());

        sTitle = aURL.getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET);
    }
    else
    {
        if (aURL.hasExtension())
            sTitle = aURL.getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET);

        if ( sTitle.isEmpty() )
            sTitle = aURL.GetHostPort(INetURLObject::DECODE_WITH_CHARSET);

        if ( sTitle.isEmpty() )
            sTitle = aURL.GetURLNoPass(INetURLObject::DECODE_WITH_CHARSET);
    }

    return sTitle;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
