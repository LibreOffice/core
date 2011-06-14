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
#ifndef _FINALTHREADMANAGER_HXX
#define _FINALTHREADMANAGER_HXX

#include "sal/config.h"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/implbase3.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/util/XJobManager.hpp"
#include "com/sun/star/frame/XTerminateListener2.hpp"


#include <osl/mutex.hxx>

#include <list>

class CancelJobsThread;
class TerminateOfficeThread;
class SwPauseThreadStarting;

// service helper namespace
namespace comp_FinalThreadManager {

// component and service helper functions:
::rtl::OUString SAL_CALL _getImplementationName();
com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames();
com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL _create(
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const & context );

} // closing service helper namespace


class FinalThreadManager : public ::cppu::WeakImplHelper3< com::sun::star::lang::XServiceInfo,
                                                           com::sun::star::util::XJobManager,
                                                           com::sun::star::frame::XTerminateListener2 >
{
public:
    explicit FinalThreadManager(com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const & context);

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (com::sun::star::uno::RuntimeException);
    virtual com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XJobManager:
    virtual void SAL_CALL registerJob(const com::sun::star::uno::Reference< com::sun::star::util::XCancellable > & Job) throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL releaseJob(const com::sun::star::uno::Reference< com::sun::star::util::XCancellable > & Job) throw (com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancelAllJobs() throw (com::sun::star::uno::RuntimeException);

    // ::com::sun::star::frame::XTerminateListener2
    virtual void SAL_CALL cancelTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::frame::XTerminateListener (inherited via com::sun::star::frame::XTerminateListener2)
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& Event ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun:star::lang::XEventListener (inherited via com::sun::star::frame::XTerminateListener)
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

private:
    FinalThreadManager(FinalThreadManager &); // not defined
    void operator =(FinalThreadManager &); // not defined

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
