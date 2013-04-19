/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
 *   (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef INCLUDED_SALHELPER_THREAD_HXX
#define INCLUDED_SALHELPER_THREAD_HXX

#include "sal/config.h"

#include <cstddef>

#include "osl/thread.hxx"
#include "sal/types.h"
#include "salhelper/salhelperdllapi.h"
#include "salhelper/simplereferenceobject.hxx"

namespace salhelper {

/**
   A safe encapsulation of ::osl::Thread.

   @since LibreOffice 3.6
*/
class SALHELPER_DLLPUBLIC Thread:
    public salhelper::SimpleReferenceObject, private osl::Thread
{
public:
    /**
       @param name the thread name, see ::osl_setThreadName; must be a non-null
       null terminated string
     */
    Thread(char const * name);

    /**
       Launch the thread.

       This function must be called at most once.

       Each call of this function should eventually be followed by a call to
       ::osl::Thread::join before exit(3), to ensure the thread is no longer
       relying on any infrastructure while that infrastructure is being shut
       down in atexit handlers.
    */
    void launch();

    using osl::Thread::getIdentifier;
    using osl::Thread::join;
    using osl::Thread::schedule;
    using osl::Thread::terminate;

    // While the below static member functions should arguably always be called
    // with qualified (osl::Thread) names, compilers would still complain that
    // they are inaccessible from within derivations of salhelper::Thread (an
    // alternative would be to force such derivations to use global names,
    // prefixed with ::osl::Thread):
    using osl::Thread::getCurrentIdentifier;
    using osl::Thread::wait;
    using osl::Thread::yield;

    static inline void * operator new(std::size_t size)
    { return SimpleReferenceObject::operator new(size); }

    static inline void operator delete(void * pointer)
    { SimpleReferenceObject::operator delete(pointer); }

protected:
    virtual ~Thread();

    /**
       The main function executed by the thread.

       Any uncaught exceptions lead to std::terminate.
    */
    virtual void execute() = 0;

private:
    virtual void SAL_CALL run();

    virtual void SAL_CALL onTerminated();

    char const * name_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
