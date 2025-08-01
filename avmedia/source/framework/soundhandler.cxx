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

#include "soundhandler.hxx"

#include <unotools/mediadescriptor.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/frame/DispatchResultState.hpp>

#include <avmedia/mediawindow.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace avmedia{


//  XServiceInfo
OUString SAL_CALL SoundHandler::getImplementationName()
{
    return u"com.sun.star.comp.framework.SoundHandler"_ustr;
}

// XServiceInfo
sal_Bool SAL_CALL SoundHandler::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

// XServiceInfo
css::uno::Sequence< OUString > SAL_CALL SoundHandler::getSupportedServiceNames()
{
    return { u"com.sun.star.frame.ContentHandler"_ustr };
}

/*-************************************************************************************************************
    @short      standard ctor
    @descr      These initialize a new instance of this class with needed information for work.

    @seealso    using at owner

    @param      "xFactory", reference to service manager for creation of new services
    @onerror    Show an assertion and do nothing else.
    @threadsafe yes
*//*-*************************************************************************************************************/
SoundHandler::SoundHandler()
        // Init member
    :   m_bError        ( false    )
    ,   m_aUpdateIdle   ( "avmedia SoundHandler Update" )
{
    m_aUpdateIdle.SetInvokeHandler(LINK(this, SoundHandler, implts_PlayerNotify));
}

/*-************************************************************************************************************
    @short      standard dtor
*//*-*************************************************************************************************************/
SoundHandler::~SoundHandler()
{
    if (m_xListener.is())
    {
        css::frame::DispatchResultEvent aEvent;
        aEvent.State = css::frame::DispatchResultState::FAILURE;
        m_xListener->dispatchFinished(aEvent);
        m_xListener.clear();
    }
}

/*-************************************************************************************************************
    @interface  css::frame::XDispatch

    @short      try to load audio file
    @descr      This method try to load given audio file by URL and play it. We use vcl/Sound class to do that.
                Playing of sound is asynchronous every time.

    @attention  We must hold us alive by ourself ... because we use async. vcl sound player ... but playing is started
                in async interface call "dispatch()" too. And caller forget us immediately. But then our uno ref count
                will decreased to 0 and will die. The only solution is to use own reference to our implementation.
                But we do it for really started jobs only and release it during call back of vcl.

    @seealso    class vcl/Sound
    @seealso    method implts_PlayerNotify()

    @param      "aURL"      , URL to dispatch.
    @param      "lArguments", list of optional arguments.
    @onerror    We do nothing.
    @threadsafe yes
*//*-*************************************************************************************************************/
void SAL_CALL SoundHandler::dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                     const css::uno::Sequence< css::beans::PropertyValue >&            lDescriptor,
                                                     const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    // SAFE {
    const std::unique_lock aLock(m_aMutex);

    utl::MediaDescriptor aDescriptor(lDescriptor);

    {
    //close streams otherwise on windows we can't reopen the file in the
    //media player when we pass the url to directx as it'll already be open
    css::uno::Reference< css::io::XInputStream > xInputStream =
        aDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_INPUTSTREAM,
        css::uno::Reference< css::io::XInputStream >());
    if (xInputStream.is()) xInputStream->closeInput();
    }

    // If player currently used for other dispatch() requests ...
    // cancel it by calling stop()!
    m_aUpdateIdle.Stop();
    if (m_xPlayer.is())
    {
        if (m_xPlayer->isPlaying())
            m_xPlayer->stop();
        m_xPlayer.clear();
    }

    // Try to initialize player.
    m_xListener = xListener;
    try
    {
        m_bError = false;
        m_xPlayer.set( avmedia::MediaWindow::createPlayer( aURL.Complete, aDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_REFERRER, OUString()) ), css::uno::UNO_SET_THROW );
        // OK- we can start async playing ...
        // Count this request and initialize self-holder against dying by uno ref count ...
        m_xSelfHold.set(getXWeak());
        m_xPlayer->start();
        m_aUpdateIdle.SetPriority( TaskPriority::HIGH_IDLE );
        m_aUpdateIdle.Start();
    }
    catch( css::uno::Exception& )
    {
        m_bError = true;
        m_xPlayer.clear();
    }

    // } SAFE
}

void SAL_CALL SoundHandler::dispatch( const css::util::URL&                                  aURL       ,
                                      const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
{
    dispatchWithNotification(aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >());
}

/*-************************************************************************************************************
    @interface  css::document::XExtendedFilterDetection

    @short      try to detect file (given as argument included in "lDescriptor")
    @descr      We try to detect, if given file could be handled by this class and is a well known one.
                If it is - we return right internal type name - otherwise we return nothing!
                So call can search for another detect service and ask him too.

    @attention  a) We don't need any mutex here ... because we don't use any member!
                b) Don't use internal player instance "m_pPlayer" to detect given sound file!
                   It's not necessary to do that ... and we can use temp. variable to do the same.
                   This way is easy - we don't must synchronize it with currently played sounds!
                   Another reason to do so ... We are a listener on our internal ma_Player object.
                   If you would call "IsSoundFile()" on this instance, he would call us back and
                   we make some unnecessary things ...
    @param      "lDescriptor", description of file to detect
    @return     Internal type name which match this file ... or nothing if it is unknown.

    @onerror    We return nothing.
    @threadsafe yes
*//*-*************************************************************************************************************/
OUString SAL_CALL SoundHandler::detect( css::uno::Sequence< css::beans::PropertyValue >& lDescriptor )
{
    // Our default is "nothing". So we can return it, if detection failed or file type is really unknown.
    OUString sTypeName;

    // Analyze given descriptor to find filename or input stream or ...
    utl::MediaDescriptor aDescriptor(lDescriptor);
    OUString                      sURL       = aDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_URL, OUString());
    OUString                      sReferer   = aDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_REFERRER, OUString());

    if (
        !sURL.isEmpty() &&
        (avmedia::MediaWindow::isMediaURL(sURL, sReferer))
       )
    {
        // If the file type is supported depends on the OS, so...
        // I think we can the following ones:
        //  a) look for given extension of url to map our type decision HARD CODED!!!
        //  b) return preferred type every time... it's easy :-)
        sTypeName = u"wav_Wave_Audio_File"_ustr;
        aDescriptor[utl::MediaDescriptor::PROP_TYPENAME] <<= sTypeName;
        aDescriptor >> lDescriptor;
    }

    // Return our decision.
    return sTypeName;
}

/*-************************************************************************************************************
    @short      call back of sound player
    @descr      Our player call us back to give us some information.
                We use this information to callback our might existing listener.

    @seealso    method dispatchWithNotification()
    @return     0 every time... it doesn't matter for us.
    @threadsafe yes
*//*-*************************************************************************************************************/
IMPL_LINK_NOARG(SoundHandler, implts_PlayerNotify, Timer *, void)
{
    // SAFE {
    std::unique_lock aLock(m_aMutex);

    if (m_xPlayer.is() && m_xPlayer->isPlaying() && m_xPlayer->getMediaTime() < m_xPlayer->getDuration())
    {
        m_aUpdateIdle.Start();
        return;
    }
    m_xPlayer.clear();

    // We use m_xSelfHold to let us die ... but we must live till real finishing of this method too!!!
    // So we SHOULD use another "self-holder" temp. to provide that ...
    css::uno::Reference< css::uno::XInterface > xOperationHold = m_xSelfHold;
    m_xSelfHold.clear();

    // notify might existing listener
    // And forget this listener!
    // Because the corresponding dispatch was finished.
    if (m_xListener.is())
    {
        css::frame::DispatchResultEvent aEvent;
        if (!m_bError)
            aEvent.State = css::frame::DispatchResultState::SUCCESS;
        else
            aEvent.State = css::frame::DispatchResultState::FAILURE;
        m_xListener->dispatchFinished(aEvent);
        m_xListener.clear();
    }

    // } SAFE
    //release aLock before end of method at which point xOperationHold goes out of scope and pThis dies
    aLock.unlock();
}

} // namespace framework


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_framework_SoundHandler_get_implementation(css::uno::XComponentContext*,
                                                            css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new avmedia::SoundHandler);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
