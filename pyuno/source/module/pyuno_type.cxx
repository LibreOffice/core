/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#include "pyuno_impl.hxx"

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <typelib/typedescription.hxx>

using rtl::OString;
using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OUStringToOString;
using rtl::OStringBuffer;

using com::sun::star::uno::TypeClass;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Any;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Reference;
using com::sun::star::uno::TypeDescription;

#define USTR_ASCII(x) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )
namespace pyuno
{
const char *typeClassToString( TypeClass t )
{
    const char * ret = 0;
    switch (t)
    {
    case com::sun::star::uno::TypeClass_VOID:
        ret = "VOID"; break;
    case com::sun::star::uno::TypeClass_CHAR:
        ret = "CHAR"; break;
    case com::sun::star::uno::TypeClass_BOOLEAN:
        ret = "BOOLEAN"; break;
    case com::sun::star::uno::TypeClass_BYTE:
        ret = "BYTE"; break;
    case com::sun::star::uno::TypeClass_SHORT:
        ret = "SHORT"; break;
    case com::sun::star::uno::TypeClass_UNSIGNED_SHORT:
        ret = "UNSIGNED_SHORT"; break;
    case com::sun::star::uno::TypeClass_LONG:
        ret = "LONG"; break;
    case com::sun::star::uno::TypeClass_UNSIGNED_LONG:
        ret = "UNSIGNED_LONG"; break;
    case com::sun::star::uno::TypeClass_HYPER:
        ret = "HYPER"; break;
    case com::sun::star::uno::TypeClass_UNSIGNED_HYPER:
        ret = "UNSIGNED_HYPER"; break;
    case com::sun::star::uno::TypeClass_FLOAT:
        ret = "FLOAT"; break;
    case com::sun::star::uno::TypeClass_DOUBLE:
        ret = "DOUBLE"; break;
    case com::sun::star::uno::TypeClass_STRING:
        ret = "STRING"; break;
    case com::sun::star::uno::TypeClass_TYPE:
        ret = "TYPE"; break;
    case com::sun::star::uno::TypeClass_ANY:
        ret = "ANY";break;
    case com::sun::star::uno::TypeClass_ENUM:
        ret = "ENUM";break;
    case com::sun::star::uno::TypeClass_STRUCT:
        ret = "STRUCT"; break;
    case com::sun::star::uno::TypeClass_EXCEPTION:
        ret = "EXCEPTION"; break;
    case com::sun::star::uno::TypeClass_SEQUENCE:
        ret = "SEQUENCE"; break;
    case com::sun::star::uno::TypeClass_INTERFACE:
        ret = "INTERFACE"; break;
    case com::sun::star::uno::TypeClass_TYPEDEF:
        ret = "TYPEDEF"; break;
    case com::sun::star::uno::TypeClass_UNION:
        ret = "UNION"; break;
    case com::sun::star::uno::TypeClass_ARRAY:
        ret = "ARRAY"; break;
    case com::sun::star::uno::TypeClass_SERVICE:
        ret = "SERVICE"; break;
    case com::sun::star::uno::TypeClass_MODULE:
        ret = "MODULE"; break;
    case com::sun::star::uno::TypeClass_INTERFACE_METHOD:
        ret = "INTERFACE_METHOD"; break;
    case com::sun::star::uno::TypeClass_INTERFACE_ATTRIBUTE:
        ret = "INTERFACE_ATTRIBUTE"; break;
    default:
        ret = "UNKNOWN"; break;
    }
    return ret;
}

static PyRef getClass( const Runtime & r , const char * name)
{
    return PyRef( PyDict_GetItemString( r.getImpl()->cargo->getUnoModule().get(), (char*) name ) );
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


sal_Unicode PyChar2Unicode( PyObject *obj ) throw ( RuntimeException )
{
    PyRef value( PyObject_GetAttrString( obj, const_cast< char * >("value") ), SAL_NO_ACQUIRE );
    if( ! PyUnicode_Check( value.get() ) )
    {
        throw RuntimeException(
            USTR_ASCII( "attribute value of uno.Char is not a unicode string" ),
            Reference< XInterface > () );
    }

    if( PyUnicode_GetSize( value.get() ) < 1 )
    {
        throw RuntimeException(
            USTR_ASCII( "uno.Char contains an empty unicode string" ),
            Reference< XInterface > () );
    }
#if PY_VERSION_HEX >= 0x03030000
    sal_Unicode c = (sal_Unicode)PyUnicode_ReadChar( value.get(), 0 );
#else
    sal_Unicode c = (sal_Unicode)PyUnicode_AsUnicode( value.get() )[0];
#endif
    return c;
}

Any PyEnum2Enum( PyObject *obj ) throw ( RuntimeException )
{
    Any ret;
    PyRef typeName( PyObject_GetAttrString( obj,const_cast< char * >("typeName") ), SAL_NO_ACQUIRE);
    PyRef value( PyObject_GetAttrString( obj, const_cast< char * >("value") ), SAL_NO_ACQUIRE);
    if( !PYSTR_CHECK( typeName.get() ) || ! PYSTR_CHECK( value.get() ) )
    {
        throw RuntimeException(
            USTR_ASCII( "attributes typeName and/or value of uno.Enum are not strings" ),
            Reference< XInterface > () );
    }

    OUString strTypeName( pyString2ustring( typeName.get() ) );
    OUString strValue( pyString2ustring( value.get() ) );

    TypeDescription desc( strTypeName );
    if( desc.is() )
    {
        if(desc.get()->eTypeClass != typelib_TypeClass_ENUM )
        {
            OUStringBuffer buf;
            buf.appendAscii( "pyuno.checkEnum: " ).append( strTypeName ).appendAscii( " is a " );
            buf.appendAscii(
                typeClassToString( (com::sun::star::uno::TypeClass) desc.get()->eTypeClass));
            buf.appendAscii( ", expected ENUM" );
            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface>  () );
        }

        desc.makeComplete();

        typelib_EnumTypeDescription *pEnumDesc = (typelib_EnumTypeDescription*) desc.get();
        int i = 0;
        for( i = 0; i < pEnumDesc->nEnumValues ; i ++ )
        {
            if( (*((OUString *)&pEnumDesc->ppEnumNames[i])).compareTo( strValue ) == 0 )
            {
                break;
            }
        }
        if( i == pEnumDesc->nEnumValues )
        {
            OUStringBuffer buf;
            buf.appendAscii( "value " ).append( strValue ).appendAscii( " is unknown in enum " );
            buf.append( strTypeName );
            throw RuntimeException( buf.makeStringAndClear(), Reference<XInterface> () );
        }
        ret = Any( &pEnumDesc->pEnumValues[i], desc.get()->pWeakRef );
    }
    else
    {
        OUStringBuffer buf;
        buf.appendAscii( "enum " ).append( strTypeName ).appendAscii( " is unknown" );
        throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface>  () );
    }
    return ret;
}


Type PyType2Type( PyObject * o ) throw(RuntimeException )
{
    PyRef pyName( PyObject_GetAttrString( o, const_cast< char * >("typeName") ), SAL_NO_ACQUIRE);
    if( !PYSTR_CHECK( pyName.get() ) )
    {
        throw RuntimeException(
            USTR_ASCII( "type object does not have typeName property" ),
            Reference< XInterface > () );
    }

    PyRef pyTC( PyObject_GetAttrString( o, const_cast< char * >("typeClass") ), SAL_NO_ACQUIRE );
    Any enumValue = PyEnum2Enum( pyTC.get() );

    OUString name( pyString2ustring( pyName.get() ) );
    TypeDescription desc( name );
    if( ! desc.is() )
    {
        OUStringBuffer buf;
        buf.appendAscii( "type " ).append(name).appendAscii( " is unknown" );
        throw RuntimeException(
            buf.makeStringAndClear(), Reference< XInterface > () );
    }
    if( desc.get()->eTypeClass != (typelib_TypeClass) *(sal_Int32*)enumValue.getValue() )
    {
        OUStringBuffer buf;
        buf.appendAscii( "pyuno.checkType: " ).append(name).appendAscii( " is a " );
        buf.appendAscii( typeClassToString( (TypeClass) desc.get()->eTypeClass) );
        buf.appendAscii( ", but type got construct with typeclass " );
        buf.appendAscii( typeClassToString( (TypeClass) *(sal_Int32*)enumValue.getValue() ) );
        throw RuntimeException(
            buf.makeStringAndClear(), Reference< XInterface > () );
    }
    return desc.get()->pWeakRef;
}

PyObject *importToGlobal(PyObject *str, PyObject *dict, PyObject *target)
{
    // maybe a constant ?
    PyObject *ret = 0;
    OUString name = pyString2ustring(str);
    try
    {
        Runtime runtime;
        TypeDescription desc(name );
        desc.makeComplete();
        if( desc.is() )
        {
            com::sun::star::uno::TypeClass tc =
                (com::sun::star::uno::TypeClass)desc.get()->eTypeClass;

            PyRef typesModule( PyDict_GetItemString( dict, "unotypes" ) );
            if( ! typesModule.is() || ! PyModule_Check( typesModule.get() ))
            {
                typesModule = PyRef( PyModule_New( const_cast< char * >("unotypes") ), SAL_NO_ACQUIRE );
                Py_INCREF( typesModule.get() );
                PyDict_SetItemString( dict, "unotypes" , typesModule.get() );
            }
#if PY_VERSION_HEX >= 0x03030000
            const char *targetName = PyUnicode_AsUTF8( target );
            const char *typeName = PyUnicode_AsUTF8( str );
#elif PY_MAJOR_VERSION > 3
            PyRef pUtf8( PyUnicode_AsUTF8String( target ), SAL_NO_ACQUIRE );
            const char *targetName = PyBytes_AsString( pUtf8.get() );
            PyRef pTypeName( PyUnicode_AsUTF8String( str ), SAL_NO_ACQUIRE );
            const char *typeName = PyBytes_AsString( pTypeName.get() );
#else
            /*const*/ char *targetName = PyBytes_AsString( target );
            const char *typeName = PyBytes_AsString( str );
#endif
            PyModule_AddObject(
                typesModule.get(),
                targetName,
                PyUNO_Type_new( typeName, tc, runtime ) );

            if( com::sun::star::uno::TypeClass_EXCEPTION == tc ||
                com::sun::star::uno::TypeClass_STRUCT    == tc )
            {
                PyRef exc = getClass( name, runtime );
                PyDict_SetItem( dict, target, exc.getAcquired() );
            }
            else if( com::sun::star::uno::TypeClass_ENUM == tc )
            {
                // introduce all enums into the dictionary !
                typelib_EnumTypeDescription *pDesc =
                    (typelib_EnumTypeDescription *) desc.get();
                for( int i = 0 ; i < pDesc->nEnumValues; i ++ )
                {
                    OString enumElementName(
                        OUStringToOString( pDesc->ppEnumNames[i], RTL_TEXTENCODING_ASCII_US) );
#if PY_VERSION_HEX >= 0x03030000
                    const char *name = PyUnicode_AsUTF8(str);
#elif PY_MAJOR_VERSION > 3
                    PyRef *pUtf8( PyUnicode_AsUTF8String( str ), SAL_NO_ACQUIRE );
                    const char *name = PyBytes_AsString( pUtf8.get() );
#else
                    const char *name = PyBytes_AsString(str);
#endif
                    PyDict_SetItemString(
                        dict, (char*)enumElementName.getStr(),
                        PyUNO_Enum_new(name, enumElementName.getStr(), runtime ) );
                }
            }
            Py_INCREF( Py_None );
            ret = Py_None;
        }
        else
        {
            Any a = runtime.getImpl()->cargo->xTdMgr->getByHierarchicalName(name);
            if(a.hasValue())
            {
                PyRef constant = runtime.any2PyObject( a );
                if( constant.is() )
                {
                    Py_INCREF( constant.get() );
                    PyDict_SetItem( dict, target , constant.get());
                    ret = constant.get();
                }
                else
                {
                    OUStringBuffer buf;
                    buf.appendAscii( "constant " ).append(pyString2ustring(str)).appendAscii(  " unknown" );
                    PyErr_SetString(
                        PyExc_RuntimeError,
                        OUStringToOString( buf.makeStringAndClear(), RTL_TEXTENCODING_UTF8).getStr() );
                }
            }
            else
            {
                OUStringBuffer buf;
                buf.appendAscii( "pyuno.imp unknown type " );
                buf.append( name );
                PyErr_SetString(
                    PyExc_RuntimeError,
                    OUStringToOString( buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US).getStr() );
            }
        }
    }
    catch( com::sun::star::container::NoSuchElementException & )
    {
        OUStringBuffer buf;
        buf.appendAscii( "pyuno.imp unknown type " );
        buf.append( name );
        PyErr_SetString(
            PyExc_RuntimeError,
            OUStringToOString( buf.makeStringAndClear(), RTL_TEXTENCODING_ASCII_US).getStr() );
    }
    catch( com::sun::star::script::CannotConvertException & e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( com::sun::star::lang::IllegalArgumentException & e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( RuntimeException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ));
    }
    return ret;
}

static PyObject* callCtor( const Runtime &r , const char * clazz, const PyRef & args )
{
    PyRef code( PyDict_GetItemString( r.getImpl()->cargo->getUnoModule().get(), (char*)clazz ) );
    if( ! code.is() )
    {
        OStringBuffer buf;
        buf.append( "couldn't access uno." );
        buf.append( clazz );
        PyErr_SetString( PyExc_RuntimeError, buf.getStr() );
        return NULL;
    }
    PyRef instance( PyObject_CallObject( code.get(), args.get()  ), SAL_NO_ACQUIRE);
    Py_XINCREF( instance.get() );
    return instance.get();

}


PyObject *PyUNO_Enum_new( const char *enumBase, const char *enumValue, const Runtime &r )
{
    PyRef args( PyTuple_New( 2 ), SAL_NO_ACQUIRE );
    PyTuple_SetItem( args.get() , 0 , PYSTR_FROMSTR( enumBase ) );
    PyTuple_SetItem( args.get() , 1 , PYSTR_FROMSTR( enumValue ) );

    return callCtor( r, "Enum" , args );
}


PyObject* PyUNO_Type_new (const char *typeName , TypeClass t , const Runtime &r )
{
    // retrieve type object
    PyRef args( PyTuple_New( 2 ), SAL_NO_ACQUIRE );

    PyTuple_SetItem( args.get() , 0 , PYSTR_FROMSTR( typeName ) );
    PyObject *typeClass = PyUNO_Enum_new( "com.sun.star.uno.TypeClass" , typeClassToString(t), r );
    if( ! typeClass )
        return NULL;
    PyTuple_SetItem( args.get() , 1 , typeClass);

    return callCtor( r, "Type" , args );
}

PyObject* PyUNO_char_new ( sal_Unicode val , const Runtime &r )
{
    // retrieve type object
    PyRef args( PyTuple_New( 1 ), SAL_NO_ACQUIRE );

#if PY_VERSION_HEX >= 0x03030000
    Py_UCS2 u[1];
    u[0] = val;
    PyTuple_SetItem( args.get(), 0, PyUnicode_FromKindAndData( PyUnicode_2BYTE_KIND, u, 1 ) );
#else
    Py_UNICODE u[2];
    u[0] = val;
    u[1] = 0;
    PyTuple_SetItem( args.get() , 0 , PyUnicode_FromUnicode( u ,1) );
#endif

    return callCtor( r, "Char" , args );
}

PyObject *PyUNO_ByteSequence_new(
    const com::sun::star::uno::Sequence< sal_Int8 > &byteSequence, const Runtime &r )
{
    PyRef str(
        PyBytes_FromStringAndSize( (char*)byteSequence.getConstArray(), byteSequence.getLength()),
        SAL_NO_ACQUIRE );
    PyRef args( PyTuple_New( 1 ), SAL_NO_ACQUIRE );
    PyTuple_SetItem( args.get() , 0 , str.getAcquired() );
    return callCtor( r, "ByteSequence" , args );

}
}
