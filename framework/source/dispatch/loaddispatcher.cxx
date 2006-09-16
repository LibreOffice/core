/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loaddispatcher.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:54:47 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_______________________________________________
// my own includes

#ifndef __FRAMEWORK_DISPATCH_LOADDISPATCHER_HXX_
#include <dispatch/loaddispatcher.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_READGUARD_HXX_
#include <threadhelp/readguard.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_WRITEGUARD_HXX_
#include <threadhelp/writeguard.hxx>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif

//_______________________________________________
// includes of other projects

//_______________________________________________
// namespace

namespace framework{

namespace css = ::com::sun::star;

//_______________________________________________
// declarations

/*-----------------------------------------------
    20.08.2003 09:52
-----------------------------------------------*/
LoadDispatcher::LoadDispatcher(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR       ,
                               const css::uno::Reference< css::frame::XFrame >&              xOwnerFrame ,
                               const ::rtl::OUString                                         sTargetName ,
                                     sal_Int32                                               nSearchFlags)
    : ThreadHelpBase(            )
    , m_xSMGR       (xSMGR       )
    , m_xOwnerFrame (xOwnerFrame )
    , m_sTarget     (sTargetName )
    , m_nSearchFlags(nSearchFlags)
    , m_aLoader     (xSMGR       )
{
}

/*-----------------------------------------------
    20.08.2003 09:12
-----------------------------------------------*/
LoadDispatcher::~LoadDispatcher()
{
    m_xSMGR.clear();
}

/*-----------------------------------------------
    20.08.2003 09:58
-----------------------------------------------*/
void SAL_CALL LoadDispatcher::dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                       const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                       const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
    throw(css::uno::RuntimeException)
{
    // Attention: May be nobody outside hold such temp. dispatch object alive (because
    // the container in which we resists isnt implemented threadsafe but updated by a timer
    // and clear our reference ...) we should hold us self alive!
    css::uno::Reference< css::uno::XInterface > xThis(static_cast< css::frame::XNotifyingDispatch* >(this), css::uno::UNO_QUERY);

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    // We are the only client of this load env object ... but
    // may a dispatch request before is still in progress (?!).
    // Then we should wait a little bit and block this new request.
    // In case we run into the timeout, we should reject this new request
    // and return "FAILED" as result. Otherwhise we can start this new operation.
    if (!m_aLoader.waitWhileLoading(2000)) // => 2 sec.
    {
        if (xListener.is())
            xListener->dispatchFinished(
                css::frame::DispatchResultEvent(xThis, css::frame::DispatchResultState::DONTKNOW, css::uno::Any())); // DONTKNOW? ... not realy started ... not realy failed :-)
    }

    css::uno::Reference< css::frame::XFrame > xBaseFrame(m_xOwnerFrame.get(), css::uno::UNO_QUERY);
    if (!xBaseFrame.is())
    {
        if (xListener.is())
            xListener->dispatchFinished(
                css::frame::DispatchResultEvent(xThis, css::frame::DispatchResultState::FAILURE, css::uno::Any()));
    }

    // OK ... now the internal loader seems to be useable for new requests
    // and our owner frame seems to be valid for such operations.
    // Initialize it with all new but needed properties and start the loading.
    css::uno::Reference< css::lang::XComponent > xComponent;
    try
    {
        m_aLoader.initializeLoading(aURL.Complete, lArguments, xBaseFrame, m_sTarget, m_nSearchFlags, (LoadEnv::EFeature)(LoadEnv::E_ALLOW_CONTENTHANDLER | LoadEnv::E_WORK_WITH_UI));
        m_aLoader.startLoading();
        m_aLoader.waitWhileLoading(); // wait for ever!
        xComponent = m_aLoader.getTargetComponent();

        // TODO thinking about asynchronous operations and listener support
    }
    catch(const LoadEnvException&)
        { xComponent.clear(); }

    if (xListener.is())
    {
        if (xComponent.is())
            xListener->dispatchFinished(
                css::frame::DispatchResultEvent(xThis, css::frame::DispatchResultState::SUCCESS, css::uno::Any()));
        else
            xListener->dispatchFinished(
                css::frame::DispatchResultEvent(xThis, css::frame::DispatchResultState::FAILURE, css::uno::Any()));
    }

    aReadLock.unlock();
    // <- SAFE ----------------------------------
}

/*-----------------------------------------------
    20.08.2003 09:16
-----------------------------------------------*/
void SAL_CALL LoadDispatcher::dispatch(const css::util::URL&                                  aURL      ,
                                       const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
    throw(css::uno::RuntimeException)
{
    dispatchWithNotification(aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >());
}

/*-----------------------------------------------
    20.08.2003 10:48
-----------------------------------------------*/
void SAL_CALL LoadDispatcher::addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/,
                                                const css::util::URL&                                     /*aURL*/     )
    throw(css::uno::RuntimeException)
{
}

/*-----------------------------------------------
    20.08.2003 10:49
-----------------------------------------------*/
void SAL_CALL LoadDispatcher::removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& /*xListener*/,
                                                   const css::util::URL&                                     /*aURL*/     )
    throw(css::uno::RuntimeException)
{
}

} // namespace framework
