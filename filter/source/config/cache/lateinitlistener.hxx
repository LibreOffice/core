/*************************************************************************
 *
 *  $RCSfile: lateinitlistener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-29 13:42:34 $
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

#ifndef __FILTER_CONFIG_LATEINITLISTENER_HXX_
#define __FILTER_CONFIG_LATEINITLISTENER_HXX_

//_______________________________________________
// includes

#ifndef __FILTER_CONFIG_CACHEITEM_HXX_
#include "cacheitem.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTLISTENER_HPP_
#include <com/sun/star/document/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//_______________________________________________
// namespace

namespace filter{
    namespace config{

//_______________________________________________
// definitions

//_______________________________________________

/** @short      implements a listener, which will update the
                global filter cache of an office, after zje office
                startup was finished.

    @descr      To perform startup of an office, the filter cache starts
                with a minimum set of properties only. After the first document
                was loaded successfully a thread will be started to fill the
                cache with all other proeprties, so it can work with the whole
                filter configuration.
 */
class LateInitListener : public BaseLock // must be the first one to guarantee right initialized mutex member!
                       , public ::cppu::WeakImplHelper1< css::document::XEventListener >
{
    //-------------------------------------------
    // member

    private:

        /** @short  reference to an uno service manager, which can be used
                    to create own needed services. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** @short  reference to the global event broadcaster, which is usde to find
                    out, when the first office document was opened successfully. */
        css::uno::Reference< css::document::XEventBroadcaster > m_xBroadcaster;

    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        // ctor/dtor

        /** @short  initialize new instance of this class.

            @descr  It set a reference to the global filter cache singleton,
                    which should be updated here. Further it starts listening
                    on the global event broadcaster to get the information, when
                    loading of the first document was finished.

            @param  xSMGR
                    reference to a service manager, which can be used to create
                    own needed uno services.
         */
        LateInitListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------

        /** @short  standard dtor.
         */
        virtual ~LateInitListener();

    //-------------------------------------------
    // uno interface

    public:

        //---------------------------------------
        // document.XEventListener

        virtual void SAL_CALL notifyEvent(const css::document::EventObject& aEvent)
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);
};

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_LATEINITLISTENER_HXX_
