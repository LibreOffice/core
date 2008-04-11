/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: loaddispatchlistener.hxx,v $
 * $Revision: 1.5 $
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

#ifndef __FRAMEWORK_LOADENV_LOADDISPATCHLISTENER_HXX_
#define __FRAMEWORK_LOADENV_LOADDISPATCHLISTENER_HXX_

//_______________________________________________
// includes of own project

#include <threadhelp/threadhelpbase.hxx>
#include <threadhelp/gate.hxx>
#include <macros/xinterface.hxx>

//_______________________________________________
// includes of uno interface
#include <com/sun/star/frame/XDispatchResultListener.hpp>

/*
#include <com/sun/star/frame/DispatchResultEvent.hpp>
*/

//_______________________________________________
// includes of an other project

#ifndef _OSL_CONDITN_HXX_
#include <osl/condition.hxx>
#endif
#include <cppuhelper/weak.hxx>

//_______________________________________________
// namespace

namespace framework{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

/** @short  listen for finished dispatches, where document will be loaded.

    @descr  This listener can be bound to an URL - so its well known
            for which load request this event was triggered.
            Thats needed - but not supported by the XDispatchResultListener
            notification.
            Further a condition can be used to synchronize any outside code
            against the occurence of this event.

    @author as96863
 */
class LoadDispatchListener : public  css::frame::XDispatchResultListener // => css.lang.XEventListener
                           , private ThreadHelpBase
                           , public  ::cppu::OWeakObject
{
    //___________________________________________
    // member

    private:

        /** @short  the URL which is bound to this callback. */
        ::rtl::OUString m_sURL;

        /** @short  the original event, which was notified to this object. */
        css::frame::DispatchResultEvent m_aResult;

        /** @short  used to let the user of this instance wait, till an
                    event occures.
         */
        ::osl::Condition m_aUserWait;

    //___________________________________________
    // native interface

    public:

        //_______________________________________
        /** @short  initialize a new instance of this class. */
        LoadDispatchListener();

        //_______________________________________
        /** @short  deinitialize an instance of this class. */
        virtual ~LoadDispatchListener();

        //_______________________________________
        /** @short  bind this listenerr to a new URL.

            @param  sURL
                    the new URL bound to this instance.
         */
        void setURL(const ::rtl::OUString & sURL);

        //_______________________________________
        /** @short  let the user of this instance wait.

            @descr  If the call timed out - false is returned.
                    Otherwise it returns true.
                    Then the method getResult() has to be called,
                    to get the origianl event.

            @param  nWait_ms
                    the time for wait in [ms].
                    If its set to 0 this call is blocked till
                    an event occures!

            @return TRUE if an event occured in time - FALSE otherwhise.
        */
        sal_Bool wait(sal_Int32 nWait_ms);

        //_______________________________________
        /** @short  returns the result of this listener operation.

            @descr  If wait() (which must be called before!) returns FALSE
                    the return of getResult() is undefined!

            @return The result of the got listener notification.
         */
        css::frame::DispatchResultEvent getResult() const;

    //___________________________________________
    // uno interface

    public:

        //_______________________________________
        // css.uno.XInterface
        FWK_DECLARE_XINTERFACE

        //_______________________________________
        // css.frame.XDispatchResultListener
        virtual void SAL_CALL dispatchFinished(const css::frame::DispatchResultEvent& aEvent)
            throw(css::uno::RuntimeException);

        //_______________________________________
        // css.lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);
};

} // namespace framework

#endif // __FRAMEWORK_LOADENV_LOADDISPATCHLISTENER_HXX_
