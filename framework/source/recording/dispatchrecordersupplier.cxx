/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dispatchrecordersupplier.cxx,v $
 * $Revision: 1.6 $
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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
// include own things
#include <recording/dispatchrecordersupplier.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/readguard.hxx>
#include <services.h>

//_________________________________________________________________________________________________________________
// include interfaces
#include <com/sun/star/frame/XRecordableDispatch.hpp>

//_________________________________________________________________________________________________________________
// include other projects
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
// namespace

namespace framework{

//_________________________________________________________________________________________________________________
// non exported const

//_________________________________________________________________________________________________________________
// non exported definitions

//_________________________________________________________________________________________________________________
// declarations

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_3(
    DispatchRecorderSupplier,
    OWeakObject,
    DIRECT_INTERFACE(css::lang::XTypeProvider),
    DIRECT_INTERFACE(css::lang::XServiceInfo),
    DIRECT_INTERFACE(css::frame::XDispatchRecorderSupplier))

DEFINE_XTYPEPROVIDER_3(
    DispatchRecorderSupplier,
    css::lang::XTypeProvider,
    css::lang::XServiceInfo,
    css::frame::XDispatchRecorderSupplier)

DEFINE_XSERVICEINFO_MULTISERVICE(
    DispatchRecorderSupplier,
    ::cppu::OWeakObject,
    SERVICENAME_DISPATCHRECORDERSUPPLIER,
    IMPLEMENTATIONNAME_DISPATCHRECORDERSUPPLIER)

DEFINE_INIT_SERVICE(
    DispatchRecorderSupplier,
    {
        /*Attention
            I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
            to create a new instance of this class by our own supported service factory.
            see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
        */
    }
)

//_____________________________________________________________________________
/**
    @short  standard constructor to create instance
    @descr  Because an instance will be initialized by her interface methods
            it's not neccessary to do anything here.
 */
DispatchRecorderSupplier::DispatchRecorderSupplier( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory )
        //  init baseclasses first!
        //  Attention: Don't change order of initialization!
        : ThreadHelpBase     ( &Application::GetSolarMutex() )
        , ::cppu::OWeakObject(                               )
        //  init member
        , m_xDispatchRecorder( NULL                          )
        , m_xFactory         ( xFactory                      )
{
}

//_____________________________________________________________________________
/**
    @short  standard destructor
    @descr  We are a helper and not a real service. So we doesn't provide
            dispose() functionality. This supplier dies by ref count mechanism
            and should release all internal used ones too.
 */
DispatchRecorderSupplier::~DispatchRecorderSupplier()
{
    m_xFactory          = NULL;
    m_xDispatchRecorder = NULL;
}

//_____________________________________________________________________________
/**
    @short      set a new dispatch recorder on this supplier
    @descr      Because there can exist more then one recorder implementations
                (to generate java/basic/... scripts from recorded data) it must
                be possible to set it on a supplier.

    @see        getDispatchRecorder()

    @param      xRecorder
                the new recorder to set it
                <br><NULL/> isn't recommended, because recording without a
                valid recorder can't work. But it's not checked here. So user
                of this supplier can decide that without changing this
                implementation.

    @change     09.04.2002 by Andreas Schluens
 */
void SAL_CALL DispatchRecorderSupplier::setDispatchRecorder( const css::uno::Reference< css::frame::XDispatchRecorder >& xRecorder ) throw (css::uno::RuntimeException)
{
    // SAFE =>
    WriteGuard aWriteLock(m_aLock);
    m_xDispatchRecorder=xRecorder;
    // => SAFE
}
//_____________________________________________________________________________
/**
    @short      provides access to the dispatch recorder of this supplier
    @descr      Such recorder can be used outside to record dispatches.
                But normaly he is used internaly only. Of course he must used
                from outside to get the recorded data e.g. for saving it as a
                script.

    @see        setDispatchRecorder()

    @return     the internal used dispatch recorder
                <br>May it can be <NULL/> if no one was set before.

    @change     09.04.2002 by Andreas Schluens
 */
css::uno::Reference< css::frame::XDispatchRecorder > SAL_CALL DispatchRecorderSupplier::getDispatchRecorder() throw (css::uno::RuntimeException)
{
    // SAFE =>
    ReadGuard aReadLock(m_aLock);
    return m_xDispatchRecorder;
    // => SAFE
}

//_____________________________________________________________________________
/**
    @short      execute a dispatch request and record it
    @descr      If given dispatch object provides right recording interface it
                will be used. If it's not supported it record the pure dispatch
                parameters only. There is no code neither the possibility to
                check if recording is enabled or not.

    @param      aURL            the command URL
    @param      lArguments      optional arguments (see com.sun.star.document.MediaDescriptor for further informations)
    @param      xDispatcher     the original dispatch object which should be recorded

    @change     09.04.2002 by Andreas Schluens
 */
void SAL_CALL DispatchRecorderSupplier::dispatchAndRecord( const css::util::URL&                                  aURL        ,
                                                           const css::uno::Sequence< css::beans::PropertyValue >& lArguments  ,
                                                           const css::uno::Reference< css::frame::XDispatch >&    xDispatcher ) throw (css::uno::RuntimeException)
{
    // SAFE =>
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XDispatchRecorder > xRecorder = m_xDispatchRecorder;
    aReadLock.unlock();
    // => SAFE

    // clear unspecific situations
    if (!xDispatcher.is())
        throw css::uno::RuntimeException(DECLARE_ASCII("specification violation: dispatcher is NULL"), static_cast< ::cppu::OWeakObject* >(this));

    if (!xRecorder.is())
        throw css::uno::RuntimeException(DECLARE_ASCII("specification violation: no valid dispatch recorder available"), static_cast< ::cppu::OWeakObject* >(this));

    // check, if given dispatch supports record functionality by itself ...
    // or must be wrapped.
    css::uno::Reference< css::frame::XRecordableDispatch > xRecordable(
        xDispatcher,
        css::uno::UNO_QUERY);

    if (xRecordable.is())
        xRecordable->dispatchAndRecord(aURL,lArguments,xRecorder);
    else
    {
        // There is no reason to wait for information about success
        // of this request. Because status information of a dispatch
        // are not guaranteed. So we execute it and record used
        // parameters only.
        xDispatcher->dispatch(aURL,lArguments);
        xRecorder->recordDispatch(aURL,lArguments);
    }
}

}   // namespace framework
