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

#ifndef INCLUDED_CANVAS_BASE_DISAMBIGUATIONHELPER_HXX
#define INCLUDED_CANVAS_BASE_DISAMBIGUATIONHELPER_HXX

#include <com/sun/star/lang/EventObject.hpp>
#include <osl/mutex.hxx>


/* Definition of the DisambiguationHelper class */

namespace canvas
{
    /** Base class, initializing its own baseclass with m_aMutex.

        This is necessary to make the CanvasBase, GraphicDeviceBase,
        etc. classes freely combinable - letting them perform this
        initialization would prohibit deriving e.g. CanvasBase from
        GraphicDeviceBase.

        On top of that, disambiguates XEventListener::disposing and
        WeakComponentImplHelper::disposing.

        Having two virtual methods with the same name, and not
        overriding them in every derived class, will hide one of
        them. Later trying to override the same method, will generate
        a new vtable slot, and lead to very hard to spot errors.
     */
    template< class Base > class DisambiguationHelper : public Base
    {
    protected:
        /** Construct DisambiguationHelper

            This method is the whole purpose of this template:
            initializing a base class with the provided m_aMutex
            member (the WeakComponentImplHelper templates need that,
            as they require the lifetime of the mutex to extend
            theirs).
         */
        DisambiguationHelper() :
            Base( m_aMutex )
        {
        }

        virtual void disposeThis()
        {}
        virtual void disposeEventSource( const css::lang::EventObject& ) throw (css::uno::RuntimeException)
        {}

        mutable ::osl::Mutex m_aMutex;

    private:
        virtual void SAL_CALL disposing() override
        { disposeThis(); }

        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException) override
        { disposeEventSource(Source); }

    };
}

#endif // INCLUDED_CANVAS_BASE_DISAMBIGUATIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
