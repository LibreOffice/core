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

#include <typelib/typedescription.hxx>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XServiceTypeDescription2.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>


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
            PyRef type( getClass( anyExc.getValueTypeName(),runtime ) );
            PyErr_SetObject( type.get(), exc.get());
        }
        else
        {
            css::uno::Exception e;
            anyExc >>= e;

            OUString buf = "Couldn't convert uno exception to a python exception (" +
                anyExc.getValueTypeName() + ": " + e.Message + ")";
            PyErr_SetString(
                PyExc_SystemError,
                OUStringToOString(buf,RTL_TEXTENCODING_ASCII_US).getStr() );
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
static PyRef createClassFromTypeDescription(
    std::u16string_view name, typelib_TypeDescription* pType, const Runtime& runtime )
{
    bool isStruct = pType->eTypeClass == typelib_TypeClass_STRUCT;
    bool isExc = pType->eTypeClass == typelib_TypeClass_EXCEPTION;
    bool isInterface = pType->eTypeClass == typelib_TypeClass_INTERFACE;
    if( !isStruct  && !isExc && ! isInterface )
    {
        throw RuntimeException( "pyuno.getClass: " + OUString::Concat(name) + "is a " +
                    OUString::createFromAscii( typeClassToString( static_cast<css::uno::TypeClass>(pType->eTypeClass)) ) +
                    ", expected EXCEPTION, STRUCT or INTERFACE" );
    }

    // retrieve base class
    PyRef base;
    if( isInterface )
    {
        typelib_InterfaceTypeDescription *pDesc = reinterpret_cast<typelib_InterfaceTypeDescription *>(pType);
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
        typelib_CompoundTypeDescription *pDesc = reinterpret_cast<typelib_CompoundTypeDescription*>(pType);
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

static PyRef createEmptyPyTypeForTypeDescription(
    const css::uno::Reference<css::reflection::XTypeDescription>& xType)
{
    PyRef ret(
        PyObject_CallFunctionObjArgs(
            reinterpret_cast<PyObject*>(&PyType_Type),
            ustring2PyString(xType->getName()).get(),
            PyRef(PyTuple_New(0), SAL_NO_ACQUIRE).get(), // bases
            PyRef(PyDict_New(), SAL_NO_ACQUIRE).get(),
            nullptr),
        SAL_NO_ACQUIRE);

    return ret;
}

static PyRef createClassForService(
    const css::uno::Reference<css::reflection::XServiceTypeDescription2>& xService)
{
    PyRef ret = createEmptyPyTypeForTypeDescription(xService);

    // Set an attribute on the class for each of the constructors
    for (const auto& xConstructor : xService->getConstructors())
    {
        OUString sName = xConstructor->getName();

        // The name is empty for the default constructor
        if (sName.isEmpty())
        {
            if (xConstructor->isDefaultConstructor())
                sName = "create";
            else
                continue;
        }

        PyObject_SetAttr(
            ret.get(),
            ustring2PyString(sName).get(),
            PyUNO_service_constructor_new(xService, xConstructor).get());
    }

    return ret;
}

/// @throws RuntimeException
static PyRef createClass( const OUString & name, const Runtime &runtime )
{
    // assuming that this is never deleted !
    // note I don't have the knowledge how to initialize these type objects correctly !
    TypeDescription desc(name);
    if (desc.is())
        return createClassFromTypeDescription(name, desc.get(), runtime);

    // If there’s no type description from the typelib then check if it’s a service using the type
    // description manager.
    css::uno::Any xType;

    try
    {
        xType = runtime.getImpl()->cargo->xTdMgr->getByHierarchicalName(name);
    }
    catch (css::container::NoSuchElementException&)
    {
        // This will flow through to throw a runtime exception below
    }

    css::uno::Reference<css::reflection::XServiceTypeDescription2> xService;

    if ((xType >>= xService) && xService.is())
        return createClassForService(xService);

    throw RuntimeException("pyuno.getClass: uno exception " + name + " is unknown");
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
