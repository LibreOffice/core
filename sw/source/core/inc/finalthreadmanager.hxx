/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: finalthreadmanager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 13:33:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

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
