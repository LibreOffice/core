/*************************************************************************
 *
 *  $RCSfile: closedispatcher.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 13:19:11 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_DISPATCH_CLOSEDISPATCHER_HXX_
#define __FRAMEWORK_DISPATCH_CLOSEDISPATCHER_HXX_

//_______________________________________________
// my own includes

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XNOTIFYINGDISPATCH_HPP_
#include <com/sun/star/frame/XNotifyingDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRESULTLISTENER_HPP_
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTSTATE_HPP_
#include <com/sun/star/frame/DispatchResultState.hpp>
#endif

//_______________________________________________
// other includes

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _VCL_EVNTPOST_HXX
#include <vcl/evntpost.hxx>
#endif

//_______________________________________________
// namespace

namespace framework{

//-----------------------------------------------
/**
    @short          helper to dispatch the URLs ".uno:CloseDoc"/".uno:CloseWin"/".uno:CloseFrame"
                    to close a frame/document or the whole application implicitly in case it was the last frame

    @descr          These URLs implements a special functionality to close a document or the whole frame ...
                    and handle the state, it was the last frame or document. Then we create the
                    default backing document which can be used to open new ones using the file open dialog
                    or some other menu entries. Or we terminate the whole application in case this backing mode shouldnt
                    be used.
 */
class CloseDispatcher : public css::lang::XTypeProvider
                      , public css::frame::XNotifyingDispatch   // => XDispatch
                      , public css::frame::XStatusListener      // => XEventListener
                        // baseclasses ... order is neccessary for right initialization!
                      , private ThreadHelpBase
                      , public  ::cppu::OWeakObject
{
    //-------------------------------------------
    // types

    private:

        //---------------------------------------
        /** @short  describe, which request must be done here.
        @descr      The incoming URLs {.uno:CloseDoc/CloseWin and CloseFrame
                    can be classified so and checked later performant.}*/
        enum EOperation
        {
            E_CLOSE_DOC,
            E_CLOSE_FRAME,
            E_CLOSE_WIN
        };

    //-------------------------------------------
    // member

    private:

        //---------------------------------------
        /** @short reference to an uno service manager,
                   which can be used to create own needed
                   uno resources. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        //---------------------------------------
        /** @short  reference to the target frame, which should be
                    closed by this dispatch. */
        css::uno::Reference< css::frame::XFrame > m_xCloseFrame;

        //---------------------------------------
        /** @short  used for asynchronous callbacks within the main thread.
            @descr  Internaly we work asynchronous. Because our callis
                    are not aware, that her request can kill its own environment ... */
        ::vcl::EventPoster m_aAsyncCallback;

        //---------------------------------------
        /** @short  used inside asyncronous callback to decide,
                    which operation must be executed. */
        EOperation m_eOperation;

        //---------------------------------------
        /** @short  for asynchronous operations we must hold us self alive! */
        css::uno::Reference< css::uno::XInterface > m_xSelfHold;

        //---------------------------------------
        /** @short  list of registered status listener */
        ListenerHash m_lStatusListener;

        //---------------------------------------
        /** @short  holded alive for internaly asynchronous operations! */
        css::uno::Reference< css::frame::XDispatchResultListener > m_xResultListener;

    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        /** @short  connect a new CloseDispatcher instance to its frame.
            @descr  One CloseDispatcher instance is bound to onw frame only.
                    That makes an implementation (e.g. of listener support)
                    much more easier .-)

            @param  xSMGR
                    an un oservice manager, which is needed to create uno resource
                    internaly.

            @param  xCloseFrame
                    the frame, where we must work on.
         */
        CloseDispatcher(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR      ,
                        const css::uno::Reference< css::frame::XFrame >&              xCloseFrame);

        //---------------------------------------
        /** @short  does nothing real. */
        virtual ~CloseDispatcher();

    //-------------------------------------------
    // uno interface

    public:

        //---------------------------------------
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        //---------------------------------------
        // XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                        const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw(css::uno::RuntimeException);

        //---------------------------------------
        // XDispatch
        virtual void SAL_CALL dispatch            ( const css::util::URL&                                     aURL      ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&    lArguments) throw(css::uno::RuntimeException);
        virtual void SAL_CALL addStatusListener   ( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException);

        //---------------------------------------
        // XStatusListener
        virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& aState ) throw(css::uno::RuntimeException);

        //---------------------------------------
        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aSource ) throw(css::uno::RuntimeException);

    //-------------------------------------------
    // internal helper

    private:

        //---------------------------------------
        /** @short  a callback for asynchronous started operations.

            @descr  As already mentione, we make internaly all operations
                    asynchronous. Otherwhise our callis kill its own environment
                    during they call us ...
        */
        DECL_LINK( impl_asyncCallback, void* );

        //---------------------------------------
        /** @short  close the document view of our m_xCloseFrame.

            @descr  Thats needed to be shure, that the document cant disagree
                    later with e.g. an office termination.
                    The problem: Closing of documents can show UI. If the user
                    ignores it and open/close other documents, we cant know
                    which state the office has after closing of this frame.

            @param  bAllowSuspend
                    force calling of XController->suspend().

            @param  bCloseAllOtherViewsToo
                    if there are other top level frames, which
                    contains views to the same document then our m_xCloseFrame,
                    they are forced to be closed too.
                    We need it to implement the CLOSE_DOC semantic.

            @return [boolean]
                    TRUE if closing was successfully.
         */
        sal_Bool implts_closeView(sal_Bool bAllowSuspend         ,
                                  sal_Bool bCloseAllOtherViewsToo);

        //---------------------------------------
        /** @short  close the member m_xCloseFrame.

            @descr  This method does not look for any document
                    inside this frame. Such views must be cleared
                    before (e.g. by calling implts_closeView()!

                    Otherwhise e.g. the XController->suspend()
                    call isnt made and no UI warn the user about
                    loosing document changes. Because the
                    frame is closed ....

            @return [bool]
                    TRUE if closing was successfully.
         */
        sal_Bool implts_closeFrame();

        //---------------------------------------
        /** @short  set the special BackingComponent (now StartModule)
                    as new component of our m_xCloseFrame.

            @return [bool]
                    TRUE if operation was successfully.
         */
        sal_Bool implts_establishBackingMode();

        //---------------------------------------
        /** @short  calls XDesktop->terminate().

            @descr  No office code has to be called
                    afterwards! Because the process is dieing ...
                    The only exception is a might be registered
                    listener at this instance here.
                    Because he should know, that such things will happen :-)

            @return [bool]
                    TRUE if termination of the application was started ...
         */
        sal_Bool implts_terminateApplication();

        //---------------------------------------
        /** @short  notify a DispatchResultListener.

            @descr  We check the listener reference before we use it.
                    So this method can be called everytimes!

            @parama xListener
                    the listener, which should be notified.
                    Can be null!

            @param  nState
                    directly used as css::frame::DispatchResultState value.

            @param  aResult
                    not used yet realy ...
         */
        void implts_notifyResultListener(const css::uno::Reference< css::frame::XDispatchResultListener >& xListener,
                                               sal_Int16                                                   nState   ,
                                         const css::uno::Any&                                              aResult  );

}; // class CloseDispatcher

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_CLOSEDISPATCHER_HXX_
