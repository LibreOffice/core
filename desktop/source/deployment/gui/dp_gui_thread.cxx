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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "sal/config.h"

#include <cstddef>
#include <new>

#include "osl/thread.hxx"
#include "salhelper/simplereferenceobject.hxx"

#include "dp_gui_thread.hxx"

using dp_gui::Thread;

Thread::Thread() {}

void Thread::launch() {
    // Assumption is that osl::Thread::create returns normally iff it causes
    // osl::Thread::run to start executing:
    acquire();
    try {
        create();
    } catch (...) {
        release();
        throw;
    }
}

void * Thread::operator new(std::size_t size)
    throw (std::bad_alloc)
{
    return SimpleReferenceObject::operator new(size);
}

void Thread::operator delete(void * p) throw () {
    SimpleReferenceObject::operator delete(p);
}

Thread::~Thread() {}

void Thread::run() {
    try {
        execute();
    } catch (...) {
        // Work around the problem that onTerminated is not called if run throws
        // an exception:
        onTerminated();
        throw;
    }
}

void Thread::onTerminated() {
    release();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
