/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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


TitleHelper::~TitleHelper()
{
}


void TitleHelper::setOwner(const css::uno::Reference< css::uno::XInterface >& xOwner)
{
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_xOwner = xOwner;

    aLock.clear ();
    

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
    throw (css::uno::RuntimeException)
{
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        
        
        
        if (m_bExternalTitle)
            return m_sTitle;

        
        if (!m_sTitle.isEmpty())
            return m_sTitle;

        
        impl_updateTitle (true);

        return m_sTitle;

    
}


void TitleHelper::connectWithUntitledNumbers (const css::uno::Reference< css::frame::XUntitledNumbers >& xNumbers)
{
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_xUntitledNumbers = xNumbers;

    
}


void SAL_CALL TitleHelper::setTitle(const OUString& sTitle)
    throw (css::uno::RuntimeException)
{
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        m_bExternalTitle = sal_True;
        m_sTitle         = sTitle;

    aLock.clear ();
    

    impl_sendTitleChangedEvent ();
}


void SAL_CALL TitleHelper::addTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
    throw (css::uno::RuntimeException)
{
    
    m_aListener.addInterface( ::getCppuType( (const css::uno::Reference< css::frame::XTitleChangeListener >*)NULL ), xListener );
}


void SAL_CALL TitleHelper::removeTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
    throw (css::uno::RuntimeException)
{
    
    m_aListener.removeInterface( ::getCppuType( (const css::uno::Reference< css::frame::XTitleChangeListener >*)NULL ), xListener );
}


void SAL_CALL TitleHelper::titleChanged(const css::frame::TitleChangedEvent& aEvent)
    throw (css::uno::RuntimeException)
{
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::uno::Reference< css::frame::XTitle > xSubTitle(m_xSubTitle.get (), css::uno::UNO_QUERY);

    aLock.clear ();
    

    if (aEvent.Source != xSubTitle)
        return;

    impl_updateTitle ();
}


void SAL_CALL TitleHelper::notifyEvent(const css::document::EventObject& aEvent)
    throw (css::uno::RuntimeException)
{
    if ( ! aEvent.EventName.equalsIgnoreAsciiCase("OnSaveAsDone")
      && ! aEvent.EventName.equalsIgnoreAsciiCase("OnModeChanged")
      && ! aEvent.EventName.equalsIgnoreAsciiCase("OnTitleChanged"))
        return;

    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::uno::Reference< css::frame::XModel > xOwner(m_xOwner.get (), css::uno::UNO_QUERY);

    aLock.clear ();
    

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
    throw(css::uno::RuntimeException)
{
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::uno::Reference< css::frame::XFrame > xOwner(m_xOwner.get (), css::uno::UNO_QUERY);

    aLock.clear ();
    

    if (aEvent.Source != xOwner)
        return;

    
    
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
    throw (css::uno::RuntimeException)
{
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);
        css::uno::Reference< css::uno::XInterface >         xOwner        (m_xOwner.get()          , css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XUntitledNumbers > xNumbers      (m_xUntitledNumbers.get(), css::uno::UNO_QUERY);
        ::sal_Int32                                         nLeasedNumber = m_nLeasedNumber;
    aLock.clear ();
    

    if ( ! xOwner.is ())
        return;

    if (xOwner != aEvent.Source)
        return;

    if (
        (xNumbers.is ()                                                   ) &&
        (nLeasedNumber != css::frame::UntitledNumbersConst::INVALID_NUMBER)
       )
       xNumbers->releaseNumber (nLeasedNumber);

    
    aLock.reset ();

         m_sTitle        = OUString ();
         m_nLeasedNumber = css::frame::UntitledNumbersConst::INVALID_NUMBER;

    aLock.clear ();
    

    impl_sendTitleChangedEvent ();
}


void TitleHelper::impl_sendTitleChangedEvent ()
{
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::frame::TitleChangedEvent aEvent(m_xOwner.get (), m_sTitle);

    aLock.clear ();
    

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


void TitleHelper::impl_updateTitle (bool init)
{
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::uno::Reference< css::frame::XModel >      xModel     (m_xOwner.get(), css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XController > xController(m_xOwner.get(), css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XFrame >      xFrame     (m_xOwner.get(), css::uno::UNO_QUERY);

    aLock.clear ();
    

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
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        
        
        if (m_bExternalTitle)
            return;

        css::uno::Reference< css::uno::XInterface >         xOwner        (m_xOwner.get()          , css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XUntitledNumbers > xNumbers      (m_xUntitledNumbers.get(), css::uno::UNO_QUERY);
        ::sal_Int32                                         nLeasedNumber = m_nLeasedNumber;

    aLock.clear ();
    

    if (
        ( ! xOwner.is    ()) ||
        ( ! xNumbers.is  ()) ||
        ( ! xModel.is    ())
       )
        return;

    OUString sTitle;
    OUString sURL  ;

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
            sNewTitle.appendAscii("?");

        sTitle = sNewTitle.makeStringAndClear ();
    }

    
    aLock.reset ();

    
    
    sal_Bool bChanged        = !init; 

             m_sTitle        = sTitle;
             m_nLeasedNumber = nLeasedNumber;

    aLock.clear ();
    

    if (bChanged)
        impl_sendTitleChangedEvent ();
}


void TitleHelper::impl_updateTitleForController (const css::uno::Reference< css::frame::XController >& xController, bool init)
{
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        
        
        if (m_bExternalTitle)
            return;

        css::uno::Reference< css::uno::XInterface >         xOwner        (m_xOwner.get()          , css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XUntitledNumbers > xNumbers      (m_xUntitledNumbers.get(), css::uno::UNO_QUERY);
        ::sal_Int32                                         nLeasedNumber = m_nLeasedNumber;

    aLock.clear ();
    

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

    
    aLock.reset ();

        OUString sNewTitle       = sTitle.makeStringAndClear ();
        sal_Bool        bChanged        = !init && m_sTitle != sNewTitle;
                        m_sTitle        = sNewTitle;
                        m_nLeasedNumber = nLeasedNumber;

    aLock.clear ();
    

    if (bChanged)
        impl_sendTitleChangedEvent ();
}


void TitleHelper::impl_updateTitleForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame, bool init)
{
    if ( ! xFrame.is ())
        return;

    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        
        
        if (m_bExternalTitle)
            return;

    aLock.clear ();
    

    css::uno::Reference< css::uno::XInterface > xComponent;
    xComponent = xFrame->getController ();
    if ( ! xComponent.is ())
        xComponent = xFrame->getComponentWindow ();

    OUStringBuffer sTitle (256);

    impl_appendComponentTitle   (sTitle, xComponent);
#ifndef MACOSX
    
    
    impl_appendProductName      (sTitle);
    impl_appendModuleName       (sTitle);
    impl_appendDebugVersion     (sTitle);
#endif
    
    aLock.reset ();

        OUString sNewTitle = sTitle.makeStringAndClear ();
        sal_Bool        bChanged  = !init && m_sTitle != sNewTitle;
                        m_sTitle  = sNewTitle;

    aLock.clear ();
    

    if (bChanged)
        impl_sendTitleChangedEvent ();
}


void TitleHelper::impl_appendComponentTitle (      OUStringBuffer&                       sTitle    ,
                                             const css::uno::Reference< css::uno::XInterface >& xComponent)
{
    css::uno::Reference< css::frame::XTitle > xTitle(xComponent, css::uno::UNO_QUERY);

    
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
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        css::uno::Reference< css::uno::XInterface >        xOwner   = m_xOwner.get();
        css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;

    aLock.clear ();
    

    try
    {
        css::uno::Reference< css::frame::XModuleManager2 > xModuleManager =
            css::frame::ModuleManager::create(xContext);

        const OUString                 sID     = xModuleManager->identify(xOwner);
              ::comphelper::SequenceAsHashMap lProps  = xModuleManager->getByName (sID);
        const OUString                 sUIName = lProps.getUnpackedValueOrDefault (OFFICEFACTORY_PROPNAME_UINAME, OUString());

        
        
        if (!sUIName.isEmpty())
        {
            sTitle.appendAscii (" "    );
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
    sTitle.append("]");
}
#else
void TitleHelper::impl_appendDebugVersion (OUStringBuffer&)
{
}
#endif


void TitleHelper::impl_startListeningForModel (const css::uno::Reference< css::frame::XModel >& xModel)
{
    css::uno::Reference< css::document::XEventBroadcaster > xBroadcaster(xModel, css::uno::UNO_QUERY);
    if ( ! xBroadcaster.is ())
        return;

    xBroadcaster->addEventListener (static_cast< css::document::XEventListener* >(this));
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
    
    ::osl::ResettableMutexGuard aLock(m_aMutex);

        
        css::uno::Reference< css::frame::XTitle > xOldSubTitle(m_xSubTitle.get(), css::uno::UNO_QUERY);
        if (xOldSubTitle == xSubTitle)
            return;

        m_xSubTitle = xSubTitle;

    aLock.clear ();
    

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

    if (aURL.GetProtocol() == INET_PROT_FILE)
    {
        if (aURL.HasMark())
            aURL = INetURLObject(aURL.GetURLNoMark());

        sTitle = aURL.getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET);
    }
    else
    {
        if (aURL.hasExtension(INetURLObject::LAST_SEGMENT))
            sTitle = aURL.getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET);

        if ( sTitle.isEmpty() )
            sTitle = aURL.GetHostPort(INetURLObject::DECODE_WITH_CHARSET);

        if ( sTitle.isEmpty() )
            sTitle = aURL.GetURLNoPass(INetURLObject::DECODE_WITH_CHARSET);
    }

    return sTitle;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
