/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

    static void * operator new(std::size_t size)
    { return SimpleReferenceObject::operator new(size); }

    static void operator delete(void * pointer)
    { SimpleReferenceObject::operator delete(pointer); }

protected:
    virtual ~Thread() SAL_OVERRIDE;

    /**
       The main function executed by the thread.

       Any uncaught exceptions lead to std::terminate.
    */
    virtual void execute() = 0;

private:
    virtual void SAL_CALL run() SAL_OVERRIDE;

    virtual void SAL_CALL onTerminated() SAL_OVERRIDE;

    char const * name_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
