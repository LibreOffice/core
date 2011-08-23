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

#ifndef __FILTER_CONFIG_LATEINITTHREAD_HXX_
#define __FILTER_CONFIG_LATEINITTHREAD_HXX_

//_______________________________________________
// includes

#include "filtercache.hxx"
#include <salhelper/singletonref.hxx>
#include <osl/thread.hxx>

//_______________________________________________
// namespace

namespace filter{
    namespace config{

//_______________________________________________
// definitions

//_______________________________________________

/** @short      implements a thread, which will update the
                global filter cache of an office, after its
                startup was finished.

    @descr      Its started by a LateInitListener instance ...

    @see        LateInitListener

    @attention  The filter cache will be blocked during this thrad runs!
 */
class LateInitThread : public ::osl::Thread
{
    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        // ctor/dtor

        /** @short  initialize new instance of this class.
         */
        LateInitThread();

        //---------------------------------------

        /** @short  standard dtor.
         */
        virtual ~LateInitThread();

        //---------------------------------------

        /** @short  thread function.
         */
        virtual void SAL_CALL run();
};

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_LATEINITTHREAD_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
