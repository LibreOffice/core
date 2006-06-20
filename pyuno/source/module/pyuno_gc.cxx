/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pyuno_gc.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:03:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
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
