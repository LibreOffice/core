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

#ifndef INCLUDED_CANVAS_BASE_BASEMUTEXHELPER_HXX
#define INCLUDED_CANVAS_BASE_BASEMUTEXHELPER_HXX

#include <sal/config.h>

#include <osl/mutex.hxx>

/* Definition of the BaseMutexHelper class */

namespace canvas
{
    /** Base class, initializing its own baseclass with m_aMutex.

        This is necessary to make the CanvasBase, GraphicDeviceBase,
        etc. classes freely combinable - letting them perform this
        initialization would prohibit deriving e.g. CanvasBase from
        GraphicDeviceBase.
     */
    template< class Base > class BaseMutexHelper : public Base
    {
    protected:
        /** Construct BaseMutexHelper

            This method is the whole purpose of this template:
            initializing a base class with the provided m_aMutex
            member (the WeakComponentImplHelper templates need that,
            as they require the lifetime of the mutex to extend
            theirs).
         */
        BaseMutexHelper() :
            Base( m_aMutex )
        {
        }

        virtual void disposeThis() {}

        mutable osl::Mutex m_aMutex;

    private:
        virtual void SAL_CALL disposing() override
        { disposeThis(); }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
