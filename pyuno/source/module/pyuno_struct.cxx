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

#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/XInvocation2.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "pyuno_impl.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::TypeClass;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Exception;
using com::sun::star::lang::XSingleServiceFactory;
using com::sun::star::script::XInvocation2;
using com::sun::star::beans::XMaterialHolder;

namespace pyuno
{

static void PyUNOStruct_del( PyObject* self )
{
    PyUNO *me = reinterpret_cast<PyUNO*>( self );
    {
        PyThreadDetach antiguard;
        delete me->members;
    }
    PyObject_Del( self );
}

static PyObject *PyUNOStruct_str( PyObject *self )
{
    PyUNO *me = reinterpret_cast<PyUNO*>( self );
    OString buf;

    Reference<XMaterialHolder> rHolder( me->members->xInvocation,UNO_QUERY );
    if( rHolder.is() )
    {
        PyThreadDetach antiguard;
        Any a = rHolder->getMaterial();
        OUString s = val2str( a.getValue(), a.getValueType().getTypeLibType() );
        buf = OUStringToOString( s, RTL_TEXTENCODING_ASCII_US );
    }

    return PyUnicode_FromString( buf.getStr());
}

static PyObject *PyUNOStruct_repr( PyObject *self )
{
    PyUNO *me = reinterpret_cast<PyUNO*>( self );
    PyObject *ret = nullptr;

    if( me->members->wrappedObject.getValueType().getTypeClass()
        == css::uno::TypeClass_EXCEPTION )
    {
        Reference< XMaterialHolder > rHolder(me->members->xInvocation,UNO_QUERY);
        if( rHolder.is() )
        {
            Any a = rHolder->getMaterial();
            Exception e;
            a >>= e;
            ret = ustring2PyUnicode(e.Message ).getAcquired();
        }
    }
    else
    {
        ret = PyUNOStruct_str( self );
    }

    return ret;
}

static PyObject* PyUNOStruct_dir( PyObject *self )
{
    PyUNO *me = reinterpret_cast<PyUNO*>( self );

    PyObject* member_list = nullptr;

    try
    {
        member_list = PyList_New( 0 );
        const css::uno::Sequence<OUString> aMemberNames = me->members->xInvocation->getMemberNames();
        for( const auto& aMember : aMemberNames )
        {
            // setitem steals a reference
            PyList_Append( member_list, ustring2PyString( aMember ).getAcquired() );
        }
    }
    catch( const RuntimeException &e )
    {
        raisePyExceptionWithAny( Any(e) );
    }

    return member_list;
}

static PyObject* PyUNOStruct_getattr( PyObject* self, char* name )
{
    PyUNO *me = reinterpret_cast<PyUNO*>( self );

    try
    {
        Runtime runtime;

        me = reinterpret_cast<PyUNO*>(self);
        if (strcmp (name, "__dict__") == 0)
        {
            Py_INCREF (Py_TYPE(me)->tp_dict);
            return Py_TYPE(me)->tp_dict;
        }
        if( strcmp( name, "__class__" ) == 0 )
        {
            return getClass(
                me->members->wrappedObject.getValueType().getTypeName(), runtime ).getAcquired();
        }

        PyObject *pRet = PyObject_GenericGetAttr( self, PyUnicode_FromString( name ) );
        if( pRet )
            return pRet;
        PyErr_Clear();

        OUString attrName( OUString::createFromAscii( name ) );
        if( me->members->xInvocation->hasProperty( attrName ) )
        {
            //Return the value of the property
            Any anyRet;
            {
                PyThreadDetach antiguard;
                anyRet = me->members->xInvocation->getValue( attrName );
            }
            PyRef ret = runtime.any2PyObject( anyRet );
            Py_XINCREF( ret.get() );
            return ret.get();
        }

        //or else...
        PyErr_SetString (PyExc_AttributeError, name);
    }
    catch( const css::reflection::InvocationTargetException & e )
    {
        raisePyExceptionWithAny( e.TargetException );
    }
    catch( const css::beans::UnknownPropertyException & e )
    {
        raisePyExceptionWithAny( Any(e) );
    }
    catch( const css::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( Any(e) );
    }
    catch( const css::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( Any(e) );
    }
    catch( const RuntimeException &e )
    {
        raisePyExceptionWithAny( Any(e) );
    }

    return nullptr;
}

static int PyUNOStruct_setattr (PyObject* self, char* name, PyObject* value)
{
    PyUNO* me;

    me = reinterpret_cast<PyUNO*>(self);
    try
    {
        Runtime runtime;
        Any val= runtime.pyObject2Any(value, ACCEPT_UNO_ANY);

        OUString attrName( OUString::createFromAscii( name ) );
        {
            PyThreadDetach antiguard;
            if (me->members->xInvocation->hasProperty (attrName))
            {
                me->members->xInvocation->setValue (attrName, val);
                return 0; //Keep with Python's boolean system
            }
        }
    }
    catch( const css::reflection::InvocationTargetException & e )
    {
        raisePyExceptionWithAny( e.TargetException );
        return 1;
    }
    catch( const css::beans::UnknownPropertyException & e )
    {
        raisePyExceptionWithAny( Any(e) );
        return 1;
    }
    catch( const css::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( Any(e) );
        return 1;
    }
    catch( const RuntimeException & e )
    {
        raisePyExceptionWithAny( Any( e ) );
        return 1;
    }
    PyErr_SetString (PyExc_AttributeError, name);
    return 1; //as above.
}


static PyObject* PyUNOStruct_cmp( PyObject *self, PyObject *that, int op )
{
    PyObject *result;

    if(op != Py_EQ && op != Py_NE)
    {
        PyErr_SetString( PyExc_TypeError, "only '==' and '!=' comparisons are defined" );
        return nullptr;
    }
    if( self == that )
    {
        result = (op == Py_EQ ? Py_True : Py_False);
        Py_INCREF( result );
        return result;
    }
    try
    {
        Runtime runtime;
        if( PyObject_IsInstance( that, getPyUnoStructClass().get() ) )
        {

            PyUNO *me = reinterpret_cast< PyUNO * > ( self );
            PyUNO *other = reinterpret_cast< PyUNO * > ( that );
            css::uno::TypeClass tcMe = me->members->wrappedObject.getValueTypeClass();
            css::uno::TypeClass tcOther = other->members->wrappedObject.getValueTypeClass();

            if( tcMe == tcOther )
            {
                if( tcMe == css::uno::TypeClass_STRUCT ||
                    tcMe == css::uno::TypeClass_EXCEPTION )
                {
                    Reference< XMaterialHolder > xMe( me->members->xInvocation,UNO_QUERY );
                    Reference< XMaterialHolder > xOther( other->members->xInvocation,UNO_QUERY );
                    if( xMe->getMaterial() == xOther->getMaterial() )
                    {
                        result = (op == Py_EQ ? Py_True : Py_False);
                        Py_INCREF( result );
                        return result;
                    }
                }
            }
        }
    }
    catch( const css::uno::RuntimeException & e)
    {
        raisePyExceptionWithAny( Any( e ) );
    }

    result = (op == Py_EQ ? Py_False : Py_True);
    Py_INCREF(result);
    return result;
}

static PyMethodDef PyUNOStructMethods[] =
{
    {"__dir__",    reinterpret_cast<PyCFunction>(PyUNOStruct_dir),    METH_NOARGS,  nullptr},
    {nullptr,         nullptr,                                              0,            nullptr}
};

static PyTypeObject PyUNOStructType =
{
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    "pyuno.struct",
    sizeof (PyUNO),
    0,
    PyUNOStruct_del,
#if PY_VERSION_HEX >= 0x03080000
    0, // Py_ssize_t tp_vectorcall_offset
#else
    nullptr, // printfunc tp_print
#endif
    PyUNOStruct_getattr,
    PyUNOStruct_setattr,
    /* this type does not exist in Python 3: (cmpfunc) */ nullptr,
    PyUNOStruct_repr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    PyUNOStruct_str,
    nullptr,
    nullptr,
    nullptr,
    Py_TPFLAGS_HAVE_RICHCOMPARE,
    nullptr,
    nullptr,
    nullptr,
    PyUNOStruct_cmp,
    0,
    nullptr,
    nullptr,
    PyUNOStructMethods,
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

int PyUNOStruct_initType()
{
    return PyType_Ready( &PyUNOStructType );
}

PyRef getPyUnoStructClass()
{
    return PyRef( reinterpret_cast< PyObject * > ( &PyUNOStructType ) );
}

PyRef PyUNOStruct_new (
    const Any &targetInterface,
    const Reference<XSingleServiceFactory> &ssf )
{
    Reference<XInvocation2> xInvocation;

    {
        PyThreadDetach antiguard;
        xInvocation.set(
            ssf->createInstanceWithArguments( Sequence<Any>( &targetInterface, 1 ) ), css::uno::UNO_QUERY_THROW );
    }
    if( !Py_IsInitialized() )
        throw RuntimeException();

    PyUNO* self = PyObject_New (PyUNO, &PyUNOStructType);
    if (self == nullptr)
        return PyRef(); // == error
    self->members = new PyUNOInternals;
    self->members->xInvocation = xInvocation;
    self->members->wrappedObject = targetInterface;
    return PyRef( reinterpret_cast<PyObject*>(self), SAL_NO_ACQUIRE );

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
