/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_thread.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-20 14:23:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
