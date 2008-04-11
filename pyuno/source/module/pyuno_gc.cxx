/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pyuno_gc.cxx,v $
 * $Revision: 1.6 $
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
#include <pyuno_impl.hxx>
#include <osl/thread.hxx>
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

class GCThread : public ::osl::Thread
{
    PyObject *mPyObject;
    PyInterpreterState *mPyInterpreter;
    GCThread( const GCThread & ); // not implemented
    GCThread &operator =( const GCThread & ); // not implemented

public:
    GCThread( PyInterpreterState *interpreter, PyObject * object );
    virtual void SAL_CALL run();
    virtual void SAL_CALL onTerminated();
};


GCThread::GCThread( PyInterpreterState *interpreter, PyObject * object ) :
    mPyObject( object ), mPyInterpreter( interpreter )
{}

void GCThread::run()
{
    //  otherwise we crash here, when main has been left already
    if( g_destructorsOfStaticObjectsHaveBeenCalled )
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
    catch( com::sun::star::uno::RuntimeException & e )
    {
        rtl::OString msg;
        msg = rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
        fprintf( stderr, "Leaking python objects bridged to UNO for reason %s\n",msg.getStr());
    }
}


void GCThread::onTerminated()
{
    delete this;
}

void decreaseRefCount( PyInterpreterState *interpreter, PyObject *object )
{
    //  otherwise we crash in the last after main ...
    if( g_destructorsOfStaticObjectsHaveBeenCalled )
        return;

    // delegate to a new thread, because there does not seem
    // to be a method, which tells, whether the global
    // interpreter lock is held or not
    // TODO: Look for a more efficient solution
    osl::Thread *t = new GCThread( interpreter, object );
    t->create();
}

}
