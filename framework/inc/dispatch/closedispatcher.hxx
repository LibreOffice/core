/*************************************************************************
 *
 *  $RCSfile: closedispatcher.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:37 $
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

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

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

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
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

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _VCL_EVNTPOST_HXX
#include <vcl/evntpost.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          helper to dispatch the URLs ".uno:CloseDoc" and ".uno:CloseWin" to close a targeted frame/document
    @descr          These URLs implements a special functionality to close a document or the whole frame ...
                    and handle the state, it was the last frame or document. Then we create the default backing document
                    which can be used to open new ones using the file open dialog or some other menu entries.

    @devstatus      ready to use
    @threadsafe     yes
*//*-*************************************************************************************************************/
class CloseDispatcher : public css::lang::XTypeProvider
                      , public css::frame::XNotifyingDispatch // => XDispatch
                        // baseclasses ... order is neccessary for right initialization!
                      , private ThreadHelpBase
                      , public  ::cppu::OWeakObject
{
    //_____________________________________________
    // types

    private:

        enum EOperation
        {
            E_ESTABLISH_BACKINGMODE,
            E_CLOSE_TARGET,
            E_EXIT_APP
        };

    //_____________________________________________
    // member

    private:

        /** reference to the uno service manager */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** reference to the target frame */
        css::uno::Reference< css::frame::XFrame > m_xTarget;

        /** used for asynchronous callbacks within the main thread, to
            establish the backing mode or close the target frame. */
        ::vcl::EventPoster m_aAsyncCallback;

        /** used inside asyncronous callback to decide, which operation must be executed. */
        EOperation m_eAsyncOperation;

        /** for asynchronous operations we must hold us self alive! */
        css::uno::Reference< css::uno::XInterface > m_xSelfHold;

        /** list of registered status listener */
        ListenerHash m_lStatusListener;

        /** holded alive for internaly asynchronous operations! */
        css::uno::Reference< css::frame::XDispatchResultListener > m_xResultListener;

    //_____________________________________________
    // native interface

    public:

                 CloseDispatcher( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR   ,
                                  const css::uno::Reference< css::frame::XFrame >&              xTarget );
        virtual ~CloseDispatcher(                                                                       );

    //_____________________________________________
    // uno interface

    public:

        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        // XNotifyingDispatch
        virtual void SAL_CALL dispatchWithNotification( const css::util::URL&                                             aURL      ,
                                                        const css::uno::Sequence< css::beans::PropertyValue >&            lArguments,
                                                        const css::uno::Reference< css::frame::XDispatchResultListener >& xListener ) throw(css::uno::RuntimeException);

        // XDispatch
        virtual void SAL_CALL dispatch            ( const css::util::URL&                                     aURL      ,
                                                    const css::uno::Sequence< css::beans::PropertyValue >&    lArguments) throw(css::uno::RuntimeException);
        virtual void SAL_CALL addStatusListener   ( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& xListener ,
                                                    const css::util::URL&                                     aURL      ) throw(css::uno::RuntimeException);

    //_____________________________________________
    // internal helper

    private:

        void     impl_dispatchCloseDoc    ( const css::uno::Sequence< css::beans::PropertyValue >&                 lArguments   ,
                                            const css::uno::Reference< css::frame::XDispatchResultListener >&      xListener    );
        void     impl_dispatchCloseWin    ( const css::uno::Sequence< css::beans::PropertyValue >&                 lArguments   ,
                                            const css::uno::Reference< css::frame::XDispatchResultListener >&      xListener    );
        void     impl_dispatchCloseFrame  ( const css::uno::Sequence< css::beans::PropertyValue >&                 lArguments   ,
                                            const css::uno::Reference< css::frame::XDispatchResultListener >&      xListener    );
        sal_Bool impl_closeFrame          (       css::uno::Reference< css::frame::XFrame >&                       xFrame       ,
                                                  sal_Bool                                                         bCallSuspend );
        sal_Bool impl_exitApp             (       css::uno::Reference< css::frame::XFrame >&                       xTarget      );
        void     impl_notifyResultListener( const css::uno::Reference< css::frame::XDispatchResultListener >&      xListener    ,
                                                  sal_Int16                                                        nState       ,
                                            const css::uno::Any&                                                   aResult      );
        sal_Bool impl_establishBackingMode(                                                                                     );
        DECL_LINK( impl_asyncCallback, void* );

}; // class CloseDispatcher

} // namespace framework

#endif // #ifndef __FRAMEWORK_DISPATCH_CLOSEDISPATCHER_HXX_
