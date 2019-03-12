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

#include <rtl/ustrbuf.hxx>

#include <typelib/typedescription.hxx>


using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::TypeDescription;

namespace pyuno
{

void raisePyExceptionWithAny( const css::uno::Any &anyExc )
{
    try
    {
        Runtime runtime;
        PyRef exc = runtime.any2PyObject( anyExc );
        if( exc.is() )
        {
            PyRef type( getClass( anyExc.getValueType().getTypeName(),runtime ) );
            PyErr_SetObject( type.get(), exc.get());
        }
        else
        {
            css::uno::Exception e;
            anyExc >>= e;

            OUStringBuffer buf;
            buf.append( "Couldn't convert uno exception to a python exception (" );
            buf.append(anyExc.getValueType().getTypeName());
            buf.append( ": " );
            buf.append(e.Message );
            buf.append( ")" );
            PyErr_SetString(
                PyExc_SystemError,
                OUStringToOString(buf.makeStringAndClear(),RTL_TEXTENCODING_ASCII_US).getStr() );
        }
    }
    catch(const css::lang::IllegalArgumentException & e)
    {
        PyErr_SetString( PyExc_SystemError,
                         OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US).getStr() );
    }
    catch(const css::script::CannotConvertException & e)
    {
        PyErr_SetString( PyExc_SystemError,
                         OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US).getStr() );
    }
    catch(const RuntimeException & e)
    {
        PyErr_SetString( PyExc_SystemError,
                         OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US).getStr() );
    }
}

/// @throws RuntimeException
static PyRef createClass( const OUString & name, const Runtime &runtime )
{
    // assuming that this is never deleted !
    // note I don't have the knowledge how to initialize these type objects correctly !
    TypeDescription desc( name );
    if( ! desc.is() )
    {
        throw RuntimeException( "pyuno.getClass: uno exception " + name + " is unknown" );
    }

    bool isStruct = desc.get()->eTypeClass == typelib_TypeClass_STRUCT;
    bool isExc = desc.get()->eTypeClass == typelib_TypeClass_EXCEPTION;
    bool isInterface = desc.get()->eTypeClass == typelib_TypeClass_INTERFACE;
    if( !isStruct  && !isExc && ! isInterface )
    {
        throw RuntimeException( "pyuno.getClass: " + name + "is a " +
                    OUString::createFromAscii( typeClassToString( static_cast<css::uno::TypeClass>(desc.get()->eTypeClass)) ) +
                    ", expected EXCEPTION, STRUCT or INTERFACE" );
    }

    // retrieve base class
    PyRef base;
    if( isInterface )
    {
        typelib_InterfaceTypeDescription *pDesc = reinterpret_cast<typelib_InterfaceTypeDescription *>(desc.get());
        if( pDesc->pBaseTypeDescription )
        {
            base = getClass( pDesc->pBaseTypeDescription->aBase.pTypeName, runtime );
        }
        else
        {
            // must be XInterface !
        }
    }
    else
    {
        typelib_CompoundTypeDescription *pDesc = reinterpret_cast<typelib_CompoundTypeDescription*>(desc.get());
        if( pDesc->pBaseTypeDescription )
        {
            base = getClass( pDesc->pBaseTypeDescription->aBase.pTypeName, runtime );
        }
        else
        {
            if( isExc )
                // we are currently creating the root UNO exception
                base = PyRef(PyExc_Exception);
        }
    }
    PyRef args( PyTuple_New( 3 ), SAL_NO_ACQUIRE, NOT_NULL );

    PyRef pyTypeName = ustring2PyString( name /*.replace( '.', '_' )*/ );

    PyRef bases;
    if( base.is() )
    {
        { // for CC, keeping ref-count being 1
        bases = PyRef( PyTuple_New( 1 ), SAL_NO_ACQUIRE );
        }
        PyTuple_SetItem( bases.get(), 0 , base.getAcquired() );
    }
    else
    {
        bases = PyRef( PyTuple_New( 0 ), SAL_NO_ACQUIRE );
    }

    PyTuple_SetItem( args.get(), 0, pyTypeName.getAcquired());
    PyTuple_SetItem( args.get(), 1, bases.getAcquired() );
    PyTuple_SetItem( args.get(), 2, PyDict_New() );

    PyRef ret(
        PyObject_CallObject(reinterpret_cast<PyObject *>(&PyType_Type) , args.get()),
        SAL_NO_ACQUIRE );

    // now overwrite ctor and attrib functions
    if( isInterface )
    {
        PyObject_SetAttrString(
            ret.get(), "__pyunointerface__",
            ustring2PyString(name).get() );
    }
    else
    {
        PyRef ctor = getObjectFromUnoModule( runtime,"_uno_struct__init__" );
        PyRef setter = getObjectFromUnoModule( runtime,"_uno_struct__setattr__" );
        PyRef getter = getObjectFromUnoModule( runtime,"_uno_struct__getattr__" );
        PyRef repr = getObjectFromUnoModule( runtime,"_uno_struct__repr__" );
        PyRef eq = getObjectFromUnoModule( runtime,"_uno_struct__eq__" );
        PyRef ne = getObjectFromUnoModule( runtime,"_uno_struct__ne__" );

        PyObject_SetAttrString(
            ret.get(), "__pyunostruct__",
            ustring2PyString(name).get() );
        PyObject_SetAttrString(
            ret.get(), "typeName",
            ustring2PyString(name).get() );
        PyObject_SetAttrString(
            ret.get(), "__init__", ctor.get() );
        PyObject_SetAttrString(
            ret.get(), "__getattr__", getter.get() );
        PyObject_SetAttrString(
            ret.get(), "__setattr__", setter.get() );
        PyObject_SetAttrString(
            ret.get(), "__repr__", repr.get() );
        PyObject_SetAttrString(
            ret.get(), "__str__", repr.get() );
        PyObject_SetAttrString(
            ret.get(), "__eq__", eq.get() );
        PyObject_SetAttrString(
            ret.get(), "__ne__", ne.get() );
    }
    return ret;
}

bool isInstanceOfStructOrException( PyObject *obj)
{
    PyRef attr(
        PyObject_GetAttrString(obj, "__class__"),
        SAL_NO_ACQUIRE );
    if(attr.is())
        return PyObject_HasAttrString(attr.get(), "__pyunostruct__");
    else
        return false;
}

bool isInterfaceClass( const Runtime &runtime, PyObject * obj )
{
    const ClassSet & set = runtime.getImpl()->cargo->interfaceSet;
    return set.find( obj ) != set.end();
}

PyRef getClass( const OUString & name , const Runtime &runtime)
{
    PyRef ret;

    RuntimeCargo *cargo =runtime.getImpl()->cargo;
    ExceptionClassMap::iterator ii = cargo->exceptionMap.find( name );
    if( ii == cargo->exceptionMap.end() )
    {
        ret = createClass( name, runtime );
        cargo->exceptionMap[name] = ret;
        if( PyObject_HasAttrString(
                ret.get(), "__pyunointerface__" ) )
            cargo->interfaceSet.insert( ret );

        PyObject_SetAttrString(
            ret.get(), "__pyunointerface__",
            ustring2PyString(name).get() );
    }
    else
    {
        ret = ii->second;
    }

    return ret;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
