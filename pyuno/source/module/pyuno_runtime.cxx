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

#include <osl/thread.h>
#include <osl/module.h>
#include <osl/process.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <locale.h>

#include <typelib/typedescription.hxx>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/beans/Introspection.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/script/InvocationAdapterFactory.hpp>
#include <com/sun/star/reflection/theCoreReflection.hpp>


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
using com::sun::star::reflection::XIdlReflection;
using com::sun::star::reflection::InvocationTargetException;
using com::sun::star::script::Converter;
using com::sun::star::script::XTypeConverter;
using com::sun::star::script::XInvocationAdapterFactory2;
using com::sun::star::script::XInvocation;
using com::sun::star::beans::XMaterialHolder;
using com::sun::star::beans::XIntrospection;
using com::sun::star::beans::Introspection;

#include <vector>

namespace pyuno
{

static PyTypeObject RuntimeImpl_Type =
{
    PyVarObject_HEAD_INIT (&PyType_Type, 0)
    "pyuno_runtime",
    sizeof (RuntimeImpl),
    0,
    (destructor) RuntimeImpl::del,
    (printfunc) 0,
    (getattrfunc) 0,
    (setattrfunc) 0,
    0,
    (reprfunc) 0,
    0,
    0,
    0,
    (hashfunc) 0,
    (ternaryfunc) 0,
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
#if PY_VERSION_HEX >= 0x02060000
    , 0
#endif
};

/*----------------------------------------------------------------------
  Runtime implementation
 -----------------------------------------------------------------------*/
static void getRuntimeImpl( PyRef & globalDict, PyRef &runtimeImpl )
    throw ( com::sun::star::uno::RuntimeException )
{
    PyThreadState * state = PyThreadState_Get();
    if( ! state )
    {
        throw RuntimeException( OUString( "python global interpreter must be held (thread must be attached)" ),
                                Reference< XInterface > () );
    }

    globalDict = PyRef( PyModule_GetDict(PyImport_AddModule("__main__")));

    if( ! globalDict.is() ) // FATAL !
    {
        throw RuntimeException("can't find __main__ module", Reference< XInterface > ());
    }
    runtimeImpl = PyDict_GetItemString( globalDict.get() , "pyuno_runtime" );
}

static PyRef importUnoModule( ) throw ( RuntimeException )
{
    // import the uno module
    PyRef module( PyImport_ImportModule( "uno" ), SAL_NO_ACQUIRE );
    if( PyErr_Occurred() )
    {
        PyRef excType, excValue, excTraceback;
        PyErr_Fetch( (PyObject **)&excType, (PyObject**)&excValue,(PyObject**)&excTraceback);
        // As of Python 2.7 this gives a rather non-useful "<traceback object at 0xADDRESS>",
        // but it is the best we can do in the absence of uno._uno_extract_printable_stacktrace
        // Who knows, a future Python might print something better.
        PyRef str( PyObject_Str( excTraceback.get() ), SAL_NO_ACQUIRE );

        OUStringBuffer buf;
        buf.appendAscii( "python object raised an unknown exception (" );
        PyRef valueRep( PyObject_Repr( excValue.get() ), SAL_NO_ACQUIRE );
        buf.appendAscii( PyStr_AsString( valueRep.get())).appendAscii( ", traceback follows\n" );
        buf.appendAscii( PyStr_AsString( str.get() ) );
        buf.appendAscii( ")" );
        throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface > () );
    }
    PyRef dict( PyModule_GetDict( module.get() ) );
    return dict;
}

static void readLoggingConfig( sal_Int32 *pLevel, FILE **ppFile )
{
    *pLevel = LogLevel::NONE;
    *ppFile = 0;
    OUString fileName;
    osl_getModuleURLFromFunctionAddress(
        reinterpret_cast< oslGenericFunction >(readLoggingConfig),
        (rtl_uString **) &fileName );
    fileName = OUString( fileName.getStr(), fileName.lastIndexOf( '/' )+1 );
#if HAVE_FEATURE_MACOSX_MACLIKE_APP_STRUCTURE
    fileName += "../" LIBO_ETC_FOLDER "/";
#endif
    fileName += OUString(  SAL_CONFIGFILE("pyuno" ));
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
                    0 , osl_Process_IDENTIFIER , &data );
                osl_getSystemPathFromFileURL( str.pData, &str.pData);
                OString o = OUStringToOString( str, osl_getThreadTextEncoding() );
                o += ".";
                o += OString::number( data.Ident );

                *ppFile = fopen( o.getStr() , "w" );
                if ( *ppFile )
                {
                    // do not buffer (useful if e.g. analyzing a crash)
                    setvbuf( *ppFile, 0, _IONBF, 0 );
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
    throw( com::sun::star::uno::RuntimeException )
{
    RuntimeImpl *me = PyObject_New (RuntimeImpl, &RuntimeImpl_Type);
    if( ! me )
        throw RuntimeException(
            OUString(  "cannot instantiate pyuno::RuntimeImpl"  ),
            Reference< XInterface > () );
    me->cargo = 0;
    // must use a different struct here, as the PyObject_New
    // makes C++ unusable
    RuntimeCargo *c = new RuntimeCargo();
    readLoggingConfig( &(c->logLevel) , &(c->logFile) );
    log( c, LogLevel::CALL, "Instantiating pyuno bridge" );

    c->valid = 1;
    c->xContext = ctx;
    c->xInvocation = Reference< XSingleServiceFactory > (
        ctx->getServiceManager()->createInstanceWithContext(
            OUString(  "com.sun.star.script.Invocation"  ),
            ctx ),
        UNO_QUERY );
    if( ! c->xInvocation.is() )
        throw RuntimeException(
            OUString(  "pyuno: couldn't instantiate invocation service"  ),
            Reference< XInterface > () );

    c->xTypeConverter = Converter::create(ctx);
    if( ! c->xTypeConverter.is() )
        throw RuntimeException(
            OUString(  "pyuno: couldn't instantiate typeconverter service" ),
            Reference< XInterface > () );

    c->xCoreReflection = theCoreReflection::get(ctx);

    c->xAdapterFactory = css::script::InvocationAdapterFactory::create(ctx);

    c->xIntrospection = Introspection::create(ctx);

    Any a = ctx->getValueByName("/singletons/com.sun.star.reflection.theTypeDescriptionManager");
    a >>= c->xTdMgr;
    if( ! c->xTdMgr.is() )
        throw RuntimeException(
            OUString(  "pyuno: couldn't retrieve typedescriptionmanager" ),
            Reference< XInterface > () );

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
    throw ( RuntimeException )
{
    PyRef globalDict, runtime;
    getRuntimeImpl( globalDict , runtime );
    RuntimeImpl *impl = reinterpret_cast< RuntimeImpl * > (runtime.get());

    if( runtime.is() && impl->cargo->valid )
    {
        throw RuntimeException("pyuno runtime has already been initialized before",
                                Reference< XInterface > () );
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
    : impl( 0 )
{
    PyRef globalDict, runtime;
    getRuntimeImpl( globalDict , runtime );
    if( ! runtime.is() )
    {
        throw RuntimeException(
            OUString( "pyuno runtime is not initialized, "
                                                  "(the pyuno.bootstrap needs to be called before using any uno classes)"),
            Reference< XInterface > () );
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
    throw ( com::sun::star::script::CannotConvertException,
            com::sun::star::lang::IllegalArgumentException,
            RuntimeException)
{
    if( ! impl->cargo->valid )
    {
        throw RuntimeException("pyuno runtime must be initialized before calling any2PyObject",
                                Reference< XInterface > () );
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
        sal_Unicode c = *(sal_Unicode*)a.getValue();
        return PyRef( PyUNO_char_new( c , *this ), SAL_NO_ACQUIRE );
    }
    case typelib_TypeClass_BOOLEAN:
    {
        sal_Bool b = sal_Bool();
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
                o.getStr(),  (com::sun::star::uno::TypeClass)t.getTypeClass(), *this),
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
        sal_Int32 l = *(sal_Int32 *) a.getValue();
        TypeDescription desc( a.getValueType() );
        if( desc.is() )
        {
            desc.makeComplete();
            typelib_EnumTypeDescription *pEnumDesc =
                (typelib_EnumTypeDescription *) desc.get();
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
        buf.appendAscii( "Any carries enum " );
        buf.append( a.getValueType().getTypeName());
        buf.appendAscii( " with invalid value " ).append( l );
        throw RuntimeException( buf.makeStringAndClear() , Reference< XInterface > ()  );
    }
    case typelib_TypeClass_EXCEPTION:
    case typelib_TypeClass_STRUCT:
    {
        PyRef excClass = getClass( a.getValueType().getTypeName(), *this );
        PyRef value = PyRef( PyUNO_new_UNCHECKED (a, getImpl()->cargo->xInvocation), SAL_NO_ACQUIRE);
        PyRef argsTuple( PyTuple_New( 1 ) , SAL_NO_ACQUIRE );
        PyTuple_SetItem( argsTuple.get() , 0 , value.getAcquired() );
        PyRef ret( PyObject_CallObject( excClass.get() , argsTuple.get() ), SAL_NO_ACQUIRE );
        if( ! ret.is() )
        {
            OUStringBuffer buf;
            buf.appendAscii( "Couldn't instantiate python representation of structered UNO type " );
            buf.append( a.getValueType().getTypeName() );
            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface > () );
        }

        if( com::sun::star::uno::TypeClass_EXCEPTION == a.getValueTypeClass() )
        {
            // add the message in a standard python way !
            PyRef args( PyTuple_New( 1 ), SAL_NO_ACQUIRE );

            // assuming that the Message is always the first member, wuuuu
            void *pData = (void*)a.getValue();
            OUString message = *(OUString * )pData;
            PyRef pymsg = ustring2PyString( message );
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
            tc->convertTo (a, ::getCppuType (&s)) >>= s;
            PyRef tuple( PyTuple_New (s.getLength()), SAL_NO_ACQUIRE);
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
            catch( com::sun::star::uno::Exception & )
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
        Reference< XUnoTunnel > tunnel;
        a >>= tunnel;
        if( tunnel.is() )
        {
            sal_Int64 that = tunnel->getSomething( ::pyuno::Adapter::getUnoTunnelImplementationId() );
            if( that )
                return ((Adapter*)sal::static_int_cast< sal_IntPtr >(that))->getWrappedObject();
        }
        //This is just like the struct case:
        return PyRef( PyUNO_new (a, getImpl()->cargo->xInvocation), SAL_NO_ACQUIRE );
    }
    default:
    {
        OUStringBuffer buf;
        buf.appendAscii( "Unknonwn UNO type class " );
        buf.append( (sal_Int32 ) a.getValueTypeClass() );
        throw RuntimeException(buf.makeStringAndClear( ), Reference< XInterface > () );
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
        PyRef types( PyObject_CallObject( method.get(), 0 ) , SAL_NO_ACQUIRE );
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
            ret[size] = getCppuType( (Reference< com::sun::star::lang::XUnoTunnel> *) 0 );
        }
    }
    return ret;
}

static OUString
lcl_ExceptionMessage(PyObject *const o, OUString const*const pWrapped)
{
    OUStringBuffer buf;
    buf.appendAscii("Couldn't convert ");
    PyRef reprString( PyObject_Str(o), SAL_NO_ACQUIRE );
    buf.appendAscii( PyStr_AsString(reprString.get()) );
    buf.appendAscii(" to a UNO type");
    if (pWrapped)
    {
        buf.appendAscii("; caught exception: ");
        buf.append(*pWrapped);
    }
    return buf.makeStringAndClear();
}

Any Runtime::pyObject2Any ( const PyRef & source, enum ConversionMode mode ) const
    throw ( com::sun::star::uno::RuntimeException )
{
    if( ! impl->cargo->valid )
    {
        throw RuntimeException("pyuno runtime must be initialized before calling any2PyObject",
                                Reference< XInterface > () );
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
            sal_Bool b = sal_True;
            a = Any( &b, getBooleanCppuType() );
        }
        else if ( o == Py_False )
        {
            sal_Bool b = sal_False;
            a = Any( &b, getBooleanCppuType() );
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
            sal_Bool b = sal_True;
            a = Any(&b, getBooleanCppuType());
        }
        else if(o == Py_False)
        {
            sal_Bool b = sal_False;
            a = Any(&b, getBooleanCppuType());
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
        for (int i = 0; i < PyTuple_Size (o); i++)
        {
            s[i] = pyObject2Any (PyTuple_GetItem (o, i), mode );
        }
        a <<= s;
    }
    else
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
                    (sal_Int8*) PyStrBytes_AsString(str.get()), PyStrBytes_Size(str.get()));
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
            PyUNO * obj = (PyUNO*)struc.get();
            Reference< XMaterialHolder > holder( obj->members->xInvocation, UNO_QUERY );
            if( holder.is( ) )
                a = holder->getMaterial();
            else
            {
                throw RuntimeException(
                    "struct or exception wrapper does not support XMaterialHolder",
                    Reference< XInterface > () );
            }
        }
        else if( PyObject_IsInstance( o, getPyUnoClass().get() ) )
        {
            PyUNO* o_pi;
            o_pi = (PyUNO*) o;
            if (o_pi->members->wrappedObject.getValueTypeClass () ==
                com::sun::star::uno::TypeClass_STRUCT ||
                o_pi->members->wrappedObject.getValueTypeClass () ==
                com::sun::star::uno::TypeClass_EXCEPTION)
            {
                Reference<XMaterialHolder> my_mh (o_pi->members->xInvocation, UNO_QUERY);

                if (!my_mh.is ())
                {
                    throw RuntimeException(
                        "struct wrapper does not support XMaterialHolder",
                        Reference< XInterface > () );
                }
                else
                    a = my_mh->getMaterial ();
            }
            else
            {
                a = o_pi->members->wrappedObject;
            }
        }
        else if( PyObject_IsInstance( o, getCharClass( runtime ).get() ) )
        {
            sal_Unicode c = PyChar2Unicode( o );
            a.setValue( &c, getCharCppuType( ));
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
                catch( const com::sun::star::uno::Exception & e )
                {
                    throw WrappedTargetRuntimeException(
                            e.Message, e.Context, makeAny(e));
                }
            }
            else
            {
                throw RuntimeException(
                    OUString( "uno.Any instance not accepted during method call, "
                                  "use uno.invoke instead"  ),
                    Reference< XInterface > () );
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
                Reference< com::sun::star::lang::XUnoTunnel > tunnel( adapterObject, UNO_QUERY );

                Adapter *pAdapter = ( Adapter * )
                    sal::static_int_cast< sal_IntPtr >(
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
                            com::sun::star::uno::WeakReference< XInvocation > ( pAdapter );
                    }
                } catch (InvocationTargetException const& e) {
                    OUString const msg(lcl_ExceptionMessage(o, &e.Message));
                    throw WrappedTargetRuntimeException( // re-wrap that
                            msg, e.Context, e.TargetException);
                }
            }
            if( mappedObject.is() )
            {
                a = com::sun::star::uno::makeAny( mappedObject );
            }
            else
            {
                OUString const msg(lcl_ExceptionMessage(o, 0));
                throw RuntimeException(msg, Reference<XInterface>());
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
                PyRef args( PyTuple_New( 1), SAL_NO_ACQUIRE );
                PyTuple_SetItem( args.get(), 0, excTraceback.getAcquired() );
                PyRef pyStr( PyObject_CallObject( extractTraceback.get(),args.get() ), SAL_NO_ACQUIRE);
                str = OUString::createFromAscii( PyStr_AsString(pyStr.get()) );
            }
            else
            {
                str = OUString("Couldn't find uno._uno_extract_printable_stacktrace");
            }
        }
        else
        {
            str = OUString("Could not load uno.py, no stacktrace available");
            if ( !e.Message.isEmpty() )
            {
                str += OUString (" (Error loading uno.py: ");
                str += e.Message;
                str += OUString (")");
            }
        }

    }
    else
    {
        // it may occur, that no traceback is given (e.g. only native code below)
        str = OUString(  "no traceback available"  );
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
            buf.appendAscii( "no typename available" );
        }
        buf.appendAscii( ": " );
        PyRef valueRep( PyObject_Str( excValue.get() ), SAL_NO_ACQUIRE );
        if( valueRep.is() )
        {
            buf.appendAscii( PyStr_AsString( valueRep.get()));
        }
        else
        {
            buf.appendAscii( "Couldn't convert exception value to a string" );
        }
        buf.appendAscii( ", traceback follows\n" );
        if( !str.isEmpty() )
        {
            buf.append( str );
            buf.appendAscii( "\n" );
        }
        else
        {
            buf.appendAscii( ", no traceback available\n" );
        }
        RuntimeException e;
        e.Message = buf.makeStringAndClear();
#if OSL_DEBUG_LEVEL > 0
        fprintf( stderr, "Python exception: %s\n",
                 OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr() );
#endif
        ret = com::sun::star::uno::makeAny( e );
    }
    return ret;
}


static const char * g_NUMERICID = "pyuno.lcNumeric";
static ::std::vector< OString > g_localeList;

static const char *ensureUnlimitedLifetime( const char *str )
{
    int size = g_localeList.size();
    int i;
    for( i = 0 ; i < size ; i ++ )
    {
        if( 0 == strcmp( g_localeList[i].getStr(), str ) )
            break;
    }
    if( i == size )
    {
        g_localeList.push_back( str );
    }
    return g_localeList[i].getStr();
}


PyThreadAttach::PyThreadAttach( PyInterpreterState *interp)
    throw ( com::sun::star::uno::RuntimeException )
{
    tstate = PyThreadState_New( interp );
    if( !tstate  )
        throw RuntimeException(
            OUString( "Couldn't create a pythreadstate"  ),
            Reference< XInterface > () );
    PyEval_AcquireThread( tstate);
    // set LC_NUMERIC to "C"
    const char * oldLocale =
        ensureUnlimitedLifetime( setlocale( LC_NUMERIC, 0 )  );
    setlocale( LC_NUMERIC, "C" );
    PyRef locale( // python requires C locale
        PyLong_FromVoidPtr( (void*)oldLocale ), SAL_NO_ACQUIRE);
    PyDict_SetItemString(
        PyThreadState_GetDict(), g_NUMERICID, locale.get() );
}

PyThreadAttach::~PyThreadAttach()
{
    PyObject *value =
        PyDict_GetItemString( PyThreadState_GetDict( ), g_NUMERICID );
    if( value )
        setlocale( LC_NUMERIC, (const char * ) PyLong_AsVoidPtr( value ) );
    PyThreadState_Clear( tstate );
    PyEval_ReleaseThread( tstate );
    PyThreadState_Delete( tstate );

}

PyThreadDetach::PyThreadDetach() throw ( com::sun::star::uno::RuntimeException )
{
    tstate = PyThreadState_Get();
    PyObject *value =
        PyDict_GetItemString( PyThreadState_GetDict( ), g_NUMERICID );
    if( value )
        setlocale( LC_NUMERIC, (const char * ) PyLong_AsVoidPtr( value ) );
    PyEval_ReleaseThread( tstate );
}

    /** Acquires the global interpreter lock again

    */
PyThreadDetach::~PyThreadDetach()
{
    PyEval_AcquireThread( tstate );
//     PyObject *value =
//         PyDict_GetItemString( PyThreadState_GetDict( ), g_NUMERICID );

    // python requires C LC_NUMERIC locale,
    // always set even when it is already "C"
    setlocale( LC_NUMERIC, "C" );
}


PyRef RuntimeCargo::getUnoModule()
{
    if( ! dictUnoModule.is() )
    {
        dictUnoModule = importUnoModule();
    }
    return dictUnoModule;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
