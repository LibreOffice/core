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



#ifndef __FRAMEWORK_DISPATCH_LOADDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_LOADDISPATCHER_HXX_

//_______________________________________________
// my own includes

#include <loadenv/loadenv.hxx>

//_______________________________________________
// interface includes
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#include <com/sun/star/frame/XSynchronousDispatch.hpp>

//_______________________________________________
// other includes

#include <cppuhelper/implbase2.hxx>

//_______________________________________________
// namespace

namespace framework{

namespace css = ::com::sun::star;

//_______________________________________________
// exported const

//_______________________________________________
// exported definitions

/** @short      implements a dispatch object which can be used to load
                non-visible components (by using the mechanism of ContentHandler)
                or visible-components (by using the mechanism of FrameLoader).

    @author     as96863
 */
class LoadDispatcher : private ThreadHelpBase
                     , public  ::cppu::WeakImplHelper2< css::frame::XNotifyingDispatch,          // => XDispatch => XInterface
                                                        css::frame::XSynchronousDispatch >
{
    //___________________________________________
    // member

    private:

        /** @short  can be used to create own needed services on demand. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** @short  TODO document me */
        css::uno::WeakReference< css::frame::XFrame > m_xOwnerFrame;

        /** @short  TODO document me */
        ::rtl::OUString m_sTarget;

        /** @short  TODO document me */
        sal_Int32 m_nSearchFlags;

        /** @short  TODO document me */
        LoadEnv m_aLoader;

    //___________________________________________
    // native interface

    public:

        /** @short  creates a new instance and initialize it with all necessary parameters.

            @descr  Every instance of such LoadDispatcher can be used for the specified context only.
                    That means: It can be used to load any further requested content into tzhe here(!)
                    specified target frame.

            @param  xSMGR
                    will be used to create own needed services on demand.

            @param  xOwnerFrame
                    used as startpoit to locate the right target frame.

            @param  sTargetName
                    the name or the target frame for loading or a special qualifier
                    which define such target.

            @param  nSearchFlags
                    used in case sTargetFrame isn't a special one.
         */
        LoadDispatcher(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR       ,
                       const css::uno::Reference< css::frame::XFrame >&              xOwnerFrame ,
                       const ::rtl::OUString                                         sTargetName ,
                             sal_Int32                                               nSearchFlags);

        //_______________________________________

        /** @short  used to free internal resources.
         */
        virtual ~LoadDispatcher();

    //___________________________________________
    // uno interface

    public:

        // XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification(const css::util::URL&                                             aURL      ,
                                                       const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                       const css::uno::Reference< css::frame::XDispatchResultListener >& xListener )
            throw(css::uno::RuntimeException);

        // XDispatch
        virtual void SAL_CALL dispatch(const css::util::URL&                                  aURL      ,
                                       const css::uno::Sequence< css::beans::PropertyValue >& lArguments)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                   const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);

        // XSynchronousDispatch
        virtual css::uno::Any SAL_CALL dispatchWithReturnValue( const css::util::URL&                                  aURL      ,
                                                                const css::uno::Sequence< css::beans::PropertyValue >& lArguments )
            throw( css::uno::RuntimeException );

    private:
        css::uno::Any impl_dispatch( const css::util::URL& rURL,
                                     const css::uno::Sequence< css::beans::PropertyValue >& lArguments,
                                     const css::uno::Reference< css::frame::XDispatchResultListener >& xListener );
}; // class LoadDispatcher

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_LOADDISPATCHER_HXX_
