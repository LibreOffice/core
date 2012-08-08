/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FILTER_CONFIG_LATEINITLISTENER_HXX_
#define __FILTER_CONFIG_LATEINITLISTENER_HXX_

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
