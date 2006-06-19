/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helpagentdispatcher.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:50:40 $
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

#ifndef __FRAMEWORK_DISPATCH_HELPAGENTDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_HELPAGENTDISPATCHER_HXX_

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOWLISTENER_HPP_
#include <com/sun/star/awt/XWindowListener.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _SVTOOLS_HELPAGENTWIDNOW_HXX_
#include <svtools/helpagentwindow.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

#ifndef _VCL_EVNTPOST_HXX
#include <vcl/evntpost.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

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
                    TRUE in case the member m_xAgentWindow is a valid reference;
                    FALSE otherwise.
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

