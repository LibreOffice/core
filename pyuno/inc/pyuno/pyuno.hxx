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

#ifndef INCLUDED_PYUNO_INC_PYUNO_PYUNO_HXX
#define INCLUDED_PYUNO_INC_PYUNO_PYUNO_HXX

#ifndef Py_PYTHON_H
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <Python.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#endif // #ifdef Py_PYTHON_H
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

/**
   External interface of the Python UNO bridge.

   This is a C++ interface, because the core UNO components
   invocation and proxyfactory are used to implement the bridge.

   This interface is somewhat private and my change in future.

   A scripting framework implementation may use this interface
   to do the necessary conversions.
*/

#if defined LO_DLLIMPLEMENTATION_PYUNO
#define LO_DLLPUBLIC_PYUNO SAL_DLLPUBLIC_EXPORT
#else
#define LO_DLLPUBLIC_PYUNO SAL_DLLPUBLIC_IMPORT
#endif

/** function called by the python runtime to initialize the
    pyuno module.

    preconditions: python has been initialized before and
                   the global interpreter lock is held
*/

extern "C" LO_DLLPUBLIC_PYUNO
#if PY_MAJOR_VERSION >= 3
    PyObject* SAL_CALL PyInit_pyuno();
#else
   void SAL_CALL initpyuno();
#endif

namespace pyuno
{

enum __NotNull
{
    /** definition of a no acquire enum for ctors
     */
    NOT_NULL
};

/** Helper class for keeping references to python objects.
    BEWARE: Look up every python function you use to check
    whether you get an acquired or not acquired object pointer
    (python terminus for a not acquired object pointer
    is 'borrowed reference'). Use in the acquired pointer cases the
    PyRef( pointer, SAL_NO_ACQUIRE) ctor.

    precondition: python has been initialized before and
    the global interpreter lock is held

*/
class PyRef
{
    PyObject *m;
public:
    PyRef () : m(nullptr) {}
    PyRef( PyObject * p ) : m( p ) { Py_XINCREF( m ); }

    PyRef( PyObject * p, __sal_NoAcquire ) : m( p ) {}

    PyRef( PyObject * p, __sal_NoAcquire, __NotNull ) : m( p )
    {
        if (!m)
            throw std::bad_alloc();
    }

    PyRef( const PyRef &r ) : m( r.get() ) { Py_XINCREF( m ); }

    ~PyRef() { Py_XDECREF( m ); }

    PyObject *get() const { return m; }

    PyObject * getAcquired() const
    {
        Py_XINCREF( m );
        return m;
    }

    PyRef & operator = (  const PyRef & r )
    {
        PyObject *tmp = m;
        m = r.getAcquired();
        Py_XDECREF( tmp );
        return *this;
    }

    bool operator == (  const PyRef & r ) const
    {
        return r.get() == m;
    }

    /** clears the reference without decreasing the reference count
        only seldom needed ! */
    void scratch()
    {
        m = nullptr;
    }

    /** returns 1 when the reference points to a python object python object,
        otherwise 0.
    */
    bool is() const
    {
        return m != nullptr;
    }

    struct Hash
    {
        sal_IntPtr operator () ( const PyRef &r) const { return sal_IntPtr( r.get() ); }
    };
};

struct stRuntimeImpl;
typedef struct stRuntimeImpl RuntimeImpl;

enum ConversionMode { ACCEPT_UNO_ANY, REJECT_UNO_ANY };


/** The pyuno::Runtime class keeps the internal state of the python UNO bridge
    for the currently in use python interpreter.

    You may keep a Runtime instance, use it from a different thread, etc. But you must
    make sure to fulfill all preconditions mentioned for the specific methods.
*/

class LO_DLLPUBLIC_PYUNO Runtime
{
    RuntimeImpl *impl;

    /**
        Safely unpacks a Python iterator into a sequence, then
        stores it in an Any. Used internally by pyObject2Any
    */
    bool pyIterUnpack( PyObject *const, css::uno::Any & ) const;
public:
    ~Runtime( );

    /**
        preconditions: python has been initialized before,
        the global interpreter lock is held and pyuno
        has been initialized for the currently used interpreter.

        Note: This method exists for efficiency reasons to save
        lookup costs for any2PyObject and pyObject2Any

        @throw RuntimeException in case the runtime has not been
               initialized before
     */
    Runtime() throw( css::uno::RuntimeException );

    Runtime( const Runtime & );
    Runtime & operator = ( const Runtime & );

    /** Initializes the python-UNO bridge. May be called only once per python interpreter.

        @param ctx the component context is used to instantiate bridge services needed
        for bridging such as invocation, typeconverter, invocationadapterfactory, etc.

        preconditions: python has been initialized before and
        the global interpreter lock is held and pyuno is not
        initialized (see isInitialized() ).

        @throw RuntimeException in case the thread is not attached or the runtime
                                has not been initialized.
    */
    static void SAL_CALL initialize(
        const css::uno::Reference< css::uno::XComponentContext > & ctx )
        throw ( css::uno::RuntimeException, std::exception );

    /** Checks, whether the uno runtime is already initialized in the current python interpreter.
     */
    static bool SAL_CALL isInitialized() throw (css::uno::RuntimeException);

    /** converts something contained in an UNO Any to a Python object

        preconditions: python has been initialized before,
        the global interpreter lock is held and pyuno::Runtime
        has been initialized.
    */
    PyRef any2PyObject (const css::uno::Any &source ) const
        throw ( css::script::CannotConvertException,
                css::lang::IllegalArgumentException,
                css::uno::RuntimeException );

    /** converts a Python object to a UNO any

        preconditions: python has been initialized before,
        the global interpreter lock is held and pyuno
        has been initialized
    */
    css::uno::Any pyObject2Any (
        const PyRef & source , enum ConversionMode mode = REJECT_UNO_ANY ) const
        throw ( css::uno::RuntimeException);

    /** extracts a proper uno exception from a given python exception
     */
    css::uno::Any extractUnoException(
        const PyRef & excType, const PyRef & excValue, const PyRef & excTraceback) const;

    /** Returns the internal handle. Should only be used by the module implementation
     */
    RuntimeImpl *getImpl() const { return impl; }
};


/** helper class for attaching the current thread to the python runtime.

    Attaching is done creating a new threadstate for the given interpreter
    and acquiring the global interpreter lock.

    Usage:

    ... don't use python here
    {
        PyThreadAttach guard( PyInterpreterState_Head() );
        {
            ... do whatever python code you want
            {
               PyThreadDetach antiguard;
               ... don't use python here
            }
            ... do whatever python code you want
        }
    }
    ... don't use python here

    Note: The additional scope brackets after the PyThreadAttach are needed,
          e.g. when you would leave them away, dtors of potential pyrefs
          may be called after the thread has detached again.
 */
class LO_DLLPUBLIC_PYUNO PyThreadAttach
{
    PyThreadState *tstate;
    PyThreadAttach ( const PyThreadAttach & ) = delete;
    PyThreadAttach & operator = ( const PyThreadAttach & ) = delete;
public:

    /** Creates a new python threadstate and acquires the global interpreter lock.
        precondition: The current thread MUST NOT hold the global interpreter lock.
        postcondition: The global interpreter lock is acquired

        @raises css::uno::RuntimeException
             in case no pythread state could be created
     */
    PyThreadAttach( PyInterpreterState *interp) throw ( css::uno::RuntimeException );


    /** Releases the global interpreter lock and destroys the thread state.
     */
    ~PyThreadAttach();
};

/** helper class for detaching the current thread from the python runtime
    to do some blocking, non-python related operation.

    @see PyThreadAttach
*/
class LO_DLLPUBLIC_PYUNO PyThreadDetach
{
    PyThreadState *tstate;
    PyThreadDetach ( const PyThreadDetach & ) = delete;
    PyThreadDetach & operator = ( const PyThreadDetach & ) = delete;

public:
    /** Releases the global interpreter lock.

       precondition: The current thread MUST hold the global interpreter lock.
       postcondition: The current thread does not hold the global interpreter lock anymore.
    */
    PyThreadDetach() throw ( css::uno::RuntimeException );
    /** Acquires the global interpreter lock again
    */
    ~PyThreadDetach();
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
