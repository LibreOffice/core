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

#ifndef __FRAMEWORK_DISPATCH_HELPAGENTDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_HELPAGENTDISPATCHER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <svtools/helpagentwindow.hxx>
#include <vcl/timer.hxx>
#include <vcl/evntpost.hxx>
#include <cppuhelper/weak.hxx>

//........................................................................
namespace framework
{

// define css alias ... and undefine it at the end of this file !!!
#ifdef css
    #error "I tried to use css as namespace define inside non exported header ... but it was already defined by somwhere else. .-)"
#else
    #define css ::com::sun::star
#endif

//........................................................................

class HelpAgentDispatcher : public  css::lang::XTypeProvider
                          , public  css::frame::XDispatch
                          , public  css::awt::XWindowListener // => css::lang::XEventListener
                          , public  ::svt::IHelpAgentCallback
                          , private ThreadHelpBase
                          , public  ::cppu::OWeakObject
{
    private:

        //---------------------------------------
        /// @short  represent the current active help URL, which must be used to show the right help page
        ::rtl::OUString m_sCurrentURL;

        //---------------------------------------
        /// @short  parent of the agent window.
        css::uno::Reference< css::awt::XWindow > m_xContainerWindow;

        //---------------------------------------
        /// @short  the agent window itself (implemented in svtools)
        css::uno::Reference< css::awt::XWindow > m_xAgentWindow;

        //---------------------------------------
        /// @short  the timer for showing the agent window
        Timer m_aTimer;

        //---------------------------------------
        /** @short  hold this dispatcher alive till the timer was killed or expired!
            @descr  Because the vcl timer knows us by using a pointer ... and our instance is used
                    ref counted normaly it can happen that our reference goes down to 0 ... and the timer
                    runs into some trouble. So we hold us self alive till the timer could be stopped or expired.
        */
        css::uno::Reference< css::uno::XInterface > m_xSelfHold;

    public:

        HelpAgentDispatcher(const css::uno::Reference< css::frame::XFrame >& xParentFrame);

        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        // css::frame::XDispatch
        virtual void SAL_CALL dispatch(const css::util::URL&                                  sURL ,
                                       const css::uno::Sequence< css::beans::PropertyValue >& lArgs)
            throw(css::uno::RuntimeException);
        virtual void SAL_CALL addStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);
        virtual void SAL_CALL removeStatusListener(const css::uno::Reference< css::frame::XStatusListener >& xListener,
                                                   const css::util::URL&                                     aURL     )
            throw(css::uno::RuntimeException);

        // css::awt::XWindowListener
        virtual void SAL_CALL windowResized(const css::awt::WindowEvent& aSource)
            throw(css::uno::RuntimeException);
        virtual void SAL_CALL windowMoved(const css::awt::WindowEvent& aSource)
            throw(css::uno::RuntimeException);
        virtual void SAL_CALL windowShown(const css::lang::EventObject& aSource)
            throw(css::uno::RuntimeException);
        virtual void SAL_CALL windowHidden(const css::lang::EventObject& aSource)
            throw(css::uno::RuntimeException);

        // css::lang::XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aSource)
            throw(css::uno::RuntimeException);

    protected:

        ~HelpAgentDispatcher();

    protected:

        /// IHelpAgentCallback overridables
        virtual void helpRequested();
        virtual void closeAgent();

    private:

        //---------------------------------------
        /** @short  mark the current set URL as "accepted by user" and show the right help window
         */
        void implts_acceptCurrentURL();

        //---------------------------------------
        /** @short  mark the current set URL as "ignored by user"
         */
        void implts_ignoreCurrentURL();

        //---------------------------------------
        /** @short  ensures that the agent's window exists
            @descr  We create the agent window on demand. But afterwards we hold it alive till
                    this helpagent dispatcher dies. The agent window will be made visible/hidden
                    in case a new dispatch occures or in case the timer expired.

            @return [sal_Bool]
                    sal_True in case the member m_xAgentWindow is a valid reference;
                    sal_False otherwise.
        */
        css::uno::Reference< css::awt::XWindow > implts_ensureAgentWindow();

        //---------------------------------------
        /** @short  show the agent window.
            @descr  If the agent window does not exists, it will be created on demand.
                    (see implts_ensureAgentWindow). Further it's checked if the parent container
                    window is currently visible or not. Only if its visible the agent window will
                    be shown too.
         */
        void implts_showAgentWindow();

        //---------------------------------------
        /** @short  hide the agent window.
         */
        void implts_hideAgentWindow();

        //---------------------------------------
        /** @short  set the new position and size of the agent window.
            @descr  If the agent window does not exists, it will be created on demand.
                    (see implts_ensureAgentWindow).
                    If the agent window exists, its position and size will be calculated
                    and set.
         */
        void implts_positionAgentWindow();

        //---------------------------------------
        /** @short  starts the timer for showing the agent window.
            @descr  The timer wont be started twice ... this method checks the current running state .-)
         */
        void implts_startTimer();

        //---------------------------------------
        /** @short  stop the timer.
            @descr  The timer wont be stopped twice ... this method checks the current running state .-)
                    Further this method marks the current help URL (m_xCurrentURL) as "ignorable".
                    Cause the user ignored it !
         */
        void implts_stopTimer();

        //---------------------------------------
        /** @short  callback of our internal timer.
         */
        DECL_LINK(implts_timerExpired, void*);
};

#undef css

//........................................................................
}   // namespace framework
//........................................................................

#endif // _FRAMEWORK_DISPATCH_HELPAGENTDISPATCHER_HXX_

