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

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <rtl/ustrbuf.hxx>

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::RuntimeException;
using com::sun::star::script::XInvocation2;

namespace pyuno
{
struct PyUNO_callable_Internals
{
    Reference<XInvocation2> xInvocation;
    OUString methodName;
    ConversionMode mode;
};

struct PyUNO_callable
{
    PyObject_HEAD
    PyUNO_callable_Internals* members;
};

static void PyUNO_callable_del (PyObject* self)
{
    PyUNO_callable* me;

    me = reinterpret_cast<PyUNO_callable*>(self);
    delete me->members;
    PyObject_Del (self);
}

static PyObject* PyUNO_callable_call(
    PyObject* self, PyObject* args, SAL_UNUSED_PARAMETER PyObject*)
{
    PyUNO_callable* me;

    Sequence<short> aOutParamIndex;
    Sequence<Any> aOutParam;
    Sequence<Any> aParams;
    Any any_params;
    Any ret_value;
    RuntimeCargo *cargo = nullptr;
    me = reinterpret_cast<PyUNO_callable*>(self);

    PyRef ret;
    try
    {
        Runtime runtime;
        cargo = runtime.getImpl()->cargo;
        any_params = runtime.pyObject2Any (args, me->members->mode);

        if (any_params.getValueTypeClass () == css::uno::TypeClass_SEQUENCE)
        {
            any_params >>= aParams;
        }
        else
        {
            aParams.realloc (1);
            aParams [0] = any_params;
        }

        {
            PyThreadDetach antiguard; //python free zone

            // do some logging if desired ...
            if( isLog( cargo, LogLevel::CALL ) )
            {
                logCall( cargo, "try     py->uno[0x", me->members->xInvocation.get(),
                         me->members->methodName, aParams );
            }

            // do the call
            ret_value = me->members->xInvocation->invoke (
                me->members->methodName, aParams, aOutParamIndex, aOutParam);

            // log the reply, if desired
            if( isLog( cargo, LogLevel::CALL ) )
            {
                logReply( cargo, "success py->uno[0x", me->members->xInvocation.get(),
                          me->members->methodName, ret_value, aOutParam);
            }
        }


        PyRef temp = runtime.any2PyObject (ret_value);
        if( aOutParam.getLength() )
        {
            PyRef return_list( PyTuple_New (1+aOutParam.getLength()), SAL_NO_ACQUIRE, NOT_NULL );
            PyTuple_SetItem (return_list.get(), 0, temp.getAcquired());

            // initialize with defaults in case of exceptions
            int i;
            for( i = 1 ; i < 1+aOutParam.getLength() ; i ++ )
            {
                Py_INCREF( Py_None );
                PyTuple_SetItem( return_list.get() , i , Py_None );
            }

            for( i = 0 ; i < aOutParam.getLength() ; i ++ )
            {
                PyRef ref = runtime.any2PyObject( aOutParam[i] );
                PyTuple_SetItem (return_list.get(), 1+i, ref.getAcquired());
            }
            ret = return_list;
        }
        else
        {
            ret = temp;
        }
    }
    catch( const css::reflection::InvocationTargetException & e )
    {

        if( isLog( cargo, LogLevel::CALL ) )
        {
            logException( cargo, "except  py->uno[0x", me->members->xInvocation.get() ,
                          me->members->methodName, e.TargetException.getValue(), e.TargetException.getValueTypeRef());
        }
        raisePyExceptionWithAny( e.TargetException );
    }
    catch( const css::script::CannotConvertException &e )
    {
        if( isLog( cargo, LogLevel::CALL ) )
        {
            logException( cargo, "error  py->uno[0x", me->members->xInvocation.get() ,
                          me->members->methodName, &e, cppu::UnoType<decltype(e)>::get().getTypeLibType());
        }
        raisePyExceptionWithAny( css::uno::makeAny( e ) );
    }
    catch( const css::lang::IllegalArgumentException &e )
    {
        if( isLog( cargo, LogLevel::CALL ) )
        {
            logException( cargo, "error  py->uno[0x", me->members->xInvocation.get() ,
                          me->members->methodName, &e, cppu::UnoType<decltype(e)>::get().getTypeLibType());
        }
        raisePyExceptionWithAny( css::uno::makeAny( e ) );
    }
    catch (const css::uno::RuntimeException &e)
    {
        if( cargo && isLog( cargo, LogLevel::CALL ) )
        {
            logException( cargo, "error  py->uno[0x", me->members->xInvocation.get() ,
                          me->members->methodName, &e, cppu::UnoType<decltype(e)>::get().getTypeLibType());
        }
        raisePyExceptionWithAny( css::uno::makeAny( e ) );
    }

    return ret.getAcquired();
}


static PyTypeObject PyUNO_callable_Type =
{
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    "PyUNO_callable",
    sizeof (PyUNO_callable),
    0,
    ::pyuno::PyUNO_callable_del,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    ::pyuno::PyUNO_callable_call,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    0,
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
    nullptr
    , 0
#if PY_VERSION_HEX >= 0x03040000
    , nullptr
#endif
};

PyRef PyUNO_callable_new (
    const Reference<XInvocation2> &my_inv,
    const OUString & methodName,
    enum ConversionMode mode )
{
    PyUNO_callable* self;

    OSL_ENSURE (my_inv.is(), "XInvocation must be valid");

    self = PyObject_New (PyUNO_callable, &PyUNO_callable_Type);
    if (self == nullptr)
        return nullptr; //NULL == Error!

    self->members = new PyUNO_callable_Internals;
    self->members->xInvocation = my_inv;
    self->members->methodName = methodName;
    self->members->mode = mode;

    return PyRef( reinterpret_cast<PyObject*>(self), SAL_NO_ACQUIRE );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
