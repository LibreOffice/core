/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    
    if( isAfterUnloadOrPy_Finalize() )
        return;
    try
    {
        PyThreadAttach g( (PyInterpreterState*)mPyInterpreter );
        {
            Runtime runtime;

            
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
    
    if( isAfterUnloadOrPy_Finalize() )
        return;

    
    
    
    
    rtl::Reference< GCThread >(new GCThread(interpreter, object))->launch();
        
        
        
        
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
