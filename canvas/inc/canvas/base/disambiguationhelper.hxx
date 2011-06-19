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

#ifndef INCLUDED_CANVAS_DISAMBIGUATIONHELPER_HXX
#define INCLUDED_CANVAS_DISAMBIGUATIONHELPER_HXX

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
    public:
        typedef Base BaseType;

        /** Construct DisambiguationHelper

            This method is the whole purpose of this template:
            initializing a base class with the provided m_aMutex
            member (the WeakComponentImplHelper templates need that,
            as they require the lifetime of the mutex to extend
            theirs).
         */
        DisambiguationHelper() :
            BaseType( m_aMutex )
        {
        }

        virtual void SAL_CALL disposing()
        { disposeThis(); }

        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException)
        { disposeEventSource(Source); }

        virtual void disposeThis()
        {}
        virtual void disposeEventSource( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException)
        {}

protected:
        mutable ::osl::Mutex m_aMutex;
    };
}

#endif /* INCLUDED_CANVAS_DISAMBIGUATIONHELPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
