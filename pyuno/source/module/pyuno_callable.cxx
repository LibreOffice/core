/*************************************************************************
 *
 *  $RCSfile: pyuno_callable.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 15:01:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Ralph Thomas
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Ralph Thomas, Joerg Budischewski
 *
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
    PyMem_DEL (self);

    return;
}

PyObject* PyUNO_callable_call (PyObject* self, PyObject* args, PyObject* kwords)
{
    PyUNO_callable* me;

    Sequence<short> aOutParamIndex;
    Sequence<Any> aOutParam;
    Sequence<Any> aParams;
    Sequence<Type> aParamTypes;
    Any any_params;
    Any out_params;
    PyObject* python_params;
    Any ret_value;
    int num_params_in;

    me = (PyUNO_callable*) self;

    PyRef ret;
    try
    {
        Runtime runtime;
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
            PyThreadDetach antiguard;
            PYUNO_DEBUG_2( "entering invoke %s\n",
                    OUStringToOString( me->members->methodName,RTL_TEXTENCODING_ASCII_US).getStr() );
            ret_value = me->members->xInvocation->invoke (
                me->members->methodName, aParams, aOutParamIndex, aOutParam);
            PYUNO_DEBUG_2( "leaving invoke %s\n",
                    OUStringToOString( me->members->methodName,RTL_TEXTENCODING_ASCII_US).getStr() );
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
        raisePyExceptionWithAny( e.TargetException );
    }
    catch( com::sun::star::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( com::sun::star::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch (::com::sun::star::uno::RuntimeException &e)
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }

    return ret.getAcquired();
}


static PyTypeObject PyUNO_callable_Type =
{
    PyObject_HEAD_INIT (&PyType_Type)
    0,
    "PyUNO_callable",
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
};

PyRef PyUNO_callable_new (
    const Reference<XInvocation2> &my_inv,
    const OUString & methodName,
    const Reference<XSingleServiceFactory> &xInvocationFactory,
    const Reference<XTypeConverter> &tc,
    enum ConversionMode mode )
{
    PyUNO_callable* self;

    self = PyObject_NEW (PyUNO_callable, &PyUNO_callable_Type);
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
