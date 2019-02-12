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
#include <strings.hrc>
#include <services.h>
#include <properties.h>

#include <com/sun/star/frame/UntitledNumbersConst.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>

#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/mediadescriptor.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#include <vcl/opengl/OpenGLWrapper.hxx>
#include <vcl/svapp.hxx>


using namespace css;
using namespace css::uno;
using namespace css::frame;

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
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_xOwner = xOwner;
    }
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
{
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_bExternalTitle = true;
        m_sTitle         = sTitle;
    }
    // <- SYNCHRONIZED

    impl_sendTitleChangedEvent ();
}

void SAL_CALL TitleHelper::addTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
{
    // container is threadsafe by himself
    m_aListener.addInterface( cppu::UnoType<css::frame::XTitleChangeListener>::get(), xListener );
}

void SAL_CALL TitleHelper::removeTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
{
    // container is threadsafe by himself
    m_aListener.removeInterface( cppu::UnoType<css::frame::XTitleChangeListener>::get(), xListener );
}

void SAL_CALL TitleHelper::titleChanged(const css::frame::TitleChangedEvent& aEvent)
{
    css::uno::Reference< css::frame::XTitle > xSubTitle;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        xSubTitle.set(m_xSubTitle.get (), css::uno::UNO_QUERY);
    }
    // <- SYNCHRONIZED

    if (aEvent.Source != xSubTitle)
        return;

    impl_updateTitle ();
}

void SAL_CALL TitleHelper::documentEventOccured(const css::document::DocumentEvent& aEvent)
{
    if ( ! aEvent.EventName.equalsIgnoreAsciiCase("OnSaveAsDone")
      && ! aEvent.EventName.equalsIgnoreAsciiCase("OnModeChanged")
      && ! aEvent.EventName.equalsIgnoreAsciiCase("OnTitleChanged"))
        return;

    css::uno::Reference< css::frame::XModel > xOwner;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        xOwner.set(m_xOwner.get (), css::uno::UNO_QUERY);
    }
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
{
    css::uno::Reference< css::frame::XFrame > xOwner;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        xOwner.set(m_xOwner.get (), css::uno::UNO_QUERY);
    }
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
{
    css::uno::Reference< css::uno::XInterface >         xOwner;
    css::uno::Reference< css::frame::XUntitledNumbers > xNumbers;
    ::sal_Int32                                         nLeasedNumber;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        xOwner.set(m_xOwner.get()          , css::uno::UNO_QUERY);
        xNumbers.set(m_xUntitledNumbers.get(), css::uno::UNO_QUERY);
        nLeasedNumber = m_nLeasedNumber;
    }
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
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_xOwner        = nullptr;
        m_sTitle        = OUString ();
        m_nLeasedNumber = css::frame::UntitledNumbersConst::INVALID_NUMBER;
    }
    // <- SYNCHRONIZED
}

void TitleHelper::impl_sendTitleChangedEvent ()
{
    css::uno::Reference<css::uno::XInterface> xOwner;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        xOwner = m_xOwner;
    }
    // <- SYNCHRONIZED

    css::frame::TitleChangedEvent aEvent(xOwner, m_sTitle);

    if( ! aEvent.Source.is() )
        return;

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
    css::uno::Reference< css::frame::XModel >      xModel;
    css::uno::Reference< css::frame::XController > xController;
    css::uno::Reference< css::frame::XFrame >      xFrame;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        xModel.set     (m_xOwner.get(), css::uno::UNO_QUERY);
        xController.set(m_xOwner.get(), css::uno::UNO_QUERY);
        xFrame.set     (m_xOwner.get(), css::uno::UNO_QUERY);
    }
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
    css::uno::Reference< css::uno::XInterface >         xOwner;
    css::uno::Reference< css::frame::XUntitledNumbers > xNumbers;
    ::sal_Int32                                         nLeasedNumber;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        // external title won't be updated internally!
        // It has to be set from outside new.
        if (m_bExternalTitle)
            return;

        xOwner.set     (m_xOwner.get()          , css::uno::UNO_QUERY);
        xNumbers.set   (m_xUntitledNumbers.get(), css::uno::UNO_QUERY);
        nLeasedNumber = m_nLeasedNumber;
    }
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

    utl::MediaDescriptor aDescriptor(xModel->getArgs());
    const OUString sSuggestedSaveAsName = aDescriptor.getUnpackedValueOrDefault(
        utl::MediaDescriptor::PROP_SUGGESTEDSAVEASNAME(), OUString());

    if (!sURL.isEmpty())
    {
        sTitle = impl_convertURL2Title(sURL);
        if (nLeasedNumber != css::frame::UntitledNumbersConst::INVALID_NUMBER)
            xNumbers->releaseNumber (nLeasedNumber);
        nLeasedNumber = css::frame::UntitledNumbersConst::INVALID_NUMBER;
    }
    else if (!sSuggestedSaveAsName.isEmpty())
    {
        // tdf#121537 Use suggested save as name for title if file has not yet been saved
        sTitle = sSuggestedSaveAsName;
    }
    else
    {
        if (nLeasedNumber == css::frame::UntitledNumbersConst::INVALID_NUMBER)
            nLeasedNumber = xNumbers->leaseNumber (xOwner);

        OUStringBuffer sNewTitle(256);
        sNewTitle.append (xNumbers->getUntitledPrefix ());
        if (nLeasedNumber != css::frame::UntitledNumbersConst::INVALID_NUMBER)
            sNewTitle.append(nLeasedNumber);
        else
            sNewTitle.append("?");

        sTitle = sNewTitle.makeStringAndClear ();
    }

    bool     bChanged;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        // WORKAROUND: the notification is currently sent always,
        //             can be changed after shared mode is supported per UNO API
        bChanged        = !init; // && m_sTitle != sTitle

        m_sTitle        = sTitle;
        m_nLeasedNumber = nLeasedNumber;
    }
    // <- SYNCHRONIZED

    if (bChanged)
        impl_sendTitleChangedEvent ();
}

void TitleHelper::impl_updateTitleForController (const css::uno::Reference< css::frame::XController >& xController, bool init)
{
    css::uno::Reference< css::uno::XInterface >         xOwner;
    css::uno::Reference< css::frame::XUntitledNumbers > xNumbers;
    ::sal_Int32                                         nLeasedNumber;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        // external title won't be updated internally!
        // It has to be set from outside new.
        if (m_bExternalTitle)
            return;

        xOwner.set      (m_xOwner.get()          , css::uno::UNO_QUERY);
        xNumbers.set    (m_xUntitledNumbers.get(), css::uno::UNO_QUERY);
        nLeasedNumber = m_nLeasedNumber;
    }
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
            sTitle.append(" : ");
            sTitle.append(nLeasedNumber);
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
            sTitle.append(nLeasedNumber  );
        }
    }

    bool     bChanged;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        OUString sNewTitle       = sTitle.makeStringAndClear ();
        bChanged        = !init && m_sTitle != sNewTitle;
        m_sTitle        = sNewTitle;
        m_nLeasedNumber = nLeasedNumber;
    }
    // <- SYNCHRONIZED

    if (bChanged)
        impl_sendTitleChangedEvent ();
}

void TitleHelper::impl_updateTitleForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame, bool init)
{
    if ( ! xFrame.is ())
        return;

    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        // external title won't be updated internally!
        // It has to be set from outside new.
        if (m_bExternalTitle)
            return;
    }
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
    impl_appendSafeMode         (sTitle);

    bool     bChanged;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        OUString sNewTitle = sTitle.makeStringAndClear ();
        bChanged  = !init && m_sTitle != sNewTitle;
        m_sTitle  = sNewTitle;
    }
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
    css::uno::Reference< css::uno::XInterface >        xOwner;
    css::uno::Reference< css::uno::XComponentContext > xContext;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        xOwner   = m_xOwner.get();
        xContext = m_xContext;
    }
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
    OUString sVersion = ::utl::Bootstrap::getBuildIdData("development");
    sTitle.append(" [");
    sTitle.append(sVersion);
#ifndef LIBO_HEADLESS
    if (OpenGLWrapper::isVCLOpenGLEnabled())
        sTitle.append("-GL");
#endif
    sTitle.append("]");
}
#else
void TitleHelper::impl_appendDebugVersion (OUStringBuffer&)
{
}
#endif

void TitleHelper::impl_appendSafeMode (OUStringBuffer& sTitle)
{
    if (Application::IsSafeModeEnabled())
        sTitle.append(FwkResId (STR_SAFEMODE_TITLE));
}

void TitleHelper::impl_startListeningForModel (const css::uno::Reference< css::frame::XModel >& xModel)
{
    css::uno::Reference< css::document::XDocumentEventBroadcaster > xBroadcaster(xModel, css::uno::UNO_QUERY);
    if ( ! xBroadcaster.is ())
        return;

    xBroadcaster->addDocumentEventListener (static_cast< css::document::XDocumentEventListener* >(this));
}

void TitleHelper::impl_startListeningForController (const css::uno::Reference< css::frame::XController >& xController)
{
    xController->addEventListener (static_cast< css::lang::XEventListener* > (static_cast< css::frame::XFrameActionListener* > (this) ) );
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
    css::uno::Reference< css::frame::XTitle > xOldSubTitle;
    // SYNCHRONIZED ->
    {
        ::osl::ResettableMutexGuard aLock(m_aMutex);

        // ignore duplicate calls. Makes outside using of this helper more easy :-)
        xOldSubTitle.set(m_xSubTitle.get(), css::uno::UNO_QUERY);
        if (xOldSubTitle == xSubTitle)
            return;

        m_xSubTitle = xSubTitle;
    }
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

        sTitle = aURL.getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset);
    }
    else
    {
        if (aURL.hasExtension())
            sTitle = aURL.getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset);

        if ( sTitle.isEmpty() )
            sTitle = aURL.GetHostPort(INetURLObject::DecodeMechanism::WithCharset);

        if ( sTitle.isEmpty() )
            sTitle = aURL.GetURLNoPass(INetURLObject::DecodeMechanism::WithCharset);
    }

    return sTitle;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
