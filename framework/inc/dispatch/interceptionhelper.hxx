/*************************************************************************
 *
 *  $RCSfile: interceptionhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 17:12:43 $
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

#ifndef __FRAMEWORK_HELPER_INTERCEPTIONHELPER_HXX_
#define __FRAMEWORK_HELPER_INTERCEPTIONHELPER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_SERVICES_FRAME_HXX_
#include <services/frame.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_WILDCARD_HXX_
#include <classes/wildcard.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
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

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTOR_HPP_
#include <com/sun/star/frame/XDispatchProviderInterceptor.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef __SGI_STL_DEQUE
#include <deque>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________
// definitions
//_________________________________________________________

/** @short      implements a helper to support interception with additional functionality.

    @descr      This helper implements the complete XDispatchProviderInterception interface with
                master/slave functionality AND using of optional features like registration of URL pattern!

    @attention  Don't use this class as direct member - use it dynamicly. Do not derive from this class.
                We hold a weakreference to ouer owner not to ouer superclass.
 */
class InterceptionHelper : public  css::frame::XDispatchProvider
                         , public  css::frame::XDispatchProviderInterception
                         , public  css::lang::XEventListener
                           // order of base classes is important for right initialization of mutex member!
                         , private ThreadHelpBase
                         , public  ::cppu::OWeakObject
{
    //_____________________________________________________
    // structs, helper

    /** @short bind an interceptor component to it's URL pattern registration. */
    struct InterceptorInfo
    {
        /** @short reference to the interceptor component. */
        css::uno::Reference< css::frame::XDispatchProvider > xInterceptor;

        /** @short it's registration for URL patterns.

            @descr If the interceptor component does not support the optional interface
                   XInterceptorInfo, it will be registered for one pattern "*" by default.
                   That would make it possible to handle it in the same manner then real
                   registered interceptor objects and we must not implement any special code. */
        css::uno::Sequence< ::rtl::OUString > lURLPattern;
    };

    //_____________________________________________________

    /** @short implements a list of items of type InterceptorInfo, and provides some special
               functions on it.

        @descr Because interceptor objects can be registered for URL patterns,
               it supports a wildcard search on all list items.
     */
    class InterceptorList : public ::std::deque< InterceptorInfo >
    {
        public:

            //_____________________________________________

            /** @short search for an interceptor inside this list using it's reference.

                @param xInterceptor
                        points to the interceptor object, which should be located inside this list.

                @return An iterator object, which points directly to the located item inside this list.
                        In case no interceptor could be found, it points to the end of this list!
              */
            iterator findByReference(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
            {
                css::uno::Reference< css::frame::XDispatchProviderInterceptor > xProviderInterface(xInterceptor, css::uno::UNO_QUERY);
                iterator pIt;
                for (pIt=begin(); pIt!=end(); ++pIt)
                {
                    if (pIt->xInterceptor == xProviderInterface)
                        return pIt;
                }
                return end();
            }

            //_____________________________________________

            /** @short search for an interceptor inside this list using it's reference.

                @param xInterceptor
                        points to the interceptor object, which should be located inside this list.

                @return An iterator object, which points directly to the located item inside this list.
                        In case no interceptor could be found, it points to the end of this list!
              */
            iterator findByPattern(const ::rtl::OUString& sURL)
            {
                iterator pIt;
                for (pIt=begin(); pIt!=end(); ++pIt)
                {
                    sal_Int32              c        = pIt->lURLPattern.getLength();
                    const ::rtl::OUString* pPattern = pIt->lURLPattern.getConstArray();
                    for (sal_Int32 i=0; i<c; ++i)
                    {
                        if (Wildcard::match(sURL, pPattern[i]))
                            return pIt;
                    }
                }
                return end();
            }
    };

    //_____________________________________________________
    // member

    private:

        /** @short reference to the frame, which uses this instance to implement it's own interception.

            @descr We hold a weak reference only, to make disposing operations easy. */
        css::uno::WeakReference< css::frame::XFrame > m_xOwnerWeak;

        /** @short this interception helper implements the top level master of an interceptor list ...
                   but this member is the lowest possible slave! */
        css::uno::Reference< css::frame::XDispatchProvider > m_xSlave;

        /** @short contains all registered interceptor objects. */
        InterceptorList m_lInterceptionRegs;

        /** @short it regulates, which interceptor is used first.
                   The last or the first registered one. */
        static sal_Bool m_bPreferrFirstInterceptor;

    //_____________________________________________________
    // native interface

    public:

        //_________________________________________________

        /** @short creates a new interception helper instance.

            @param xOwner
                    points to the frame, which use this instances to support it's own interception interfaces.

            @param xSlave
                    an outside creates dispatch provider, which has to be used here as lowest slave "interceptor".
         */
        InterceptionHelper(const css::uno::Reference< css::frame::XFrame >&            xOwner,
                           const css::uno::Reference< css::frame::XDispatchProvider >& xSlave);

    protected:

        //_________________________________________________

        /** @short standard destructor.

            @descr This method destruct an instance of this class and clear some member.
                   This method is protected, because its not allowed to use this class as a direct member!
                   You MUST use a dynamical instance (pointer). That's the reason for a protected dtor.
         */
        virtual ~InterceptionHelper();

    //_____________________________________________________
    // uno interface

    public:

        DECLARE_XINTERFACE

        //_________________________________________________
        // XDispatchProvider

        /** @short  query for a dispatch, which implements the requested feature.

            @descr  We search inside our list of interception registrations, to locate
                    any interested interceptor. In case no interceptor exists or nobody is
                    interested on this URL our lowest slave will be used.

            @param  aURL
                        describes the requested dispatch functionality.

            @param  sTargetFrameName
                        the name of the target frame or a special name like "_blank", "_top" ...
                        Won't be used here ... but may by one of our registered interceptor objects
                        or our slave.

            @param  nSearchFlags
                        optional search parameter for targeting, if sTargetFrameName isn't a special one.

            @return A valid dispatch object, if any interceptor or at least our slave is interested on the given URL;
                    or NULL otherwhise.
         */
        virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch(const css::util::URL&  aURL            ,
                                                                                    const ::rtl::OUString& sTargetFrameName,
                                                                                          sal_Int32        nSearchFlags    )
            throw(css::uno::RuntimeException);

        //_________________________________________________
        // XDispatchProvider

        /** @short implements an optimized queryDispatch() for remote.

            @descr It capsulate more then one queryDispatch() requests and return a lits of dispatch objects
                   as result. Because both lists (in and out) coreespond together, it's not allowed to
                   pack it - means supress NULL references!

            @param lDescriptor
                    a list of queryDispatch() arguments.

            @return A list of dispatch objects.
         */
        virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > SAL_CALL queryDispatches(const css::uno::Sequence< css::frame::DispatchDescriptor >& lDescriptor)
            throw(css::uno::RuntimeException);

        //_________________________________________________
        // XDispatchProviderInterception

        /** @short      register an interceptor.

            @descr      Somebody can register himself to intercept all or some special dispatches.
                        It's depend from his supported interfaces. If he implement XInterceptorInfo
                        he his called for some special URLs only - otherwise we call it for every request!

            @attention  We don't check for double registrations here!

            @param      xInterceptor
                        reference to interceptor, which wish to be registered here.

            @throw      A RuntimeException if the given reference is NULL!
         */
        virtual void SAL_CALL registerDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor)
            throw(css::uno::RuntimeException);

        //_________________________________________________
        // XDispatchProviderInterception

        /** @short      release an interceptor.

            @descr      Remove the registered interceptor from our internal list
                        and delete all special informations about it.

            @param      xInterceptor
                        reference to the interceptor, which wish to be deregistered.

            @throw      A RuntimeException if the given reference is NULL!
         */
        virtual void SAL_CALL releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor ) throw( css::uno::RuntimeException );

        //_________________________________________________
        // XEventListener

        /** @short      Is called from our owner frame, in case he will be disposed.

            @descr      We have to relaease all references to him then.
                        Normaly we will die by ref count too ...
         */
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

}; // class InterceptionHelper

} // namespace framework

#endif // #ifndef __FRAMEWORK_HELPER_INTERCEPTIONHELPER_HXX_
