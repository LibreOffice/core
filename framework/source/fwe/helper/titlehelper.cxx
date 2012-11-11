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

#include <framework/titlehelper.hxx>
#include <services.h>
#include <properties.h>

#include <com/sun/star/frame/UntitledNumbersConst.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>

#include <unotools/configmgr.hxx>
#include <unotools/bootstrap.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>


namespace framework{

namespace css = ::com::sun::star;

//-----------------------------------------------
TitleHelper::TitleHelper(const css::uno::Reference< css::uno::XComponentContext >& rxContext)
    : ::cppu::BaseMutex ()
    , m_xContext        (rxContext)
    , m_xOwner          ()
    , m_xUntitledNumbers()
    , m_xSubTitle       ()
    , m_bExternalTitle  (sal_False)
    , m_sTitle          ()
    , m_nLeasedNumber   (css::frame::UntitledNumbersConst::INVALID_NUMBER)
    , m_aListener       (m_aMutex)
{
}

//-----------------------------------------------
TitleHelper::~TitleHelper()
{
}

//-----------------------------------------------
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

//-----------------------------------------------
::rtl::OUString SAL_CALL TitleHelper::getTitle()
    throw (css::uno::RuntimeException)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        // An external title will win always and disable all internal logic about
        // creating/using a title value.
        // Even an empty string will be accepted as valid title !
        if (m_bExternalTitle)
            return m_sTitle;

        // Title seams to be up-to-date. Return it directly.
        if (!m_sTitle.isEmpty())
            return m_sTitle;

        // Title seams to be unused till now ... do bootstraping
        impl_updateTitle (true);

        return m_sTitle;

    // <- SYNCHRONIZED
}

//-----------------------------------------------
void TitleHelper::connectWithUntitledNumbers (const css::uno::Reference< css::frame::XUntitledNumbers >& xNumbers)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_xUntitledNumbers = xNumbers;

    // <- SYNCHRONIZED
}

//-----------------------------------------------
void SAL_CALL TitleHelper::setTitle(const ::rtl::OUString& sTitle)
    throw (css::uno::RuntimeException)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_bExternalTitle = sal_True;
        m_sTitle         = sTitle;

    aLock.clear ();
    // <- SYNCHRONIZED

    impl_sendTitleChangedEvent ();
}

//-----------------------------------------------
void SAL_CALL TitleHelper::addTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
    throw (css::uno::RuntimeException)
{
    // container is threadsafe by himself
    m_aListener.addInterface( ::getCppuType( (const css::uno::Reference< css::frame::XTitleChangeListener >*)NULL ), xListener );
}

//-----------------------------------------------
void SAL_CALL TitleHelper::removeTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
    throw (css::uno::RuntimeException)
{
    // container is threadsafe by himself
    m_aListener.removeInterface( ::getCppuType( (const css::uno::Reference< css::frame::XTitleChangeListener >*)NULL ), xListener );
}

//-----------------------------------------------
void SAL_CALL TitleHelper::titleChanged(const css::frame::TitleChangedEvent& aEvent)
    throw (css::uno::RuntimeException)
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

//-----------------------------------------------
void SAL_CALL TitleHelper::notifyEvent(const css::document::EventObject& aEvent)
    throw (css::uno::RuntimeException)
{
    if ( ! aEvent.EventName.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("OnSaveAsDone"))
      && ! aEvent.EventName.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("OnModeChanged"))
      && ! aEvent.EventName.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("OnTitleChanged")))
        return;

    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::uno::Reference< css::frame::XModel > xOwner(m_xOwner.get (), css::uno::UNO_QUERY);

    aLock.clear ();
    // <- SYNCHRONIZED

    if (aEvent.Source != xOwner
        || ((aEvent.EventName.equalsIgnoreAsciiCaseAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("OnModeChanged"))
             || aEvent.EventName.equalsIgnoreAsciiCaseAsciiL(
                 RTL_CONSTASCII_STRINGPARAM("OnTitleChanged")))
            && !xOwner.is()))
    {
        return;
    }

    impl_updateTitle ();
}

//-----------------------------------------------
void SAL_CALL TitleHelper::frameAction(const css::frame::FrameActionEvent& aEvent)
    throw(css::uno::RuntimeException)
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

//-----------------------------------------------
void SAL_CALL TitleHelper::disposing(const css::lang::EventObject& aEvent)
    throw (css::uno::RuntimeException)
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

         m_sTitle        = ::rtl::OUString ();
         m_nLeasedNumber = css::frame::UntitledNumbersConst::INVALID_NUMBER;

    aLock.clear ();
    // <- SYNCHRONIZED

    impl_sendTitleChangedEvent ();
}

//-----------------------------------------------
void TitleHelper::impl_sendTitleChangedEvent ()
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::frame::TitleChangedEvent aEvent(m_xOwner.get (), m_sTitle);

    aLock.clear ();
    // <- SYNCHRONIZED

    ::cppu::OInterfaceContainerHelper* pContainer = m_aListener.getContainer( ::getCppuType( ( const css::uno::Reference< css::frame::XTitleChangeListener >*) NULL ) );
    if ( ! pContainer)
        return;

    ::cppu::OInterfaceIteratorHelper pIt( *pContainer );
    while ( pIt.hasMoreElements() )
    {
        try
        {
            ((css::frame::XTitleChangeListener*)pIt.next())->titleChanged( aEvent );
        }
        catch(const css::uno::Exception&)
        {
            pIt.remove();
        }
    }
}

//-----------------------------------------------
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

//-----------------------------------------------
void TitleHelper::impl_updateTitleForModel (const css::uno::Reference< css::frame::XModel >& xModel, bool init)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        // external title wont be updated internaly !
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

    ::rtl::OUString sTitle;
    ::rtl::OUString sURL  ;

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

        ::rtl::OUStringBuffer sNewTitle(256);
        sNewTitle.append (xNumbers->getUntitledPrefix ());
        if (nLeasedNumber != css::frame::UntitledNumbersConst::INVALID_NUMBER)
            sNewTitle.append ((::sal_Int32)nLeasedNumber);
        else
            sNewTitle.appendAscii ("?");

        sTitle = sNewTitle.makeStringAndClear ();
    }

    // SYNCHRONIZED ->
    aLock.reset ();

    // WORKAROUND: the notification is currently sent always,
    //             can be changed after shared mode is supported per UNO API
    sal_Bool bChanged        = !init; // && m_sTitle != sTitle

             m_sTitle        = sTitle;
             m_nLeasedNumber = nLeasedNumber;

    aLock.clear ();
    // <- SYNCHRONIZED

    if (bChanged)
        impl_sendTitleChangedEvent ();
}

//-----------------------------------------------
void TitleHelper::impl_updateTitleForController (const css::uno::Reference< css::frame::XController >& xController, bool init)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        // external title wont be updated internaly !
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

    ::rtl::OUStringBuffer sTitle(256);

    if (nLeasedNumber == css::frame::UntitledNumbersConst::INVALID_NUMBER)
        nLeasedNumber = xNumbers->leaseNumber (xOwner);

    css::uno::Reference< css::frame::XTitle > xModelTitle(xController->getModel (), css::uno::UNO_QUERY);
    if (!xModelTitle.is ())
        xModelTitle.set(xController, css::uno::UNO_QUERY);
    if (xModelTitle.is ())
    {
        sTitle.append      (xModelTitle->getTitle ());
        if ( nLeasedNumber > 1 )
        {
            sTitle.appendAscii (" : ");
            sTitle.append      ((::sal_Int32)nLeasedNumber);
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

        ::rtl::OUString sNewTitle       = sTitle.makeStringAndClear ();
        sal_Bool        bChanged        = !init && m_sTitle != sNewTitle;
                        m_sTitle        = sNewTitle;
                        m_nLeasedNumber = nLeasedNumber;

    aLock.clear ();
    // <- SYNCHRONIZED

    if (bChanged)
        impl_sendTitleChangedEvent ();
}

//-----------------------------------------------
void TitleHelper::impl_updateTitleForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame, bool init)
{
    if ( ! xFrame.is ())
        return;

    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        // external title wont be updated internaly !
        // It has to be set from outside new.
        if (m_bExternalTitle)
            return;

    aLock.clear ();
    // <- SYNCHRONIZED

    css::uno::Reference< css::uno::XInterface > xComponent;
    xComponent = xFrame->getController ();
    if ( ! xComponent.is ())
        xComponent = xFrame->getComponentWindow ();

    ::rtl::OUStringBuffer sTitle (256);

    impl_appendComponentTitle   (sTitle, xComponent);
    impl_appendProductName      (sTitle);
    impl_appendModuleName       (sTitle);
    impl_appendProductExtension (sTitle);
    impl_appendDebugVersion     (sTitle);

    // SYNCHRONIZED ->
    aLock.reset ();

        ::rtl::OUString sNewTitle = sTitle.makeStringAndClear ();
        sal_Bool        bChanged  = !init && m_sTitle != sNewTitle;
                        m_sTitle  = sNewTitle;

    aLock.clear ();
    // <- SYNCHRONIZED

    if (bChanged)
        impl_sendTitleChangedEvent ();
}

//*****************************************************************************************************************
void TitleHelper::impl_appendComponentTitle (      ::rtl::OUStringBuffer&                       sTitle    ,
                                             const css::uno::Reference< css::uno::XInterface >& xComponent)
{
    css::uno::Reference< css::frame::XTitle > xTitle(xComponent, css::uno::UNO_QUERY);

    // Note: Title has to be used (even if it's empty) if the right interface is supported.
    if (xTitle.is ())
        sTitle.append (xTitle->getTitle ());
}

//*****************************************************************************************************************
void TitleHelper::impl_appendProductName (::rtl::OUStringBuffer& sTitle)
{
    rtl::OUString name(utl::ConfigManager::getProductName());
    if (!name.isEmpty())
    {
        if (sTitle.getLength() != 0)
            sTitle.appendAscii(RTL_CONSTASCII_STRINGPARAM(" - "));
        sTitle.append(name);
    }
}

//*****************************************************************************************************************
void TitleHelper::impl_appendProductExtension (::rtl::OUStringBuffer& sTitle)
{
    rtl::OUString ext(utl::ConfigManager::getProductExtension());
    if (!ext.isEmpty())
    {
        sTitle.append(' ');
        sTitle.append(ext);
    }
}

//*****************************************************************************************************************
void TitleHelper::impl_appendModuleName (::rtl::OUStringBuffer& sTitle)
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

        const ::rtl::OUString                 sID     = xModuleManager->identify(xOwner);
              ::comphelper::SequenceAsHashMap lProps  = xModuleManager->getByName (sID);
        const ::rtl::OUString                 sUIName = lProps.getUnpackedValueOrDefault (OFFICEFACTORY_PROPNAME_UINAME, ::rtl::OUString());

        // An UIname property is an optional value !
        // So please add it to the title in case it does realy exists only.
        if (!sUIName.isEmpty())
        {
            sTitle.appendAscii (" "    );
            sTitle.append      (sUIName);
        }
    }
    catch(const css::uno::Exception&)
    {}
}

//*****************************************************************************************************************
#ifdef DBG_UTIL
void TitleHelper::impl_appendDebugVersion (::rtl::OUStringBuffer& sTitle)
{
    ::rtl::OUString sDefault(RTL_CONSTASCII_USTRINGPARAM("development"));
    ::rtl::OUString sVersion = ::utl::Bootstrap::getProductSource(sDefault);
    sTitle.appendAscii(RTL_CONSTASCII_STRINGPARAM(" ["));
    sTitle.append(sVersion);
    sTitle.appendAscii(RTL_CONSTASCII_STRINGPARAM("]"));
}
#else
void TitleHelper::impl_appendDebugVersion (::rtl::OUStringBuffer&)
{
}
#endif

//-----------------------------------------------
void TitleHelper::impl_startListeningForModel (const css::uno::Reference< css::frame::XModel >& xModel)
{
    css::uno::Reference< css::document::XEventBroadcaster > xBroadcaster(xModel, css::uno::UNO_QUERY);
    if ( ! xBroadcaster.is ())
        return;

    xBroadcaster->addEventListener (static_cast< css::document::XEventListener* >(this));
}

//-----------------------------------------------
void TitleHelper::impl_startListeningForController (const css::uno::Reference< css::frame::XController >& xController)
{
    css::uno::Reference< css::frame::XTitle > xSubTitle(xController->getModel (), css::uno::UNO_QUERY);
    impl_setSubTitle (xSubTitle);
}

//-----------------------------------------------
void TitleHelper::impl_startListeningForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    xFrame->addFrameActionListener(this  );
    impl_updateListeningForFrame  (xFrame);
}

//-----------------------------------------------
void TitleHelper::impl_updateListeningForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame)
{
    css::uno::Reference< css::frame::XTitle > xSubTitle(xFrame->getController (), css::uno::UNO_QUERY);
    impl_setSubTitle (xSubTitle);
}

//-----------------------------------------------
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

//-----------------------------------------------
::rtl::OUString TitleHelper::impl_convertURL2Title(const ::rtl::OUString& sURL)
{
    INetURLObject   aURL (sURL);
    ::rtl::OUString sTitle;

    if (aURL.GetProtocol() == INET_PROT_FILE)
    {
        if (aURL.HasMark())
            aURL = INetURLObject(aURL.GetURLNoMark());

        sTitle = aURL.getName(INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::DECODE_WITH_CHARSET);
    }
    else
    {
        if (aURL.hasExtension(INetURLObject::LAST_SEGMENT))
            sTitle = aURL.getName(INetURLObject::LAST_SEGMENT, sal_True, INetURLObject::DECODE_WITH_CHARSET);

        if ( sTitle.isEmpty() )
            sTitle = aURL.GetHostPort(INetURLObject::DECODE_WITH_CHARSET);

        if ( sTitle.isEmpty() )
            sTitle = aURL.GetURLNoPass(INetURLObject::DECODE_WITH_CHARSET);
    }

    return sTitle;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
