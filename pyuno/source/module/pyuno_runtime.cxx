/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pyuno_runtime.cxx,v $
 * $Revision: 1.18 $
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
#include <osl/module.h>
#include <osl/process.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <locale.h>

#include <typelib/typedescription.hxx>

#include <com/sun/star/beans/XMaterialHolder.hpp>

using rtl::OUString;
using rtl::OUStringToOString;
using rtl::OUStringBuffer;
using rtl::OStringBuffer;
using rtl::OString;

using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;
using com::sun::star::uno::TypeDescription;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Type;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XComponentContext;
using com::sun::star::lang::XSingleServiceFactory;
using com::sun::star::lang::XUnoTunnel;
using com::sun::star::reflection::XIdlReflection;
using com::sun::star::script::XTypeConverter;
using com::sun::star::script::XInvocationAdapterFactory2;
using com::sun::star::script::XInvocation;
using com::sun::star::beans::XMaterialHolder;
using com::sun::star::beans::XIntrospection;

namespace pyuno
{
#define USTR_ASCII(x) OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )

static PyTypeObject RuntimeImpl_Type =
{
    PyObject_HEAD_INIT (&PyType_Type)
    0,
    const_cast< char * >("pyuno_runtime"),
    sizeof (RuntimeImpl),
    0,
    (destructor) RuntimeImpl::del,
    (printfunc) 0,
    (getattrfunc) 0,
    (setattrfunc) 0,
    (cmpfunc) 0,
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
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM(
            "python global interpreter must be held (thread must be attached)" )),
                                Reference< XInterface > () );
    }

    globalDict = PyRef( PyModule_GetDict(PyImport_AddModule(const_cast< char * >("__main__"))));

    if( ! globalDict.is() ) // FATAL !
    {
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM(
            "can't find __main__ module" )), Reference< XInterface > ());
    }
    runtimeImpl = PyDict_GetItemString( globalDict.get() , "pyuno_runtime" );
}

static PyRef importUnoModule( ) throw ( RuntimeException )
{
    PyRef globalDict = PyRef( PyModule_GetDict(PyImport_AddModule(const_cast< char * >("__main__"))));
    // import the uno module
    PyRef module( PyImport_ImportModule( const_cast< char * >("uno") ), SAL_NO_ACQUIRE );
    if( PyErr_Occurred() )
    {
        PyRef excType, excValue, excTraceback;
        PyErr_Fetch( (PyObject **)&excType, (PyObject**)&excValue,(PyObject**)&excTraceback);
        PyRef str( PyObject_Repr( excTraceback.get() ), SAL_NO_ACQUIRE );

        OUStringBuffer buf;
        buf.appendAscii( "python object raised an unknown exception (" );
        PyRef valueRep( PyObject_Repr( excValue.get() ), SAL_NO_ACQUIRE );
        buf.appendAscii( PyString_AsString( valueRep.get())).appendAscii( ", traceback follows\n" );
        buf.appendAscii( PyString_AsString( str.get() ) );
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
    fileName += OUString::createFromAscii(  SAL_CONFIGFILE("pyuno") );
    rtl::Bootstrap bootstrapHandle( fileName );

    OUString str;
    if( bootstrapHandle.getFrom( USTR_ASCII( "PYUNO_LOGLEVEL" ), str ) )
    {
        if( str.equalsAscii( "NONE" ) )
            *pLevel = LogLevel::NONE;
        else if( str.equalsAscii( "CALL" ) )
            *pLevel = LogLevel::CALL;
        else if( str.equalsAscii( "ARGS" ) )
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
        if( bootstrapHandle.getFrom( USTR_ASCII( "PYUNO_LOGTARGET" ), str ) )
        {
            if( str.equalsAscii( "stdout" ) )
                *ppFile = stdout;
            else if( str.equalsAscii( "stderr" ) )
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
                o += OString::valueOf( (sal_Int32)data.Ident );

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
            OUString( RTL_CONSTASCII_USTRINGPARAM( "cannot instantiate pyuno::RuntimeImpl" ) ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.Invocation" ) ),
            ctx ),
        UNO_QUERY );
    if( ! c->xInvocation.is() )
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "pyuno: couldn't instantiate invocation service" ) ),
            Reference< XInterface > () );

    c->xTypeConverter = Reference< XTypeConverter > (
        ctx->getServiceManager()->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.Converter" ) ),
            ctx ),
        UNO_QUERY );
    if( ! c->xTypeConverter.is() )
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "pyuno: couldn't instantiate typeconverter service" )),
            Reference< XInterface > () );

    c->xCoreReflection = Reference< XIdlReflection > (
        ctx->getServiceManager()->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.reflection.CoreReflection" ) ),
            ctx ),
        UNO_QUERY );
    if( ! c->xCoreReflection.is() )
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "pyuno: couldn't instantiate corereflection service" )),
            Reference< XInterface > () );

    c->xAdapterFactory = Reference< XInvocationAdapterFactory2 > (
        ctx->getServiceManager()->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.script.InvocationAdapterFactory" ) ),
            ctx ),
        UNO_QUERY );
    if( ! c->xAdapterFactory.is() )
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "pyuno: couldn't instantiate invocation adapter factory service" )),
            Reference< XInterface > () );

    c->xIntrospection = Reference< XIntrospection > (
        ctx->getServiceManager()->createInstanceWithContext(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.beans.Introspection" ) ),
            ctx ),
        UNO_QUERY );
    if( ! c->xIntrospection.is() )
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "pyuno: couldn't instantiate introspection service" )),
            Reference< XInterface > () );

    Any a = ctx->getValueByName(OUString(
        RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theTypeDescriptionManager" )) );
    a >>= c->xTdMgr;
    if( ! c->xTdMgr.is() )
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "pyuno: couldn't retrieve typedescriptionmanager" )),
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
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM(
            "pyuno runtime has already been initialized before" ) ),
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

void Runtime::finalize() throw (RuntimeException)
{
    PyRef globalDict, runtime;
    getRuntimeImpl( globalDict , runtime );
    RuntimeImpl *impl = reinterpret_cast< RuntimeImpl * > (runtime.get());
    if( !runtime.is() || ! impl->cargo->valid )
    {
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM(
            "pyuno bridge must have been initialized before finalizing" )),
                                Reference< XInterface > () );
    }
    impl->cargo->valid = false;
    impl->cargo->xInvocation.clear();
    impl->cargo->xContext.clear();
    impl->cargo->xTypeConverter.clear();
}

Runtime::Runtime() throw(  RuntimeException )
    : impl( 0 )
{
    PyRef globalDict, runtime;
    getRuntimeImpl( globalDict , runtime );
    if( ! runtime.is() )
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("pyuno runtime is not initialized, "
                                                  "(the pyuno.bootstrap needs to be called before using any uno classes)")),
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
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM(
            "pyuno runtime must be initialized before calling any2PyObject" )),
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
        return PyRef( PyInt_FromLong (l), SAL_NO_ACQUIRE );
    }
    case typelib_TypeClass_UNSIGNED_LONG:
    {
        sal_uInt32 l;
        a >>= l;
        return PyRef( PyLong_FromUnsignedLong (l), SAL_NO_ACQUIRE );
    }
    case typelib_TypeClass_HYPER:
    {
        sal_Int64 l;
        a >>= l;
        return PyRef( PyLong_FromLongLong (l), SAL_NO_ACQUIRE);
    }
    case typelib_TypeClass_UNSIGNED_HYPER:
    {
        sal_uInt64 l;
        a >>= l;
        return PyRef( PyLong_FromUnsignedLongLong (l), SAL_NO_ACQUIRE);
    }
    case typelib_TypeClass_FLOAT:
    {
        float f;
        a >>= f;
        return PyRef(PyFloat_FromDouble (f), SAL_NO_ACQUIRE);
    }
    case typelib_TypeClass_DOUBLE:
    {
        double d;
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
            PyObject_SetAttrString( ret.get(), const_cast< char * >("args"), args.get() );
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
            OUString errMsg;
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
    //We shouldn't be here...
    Py_INCREF( Py_None );
    return Py_None;
}

static Sequence< Type > invokeGetTypes( const Runtime & r , PyObject * o )
{
    Sequence< Type > ret;

    PyRef method( PyObject_GetAttrString( o , const_cast< char * >("getTypes") ), SAL_NO_ACQUIRE );
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

Any Runtime::pyObject2Any ( const PyRef & source, enum ConversionMode mode ) const
    throw ( com::sun::star::uno::RuntimeException )
{
    if( ! impl->cargo->valid )
    {
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM(
            "pyuno runtime must be initialized before calling any2PyObject" )),
                                Reference< XInterface > () );
    }

    Any a;
    PyObject *o = source.get();
    if( Py_None == o )
    {

    }
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
            sal_Int32 l = (sal_Int32) PyInt_AsLong( o );
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
    else if (PyLong_Check (o))
    {
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
    }
    else if (PyFloat_Check (o))
    {
        double d = PyFloat_AsDouble (o);
        a <<= d;
    }
    else if (PyString_Check (o))
    a <<= pyString2ustring(o);
    else if( PyUnicode_Check( o ) )
    a <<= pyString2ustring(o);
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
            PyRef str(PyObject_GetAttrString( o , const_cast< char * >("value") ),SAL_NO_ACQUIRE);
            Sequence< sal_Int8 > seq;
            if( PyString_Check( str.get() ) )
            {
                seq = Sequence<sal_Int8 > (
                    (sal_Int8*) PyString_AsString(str.get()), PyString_Size(str.get()));
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
            PyRef struc(PyObject_GetAttrString( o , const_cast< char * >("value") ),SAL_NO_ACQUIRE);
            PyUNO * obj = (PyUNO*)struc.get();
            Reference< XMaterialHolder > holder( obj->members->xInvocation, UNO_QUERY );
            if( holder.is( ) )
                a = holder->getMaterial();
            else
            {
                throw RuntimeException(
                    USTR_ASCII( "struct or exception wrapper does not support XMaterialHolder" ),
                    Reference< XInterface > () );
            }
        }
        else if( PyObject_IsInstance( o, getPyUnoClass( runtime ).get() ) )
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
                        USTR_ASCII( "struct wrapper does not support XMaterialHolder" ),
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
                a = pyObject2Any( PyRef( PyObject_GetAttrString( o , const_cast< char * >("value") ), SAL_NO_ACQUIRE) );
                Type t;
                pyObject2Any( PyRef( PyObject_GetAttrString( o, const_cast< char * >("type") ), SAL_NO_ACQUIRE ) ) >>= t;

                try
                {
                    a = getImpl()->cargo->xTypeConverter->convertTo( a, t );
                }
                catch( com::sun::star::uno::Exception & e )
                {
                    throw RuntimeException( e.Message, e.Context );
                }
            }
            else
            {
                throw RuntimeException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM(
                                  "uno.Any instance not accepted during method call, "
                                  "use uno.invoke instead" ) ),
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
                Sequence< Type > interfaces = invokeGetTypes( *this, o );
                if( interfaces.getLength() )
                {
                    Adapter *pAdapter = new Adapter( o, interfaces );
                    mappedObject =
                        getImpl()->cargo->xAdapterFactory->createAdapter(
                            pAdapter, interfaces );

                    // keep a list of exported objects to ensure object identity !
                    impl->cargo->mappedObjects[ PyRef(o) ] =
                        com::sun::star::uno::WeakReference< XInvocation > ( pAdapter );
                }
            }
            if( mappedObject.is() )
            {
                a = com::sun::star::uno::makeAny( mappedObject );
            }
            else
            {
                OUStringBuffer buf;
                buf.appendAscii( "Couldn't convert " );
                PyRef reprString( PyObject_Str( o ) , SAL_NO_ACQUIRE );
                buf.appendAscii( PyString_AsString( reprString.get() ) );
                buf.appendAscii( " to a UNO type" );
                throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface > () );
            }
        }
    }
    return a;
}

Any Runtime::extractUnoException( const PyRef & excType, const PyRef &excValue, const PyRef &excTraceback) const
{
    PyRef str;
    Any ret;
    if( excTraceback.is() )
    {
        PyRef unoModule( impl ? impl->cargo->getUnoModule() : 0 );
        if( unoModule.is() )
        {
            PyRef extractTraceback(
                PyDict_GetItemString(unoModule.get(),"_uno_extract_printable_stacktrace" ) );

            if( extractTraceback.is() )
            {
                PyRef args( PyTuple_New( 1), SAL_NO_ACQUIRE );
                PyTuple_SetItem( args.get(), 0, excTraceback.getAcquired() );
                str = PyRef( PyObject_CallObject( extractTraceback.get(),args.get() ), SAL_NO_ACQUIRE);
            }
            else
            {
                str = PyRef(
                    PyString_FromString( "Couldn't find uno._uno_extract_printable_stacktrace" ),
                    SAL_NO_ACQUIRE );
            }
        }
        else
        {
            str = PyRef(
                PyString_FromString( "Couldn't find uno.py, no stacktrace available" ),
                SAL_NO_ACQUIRE );
        }

    }
    else
    {
        // it may occur, that no traceback is given (e.g. only native code below)
        str = PyRef( PyString_FromString( "no traceback available" ), SAL_NO_ACQUIRE);
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
            buf.appendAscii( PyString_AsString( typeName.get() ) );
        }
        else
        {
            buf.appendAscii( "no typename available" );
        }
        buf.appendAscii( ": " );
        PyRef valueRep( PyObject_Str( excValue.get() ), SAL_NO_ACQUIRE );
        if( valueRep.is() )
        {
            buf.appendAscii( PyString_AsString( valueRep.get()));
        }
        else
        {
            buf.appendAscii( "Couldn't convert exception value to a string" );
        }
        buf.appendAscii( ", traceback follows\n" );
        if( str.is() )
        {
            buf.appendAscii( PyString_AsString( str.get() ) );
        }
        else
        {
            buf.appendAscii( ", no traceback available\n" );
        }
        RuntimeException e;
        e.Message = buf.makeStringAndClear();
        ret = com::sun::star::uno::makeAny( e );
    }
    return ret;
}


static const char * g_NUMERICID = "pyuno.lcNumeric";
static ::std::vector< rtl::OString > g_localeList;

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
            OUString(RTL_CONSTASCII_USTRINGPARAM( "Couldn't create a pythreadstate" ) ),
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
