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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_FINALTHREADMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FINALTHREADMANAGER_HXX

#include <sal/config.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XJobManager.hpp>
#include <com/sun/star/frame/XTerminateListener2.hpp>
#include <osl/mutex.hxx>
#include <list>

class CancelJobsThread;
class TerminateOfficeThread;
class SwPauseThreadStarting;

class FinalThreadManager : public ::cppu::WeakImplHelper< css::lang::XServiceInfo,
                                                           css::util::XJobManager,
                                                           css::frame::XTerminateListener2 >
{
public:
    explicit FinalThreadManager(css::uno::Reference< css::uno::XComponentContext > const & context);

    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString & ServiceName) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException, std::exception) override;

    // css::util::XJobManager:
    virtual void SAL_CALL registerJob(const css::uno::Reference< css::util::XCancellable > & Job) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL releaseJob(const css::uno::Reference< css::util::XCancellable > & Job) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL cancelAllJobs() throw (css::uno::RuntimeException, std::exception) override;

    // css::frame::XTerminateListener2
    virtual void SAL_CALL cancelTermination( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;

    // css::frame::XTerminateListener (inherited via css::frame::XTerminateListener2)
    virtual void SAL_CALL queryTermination( const css::lang::EventObject& Event ) throw (css::frame::TerminationVetoException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL notifyTermination( const css::lang::EventObject& Event ) throw (css::uno::RuntimeException, std::exception) override;

    // ::com::sun:star::lang::XEventListener (inherited via css::frame::XTerminateListener)
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

private:
    FinalThreadManager(FinalThreadManager &) = delete;
    void operator =(FinalThreadManager &) = delete;

    virtual ~FinalThreadManager();

    void registerAsListenerAtDesktop();

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    osl::Mutex maMutex;

    std::list< css::uno::Reference< css::util::XCancellable > > maThreads;
    CancelJobsThread* mpCancelJobsThread;
    TerminateOfficeThread* mpTerminateOfficeThread;
    SwPauseThreadStarting* mpPauseThreadStarting;

    bool mbRegisteredAtDesktop;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
