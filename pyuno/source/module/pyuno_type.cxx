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

#include <o3tl/any.hxx>

#include <typelib/typedescription.hxx>


using com::sun::star::uno::TypeClass;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Any;
using com::sun::star::uno::TypeDescription;

namespace pyuno
{
const char *typeClassToString( TypeClass t )
{
    const char * ret = nullptr;
    switch (t)
    {
    case css::uno::TypeClass_VOID:
        ret = "VOID"; break;
    case css::uno::TypeClass_CHAR:
        ret = "CHAR"; break;
    case css::uno::TypeClass_BOOLEAN:
        ret = "BOOLEAN"; break;
    case css::uno::TypeClass_BYTE:
        ret = "BYTE"; break;
    case css::uno::TypeClass_SHORT:
        ret = "SHORT"; break;
    case css::uno::TypeClass_UNSIGNED_SHORT:
        ret = "UNSIGNED_SHORT"; break;
    case css::uno::TypeClass_LONG:
        ret = "LONG"; break;
    case css::uno::TypeClass_UNSIGNED_LONG:
        ret = "UNSIGNED_LONG"; break;
    case css::uno::TypeClass_HYPER:
        ret = "HYPER"; break;
    case css::uno::TypeClass_UNSIGNED_HYPER:
        ret = "UNSIGNED_HYPER"; break;
    case css::uno::TypeClass_FLOAT:
        ret = "FLOAT"; break;
    case css::uno::TypeClass_DOUBLE:
        ret = "DOUBLE"; break;
    case css::uno::TypeClass_STRING:
        ret = "STRING"; break;
    case css::uno::TypeClass_TYPE:
        ret = "TYPE"; break;
    case css::uno::TypeClass_ANY:
        ret = "ANY";break;
    case css::uno::TypeClass_ENUM:
        ret = "ENUM";break;
    case css::uno::TypeClass_STRUCT:
        ret = "STRUCT"; break;
    case css::uno::TypeClass_EXCEPTION:
        ret = "EXCEPTION"; break;
    case css::uno::TypeClass_SEQUENCE:
        ret = "SEQUENCE"; break;
    case css::uno::TypeClass_INTERFACE:
        ret = "INTERFACE"; break;
    case css::uno::TypeClass_TYPEDEF:
        ret = "TYPEDEF"; break;
    case css::uno::TypeClass_SERVICE:
        ret = "SERVICE"; break;
    case css::uno::TypeClass_MODULE:
        ret = "MODULE"; break;
    case css::uno::TypeClass_INTERFACE_METHOD:
        ret = "INTERFACE_METHOD"; break;
    case css::uno::TypeClass_INTERFACE_ATTRIBUTE:
        ret = "INTERFACE_ATTRIBUTE"; break;
    default:
        ret = "UNKNOWN"; break;
    }
    return ret;
}

static PyRef getClass( const Runtime & r , const char * name)
{
    return PyRef( PyDict_GetItemString( r.getImpl()->cargo->getUnoModule().get(), name ) );
}

PyRef getTypeClass( const Runtime & r )
{
    return getClass( r , "Type" );
}

PyRef getEnumClass( const Runtime & r )
{
    return getClass( r , "Enum" );
}

PyRef getCharClass( const Runtime & r )
{
    return getClass( r , "Char" );
}

PyRef getByteSequenceClass( const Runtime & r )
{
    return getClass( r , "ByteSequence" );
}

PyRef getAnyClass( const Runtime & r )
{
    return getClass( r , "Any" );
}


sal_Unicode PyChar2Unicode( PyObject *obj )
{
    PyRef value( PyObject_GetAttrString( obj, "value" ), SAL_NO_ACQUIRE );
    if( ! PyUnicode_Check( value.get() ) )
    {
        throw RuntimeException(
            u"attribute value of uno.Char is not a unicode string"_ustr );
    }

    if( PyUnicode_GetLength( value.get() ) < 1 )
    {
        throw RuntimeException(
            u"uno.Char contains an empty unicode string"_ustr);
    }

    sal_Unicode c = static_cast<sal_Unicode>(PyUnicode_ReadChar( value.get(), 0));
    return c;
}

Any PyEnum2Enum( PyObject *obj )
{
    Any ret;
    PyRef typeName( PyObject_GetAttrString( obj,"typeName" ), SAL_NO_ACQUIRE);
    PyRef value( PyObject_GetAttrString( obj, "value" ), SAL_NO_ACQUIRE);
    if( !PyUnicode_Check( typeName.get() ) || ! PyUnicode_Check( value.get() ) )
    {
        throw RuntimeException(
            u"attributes typeName and/or value of uno.Enum are not strings"_ustr );
    }

    OUString strTypeName( OUString::createFromAscii( PyUnicode_AsUTF8( typeName.get() ) ) );
    char const *stringValue = PyUnicode_AsUTF8( value.get() );

    TypeDescription desc( strTypeName );
    if( !desc.is() )
    {
        throw RuntimeException( "enum " + OUString::createFromAscii( PyUnicode_AsUTF8(typeName.get()) ) + " is unknown" );
    }

    if(desc.get()->eTypeClass != typelib_TypeClass_ENUM )
    {
        throw RuntimeException( "pyuno.checkEnum: " + strTypeName +  "is a " +
            OUString::createFromAscii(typeClassToString( static_cast<css::uno::TypeClass>(desc.get()->eTypeClass))) +
            ", expected ENUM" );
    }

    desc.makeComplete();

    typelib_EnumTypeDescription *pEnumDesc = reinterpret_cast<typelib_EnumTypeDescription*>(desc.get());
    int i = 0;
    for( i = 0; i < pEnumDesc->nEnumValues ; i ++ )
    {
        if( OUString::unacquired(&pEnumDesc->ppEnumNames[i]).equalsAscii( stringValue ) )
        {
            break;
        }
    }
    if( i == pEnumDesc->nEnumValues )
    {
        throw RuntimeException( "value " + OUString::createFromAscii( stringValue ) +
            "is unknown in enum " +
            OUString::createFromAscii( PyUnicode_AsUTF8( typeName.get() ) ) );
    }
    ret = Any( &pEnumDesc->pEnumValues[i], desc.get()->pWeakRef );

    return ret;
}


Type PyType2Type( PyObject * o )
{
    PyRef pyName( PyObject_GetAttrString( o, "typeName" ), SAL_NO_ACQUIRE);
    if( !PyUnicode_Check( pyName.get() ) )
    {
        throw RuntimeException(
            u"type object does not have typeName property"_ustr );
    }

    PyRef pyTC( PyObject_GetAttrString( o, "typeClass" ), SAL_NO_ACQUIRE );
    Any enumValue = PyEnum2Enum( pyTC.get() );

    OUString name( OUString::createFromAscii( PyUnicode_AsUTF8( pyName.get() ) ) );
    TypeDescription desc( name );
    if( ! desc.is() )
    {
        throw RuntimeException( "type " + name +  " is unknown" );
    }
    css::uno::TypeClass tc = *o3tl::doAccess<css::uno::TypeClass>(enumValue);
    if( static_cast<css::uno::TypeClass>(desc.get()->eTypeClass) != tc )
    {
        throw RuntimeException( "pyuno.checkType: " + name + " is a " +
            OUString::createFromAscii( typeClassToString( static_cast<TypeClass>(desc.get()->eTypeClass)) ) +
            ", but type got construct with typeclass " +
            OUString::createFromAscii( typeClassToString( tc ) ) );
    }
    return desc.get()->pWeakRef;
}

static PyObject* callCtor( const Runtime &r , const char * clazz, const PyRef & args )
{
    PyRef code( PyDict_GetItemString( r.getImpl()->cargo->getUnoModule().get(), clazz ) );
    if( ! code.is() )
    {
        OString buf = OString::Concat("couldn't access uno.") + clazz;
        PyErr_SetString( PyExc_RuntimeError, buf.getStr() );
        return nullptr;
    }
    PyRef instance( PyObject_CallObject( code.get(), args.get()  ), SAL_NO_ACQUIRE);
    Py_XINCREF( instance.get() );
    return instance.get();

}


PyObject *PyUNO_Enum_new( const char *enumBase, const char *enumValue, const Runtime &r )
{
    PyRef args( PyTuple_New( 2 ), SAL_NO_ACQUIRE, NOT_NULL );
    PyTuple_SetItem( args.get() , 0 , PyUnicode_FromString( enumBase ) );
    PyTuple_SetItem( args.get() , 1 , PyUnicode_FromString( enumValue ) );

    return callCtor( r, "Enum" , args );
}


PyObject* PyUNO_Type_new (const char *typeName , TypeClass t , const Runtime &r )
{
    // retrieve type object
    PyRef args(PyTuple_New( 2 ), SAL_NO_ACQUIRE, NOT_NULL);

    PyTuple_SetItem( args.get() , 0 , PyUnicode_FromString( typeName ) );
    PyObject *typeClass = PyUNO_Enum_new( "com.sun.star.uno.TypeClass" , typeClassToString(t), r );
    if( ! typeClass )
        return nullptr;
    PyTuple_SetItem( args.get() , 1 , typeClass);

    return callCtor( r, "Type" , args );
}

PyObject* PyUNO_char_new ( sal_Unicode val , const Runtime &r )
{
    // retrieve type object
    PyRef args( PyTuple_New( 1 ), SAL_NO_ACQUIRE, NOT_NULL );
    static_assert(sizeof(sal_Unicode) == sizeof(Py_UCS2), "unexpected size");
    PyTuple_SetItem( args.get() , 0 , PyUnicode_FromKindAndData( PyUnicode_2BYTE_KIND, &val ,1) );

    return callCtor( r, "Char" , args );
}

PyObject *PyUNO_ByteSequence_new(
    const css::uno::Sequence< sal_Int8 > &byteSequence, const Runtime &r )
{
    PyRef str(
        PyBytes_FromStringAndSize( reinterpret_cast<char const *>(byteSequence.getConstArray()), byteSequence.getLength()),
        SAL_NO_ACQUIRE );
    PyRef args( PyTuple_New( 1 ), SAL_NO_ACQUIRE, NOT_NULL );
    PyTuple_SetItem( args.get() , 0 , str.getAcquired() );
    return callCtor( r, "ByteSequence" , args );

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
