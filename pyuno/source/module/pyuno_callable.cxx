/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pyuno_callable.cxx,v $
 * $Revision: 1.8 $
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

#include <osl/thread.h>
#include <rtl/ustrbuf.hxx>

using rtl::OUStringToOString;
using rtl::OUString;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;
using com::sun::star::uno::Type;
using com::sun::star::uno::TypeClass;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;
using com::sun::star::lang::XSingleServiceFactory;
using com::sun::star::script::XTypeConverter;
using com::sun::star::script::XInvocation2;

namespace pyuno
{
typedef struct
{
    Reference<XInvocation2> xInvocation;
    Reference<XSingleServiceFactory> xInvocationFactory;
    Reference<XTypeConverter> xTypeConverter;
    OUString methodName;
    ConversionMode mode;
} PyUNO_callable_Internals;

typedef struct
{
    PyObject_HEAD
    PyUNO_callable_Internals* members;
} PyUNO_callable;

void PyUNO_callable_del (PyObject* self)
{
    PyUNO_callable* me;

    me = (PyUNO_callable*) self;
    delete me->members;
    PyObject_Del (self);

    return;
}

PyObject* PyUNO_callable_call (PyObject* self, PyObject* args, PyObject*)
{
    PyUNO_callable* me;

    Sequence<short> aOutParamIndex;
    Sequence<Any> aOutParam;
    Sequence<Any> aParams;
    Sequence<Type> aParamTypes;
    Any any_params;
    Any out_params;
    Any ret_value;
    RuntimeCargo *cargo = 0;
    me = (PyUNO_callable*) self;

    PyRef ret;
    try
    {
        Runtime runtime;
        cargo = runtime.getImpl()->cargo;
        any_params = runtime.pyObject2Any (args, me->members->mode);

        if (any_params.getValueTypeClass () == com::sun::star::uno::TypeClass_SEQUENCE)
        {
            any_params >>= aParams;
        }
        else
        {
            aParams.realloc (1);
            aParams [0] <<= any_params;
        }

        {
            PyThreadDetach antiguard; //pyhton free zone

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
            PyRef return_list( PyTuple_New (1+aOutParam.getLength()), SAL_NO_ACQUIRE );
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
    catch( com::sun::star::reflection::InvocationTargetException & e )
    {

        if( isLog( cargo, LogLevel::CALL ) )
        {
            logException( cargo, "except  py->uno[0x", me->members->xInvocation.get() ,
                          me->members->methodName, e.TargetException.getValue(), e.TargetException.getValueTypeRef());
        }
        raisePyExceptionWithAny( e.TargetException );
    }
    catch( com::sun::star::script::CannotConvertException &e )
    {
        if( isLog( cargo, LogLevel::CALL ) )
        {
            logException( cargo, "error  py->uno[0x", me->members->xInvocation.get() ,
                          me->members->methodName, &e, getCppuType(&e).getTypeLibType());
        }
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( com::sun::star::lang::IllegalArgumentException &e )
    {
        if( isLog( cargo, LogLevel::CALL ) )
        {
            logException( cargo, "error  py->uno[0x", me->members->xInvocation.get() ,
                          me->members->methodName, &e, getCppuType(&e).getTypeLibType());
        }
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch (::com::sun::star::uno::RuntimeException &e)
    {
        if( cargo && isLog( cargo, LogLevel::CALL ) )
        {
            logException( cargo, "error  py->uno[0x", me->members->xInvocation.get() ,
                          me->members->methodName, &e, getCppuType(&e).getTypeLibType());
        }
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }

    return ret.getAcquired();
}


static PyTypeObject PyUNO_callable_Type =
{
    PyObject_HEAD_INIT (&PyType_Type)
    0,
    const_cast< char * >("PyUNO_callable"),
    sizeof (PyUNO_callable),
    0,
    (destructor) ::pyuno::PyUNO_callable_del,
    (printfunc) 0,
    (getattrfunc) 0,
    (setattrfunc) 0,
    (cmpfunc) 0,
    (reprfunc) 0,
    0,
    0,
    0,
    (hashfunc) 0,
    (ternaryfunc) ::pyuno::PyUNO_callable_call,
    (reprfunc) 0,
        (getattrofunc)0,
    (setattrofunc)0,
    NULL,
    0,
    NULL,
    (traverseproc)0,
    (inquiry)0,
    (richcmpfunc)0,
    0,
    (getiterfunc)0,
    (iternextfunc)0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    (descrgetfunc)0,
    (descrsetfunc)0,
    0,
    (initproc)0,
    (allocfunc)0,
    (newfunc)0,
    (freefunc)0,
    (inquiry)0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    (destructor)0
};

PyRef PyUNO_callable_new (
    const Reference<XInvocation2> &my_inv,
    const OUString & methodName,
    const Reference<XSingleServiceFactory> &xInvocationFactory,
    const Reference<XTypeConverter> &tc,
    enum ConversionMode mode )
{
    PyUNO_callable* self;

    self = PyObject_New (PyUNO_callable, &PyUNO_callable_Type);
    if (self == NULL)
        return NULL; //NULL == Error!

    self->members = new PyUNO_callable_Internals;
    self->members->xInvocation = my_inv;
    self->members->methodName = methodName;
    self->members->xInvocationFactory = xInvocationFactory;
    self->members->xTypeConverter = tc;
    self->members->mode = mode;

    return PyRef( (PyObject*)self, SAL_NO_ACQUIRE );
}

}
