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



#ifndef __FRAMEWORK_HELPER_TITLEBARUPDATE_HXX_
#define __FRAMEWORK_HELPER_TITLEBARUPDATE_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTITLE_HPP_
#include <com/sun/star/frame/XTitle.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XTITLECHANGELISTENER_HPP_
#include <com/sun/star/frame/XTitleChangeListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <unotools/moduleoptions.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          helps our frame on setting title/icon on the titlebar (including updates)

    @devstatus      draft
    @threadsafe     yes
*//*-*************************************************************************************************************/
class TitleBarUpdate : // interfaces
                       public css::lang::XTypeProvider
                     , public css::lang::XInitialization
                     , public css::frame::XTitleChangeListener // => XEventListener
                     , public css::frame::XFrameActionListener // => XEventListener
                       // baseclasses (order necessary for right initialization!)
                     , private ThreadHelpBase
                     , public  ::cppu::OWeakObject
{
    //________________________________
    // structs, types

    private:

        struct TModuleInfo
        {
            /// internal id of this module
            ::rtl::OUString sID;
            /// localized name for this module
            ::rtl::OUString sUIName;
            /// configured icon for this module
            ::sal_Int32 nIcon;
        };

    //________________________________
    // member

    private:

        /// may we need an uno service manager to create own services
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /// reference to the frame which was created by the office himself
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

    //________________________________
    // interface

    public:

        //____________________________
        // ctor/dtor
                 TitleBarUpdate(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~TitleBarUpdate(                                                                   );

        //____________________________
        // XInterface, XTypeProvider
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        //____________________________
        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        //____________________________
        // XFrameActionListener
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent)
            throw(css::uno::RuntimeException);

        //____________________________
        // XTitleChangeListener
        virtual void SAL_CALL titleChanged(const css::frame::TitleChangedEvent& aEvent)
            throw (css::uno::RuntimeException);

        //____________________________
        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

    //________________________________
    // helper

    private:

        //____________________________
        /** @short  identify the application module, which  is used behind the component
                    of our frame.

            @param  xFrame
                    contains the component, wich must be identified.

            @param  rInfo
                    describe the module in its details.
                    Is set only if return value is true.

            @return [sal_Bool]
                    sal_True in casee module could be identified and all needed values could be read.
                    sal_False otherwise.
         */
        ::sal_Bool implst_getModuleInfo(const css::uno::Reference< css::frame::XFrame >& xFrame,
                                              TModuleInfo&                               rInfo );

        //____________________________
        /** @short  set a new icon and title on the title bar of our connected frame window.

            @descr  It does not check if an update is really needed. That has to be done outside.
                    It retrieves all needed informations and update the title bar - nothing less -
                    nothing more.
         */
        void impl_forceUpdate();

        //____________________________
        /** @short  identify the current component (inside the connected frame)
                    and set the right module icon on the title bar.

            @param  xFrame
                    the frame which contains the component and where the icon must be set
                    on the window title bar.
         */
        void impl_updateIcon(const css::uno::Reference< css::frame::XFrame >& xFrame);

        //____________________________
        /** @short  gets the current title from the frame and set it on the window.

            @param  xFrame
                    the frame which contains the component and where the title must be set
                    on the window title bar.
         */
        void impl_updateTitle(const css::uno::Reference< css::frame::XFrame >& xFrame);

}; // class TitleBarUpdate

} // namespace framework

#endif // #ifndef __FRAMEWORK_HELPER_TITLEBARUPDATE_HXX_
