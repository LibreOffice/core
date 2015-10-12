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

class FinalThreadManager : public ::cppu::WeakImplHelper< com::sun::star::lang::XServiceInfo,
                                                           com::sun::star::util::XJobManager,
                                                           com::sun::star::frame::XTerminateListener2 >
{
public:
    explicit FinalThreadManager(com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const & context);

    // ::com::sun::star::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL supportsService(const OUString & ServiceName) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::util::XJobManager:
    virtual void SAL_CALL registerJob(const com::sun::star::uno::Reference< com::sun::star::util::XCancellable > & Job) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL releaseJob(const com::sun::star::uno::Reference< com::sun::star::util::XCancellable > & Job) throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL cancelAllJobs() throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::frame::XTerminateListener2
    virtual void SAL_CALL cancelTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun::star::frame::XTerminateListener (inherited via com::sun::star::frame::XTerminateListener2)
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ::com::sun:star::lang::XEventListener (inherited via com::sun::star::frame::XTerminateListener)
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    FinalThreadManager(FinalThreadManager &) = delete;
    void operator =(FinalThreadManager &) = delete;

    virtual ~FinalThreadManager();

    void registerAsListenerAtDesktop();

    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_xContext;

    osl::Mutex maMutex;

    std::list< com::sun::star::uno::Reference< com::sun::star::util::XCancellable > > maThreads;
    CancelJobsThread* mpCancelJobsThread;
    TerminateOfficeThread* mpTerminateOfficeThread;
    SwPauseThreadStarting* mpPauseThreadStarting;

    bool mbRegisteredAtDesktop;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
