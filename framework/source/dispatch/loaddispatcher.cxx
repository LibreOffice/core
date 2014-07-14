/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_______________________________________________
// my own includes
#include <dispatch/loaddispatcher.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

//_______________________________________________
// interface includes
#include <com/sun/star/frame/DispatchResultState.hpp>

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
    impl_dispatch( aURL, lArguments, xListener );
}

/*-----------------------------------------------
    20.08.2003 09:16
-----------------------------------------------*/
void SAL_CALL LoadDispatcher::dispatch(const css::util::URL&                                  aURL      ,
                                       const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
    throw(css::uno::RuntimeException)
{
    impl_dispatch( aURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >() );
}

/*-----------------------------------------------
    14.04.2008
-----------------------------------------------*/
css::uno::Any SAL_CALL LoadDispatcher::dispatchWithReturnValue( const css::util::URL& rURL,
                                                                const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
    throw( css::uno::RuntimeException )
{
    return impl_dispatch( rURL, lArguments, css::uno::Reference< css::frame::XDispatchResultListener >());
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

/*-----------------------------------------------
    20.08.2003 09:58
-----------------------------------------------*/
css::uno::Any LoadDispatcher::impl_dispatch( const css::util::URL& rURL,
                                             const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                             const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
{
    // Attention: May be nobody outside hold such temp. dispatch object alive (because
    // the container in which we resists isn't implemented threadsafe but updated by a timer
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
                css::frame::DispatchResultEvent(xThis, css::frame::DispatchResultState::DONTKNOW, css::uno::Any())); // DONTKNOW? ... not really started ... not really failed :-)
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
        m_aLoader.initializeLoading( rURL.Complete, lArguments, xBaseFrame, m_sTarget, m_nSearchFlags, (LoadEnv::EFeature)(LoadEnv::E_ALLOW_CONTENTHANDLER | LoadEnv::E_WORK_WITH_UI));
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

    // return the model - like loadComponentFromURL()
    css::uno::Any aRet;
    if ( xComponent.is () )
        aRet = css::uno::makeAny( xComponent );

    aReadLock.unlock();
    // <- SAFE ----------------------------------
    return aRet;
}

} // namespace framework
