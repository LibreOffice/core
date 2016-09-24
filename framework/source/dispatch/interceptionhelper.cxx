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

#include <dispatch/interceptionhelper.hxx>

#include <com/sun/star/frame/XInterceptorInfo.hpp>

#include <vcl/svapp.hxx>

namespace framework{

InterceptionHelper::InterceptionHelper(const css::uno::Reference< css::frame::XFrame >&            xOwner,
                                       const css::uno::Reference< css::frame::XDispatchProvider >& xSlave)
    : m_xOwnerWeak  (xOwner                       )
    , m_xSlave      (xSlave                       )
{
}

InterceptionHelper::~InterceptionHelper()
{
}

css::uno::Reference< css::frame::XDispatch > SAL_CALL InterceptionHelper::queryDispatch(const css::util::URL&  aURL            ,
                                                                                        const OUString& sTargetFrameName,
                                                                                              sal_Int32        nSearchFlags    )
    throw(css::uno::RuntimeException, std::exception)
{
    // SAFE {
    SolarMutexClearableGuard aReadLock;

    // a) first search an interceptor, which match to this URL by its URL pattern registration
    //    Note: if it return NULL - it does not mean an empty interceptor list automatically!
    css::uno::Reference< css::frame::XDispatchProvider > xInterceptor;
    InterceptorList::const_iterator pIt = m_lInterceptionRegs.findByPattern(aURL.Complete);
    if (pIt != m_lInterceptionRegs.end())
        xInterceptor = pIt->xInterceptor;

    // b) No match by registration - but a valid interceptor list.
    //    Find first interceptor w/o pattern, so we need to query it
    if (!xInterceptor.is() && m_lInterceptionRegs.size()>0)
    {
        InterceptorList::const_iterator pIt2;
        for (pIt2=m_lInterceptionRegs.begin(); pIt2!=m_lInterceptionRegs.end(); ++pIt2)
        {
            if (!pIt2->lURLPattern.getLength())
            {
                // no pattern -> need to ask this guy!
                xInterceptor = pIt2->xInterceptor;
                break;
            }
        }
        // if we didn't find any non-pattern interceptor, there's no-one
        // registered for this command url (we already searched for matching
        // patterns above)
    }
    // c) No registered interceptor => use our direct slave.
    //    This helper exist by design and must be valid everytimes ...
    //    But to be more feature proof - we should check that .-)
    if (!xInterceptor.is() && m_xSlave.is())
        xInterceptor = m_xSlave;

    aReadLock.clear();
    // } SAFE

    css::uno::Reference< css::frame::XDispatch > xReturn;
    if (xInterceptor.is())
        xReturn = xInterceptor->queryDispatch(aURL, sTargetFrameName, nSearchFlags);
    return xReturn;
}

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL InterceptionHelper::queryDispatches( const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor )
    throw(css::uno::RuntimeException, std::exception)
{
          sal_Int32                                                          c           = lDescriptor.getLength();
          css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > lDispatches (c);
          css::uno::Reference< css::frame::XDispatch >*                      pDispatches = lDispatches.getArray();
    const css::frame::DispatchDescriptor*                                    pDescriptor = lDescriptor.getConstArray();

    for (sal_Int32 i=0; i<c; ++i)
        pDispatches[i] = queryDispatch(pDescriptor[i].FeatureURL, pDescriptor[i].FrameName, pDescriptor[i].SearchFlags);

    return lDispatches;
}

void SAL_CALL InterceptionHelper::registerDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
    throw(css::uno::RuntimeException, std::exception)
{
    // reject incorrect calls of this interface method
    css::uno::Reference< css::frame::XDispatchProvider > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
    if (!xInterceptor.is())
        throw css::uno::RuntimeException("NULL references not allowed as in parameter", xThis);

    // Fill a new info structure for new interceptor.
    // Save his reference and try to get an additional URL/pattern list from him.
    // If no list exist register these interceptor for all dispatch events with "*"!
    InterceptorInfo aInfo;

    aInfo.xInterceptor.set(xInterceptor, css::uno::UNO_QUERY);
    css::uno::Reference< css::frame::XInterceptorInfo > xInfo(xInterceptor, css::uno::UNO_QUERY);
    if (xInfo.is())
        aInfo.lURLPattern = xInfo->getInterceptedURLs();
    else
    {
        aInfo.lURLPattern.realloc(1);
        aInfo.lURLPattern[0] = "*";
    }

    // SAFE {
    SolarMutexClearableGuard aWriteLock;

    // a) no interceptor at all - set this instance as master for given interceptor
    //    and set our slave as its slave - and put this interceptor to the list.
    //    Its place there doesn't matter. Because this list is currently empty.
    if (m_lInterceptionRegs.empty())
    {
        xInterceptor->setMasterDispatchProvider(xThis   );
        xInterceptor->setSlaveDispatchProvider (m_xSlave);
        m_lInterceptionRegs.push_back(aInfo);
    }

    // b) OK - there is at least one interceptor already registered.
    //    It's slave and it's master must be valid references ...
    //    because we created it.

    // insert it before any other existing interceptor - means at the beginning of our list.
    else
    {
        css::uno::Reference< css::frame::XDispatchProvider >            xSlaveD = m_lInterceptionRegs.begin()->xInterceptor;
        css::uno::Reference< css::frame::XDispatchProviderInterceptor > xSlaveI (xSlaveD , css::uno::UNO_QUERY);

        xInterceptor->setMasterDispatchProvider(xThis             );
        xInterceptor->setSlaveDispatchProvider (xSlaveD           );
        xSlaveI->setMasterDispatchProvider     (aInfo.xInterceptor);

        m_lInterceptionRegs.push_front(aInfo);
    }

    css::uno::Reference< css::frame::XFrame > xOwner(m_xOwnerWeak.get(), css::uno::UNO_QUERY);

    aWriteLock.clear();
    // } SAFE

    // Don't forget to send a frame action event "context changed".
    // Any cached dispatch objects must be validated now!
    if (xOwner.is())
        xOwner->contextChanged();
}

void SAL_CALL InterceptionHelper::releaseDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
    throw(css::uno::RuntimeException, std::exception)
{
    // reject wrong calling of this interface method
    css::uno::Reference< css::frame::XDispatchProvider > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY);
    if (!xInterceptor.is())
        throw css::uno::RuntimeException("NULL references not allowed as in parameter", xThis);

    // SAFE {
    SolarMutexClearableGuard aWriteLock;

    // search this interceptor ...
    // If it could be located inside cache -
    // use its slave/master relations to update the interception list;
    // set empty references for it as new master and slave;
    // and release it from out cache.
    InterceptorList::iterator pIt = m_lInterceptionRegs.findByReference(xInterceptor);
    if (pIt != m_lInterceptionRegs.end())
    {
        css::uno::Reference< css::frame::XDispatchProvider >            xSlaveD  (xInterceptor->getSlaveDispatchProvider() , css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XDispatchProvider >            xMasterD (xInterceptor->getMasterDispatchProvider(), css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XDispatchProviderInterceptor > xSlaveI  (xSlaveD                                  , css::uno::UNO_QUERY);
        css::uno::Reference< css::frame::XDispatchProviderInterceptor > xMasterI (xMasterD                                 , css::uno::UNO_QUERY);

        if (xMasterI.is())
            xMasterI->setSlaveDispatchProvider(xSlaveD);

        if (xSlaveI.is())
            xSlaveI->setMasterDispatchProvider(xMasterD);

        xInterceptor->setSlaveDispatchProvider (css::uno::Reference< css::frame::XDispatchProvider >());
        xInterceptor->setMasterDispatchProvider(css::uno::Reference< css::frame::XDispatchProvider >());

        m_lInterceptionRegs.erase(pIt);
    }

    css::uno::Reference< css::frame::XFrame > xOwner(m_xOwnerWeak.get(), css::uno::UNO_QUERY);

    aWriteLock.clear();
    // } SAFE

    // Don't forget to send a frame action event "context changed".
    // Any cached dispatch objects must be validated now!
    if (xOwner.is())
        xOwner->contextChanged();
}

#define FORCE_DESTRUCTION_OF_INTERCEPTION_CHAIN
void SAL_CALL InterceptionHelper::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException, std::exception)
{
    #ifdef FORCE_DESTRUCTION_OF_INTERCEPTION_CHAIN
    // SAFE ->
    SolarMutexResettableGuard aReadLock;

    // check call... we accept such disposing calls only from our owner frame.
    css::uno::Reference< css::frame::XFrame > xOwner(m_xOwnerWeak.get(), css::uno::UNO_QUERY);
    if (aEvent.Source != xOwner)
        return;

    // Because every interceptor hold at least one reference to us ... and we destruct this list
    // of interception objects ... we should hold ourself alive .-)
    css::uno::Reference< css::frame::XDispatchProvider > xThis(static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY_THROW);

    // We need a full copy of all currently registered interceptor objects.
    // Otherwise we can't iterate over this vector without the risk, that our iterator will be invalid.
    // Because this vetor will be influenced by every deregistered interceptor.
    InterceptionHelper::InterceptorList aCopy = m_lInterceptionRegs;

    aReadLock.clear();
    // <- SAFE

    InterceptionHelper::InterceptorList::iterator pIt;
    for (  pIt  = aCopy.begin();
           pIt != aCopy.end();
         ++pIt                 )
    {
        InterceptionHelper::InterceptorInfo& rInfo = *pIt;
        if (rInfo.xInterceptor.is())
        {
            css::uno::Reference< css::frame::XDispatchProviderInterceptor > xInterceptor(rInfo.xInterceptor, css::uno::UNO_QUERY_THROW);
            releaseDispatchProviderInterceptor(xInterceptor);
            rInfo.xInterceptor.clear();
        }
    }

    aCopy.clear();

    #if OSL_DEBUG_LEVEL > 0
    // SAFE ->
    aReadLock.reset();
    if (!m_lInterceptionRegs.empty() )
        OSL_FAIL("There are some pending interceptor objects, which seems to be registered during (!) the destruction of a frame.");
    aReadLock.clear();
    // <- SAFE
    #endif // ODL_DEBUG_LEVEL>0

    #endif // FORCE_DESTRUCTION_OF_INTERCEPTION_CHAIN
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
