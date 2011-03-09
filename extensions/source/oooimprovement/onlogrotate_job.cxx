/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include "onlogrotate_job.hxx"
#include "config.hxx"
#include "logpacker.hxx"
#include "logstorage.hxx"
#include "soaprequest.hxx"
#include "soapsender.hxx"

#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <osl/conditn.hxx>
#include <osl/thread.hxx>
#include <osl/time.h>
#include <cppuhelper/implbase1.hxx>
#include <memory>


using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;
using ::com::sun::star::frame::XTerminateListener;
using ::com::sun::star::frame::XDesktop;
using ::com::sun::star::ucb::XSimpleFileAccess;
using ::rtl::OUString;
using ::std::vector;

namespace
{
    using namespace oooimprovement;

    static void packLogs(const Reference<XMultiServiceFactory>& sf)
    {
        try
        {
            Config config(sf);
            LogPacker log_packer(sf);
            vector<OUString> csvfiles = LogStorage(sf).getUnzippedLogFiles();
            for(
                vector<OUString>::iterator item = csvfiles.begin();
                item!=csvfiles.end();
                item++)
                config.incrementEventCount(log_packer.pack(*item));
        } catch(...) {};
    };

    static void uploadLogs(const Reference<XMultiServiceFactory>& sf)
    {
        try
        {
            Config config(sf);
            Reference<XSimpleFileAccess> file_access(
                sf->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.SimpleFileAccess"))),
                UNO_QUERY_THROW);
            SoapSender sender(sf, config.getSoapUrl());
            OUString soap_id = config.getSoapId();
            vector<OUString> zipfiles = LogStorage(sf).getZippedLogFiles();
            for(
                vector<OUString>::iterator item = zipfiles.begin();
                item!=zipfiles.end();
                item++)
            {
                if(config.incrementFailedAttempts(1) > 25)
                {
                    config.giveupUploading();
                    LogStorage(sf).clear();
                    return;
                }
                sender.send(SoapRequest(sf, soap_id, *item));
                config.incrementReportCount(1);
                file_access->kill(*item);
                config.resetFailedAttempts();
            }
        } catch(...) {};
    }

    class OnLogRotateThread : public ::osl::Thread
    {
        public:
            OnLogRotateThread(Reference<XMultiServiceFactory> sf);
            virtual void SAL_CALL run();
            void stop();

        private:
            Reference<XMultiServiceFactory> m_ServiceFactory;
            ::osl::Condition m_Stop;
    };

    OnLogRotateThread::OnLogRotateThread(Reference<XMultiServiceFactory> sf)
        : m_ServiceFactory(sf)
    {
        OSL_ASSERT(sf.is());
    }

    void SAL_CALL OnLogRotateThread::run()
    {
        TimeValue wait_intervall = {30,0};
        if (m_Stop.wait(&wait_intervall) == ::osl::Condition::result_timeout)
        {
            try
            {
                if(Config(m_ServiceFactory).getInvitationAccepted())
                {
                    packLogs(m_ServiceFactory);
                    uploadLogs(m_ServiceFactory);
                }
                else
                    LogStorage(m_ServiceFactory).clear();
            }
            catch(...) {}
        }
    }

    void OnLogRotateThread::stop()
    {
        m_Stop.set();
    }

    class OnLogRotateThreadWatcher : public ::cppu::WeakImplHelper1<XTerminateListener>
    {
        public:
            OnLogRotateThreadWatcher(Reference<XMultiServiceFactory> sf)
                : m_Thread(new OnLogRotateThread(sf))
            {
                m_Thread->create();
            }
            virtual ~OnLogRotateThreadWatcher()
            {
                m_Thread->stop();
                m_Thread->join();
            };

            // XTerminateListener
            virtual void SAL_CALL queryTermination(const EventObject&) throw(RuntimeException)
                { };
            virtual void SAL_CALL notifyTermination(const EventObject&) throw(RuntimeException)
            {
                m_Thread->stop();
                m_Thread->join();
            };
            // XEventListener
            virtual void SAL_CALL disposing(const EventObject&) throw(RuntimeException)
            {
                m_Thread->stop();
                m_Thread->join();
            };
        private:
            ::std::auto_ptr<OnLogRotateThread> m_Thread;
    };
}

namespace oooimprovement
{
    OnLogRotateJob::OnLogRotateJob(const Reference<XComponentContext>& context)
        : m_ServiceFactory(Reference<XMultiServiceFactory>(
            context->getServiceManager()->createInstanceWithContext(
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.XMultiServiceFactory")), context),
            UNO_QUERY))
    { }

    OnLogRotateJob::OnLogRotateJob(const Reference<XMultiServiceFactory>& sf)
        : m_ServiceFactory(sf)
    { }

    OnLogRotateJob::~OnLogRotateJob()
    { }

    void SAL_CALL OnLogRotateJob::executeAsync(
        const Sequence<NamedValue>&,
        const Reference<XJobListener>& listener)
        throw(RuntimeException)
    {
        Reference<XDesktop> xDesktop(
            m_ServiceFactory->createInstance(OUString::createFromAscii("com.sun.star.frame.Desktop")),
            UNO_QUERY);
        if(xDesktop.is())
            xDesktop->addTerminateListener(Reference<XTerminateListener>(new OnLogRotateThreadWatcher(m_ServiceFactory)));
        Any result;
        listener->jobFinished(Reference<XAsyncJob>(this), result);
    }

    sal_Bool SAL_CALL OnLogRotateJob::supportsService(const OUString& service_name) throw(RuntimeException)
    {
        const Sequence<OUString> service_names(getSupportedServiceNames());
        for (sal_Int32 idx = service_names.getLength()-1; idx>=0; --idx)
            if(service_name == service_names[idx]) return sal_True;
        return sal_False;
    }

    OUString SAL_CALL OnLogRotateJob::getImplementationName() throw(RuntimeException)
    { return getImplementationName_static(); }

    Sequence<OUString> SAL_CALL OnLogRotateJob::getSupportedServiceNames() throw(RuntimeException)
    { return getSupportedServiceNames_static(); }

    OUString SAL_CALL OnLogRotateJob::getImplementationName_static()
    { return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.extensions.oooimprovement.OnLogRotateJob")); }

    Sequence<OUString> SAL_CALL OnLogRotateJob::getSupportedServiceNames_static()
    {
        Sequence<OUString> aServiceNames(1);
        aServiceNames[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.task.AsyncJob"));
        return aServiceNames;
    }

    Reference<XInterface> OnLogRotateJob::Create(const Reference<XComponentContext>& context)
    { return *(new OnLogRotateJob(context)); }

    Reference<XInterface> OnLogRotateJob::Create(const Reference<XMultiServiceFactory>& sf)
    { return *(new OnLogRotateJob(sf)); }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
