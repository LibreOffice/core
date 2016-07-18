/* -*- Mode: C++; eval:(c-set-style "bsd"); tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <config_features.h>
#include <config_folders.h>

#include "pyuno_impl.hxx"

#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/module.h>
#include <osl/process.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>

#include <list>
#include <typelib/typedescription.hxx>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/script/InvocationAdapterFactory.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>
#include <comphelper/sequence.hxx>


using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;
using com::sun::star::uno::TypeDescription;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Type;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;
using com::sun::star::lang::WrappedTargetRuntimeException;
using com::sun::star::lang::XSingleServiceFactory;
using com::sun::star::lang::XUnoTunnel;
using com::sun::star::reflection::theCoreReflection;
using com::sun::star::reflection::InvocationTargetException;
using com::sun::star::script::Converter;
using com::sun::star::script::XTypeConverter;
using com::sun::star::script::XInvocation;
using com::sun::star::beans::XMaterialHolder;
using com::sun::star::beans::theIntrospection;

#include <vector>

namespace pyuno
{

static PyTypeObject RuntimeImpl_Type =
{
    PyVarObject_HEAD_INIT (&PyType_Type, 0)
    "pyuno_runtime",
    sizeof (RuntimeImpl),
    0,
    RuntimeImpl::del,
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
#if PY_VERSION_HEX >= 0x02060000
    , 0
#endif
#if PY_VERSION_HEX >= 0x03040000
    , nullptr
#endif
};

/*----------------------------------------------------------------------
  Runtime implementation
 -----------------------------------------------------------------------*/
static void getRuntimeImpl( PyRef & globalDict, PyRef &runtimeImpl )
    throw ( css::uno::RuntimeException )
{
    PyThreadState * state = PyThreadState_Get();
    if( ! state )
    {
        throw RuntimeException( "python global interpreter must be held (thread must be attached)" );
    }

    PyObject* pModule = PyImport_AddModule("__main__");

    if (!pModule)
    {
        throw RuntimeException("can't import __main__ module");
    }

    globalDict = PyRef( PyModule_GetDict(pModule));

    if( ! globalDict.is() ) // FATAL !
    {
        throw RuntimeException("can't find __main__ module");
    }
    runtimeImpl = PyDict_GetItemString( globalDict.get() , "pyuno_runtime" );
}

static PyRef importUnoModule( ) throw ( RuntimeException )
{
    // import the uno module
    PyRef module( PyImport_ImportModule( "uno" ), SAL_NO_ACQUIRE, NOT_NULL );
    if( PyErr_Occurred() )
    {
        PyRef excType, excValue, excTraceback;
        PyErr_Fetch( reinterpret_cast<PyObject **>(&excType), reinterpret_cast<PyObject**>(&excValue), reinterpret_cast<PyObject**>(&excTraceback));
        // As of Python 2.7 this gives a rather non-useful "<traceback object at 0xADDRESS>",
        // but it is the best we can do in the absence of uno._uno_extract_printable_stacktrace
        // Who knows, a future Python might print something better.
        PyRef str( PyObject_Str( excTraceback.get() ), SAL_NO_ACQUIRE );

        OUStringBuffer buf;
        buf.append( "python object raised an unknown exception (" );
        PyRef valueRep( PyObject_Repr( excValue.get() ), SAL_NO_ACQUIRE );
        buf.appendAscii( PyStr_AsString( valueRep.get())).append( ", traceback follows\n" );
        buf.appendAscii( PyStr_AsString( str.get() ) );
        buf.append( ")" );
        throw RuntimeException( buf.makeStringAndClear() );
    }
    PyRef dict( PyModule_GetDict( module.get() ) );
    return dict;
}

static void readLoggingConfig( sal_Int32 *pLevel, FILE **ppFile )
{
    *pLevel = LogLevel::NONE;
    *ppFile = nullptr;
    OUString fileName;
    osl_getModuleURLFromFunctionAddress(
        reinterpret_cast< oslGenericFunction >(readLoggingConfig),
        &fileName.pData );
    fileName = fileName.copy( fileName.lastIndexOf( '/' )+1 );
#ifdef MACOSX
    fileName += "../" LIBO_ETC_FOLDER "/";
#endif
    fileName += SAL_CONFIGFILE("pyuno" );
    rtl::Bootstrap bootstrapHandle( fileName );

    OUString str;
    if( bootstrapHandle.getFrom( "PYUNO_LOGLEVEL", str ) )
    {
        if ( str == "NONE" )
            *pLevel = LogLevel::NONE;
        else if ( str == "CALL" )
            *pLevel = LogLevel::CALL;
        else if ( str == "ARGS" )
            *pLevel = LogLevel::ARGS;
        else
        {
            fprintf( stderr, "unknown loglevel %s\n",
                     OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );
        }
    }
    if( *pLevel > LogLevel::NONE )
    {
        *ppFile = stdout;
        if( bootstrapHandle.getFrom( "PYUNO_LOGTARGET", str ) )
        {
            if ( str == "stdout" )
                *ppFile = stdout;
            else if ( str == "stderr" )
                *ppFile = stderr;
            else
            {
                oslProcessInfo data;
                data.Size = sizeof( data );
                osl_getProcessInfo(
                    nullptr , osl_Process_IDENTIFIER , &data );
                osl_getSystemPathFromFileURL( str.pData, &str.pData);
                OString o = OUStringToOString( str, osl_getThreadTextEncoding() );
                o += ".";
                o += OString::number( data.Ident );

                *ppFile = fopen( o.getStr() , "w" );
                if ( *ppFile )
                {
                    // do not buffer (useful if e.g. analyzing a crash)
                    setvbuf( *ppFile, nullptr, _IONBF, 0 );
                }
                else
                {
                    fprintf( stderr, "couldn't create file %s\n",
                             OUStringToOString( str, RTL_TEXTENCODING_UTF8 ).getStr() );

                }
            }
        }
    }
}

/*-------------------------------------------------------------------
 RuntimeImpl implementations
 *-------------------------------------------------------------------*/
PyRef stRuntimeImpl::create( const Reference< XComponentContext > &ctx )
    throw( css::uno::RuntimeException, std::exception )
{
    RuntimeImpl *me = PyObject_New (RuntimeImpl, &RuntimeImpl_Type);
    if( ! me )
        throw RuntimeException( "cannot instantiate pyuno::RuntimeImpl" );
    me->cargo = nullptr;
    // must use a different struct here, as the PyObject_New
    // makes C++ unusable
    RuntimeCargo *c = new RuntimeCargo();
    readLoggingConfig( &(c->logLevel) , &(c->logFile) );
    log( c, LogLevel::CALL, "Instantiating pyuno bridge" );

    c->valid = true;
    c->xContext = ctx;
    c->xInvocation = Reference< XSingleServiceFactory > (
        ctx->getServiceManager()->createInstanceWithContext(
            "com.sun.star.script.Invocation",
            ctx ),
        UNO_QUERY );
    if( ! c->xInvocation.is() )
        throw RuntimeException( "pyuno: couldn't instantiate invocation service" );

    c->xTypeConverter = Converter::create(ctx);
    if( ! c->xTypeConverter.is() )
        throw RuntimeException( "pyuno: couldn't instantiate typeconverter service" );

    c->xCoreReflection = theCoreReflection::get(ctx);

    c->xAdapterFactory = css::script::InvocationAdapterFactory::create(ctx);

    c->xIntrospection = theIntrospection::get(ctx);

    Any a = ctx->getValueByName("/singletons/com.sun.star.reflection.theTypeDescriptionManager");
    a >>= c->xTdMgr;
    if( ! c->xTdMgr.is() )
        throw RuntimeException( "pyuno: couldn't retrieve typedescriptionmanager" );

    me->cargo =c;
    return PyRef( reinterpret_cast< PyObject * > ( me ), SAL_NO_ACQUIRE );
}

void  stRuntimeImpl::del(PyObject* self)
{
    RuntimeImpl *me = reinterpret_cast< RuntimeImpl * > ( self );
    if( me->cargo->logFile )
        fclose( me->cargo->logFile );
    delete me->cargo;
    PyObject_Del (self);
}


void Runtime::initialize( const Reference< XComponentContext > & ctx )
    throw ( RuntimeException, std::exception )
{
    PyRef globalDict, runtime;
    getRuntimeImpl( globalDict , runtime );
    RuntimeImpl *impl = reinterpret_cast< RuntimeImpl * > (runtime.get());

    if( runtime.is() && impl->cargo->valid )
    {
        throw RuntimeException("pyuno runtime has already been initialized before" );
    }
    PyRef keep( RuntimeImpl::create( ctx ) );
    PyDict_SetItemString( globalDict.get(), "pyuno_runtime" , keep.get() );
    Py_XINCREF( keep.get() );
}


bool Runtime::isInitialized() throw ( RuntimeException )
{
    PyRef globalDict, runtime;
    getRuntimeImpl( globalDict , runtime );
    RuntimeImpl *impl = reinterpret_cast< RuntimeImpl * > (runtime.get());
    return runtime.is() && impl->cargo->valid;
}

Runtime::Runtime() throw(  RuntimeException )
    : impl( nullptr )
{
    PyRef globalDict, runtime;
    getRuntimeImpl( globalDict , runtime );
    if( ! runtime.is() )
    {
        throw RuntimeException(
            "pyuno runtime is not initialized, "
            "(the pyuno.bootstrap needs to be called before using any uno classes)" );
    }
    impl = reinterpret_cast< RuntimeImpl * > (runtime.get());
    Py_XINCREF( runtime.get() );
}

Runtime::Runtime( const Runtime & r )
{
    impl = r.impl;
    Py_XINCREF( reinterpret_cast< PyObject * >(impl) );
}

Runtime::~Runtime()
{
    Py_XDECREF( reinterpret_cast< PyObject * >(impl) );
}

Runtime & Runtime::operator = ( const Runtime & r )
{
    PyRef temp( reinterpret_cast< PyObject * >(r.impl) );
    Py_XINCREF( temp.get() );
    Py_XDECREF( reinterpret_cast< PyObject * >(impl) );
    impl = r.impl;
    return *this;
}

PyRef Runtime::any2PyObject (const Any &a ) const
    throw ( css::script::CannotConvertException,
            css::lang::IllegalArgumentException,
            RuntimeException)
{
    if( ! impl->cargo->valid )
    {
        throw RuntimeException("pyuno runtime must be initialized before calling any2PyObject" );
    }

    switch (a.getValueTypeClass ())
    {
    case typelib_TypeClass_VOID:
    {
        Py_INCREF (Py_None);
        return PyRef(Py_None);
    }
    case typelib_TypeClass_CHAR:
    {
        sal_Unicode c = *o3tl::forceAccess<sal_Unicode>(a);
        return PyRef( PyUNO_char_new( c , *this ), SAL_NO_ACQUIRE );
    }
    case typelib_TypeClass_BOOLEAN:
    {
        bool b;
        if ((a >>= b) && b)
            return Py_True;
        else
            return Py_False;
    }
    case typelib_TypeClass_BYTE:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
    case typelib_TypeClass_LONG:
    {
        sal_Int32 l = 0;
        a >>= l;
        return PyRef( PyLong_FromLong (l), SAL_NO_ACQUIRE );
    }
    case typelib_TypeClass_UNSIGNED_LONG:
    {
        sal_uInt32 l = 0;
        a >>= l;
        return PyRef( PyLong_FromUnsignedLong (l), SAL_NO_ACQUIRE );
    }
    case typelib_TypeClass_HYPER:
    {
        sal_Int64 l = 0;
        a >>= l;
        return PyRef( PyLong_FromLongLong (l), SAL_NO_ACQUIRE);
    }
    case typelib_TypeClass_UNSIGNED_HYPER:
    {
        sal_uInt64 l = 0;
        a >>= l;
        return PyRef( PyLong_FromUnsignedLongLong (l), SAL_NO_ACQUIRE);
    }
    case typelib_TypeClass_FLOAT:
    {
        float f = 0.0;
        a >>= f;
        return PyRef(PyFloat_FromDouble (f), SAL_NO_ACQUIRE);
    }
    case typelib_TypeClass_DOUBLE:
    {
        double d = 0.0;
        a >>= d;
        return PyRef( PyFloat_FromDouble (d), SAL_NO_ACQUIRE);
    }
    case typelib_TypeClass_STRING:
    {
        OUString tmp_ostr;
        a >>= tmp_ostr;
        return ustring2PyUnicode( tmp_ostr );
    }
    case typelib_TypeClass_TYPE:
    {
        Type t;
        a >>= t;
        OString o = OUStringToOString( t.getTypeName(), RTL_TEXTENCODING_ASCII_US );
        return PyRef(
            PyUNO_Type_new (
                o.getStr(),  (css::uno::TypeClass)t.getTypeClass(), *this),
            SAL_NO_ACQUIRE);
    }
    case typelib_TypeClass_ANY:
    {
        //I don't think this can happen.
        Py_INCREF (Py_None);
        return Py_None;
    }
    case typelib_TypeClass_ENUM:
    {
        sal_Int32 l = *static_cast<sal_Int32 const *>(a.getValue());
        TypeDescription desc( a.getValueType() );
        if( desc.is() )
        {
            desc.makeComplete();
            typelib_EnumTypeDescription *pEnumDesc =
                reinterpret_cast<typelib_EnumTypeDescription *>(desc.get());
            for( int i = 0 ; i < pEnumDesc->nEnumValues ; i ++ )
            {
                if( pEnumDesc->pEnumValues[i] == l )
                {
                    OString v = OUStringToOString( pEnumDesc->ppEnumNames[i], RTL_TEXTENCODING_ASCII_US);
                    OString e = OUStringToOString( pEnumDesc->aBase.pTypeName, RTL_TEXTENCODING_ASCII_US);
                    return PyRef( PyUNO_Enum_new(e.getStr(),v.getStr(), *this ), SAL_NO_ACQUIRE );
                }
            }
        }
        OUStringBuffer buf;
        buf.append( "Any carries enum " );
        buf.append( a.getValueType().getTypeName());
        buf.append( " with invalid value " ).append( l );
        throw RuntimeException( buf.makeStringAndClear() );
    }
    case typelib_TypeClass_EXCEPTION:
    case typelib_TypeClass_STRUCT:
    {
        PyRef excClass = getClass( a.getValueType().getTypeName(), *this );
        PyRef value = PyUNOStruct_new( a, getImpl()->cargo->xInvocation );
        PyRef argsTuple( PyTuple_New( 1 ) , SAL_NO_ACQUIRE, NOT_NULL );
        PyTuple_SetItem( argsTuple.get() , 0 , value.getAcquired() );
        PyRef ret( PyObject_CallObject( excClass.get() , argsTuple.get() ), SAL_NO_ACQUIRE );
        if( ! ret.is() )
        {
            OUStringBuffer buf;
            buf.append( "Couldn't instantiate python representation of structured UNO type " );
            buf.append( a.getValueType().getTypeName() );
            throw RuntimeException( buf.makeStringAndClear() );
        }

        if( auto e = o3tl::tryAccess<css::uno::Exception>(a) )
        {
            // add the message in a standard python way !
            PyRef args( PyTuple_New( 1 ), SAL_NO_ACQUIRE, NOT_NULL );

            PyRef pymsg = ustring2PyString( e->Message );
            PyTuple_SetItem( args.get(), 0 , pymsg.getAcquired() );
            // the exception base functions want to have an "args" tuple,
            // which contains the message
            PyObject_SetAttrString( ret.get(), "args", args.get() );
        }
        return ret;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        Sequence<Any> s;

        Sequence< sal_Int8 > byteSequence;
        if( a >>= byteSequence )
        {
            // byte sequence is treated in a special way because of peformance reasons
            // @since 0.9.2
            return PyRef( PyUNO_ByteSequence_new( byteSequence, *this ), SAL_NO_ACQUIRE );
        }
        else
        {
            Reference< XTypeConverter > tc = getImpl()->cargo->xTypeConverter;
            Reference< XSingleServiceFactory > ssf = getImpl()->cargo->xInvocation;
            tc->convertTo (a, cppu::UnoType<decltype(s)>::get()) >>= s;
            PyRef tuple( PyTuple_New (s.getLength()), SAL_NO_ACQUIRE, NOT_NULL);
            int i=0;
            try
            {
                for ( i = 0; i < s.getLength (); i++)
                {
                    PyRef element;
                    element = any2PyObject (tc->convertTo (s[i], s[i].getValueType() ));
                    OSL_ASSERT( element.is() );
                    PyTuple_SetItem( tuple.get(), i, element.getAcquired() );
                }
            }
            catch( css::uno::Exception & )
            {
                for( ; i < s.getLength() ; i ++ )
                {
                    Py_INCREF( Py_None );
                    PyTuple_SetItem( tuple.get(), i,  Py_None );
                }
                throw;
            }
            return tuple;
        }
    }
    case typelib_TypeClass_INTERFACE:
    {
        Reference<XInterface> tmp_interface;
        a >>= tmp_interface;
        if (!tmp_interface.is ())
            return Py_None;

        return PyUNO_new( a, getImpl()->cargo->xInvocation );
    }
    default:
    {
        OUStringBuffer buf;
        buf.append( "Unknown UNO type class " );
        buf.append( (sal_Int32 ) a.getValueTypeClass() );
        throw RuntimeException(buf.makeStringAndClear( ) );
    }
    }
}

static Sequence< Type > invokeGetTypes( const Runtime & r , PyObject * o )
{
    Sequence< Type > ret;

    PyRef method( PyObject_GetAttrString( o , "getTypes" ), SAL_NO_ACQUIRE );
    raiseInvocationTargetExceptionWhenNeeded( r );
    if( method.is() && PyCallable_Check( method.get() ) )
    {
        PyRef types( PyObject_CallObject( method.get(), nullptr ) , SAL_NO_ACQUIRE );
        raiseInvocationTargetExceptionWhenNeeded( r );
        if( types.is() && PyTuple_Check( types.get() ) )
        {
            int size = PyTuple_Size( types.get() );

            // add the XUnoTunnel interface  for uno object identity concept (hack)
            ret.realloc( size + 1 );
            for( int i = 0 ; i < size ; i ++ )
            {
                Any a = r.pyObject2Any(PyTuple_GetItem(types.get(),i));
                a >>= ret[i];
            }
            ret[size] = cppu::UnoType<css::lang::XUnoTunnel>::get();
        }
    }
    return ret;
}

static OUString
lcl_ExceptionMessage(PyObject *const o, OUString const*const pWrapped)
{
    OUStringBuffer buf;
    buf.append("Couldn't convert ");
    PyRef reprString( PyObject_Str(o), SAL_NO_ACQUIRE );
    buf.appendAscii( PyStr_AsString(reprString.get()) );
    buf.append(" to a UNO type");
    if (pWrapped)
    {
        buf.append("; caught exception: ");
        buf.append(*pWrapped);
    }
    return buf.makeStringAndClear();
}

// For Python 2.7 - see https://bugs.python.org/issue24161
// Fills aSeq and returns true if pObj is a valid iterator
bool Runtime::pyIterUnpack( PyObject *const pObj, Any &a ) const
{
    if( !PyIter_Check( pObj ))
        return false;

    PyObject *pItem = PyIter_Next( pObj );
    if( !pItem )
    {
        if( PyErr_Occurred() )
        {
            PyErr_Clear();
            return false;
        }
        return true;
    }

    ::std::list<Any> items;
    do
    {
        PyRef rItem( pItem, SAL_NO_ACQUIRE );
        items.push_back( pyObject2Any( rItem.get() ) );
    }
    while( (pItem = PyIter_Next( pObj )) );
    a <<= comphelper::containerToSequence<Any>(items);
    return true;
}

Any Runtime::pyObject2Any ( const PyRef & source, enum ConversionMode mode ) const
    throw ( css::uno::RuntimeException )
{
    if( ! impl->cargo->valid )
    {
        throw RuntimeException("pyuno runtime must be initialized before calling any2PyObject" );
    }

    Any a;
    PyObject *o = source.get();
    if( Py_None == o )
    {

    }
    // In Python 3, there is no PyInt type.
#if PY_MAJOR_VERSION < 3
    else if (PyInt_Check (o))
    {
        if( o == Py_True )
        {
            a <<= true;
        }
        else if ( o == Py_False )
        {
            a <<= false;
        }
        else
        {
            sal_Int32 l = (sal_Int32) PyLong_AsLong( o );
            if( l < 128 && l >= -128 )
            {
                sal_Int8 b = (sal_Int8 ) l;
                a <<= b;
            }
            else if( l <= 0x7fff && l >= -0x8000 )
            {
                sal_Int16 s = (sal_Int16) l;
                a <<= s;
            }
            else
            {
                a <<= l;
            }
        }
    }
#endif /* PY_MAJOR_VERSION < 3 */
    else if (PyLong_Check (o))
    {
#if PY_MAJOR_VERSION >= 3
        // Convert the Python 3 booleans that are actually of type PyLong.
        if(o == Py_True)
        {
            a <<= true;
        }
        else if(o == Py_False)
        {
            a <<= false;
        }
        else
        {
#endif /* PY_MAJOR_VERSION >= 3 */
        sal_Int64 l = (sal_Int64)PyLong_AsLong (o);
        if( l < 128 && l >= -128 )
        {
            sal_Int8 b = (sal_Int8 ) l;
            a <<= b;
        }
        else if( l <= 0x7fff && l >= -0x8000 )
        {
            sal_Int16 s = (sal_Int16) l;
            a <<= s;
        }
        else if( l <= SAL_CONST_INT64(0x7fffffff) &&
                 l >= -SAL_CONST_INT64(0x80000000) )
        {
            sal_Int32 l32 = (sal_Int32) l;
            a <<= l32;
        }
        else
        {
            a <<= l;
        }
#if PY_MAJOR_VERSION >= 3
        }
#endif
    }
    else if (PyFloat_Check (o))
    {
        double d = PyFloat_AsDouble (o);
        a <<= d;
    }
    else if (PyStrBytes_Check(o) || PyUnicode_Check(o))
    {
        a <<= pyString2ustring(o);
    }
    else if (PyTuple_Check (o))
    {
        Sequence<Any> s (PyTuple_Size (o));
        for (Py_ssize_t i = 0; i < PyTuple_Size (o); i++)
        {
            s[i] = pyObject2Any (PyTuple_GetItem (o, i), mode );
        }
        a <<= s;
    }
    else if (PyList_Check (o))
    {
        Py_ssize_t l = PyList_Size (o);
        Sequence<Any> s (l);
        for (Py_ssize_t i = 0; i < l; i++)
        {
            s[i] = pyObject2Any (PyList_GetItem (o, i), mode );
        }
        a <<= s;
    }
    else if (!pyIterUnpack (o, a))
    {
        Runtime runtime;
        // should be removed, in case ByteSequence gets derived from String
        if( PyObject_IsInstance( o, getByteSequenceClass( runtime ).get() ) )
        {
            PyRef str(PyObject_GetAttrString( o , "value" ),SAL_NO_ACQUIRE);
            Sequence< sal_Int8 > seq;
            if( PyStrBytes_Check( str.get() ) )
            {
                seq = Sequence<sal_Int8 > (
                    reinterpret_cast<sal_Int8*>(PyStrBytes_AsString(str.get())), PyStrBytes_Size(str.get()));
            }
            a <<= seq;
        }
        else
        if( PyObject_IsInstance( o, getTypeClass( runtime ).get() ) )
        {
            Type t = PyType2Type( o );
            a <<= t;
        }
        else if( PyObject_IsInstance( o, getEnumClass( runtime ).get() ) )
        {
            a = PyEnum2Enum( o );
        }
        else if( isInstanceOfStructOrException( o ) )
        {
            PyRef struc(PyObject_GetAttrString( o , "value" ),SAL_NO_ACQUIRE);
            PyUNO * obj = reinterpret_cast<PyUNO*>(struc.get());
            Reference< XMaterialHolder > holder( obj->members->xInvocation, UNO_QUERY );
            if( holder.is( ) )
                a = holder->getMaterial();
            else
            {
                throw RuntimeException(
                    "struct or exception wrapper does not support XMaterialHolder" );
            }
        }
        else if( PyObject_IsInstance( o, getPyUnoClass().get() ) )
        {
            PyUNO* o_pi = reinterpret_cast<PyUNO*>(o);
            a = o_pi->members->wrappedObject;
        }
        else if( PyObject_IsInstance( o, getPyUnoStructClass().get() ) )
        {
            PyUNO* o_pi = reinterpret_cast<PyUNO*>(o);
            Reference<XMaterialHolder> my_mh (o_pi->members->xInvocation, UNO_QUERY);

            if (!my_mh.is())
            {
                throw RuntimeException(
                    "struct wrapper does not support XMaterialHolder" );
            }
            else
                a = my_mh->getMaterial();
        }
        else if( PyObject_IsInstance( o, getCharClass( runtime ).get() ) )
        {
            a <<= PyChar2Unicode( o );
        }
        else if( PyObject_IsInstance( o, getAnyClass( runtime ).get() ) )
        {
            if( ACCEPT_UNO_ANY == mode )
            {
                a = pyObject2Any( PyRef( PyObject_GetAttrString( o , "value" ), SAL_NO_ACQUIRE) );
                Type t;
                pyObject2Any( PyRef( PyObject_GetAttrString( o, "type" ), SAL_NO_ACQUIRE ) ) >>= t;

                try
                {
                    a = getImpl()->cargo->xTypeConverter->convertTo( a, t );
                }
                catch( const css::uno::Exception & e )
                {
                    throw WrappedTargetRuntimeException(
                            e.Message, e.Context, makeAny(e));
                }
            }
            else
            {
                throw RuntimeException(
                    "uno.Any instance not accepted during method call, "
                    "use uno.invoke instead" );
            }
        }
        else
        {
            Reference< XInterface > mappedObject;
            Reference< XInvocation > adapterObject;

            // instance already mapped out to the world ?
            PyRef2Adapter::iterator ii = impl->cargo->mappedObjects.find( PyRef( o ) );
            if( ii != impl->cargo->mappedObjects.end() )
            {
                adapterObject = ii->second;
            }

            if( adapterObject.is() )
            {
                // object got already bridged !
                Reference< css::lang::XUnoTunnel > tunnel( adapterObject, UNO_QUERY );

                Adapter *pAdapter = reinterpret_cast<Adapter*>(
                        tunnel->getSomething(
                            ::pyuno::Adapter::getUnoTunnelImplementationId() ) );

                mappedObject = impl->cargo->xAdapterFactory->createAdapter(
                    adapterObject, pAdapter->getWrappedTypes() );
            }
            else
            {
                try {
                    Sequence<Type> interfaces = invokeGetTypes(*this, o);
                    if (interfaces.getLength())
                    {
                        Adapter *pAdapter = new Adapter( o, interfaces );
                        mappedObject =
                            getImpl()->cargo->xAdapterFactory->createAdapter(
                                pAdapter, interfaces );

                        // keep a list of exported objects to ensure object identity !
                        impl->cargo->mappedObjects[ PyRef(o) ] =
                            css::uno::WeakReference< XInvocation > ( pAdapter );
                    }
                } catch (InvocationTargetException const& e) {
                    OUString const msg(lcl_ExceptionMessage(o, &e.Message));
                    throw WrappedTargetRuntimeException( // re-wrap that
                            msg, e.Context, e.TargetException);
                }
            }
            if( mappedObject.is() )
            {
                a = css::uno::makeAny( mappedObject );
            }
            else
            {
                OUString const msg(lcl_ExceptionMessage(o, nullptr));
                throw RuntimeException(msg);
            }
        }
    }
    return a;
}

Any Runtime::extractUnoException( const PyRef & excType, const PyRef &excValue, const PyRef &excTraceback) const
{
    OUString str;
    Any ret;
    if( excTraceback.is() )
    {
        Exception e;
        PyRef unoModule;
        if ( impl )
        {
            try
            {
                unoModule = impl->cargo->getUnoModule();
            }
            catch (const Exception &ei)
            {
                e=ei;
            }
        }
        if( unoModule.is() )
        {
            PyRef extractTraceback(
                PyDict_GetItemString(unoModule.get(),"_uno_extract_printable_stacktrace" ) );

            if( PyCallable_Check(extractTraceback.get()) )
            {
                PyRef args( PyTuple_New( 1), SAL_NO_ACQUIRE, NOT_NULL );
                PyTuple_SetItem( args.get(), 0, excTraceback.getAcquired() );
                PyRef pyStr( PyObject_CallObject( extractTraceback.get(),args.get() ), SAL_NO_ACQUIRE);
                str = OUString::createFromAscii( PyStr_AsString(pyStr.get()) );
            }
            else
            {
                str = "Couldn't find uno._uno_extract_printable_stacktrace";
            }
        }
        else
        {
            str = "Could not load uno.py, no stacktrace available";
            if ( !e.Message.isEmpty() )
            {
                str += " (Error loading uno.py: " + e.Message + ")";
            }
        }

    }
    else
    {
        // it may occur, that no traceback is given (e.g. only native code below)
        str = "no traceback available";
    }

    if( isInstanceOfStructOrException( excValue.get() ) )
    {
        ret = pyObject2Any( excValue );
    }
    else
    {
        OUStringBuffer buf;
        PyRef typeName( PyObject_Str( excType.get() ), SAL_NO_ACQUIRE );
        if( typeName.is() )
        {
            buf.appendAscii( PyStr_AsString( typeName.get() ) );
        }
        else
        {
            buf.append( "no typename available" );
        }
        buf.append( ": " );
        PyRef valueRep( PyObject_Str( excValue.get() ), SAL_NO_ACQUIRE );
        if( valueRep.is() )
        {
            buf.appendAscii( PyStr_AsString( valueRep.get()));
        }
        else
        {
            buf.append( "Couldn't convert exception value to a string" );
        }
        buf.append( ", traceback follows\n" );
        if( !str.isEmpty() )
        {
            buf.append( str );
            buf.append( "\n" );
        }
        else
        {
            buf.append( ", no traceback available\n" );
        }
        RuntimeException e;
        e.Message = buf.makeStringAndClear();
#if OSL_DEBUG_LEVEL > 0
        fprintf( stderr, "Python exception: %s\n",
                 OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr() );
#endif
        ret = css::uno::makeAny( e );
    }
    return ret;
}


PyThreadAttach::PyThreadAttach( PyInterpreterState *interp)
    throw ( css::uno::RuntimeException )
{
    tstate = PyThreadState_New( interp );
    if( !tstate  )
        throw RuntimeException( "Couldn't create a pythreadstate" );
    PyEval_AcquireThread( tstate);
}

PyThreadAttach::~PyThreadAttach()
{
    PyThreadState_Clear( tstate );
    PyEval_ReleaseThread( tstate );
    PyThreadState_Delete( tstate );
}

PyThreadDetach::PyThreadDetach() throw ( css::uno::RuntimeException )
{
    tstate = PyThreadState_Get();
    PyEval_ReleaseThread( tstate );
}

    /** Acquires the global interpreter lock again

    */
PyThreadDetach::~PyThreadDetach()
{
    PyEval_AcquireThread( tstate );
}


PyRef const & RuntimeCargo::getUnoModule()
{
    if( ! dictUnoModule.is() )
    {
        dictUnoModule = importUnoModule();
    }
    return dictUnoModule;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
