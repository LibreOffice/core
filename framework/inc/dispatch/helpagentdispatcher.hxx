/*************************************************************************
 *
 *  $RCSfile: helpagentdispatcher.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_DISPATCH_HELPAGENTDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_HELPAGENTDISPATCHER_HXX_

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
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

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef _FRAMEWORK_HELPER_TIMERHELPER_HXX_
#include <helper/timerhelper.hxx>
#endif

#ifndef _SVTOOLS_HELPAGENTWIDNOW_HXX_
#include <svtools/helpagentwindow.hxx>
#endif

class Window;

//........................................................................
namespace framework
{
//........................................................................

    //====================================================================
    //= OHelpAgentDispatcher
    //====================================================================
    typedef ::cppu::WeakImplHelper2 <   ::com::sun::star::frame::XDispatch
                                    ,   ::com::sun::star::awt::XWindowListener
                                    >   OHelpAgent_Base;

    class HelpAgentDispatcher
            :private ThreadHelpBase
            ,public OHelpAgent_Base
            ,public ITimerListener
            ,public ::svt::IHelpAgentCallback
    {
    protected:
        ::rtl::OUString     m_sCurrentURL;

        Window*             m_pContainerWindow;     // parent of the agent window.
        ::svt::HelpAgentWindow*
                            m_pAgentWindow;         // the agent window itself

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                            m_xParentFrame;         // weak reference to the frame we're responsible for

        ::vos::ORef< OTimerHelper >
                            m_xAutoCloseTimer;

    public:
        HelpAgentDispatcher( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxParentFrame );

    protected:
        ~HelpAgentDispatcher();

        // ::com::sun::star::frame::XDispatch
        virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& _rURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rArgs ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxListener, const ::com::sun::star::util::URL& _rURL ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeStatusListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& _rxListener, const ::com::sun::star::util::URL& _rURL ) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::awt::XWindowListener
        virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& _rSource ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        void    implConstruct( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _rxContainer );

        /// switches the agent to a new URL
        void        switchURL(const ::com::sun::star::util::URL& _rURL);

        /// ensures that the agent's window exists and is visible
        sal_Bool    ensureAgentWindow();
        /// check if the agent window exists
        sal_Bool    haveAgentWindow() const { return (NULL != m_pAgentWindow); }
        /** destroyes the agent window
            <p>This method has to be called with m_aMutex aqcuired _exactly_ once!</p>
        */
        void        closeAgentWindow();

        /** positions the help agent's window in an optimal position
            <p>At the moment, this is a lie. The agent's window will always be placed in the
            lower right corner of the parent window.</p>
        */
        void        positionAgentWindow();

        /// stops the timer which triggers the auto close event
        void        stopAutoCloseTimer();
        /// starts the timer which triggers the auto close event
        void        startAutoCloseTimer();

        /** ensures that m_pContainerWindow is set
        */
        sal_Bool    ensureContainerWindow();

        /** checks whether or not the given is on the ignore list
        */
        sal_Bool    approveURLRequest(const ::com::sun::star::util::URL& _rURL);

        /** mark the given URL as "ignored once more"
        */
        void        markURLIgnored( const ::rtl::OUString& _rURL );

        /// to be called when the agent window has been closed by an external instance
        void        agentClosedExternally();

    protected:
        // ITimerListener overridables
        virtual void    timerExpired();

        // IHelpAgentCallback overridables
        virtual void    helpRequested();
        virtual void    closeAgent();
    };

//........................................................................
}   // namespace framework
//........................................................................

#endif // _FRAMEWORK_DISPATCH_HELPAGENTDISPATCHER_HXX_

