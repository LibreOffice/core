/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <com/sun/star/awt/XDataTransferProviderAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/URL.hpp>

using namespace cpo::uno;
using namespace css::awt;
using namespace css::beans;
using namespace css::frame;
using namespace css::lang;
using namespace css::util;

/**
 * implements a configurable interceptor for dispatch events.
 */
class Interceptor : public cppu::WeakImplHelper<XDispatch,
                                    XDispatchProviderInterceptor,
                                    XInterceptorInfo>
{
    /** contains the list of interception URL schema's (wildcards are allowed there!)
        supported by this interceptor. It can be set from outside.
        If no external URLs are set, the default "*" is used instead.
        That would have the same effect as if this implementation would not support the
        interface XInterceptorInfo !
     */
    Sequence<OUString> m_lURLs4InterceptionInfo;



    /** These URL's will be blocked by this interceptor.
        Can be set from outside. Every queryDispatch() for these
        set of URL's will be answered with an empty dispatch object!
        If no external URLs are set the default "*" is used instead.
        So every incoming URL will be blocked .-)
     */
    std::vector<OUString> m_lURLs4Blocking;



    /** Every dispatch interceptor knows it's master and slave interceptor
        of the dispatch chain. These values must be stupid handled .-)
        They have to be set and reset in case the right interface methods are called.
        Nothing more. It's not allowed to dispose() it.
        The slave can be used inside queryDispatch() to forward requests,
        which are not handled by this interceptor instance.
     */
    Reference<XDispatchProvider> m_xSlave;
    Reference<XDispatchProvider> m_xMaster;



    /** counts calls of setSlave...().
        So the outside API test can use this value to know if this interceptor
        was really added to the interceptor chain of OOo.
     */
    sal_Int32 m_nRegistrationCount = 0;



    /** indicates if this interceptor object is currently part of the interceptor
        chain of OOo. Only true if a valid slave or master dispatch is set on this
        instance.
     */
    bool m_bIsRegistered = false;

    /** points to the global uno service manager. */
    Reference<XMultiServiceFactory> m_xMSF;

    std::recursive_mutex m_aMutex;

public:
    Interceptor(const Reference<XMultiServiceFactory>& xMSF)
    {
        m_xMSF = xMSF;
    }


    /** XInterceptorInfo */
    virtual Sequence<OUString> getInterceptedURLs() override
    {
        std::unique_lock l(m_aMutex);
        return impl_getURLs4InterceptionInfo();
    }



    /** XDispatchProviderInterceptor */
    virtual Reference<XDispatchProvider> getSlaveDispatchProvider() override
    {
        std::unique_lock l(m_aMutex);
        return m_xSlave;
    }



    /** XDispatchProviderInterceptor */
    virtual Reference<XDispatchProvider> getMasterDispatchProvider() override
    {
        std::unique_lock l(m_aMutex);
        return m_xMaster;
    }



    /** XDispatchProviderInterceptor */
    virtual void setSlaveDispatchProvider(const Reference<XDispatchProvider>& xSlave) override
    {
        std::unique_lock l(m_aMutex);
        if (xSlave)
        {
            ++m_nRegistrationCount;
            m_bIsRegistered = true;
        }
        else
        {
            m_bIsRegistered = false;
        }

        m_xSlave = xSlave;
    }

    /** XDispatchProviderInterceptor */
    virtual void setMasterDispatchProvider(const Reference<XDispatchProvider>& xMaster) override
    {
        std::unique_lock l(m_aMutex);
        m_xMaster = xMaster;
    }

private:
    Reference<XDataTransferProviderAccess> m_xToolkit;

    /** A beautiful method whose only purpose is to take and release a
     * solar mutex. If this hangs - you can see a beautiful deadlock
     * when you attach your debugger to the main process.
     */
    void checkNoSolarMutexHeld()
    {
        if (!m_xToolkit)
            m_xToolkit.set(
                m_xMSF->createInstance(u"com.sun.star.awt.Toolkit"_ustr), UNO_QUERY);

        // A Method notable only for taking the solar mutex.
        m_xToolkit->getDragSource( nullptr );
    }

public:
    /** XDispatchProvider
     */
    virtual Reference<XDispatch> queryDispatch(const URL&    aURL,
                                                const OUString& sTargetFrameName,
                                                sal_Int32    nSearchFlags) override
    {
        std::unique_lock l(m_aMutex);

        checkNoSolarMutexHeld();

        if (impl_isBlockedURL(aURL.Complete))
        {
            return nullptr;
        }

        if (m_xSlave)
        {
            return m_xSlave->queryDispatch(aURL, sTargetFrameName, nSearchFlags);
        }

        return this;
    }



    /** XDispatchProvider
     */
    virtual Sequence<Reference<XDispatch>> queryDispatches(const Sequence<DispatchDescriptor>& lRequests) override
    {
        int c = lRequests.getLength();

        Sequence<Reference<XDispatch>> lResults(c);
        for (sal_Int32 i=0; i<c; ++i)
        {
            lResults.getArray()[i] = queryDispatch(lRequests[i].FeatureURL ,
                                        lRequests[i].FrameName  ,
                                        lRequests[i].SearchFlags);
        }

        return lResults;
    }



    /** XDispatch
     */
    virtual void dispatch(const URL& , const Sequence<PropertyValue>& ) override
    {
    }



    /** XDispatch
     */
    virtual void addStatusListener(const Reference<XStatusListener>& ,
                                               const URL&     ) override
    {
    }



    /** XDispatch
     */
    virtual void removeStatusListener(const Reference<XStatusListener>& , const URL&     ) override
    {
    }



    sal_Int32 getRegistrationCount()
    {
        std::unique_lock l(m_aMutex);
        return m_nRegistrationCount;
    }



    bool isRegistered()
    {
        std::unique_lock l(m_aMutex);
        return m_bIsRegistered;
    }


    /** set a new list of URL's, which should be blocked by this interceptor.
        (that's why it's necessary to call this impl-method before the interceptor
        is used at the OOo API!)
     */
    void setURLs4URLs4Blocking(std::vector<OUString> lURLs)
    {
        std::unique_lock l(m_aMutex);
        m_lURLs4Blocking = std::move(lURLs);
    }


private:

    /** must be used internal to access the member m_lURLs4InterceptionInfo
        - threadsafe
        - and to make sure it's initialized on demand
     */
    Sequence<OUString> impl_getURLs4InterceptionInfo()
    {
        std::unique_lock l(m_aMutex);
        if (m_lURLs4InterceptionInfo.getLength() == 0)
        {
            m_lURLs4InterceptionInfo =  { u"*"_ustr };
        }

        return m_lURLs4InterceptionInfo;
    }



    /** must be used internal to access the member m_lURLs4Blocking
        - threadsafe
        - and to make sure it's initialized on demand
     */
    std::vector<OUString> impl_getURLs4Blocking()
    {
        std::unique_lock l(m_aMutex);
        if (m_lURLs4Blocking.empty())
        {
            m_lURLs4Blocking    =  { u"*"_ustr };
        }

        return m_lURLs4Blocking;
    }


    bool impl_isBlockedURL(std::u16string_view sURL)
    {
        std::vector<OUString> lBlockedURLs = impl_getURLs4Blocking();

        for (const auto & s : lBlockedURLs)
        {
            if (sURL == s)
            {
                return true;
            }
        }

        return false;
    }

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
