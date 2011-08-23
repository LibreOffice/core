/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef __FILTER_CONFIG_LATEINITLISTENER_HXX_
#define __FILTER_CONFIG_LATEINITLISTENER_HXX_

//_______________________________________________
// includes

#include "cacheitem.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <cppuhelper/implbase1.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
