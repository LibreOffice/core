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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_THREAD_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_THREAD_HXX

#include "sal/config.h"

#include <cstddef>
#include <new>
#include "osl/thread.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"

/// @HTML

namespace dp_gui {

/**
   A safe encapsulation of <code>osl::Thread</code>.
*/
class Thread: public salhelper::SimpleReferenceObject, private osl::Thread {
public:
    Thread();

    /**
       Launch the thread.

       <p>This function must be called at most once.</p>
    */
    void launch();

    using osl::Thread::join;

    static void * operator new(std::size_t size) throw (std::bad_alloc);

    static void operator delete(void * p) throw ();

protected:
    virtual ~Thread();

    /**
       The main function executed by the thread.

       <p>Any exceptions terminate the thread and are effectively ignored.</p>
    */
    virtual void execute() = 0;

private:
    Thread(Thread &); // not defined
    void operator =(Thread &); // not defined

    virtual void SAL_CALL run();

    virtual void SAL_CALL onTerminated();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
