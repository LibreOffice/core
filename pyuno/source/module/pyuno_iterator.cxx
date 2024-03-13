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

#include <sal/config.h>

#include "pyuno_impl.hxx"

#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>

using com::sun::star::container::XEnumeration;
using com::sun::star::container::XIndexAccess;
using com::sun::star::lang::IndexOutOfBoundsException;
using com::sun::star::lang::WrappedTargetException;
using com::sun::star::uno::Any;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;


namespace pyuno
{

static void PyUNO_iterator_del( PyObject* self )
{
    PyUNO_iterator* me = reinterpret_cast<PyUNO_iterator*>(self);

    {
        PyThreadDetach antiguard;
        delete me->members;
    }
    PyObject_Del( self );
}

static PyObject* PyUNO_iterator_iter( PyObject *self )
{
    Py_INCREF( self );
    return self;
}

static PyObject* PyUNO_iterator_next( PyObject *self )
{
    PyUNO_iterator* me = reinterpret_cast<PyUNO_iterator*>(self);

    Runtime runtime;
    Any aRet;

    try
    {
        bool hasMoreElements = false;

        {
            PyThreadDetach antiguard;

            hasMoreElements = me->members->xEnumeration->hasMoreElements();
            if ( hasMoreElements )
            {
                aRet = me->members->xEnumeration->nextElement();
            }
        }

        if ( hasMoreElements )
        {
            PyRef rRet = runtime.any2PyObject( aRet );
            return rRet.getAcquired();
        }

        PyErr_SetString( PyExc_StopIteration, "" );
        return nullptr;
    }
    catch( css::container::NoSuchElementException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( css::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( css::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::lang::WrappedTargetException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }

    return nullptr;
}

static PyTypeObject PyUNO_iterator_Type =
{
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    "PyUNO_iterator",
    sizeof (PyUNO_iterator),
    0,
    PyUNO_iterator_del,
#if PY_VERSION_HEX >= 0x03080000
    0, // Py_ssize_t tp_vectorcall_offset
#else
    nullptr, // printfunc tp_print
#endif
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    Py_TPFLAGS_HAVE_ITER,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0,
    PyUNO_iterator_iter, // Generic, reused between the iterator types
    PyUNO_iterator_next,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0
#if PY_VERSION_HEX >= 0x03040000
    , nullptr
#if PY_VERSION_HEX >= 0x03080000
    , nullptr // vectorcallfunc tp_vectorcall
#if PY_VERSION_HEX < 0x03090000
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
    , nullptr // tp_print
#if defined __clang__
#pragma clang diagnostic pop
#endif
#endif
#if PY_VERSION_HEX >= 0x030C00A1
    , 0 // tp_watched
#endif
#endif
#endif
};

PyObject* PyUNO_iterator_new( const Reference< XEnumeration >& xEnumeration )
{
    PyUNO_iterator* self = PyObject_New( PyUNO_iterator, &PyUNO_iterator_Type );
    if ( self == nullptr )
        return nullptr; // == error
    self->members = new PyUNO_iterator_Internals;
    self->members->xEnumeration = xEnumeration;
    return reinterpret_cast<PyObject*>(self);
}

///////////////////////////////////////////////////////////////////////////////

static void PyUNO_list_iterator_del( PyObject* self )
{
    PyUNO_list_iterator* me = reinterpret_cast<PyUNO_list_iterator*>(self);

    {
        PyThreadDetach antiguard;
        delete me->members;
    }
    PyObject_Del( self );
}


static PyObject* PyUNO_list_iterator_next( PyObject *self )
{
    PyUNO_list_iterator* me = reinterpret_cast<PyUNO_list_iterator*>(self);

    Runtime runtime;
    Any aRet;

    try
    {
        bool noMoreElements = false;
        {
            PyThreadDetach antiguard;
            try {
                aRet = me->members->xIndexAccess->getByIndex( me->members->index );
            }
            catch( const css::lang::IndexOutOfBoundsException & )
            {
                noMoreElements = true;
            }
        }

        if ( noMoreElements )
        {
            PyErr_SetString( PyExc_StopIteration, "" );
            return nullptr;
        }

        PyRef rRet = runtime.any2PyObject( aRet );
        me->members->index++;
        return rRet.getAcquired();
    }
    catch( css::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( css::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::lang::WrappedTargetException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }

    return nullptr;
}

static PyTypeObject PyUNO_list_iterator_Type =
{
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    "PyUNO_iterator",
    sizeof (PyUNO_list_iterator),
     0,
    PyUNO_list_iterator_del,
#if PY_VERSION_HEX >= 0x03080000
    0, // Py_ssize_t tp_vectorcall_offset
#else
    nullptr, // printfunc tp_print
#endif
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    Py_TPFLAGS_HAVE_ITER,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0,
    PyUNO_iterator_iter, // Generic, reused between the iterator types
    PyUNO_list_iterator_next,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0
#if PY_VERSION_HEX >= 0x03040000
    , nullptr
#if PY_VERSION_HEX >= 0x03080000
    , nullptr // vectorcallfunc tp_vectorcall
#if PY_VERSION_HEX < 0x03090000
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
    , nullptr // tp_print
#if defined __clang__
#pragma clang diagnostic pop
#endif
#endif
#if PY_VERSION_HEX >= 0x030C00A1
    , 0 // tp_watched
#endif
#endif
#endif
};

PyObject* PyUNO_list_iterator_new( const Reference<XIndexAccess> &xIndexAccess )
{
    PyUNO_list_iterator* self = PyObject_New( PyUNO_list_iterator, &PyUNO_list_iterator_Type );
    if ( self == nullptr )
        return nullptr; // == error
    self->members = new PyUNO_list_iterator_Internals;
    self->members->xIndexAccess = xIndexAccess;
    self->members->index = 0;
    return reinterpret_cast<PyObject*>(self);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
