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

    virtual void execute();

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
        PyThreadAttach g( (PyInterpreterState*)mPyInterpreter );
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
        rtl::OString msg;
        msg = rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
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
