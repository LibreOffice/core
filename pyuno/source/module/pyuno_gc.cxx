/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

static bool isAfterUnloadOrPy_Finalize()
{
    return g_destructorsOfStaticObjectsHaveBeenCalled ||
        !Py_IsInitialized();
}

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
    if( isAfterUnloadOrPy_Finalize() )
        return;

    // delegate to a new thread, because there does not seem
    // to be a method, which tells, whether the global
    // interpreter lock is held or not
    // TODO: Look for a more efficient solution
    osl::Thread *t = new GCThread( interpreter, object );
    t->create();
}

}
