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
#include <properties.h>

#include <com/sun/star/frame/UntitledNumbersConst.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/frame/XModel3.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>

#include <comphelper/configuration.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/mediadescriptor.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#include <utility>
#include <vcl/svapp.hxx>


using namespace css;
using namespace css::uno;
using namespace css::frame;

namespace framework{

TitleHelper::TitleHelper(css::uno::Reference< css::uno::XComponentContext > xContext,
                        const css::uno::Reference< css::uno::XInterface >& xOwner,
                        const css::uno::Reference< css::frame::XUntitledNumbers >& xNumbers)
    :
      m_xContext        (std::move(xContext))
    , m_xOwner          (xOwner)
    , m_xUntitledNumbers(xNumbers)
    , m_bExternalTitle  (false)
    , m_nLeasedNumber   (css::frame::UntitledNumbersConst::INVALID_NUMBER)
{
    if (css::uno::Reference<css::frame::XModel> xModel{ xOwner, css::uno::UNO_QUERY })
    {
        impl_startListeningForModel (xModel);
    }
    else if (css::uno::Reference<css::frame::XController> xController{ xOwner,
                                                                       css::uno::UNO_QUERY })
    {
        impl_startListeningForController (xController);
    }
    else if (css::uno::Reference<css::frame::XFrame> xFrame{ xOwner, css::uno::UNO_QUERY })
    {
        impl_startListeningForFrame (xFrame);
    }
}

TitleHelper::~TitleHelper()
{
}

OUString SAL_CALL TitleHelper::getTitle()
{
    // SYNCHRONIZED ->
    std::unique_lock aLock(m_aMutex);

    // An external title will win always and disable all internal logic about
    // creating/using a title value.
    // Even an empty string will be accepted as valid title !
    if (m_bExternalTitle)
        return m_sTitle;

    // Title seems to be up-to-date. Return it directly.
    if (!m_sTitle.isEmpty())
        return m_sTitle;

    // Title seems to be unused till now ... do bootstrapping
    aLock.unlock();
    impl_updateTitle (true);
    aLock.lock();

    return m_sTitle;
    // <- SYNCHRONIZED
}

void SAL_CALL TitleHelper::setTitle(const OUString& sTitle)
{
    // SYNCHRONIZED ->
    {
        std::unique_lock aLock(m_aMutex);

        m_bExternalTitle = true;
        m_sTitle         = sTitle;
    }
    // <- SYNCHRONIZED

    impl_sendTitleChangedEvent ();
}

void SAL_CALL TitleHelper::addTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
{
    std::unique_lock aLock(m_aMutex);
    m_aTitleChangeListeners.addInterface( aLock, xListener );
}

void SAL_CALL TitleHelper::removeTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
{
    std::unique_lock aLock(m_aMutex);
    m_aTitleChangeListeners.removeInterface( aLock, xListener );
}

void SAL_CALL TitleHelper::titleChanged(const css::frame::TitleChangedEvent& aEvent)
{
    css::uno::Reference< css::frame::XTitle > xSubTitle;
    // SYNCHRONIZED ->
    {
        std::unique_lock aLock(m_aMutex);

        xSubTitle = m_xSubTitle;
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
        std::unique_lock aLock(m_aMutex);

        xOwner.set(m_xOwner, css::uno::UNO_QUERY);
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
        std::unique_lock aLock(m_aMutex);

        xOwner.set(m_xOwner, css::uno::UNO_QUERY);
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
        std::unique_lock aLock(m_aMutex);

        xOwner = m_xOwner;
        xNumbers = m_xUntitledNumbers;
        nLeasedNumber = m_nLeasedNumber;
    }
    // <- SYNCHRONIZED

    if ( ! xOwner.is ())
        return;

    css::uno::Reference< css::frame::XFrame > xFrame(xOwner, css::uno::UNO_QUERY);
    if (xFrame.is())
        xFrame->removeFrameActionListener(this);

    if (xOwner != aEvent.Source)
        return;

    if (
        (xNumbers.is ()                                                   ) &&
        (nLeasedNumber != css::frame::UntitledNumbersConst::INVALID_NUMBER)
       )
       xNumbers->releaseNumber (nLeasedNumber);

    // SYNCHRONIZED ->
    {
        std::unique_lock aLock(m_aMutex);

        m_xOwner.clear();
        m_sTitle.clear();
        m_nLeasedNumber = css::frame::UntitledNumbersConst::INVALID_NUMBER;
    }
    // <- SYNCHRONIZED
}

void TitleHelper::impl_sendTitleChangedEvent ()
{
    css::uno::Reference<css::uno::XInterface> xOwner;
    // SYNCHRONIZED ->
    {
        std::unique_lock aLock(m_aMutex);

        xOwner = m_xOwner;
    }
    // <- SYNCHRONIZED

    css::frame::TitleChangedEvent aEvent(xOwner, m_sTitle);

    if( ! aEvent.Source.is() )
        return;

    std::unique_lock aLock(m_aMutex);
    comphelper::OInterfaceIteratorHelper4 pIt( aLock, m_aTitleChangeListeners );
    while ( pIt.hasMoreElements() )
    {
        aLock.unlock();
        try
        {
            uno::Reference<css::frame::XTitleChangeListener> i = pIt.next();
            i->titleChanged( aEvent );
        }
        catch(const css::uno::Exception&)
        {
            aLock.lock();
            pIt.remove(aLock);
            aLock.unlock();
        }
        aLock.lock();
    }
}

void TitleHelper::impl_updateTitle (bool init)
{
    css::uno::Reference<css::uno::XInterface> xOwner;

    // SYNCHRONIZED ->
    {
        std::unique_lock aLock(m_aMutex);

        xOwner = m_xOwner;
    }
    // <- SYNCHRONIZED

    if (css::uno::Reference<css::frame::XModel3> xModel{ xOwner, css::uno::UNO_QUERY })
    {
        impl_updateTitleForModel (xModel, init);
    }
    else if (css::uno::Reference<css::frame::XController> xController{ xOwner,
                                                                       css::uno::UNO_QUERY })
    {
        impl_updateTitleForController (xController, init);
    }
    else if (css::uno::Reference<css::frame::XFrame> xFrame{ xOwner, css::uno::UNO_QUERY })
    {
        impl_updateTitleForFrame (xFrame, init);
    }
}

static OUString getURLFromModel(const css::uno::Reference< css::frame::XModel3 >& xModel)
{
    if (css::uno::Reference<css::frame::XStorable> xURLProvider{ xModel, css::uno::UNO_QUERY })
        return xURLProvider->getLocation();
    return {};
}

void TitleHelper::impl_updateTitleForModel (const css::uno::Reference< css::frame::XModel3 >& xModel, bool init)
{
    css::uno::Reference< css::uno::XInterface >         xOwner;
    css::uno::Reference< css::frame::XUntitledNumbers > xNumbers;
    ::sal_Int32                                         nLeasedNumber;
    // SYNCHRONIZED ->
    {
        std::unique_lock aLock(m_aMutex);

        // external title won't be updated internally!
        // It has to be set from outside new.
        if (m_bExternalTitle)
            return;

        xOwner = m_xOwner;
        xNumbers = m_xUntitledNumbers;
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

    utl::MediaDescriptor aDescriptor(
        xModel->getArgs2({ utl::MediaDescriptor::PROP_DOCUMENTTITLE,
                           utl::MediaDescriptor::PROP_SUGGESTEDSAVEASNAME }));

    if (const OUString sMediaTitle = aDescriptor.getUnpackedValueOrDefault(
            utl::MediaDescriptor::PROP_DOCUMENTTITLE, OUString());
        !sMediaTitle.isEmpty())
    {
        sTitle = sMediaTitle;
    }
    else if (const OUString sURL = getURLFromModel(xModel); !sURL.isEmpty())
    {
        sTitle = impl_convertURL2Title(sURL);
        if (nLeasedNumber != css::frame::UntitledNumbersConst::INVALID_NUMBER)
            xNumbers->releaseNumber (nLeasedNumber);
        nLeasedNumber = css::frame::UntitledNumbersConst::INVALID_NUMBER;
    }
    else if (const OUString sSuggestedSaveAsName = aDescriptor.getUnpackedValueOrDefault(
                 utl::MediaDescriptor::PROP_SUGGESTEDSAVEASNAME, OUString());
             !sSuggestedSaveAsName.isEmpty())
    {
        // tdf#121537 Use suggested save as name for title if file has not yet been saved
        sTitle = sSuggestedSaveAsName;
    }
    else
    {
        if (nLeasedNumber == css::frame::UntitledNumbersConst::INVALID_NUMBER)
            nLeasedNumber = xNumbers->leaseNumber (xOwner);

        if (nLeasedNumber != css::frame::UntitledNumbersConst::INVALID_NUMBER)
            sTitle = xNumbers->getUntitledPrefix() + OUString::number(nLeasedNumber);
        else
            sTitle = xNumbers->getUntitledPrefix() + "?";
    }

    bool     bChanged;
    // SYNCHRONIZED ->
    {
        std::unique_lock aLock(m_aMutex);

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
        std::unique_lock aLock(m_aMutex);

        // external title won't be updated internally!
        // It has to be set from outside new.
        if (m_bExternalTitle)
            return;

        xOwner = m_xOwner;
        xNumbers = m_xUntitledNumbers;
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
    css::uno::Reference< css::frame::XModel > xModel = xController->getModel ();
    if (!xModelTitle.is ())
        xModelTitle.set(xController, css::uno::UNO_QUERY);
    if (xModelTitle.is ())
    {
        sTitle.append      (xModelTitle->getTitle ());
        if ( nLeasedNumber > 1 )
        {
            sTitle.append(" : " + OUString::number(nLeasedNumber));
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
        std::unique_lock aLock(m_aMutex);

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
        std::unique_lock aLock(m_aMutex);

        // external title won't be updated internally!
        // It has to be set from outside new.
        if (m_bExternalTitle)
            return;
    }
    // <- SYNCHRONIZED

    css::uno::Reference< css::uno::XInterface > xComponent = xFrame->getController ();
    if ( ! xComponent.is ())
        xComponent = xFrame->getComponentWindow ();

    OUStringBuffer sTitle (256);

    impl_appendComponentTitle   (sTitle, xComponent);
#ifndef MACOSX
    if (!comphelper::IsFuzzing())
    {
        // fdo#70376: We want the window title to contain just the
        // document name (from the above "component title").
        impl_appendProductName      (sTitle);
        impl_appendModuleName       (sTitle);
        impl_appendDebugVersion     (sTitle);
    }
#endif
    impl_appendSafeMode         (sTitle);

    bool     bChanged;
    // SYNCHRONIZED ->
    {
        std::unique_lock aLock(m_aMutex);

        OUString sNewTitle = sTitle.makeStringAndClear ();
        bChanged  = !init && m_sTitle != sNewTitle;
        m_sTitle  = sNewTitle;
    }
    // <- SYNCHRONIZED

    if (bChanged)
        impl_sendTitleChangedEvent ();
}

// static
void TitleHelper::impl_appendComponentTitle (      OUStringBuffer&                       sTitle    ,
                                             const css::uno::Reference< css::uno::XInterface >& xComponent)
{
    css::uno::Reference< css::frame::XTitle > xTitle(xComponent, css::uno::UNO_QUERY);

    // Note: Title has to be used (even if it's empty) if the right interface is supported.
    if (xTitle.is ())
        sTitle.append (xTitle->getTitle ());
}

// static
void TitleHelper::impl_appendProductName (OUStringBuffer& sTitle)
{
    OUString name(utl::ConfigManager::getProductName());
    if (!name.isEmpty())
    {
        if (!sTitle.isEmpty())
        {
            OUString separator (FwkResId (STR_EMDASH_SEPARATOR));
            sTitle.append(separator);
        }
        sTitle.append(name);
    }
}

void TitleHelper::impl_appendModuleName (OUStringBuffer& sTitle)
{
    css::uno::Reference< css::uno::XInterface >        xOwner;
    css::uno::Reference< css::uno::XComponentContext > xContext;
    // SYNCHRONIZED ->
    {
        std::unique_lock aLock(m_aMutex);

        xOwner   = m_xOwner;
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
// static
void TitleHelper::impl_appendDebugVersion (OUStringBuffer& sTitle)
{
    OUString version(utl::ConfigManager::getProductVersion());
    sTitle.append(' ');
    sTitle.append(version);
    OUString sVersion = ::utl::Bootstrap::getBuildIdData("development");
    sTitle.append(" [");
    sTitle.append(sVersion);
    sTitle.append("]");
}
#else
void TitleHelper::impl_appendDebugVersion (OUStringBuffer&)
{
}
#endif

// static
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
        std::unique_lock aLock(m_aMutex);

        // ignore duplicate calls. Makes outside using of this helper more easy :-)
        xOldSubTitle = m_xSubTitle;
        if (xOldSubTitle == xSubTitle)
            return;

        m_xSubTitle = xSubTitle;
    }
    // <- SYNCHRONIZED

    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xOldBroadcaster(xOldSubTitle                                          , css::uno::UNO_QUERY      );
    css::uno::Reference< css::frame::XTitleChangeBroadcaster > xNewBroadcaster(xSubTitle                                             , css::uno::UNO_QUERY      );
    css::uno::Reference< css::frame::XTitleChangeListener >    xThis(this);

    if (xOldBroadcaster.is())
        xOldBroadcaster->removeTitleChangeListener (xThis);

    if (xNewBroadcaster.is())
        xNewBroadcaster->addTitleChangeListener (xThis);
}

// static
OUString TitleHelper::impl_convertURL2Title(std::u16string_view sURL)
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
