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

#include "pyuno_impl.hxx"

#include "sal/config.h"

#include "rtl/ref.hxx"
#include "salhelper/thread.hxx"

namespace pyuno
{

bool g_destructorsOfStaticObjectsHaveBeenCalled;
class StaticDestructorGuard
{
public:
    ~StaticDestructorGuard()
    {
        g_destructorsOfStaticObjectsHaveBeenCalled = true;
    }
};
StaticDestructorGuard guard;

static bool isAfterUnloadOrPy_Finalize()
{
    return g_destructorsOfStaticObjectsHaveBeenCalled ||
        !Py_IsInitialized();
}

class GCThread: public salhelper::Thread {
public:
    GCThread( PyInterpreterState *interpreter, PyObject * object );

private:
    virtual ~GCThread() {}

    virtual void execute() override;

    PyObject *mPyObject;
    PyInterpreterState *mPyInterpreter;
};

GCThread::GCThread( PyInterpreterState *interpreter, PyObject * object ) :
    Thread( "pyunoGCThread" ), mPyObject( object ),
    mPyInterpreter( interpreter )
{}

void GCThread::execute()
{
    //  otherwise we crash here, when main has been left already
    if( isAfterUnloadOrPy_Finalize() )
        return;
    try
    {
        PyThreadAttach g( mPyInterpreter );
        {
            Runtime runtime;

            // remove the reference from the pythonobject2adapter map
            PyRef2Adapter::iterator ii =
                runtime.getImpl()->cargo->mappedObjects.find( mPyObject );
            if( ii != runtime.getImpl()->cargo->mappedObjects.end() )
            {
                runtime.getImpl()->cargo->mappedObjects.erase( ii );
            }

            Py_XDECREF( mPyObject );
        }
    }
    catch( const com::sun::star::uno::RuntimeException & e )
    {
        OString msg;
        msg = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
        fprintf( stderr, "Leaking python objects bridged to UNO for reason %s\n",msg.getStr());
    }
}

void decreaseRefCount( PyInterpreterState *interpreter, PyObject *object )
{
    //  otherwise we crash in the last after main ...
    if( isAfterUnloadOrPy_Finalize() )
        return;

    // delegate to a new thread, because there does not seem
    // to be a method, which tells, whether the global
    // interpreter lock is held or not
    // TODO: Look for a more efficient solution
    rtl::Reference< GCThread >(new GCThread(interpreter, object))->launch();
        //TODO: a protocol is missing how to join with the launched thread
        // before exit(3), to ensure the thread is no longer relying on any
        // infrastructure while that infrastructure is being shut down in
        // atexit handlers
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
