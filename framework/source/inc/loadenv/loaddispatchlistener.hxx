/*************************************************************************
 *
 *  $RCSfile: loaddispatchlistener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 14:31:10 $
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

#ifndef __FRAMEWORK_LOADENV_LOADDISPATCHLISTENER_HXX_
#define __FRAMEWORK_LOADENV_LOADDISPATCHLISTENER_HXX_

//_______________________________________________
// includes of own project

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_GATE_HXX_
#include <threadhelp/gate.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

//_______________________________________________
// includes of uno interface

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHRESULTLISTENER_HPP_
#include <com/sun/star/frame/XDispatchResultListener.hpp>
#endif

/*
#ifndef _COM_SUN_STAR_FRAME_DISPATCHRESULTEVENT_HPP_
#include <com/sun/star/frame/DispatchResultEvent.hpp>
#endif
*/

//_______________________________________________
// includes of an other project

#ifndef _OSL_CONDITN_HXX_
#include <osl/condition.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

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
        DECLARE_XINTERFACE

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
