/*************************************************************************
 *
 *  $RCSfile: pyuno_runtime.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 15:01:58 $
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
#include <osl/thread.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <typelib/typedescription.hxx>

#include <com/sun/star/beans/XMaterialHolder.hpp>

#include "pyuno_impl.hxx"

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
using com::sun::star::beans::XMaterialHolder;
using com::sun::star::beans::XIntrospection;

namespace pyuno
{
#define USTR_ASCII(x) OUString( RTL_CONSTASCII_USTRINGPARAM( x ) )

static PyTypeObject RuntimeImpl_Type =
{
    PyObject_HEAD_INIT (&PyType_Type)
    0,
    "pyuno_runtime",
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

    globalDict = PyRef( PyModule_GetDict(PyImport_AddModule("__main__")));

    if( ! globalDict.is() ) // FATAL !
    {
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM(
            "can't find __main__ module" )), Reference< XInterface > ());
    }
    runtimeImpl = PyDict_GetItemString( globalDict.get() , "pyuno_runtime" );
}

static PyRef importUnoModule( ) throw ( RuntimeException )
{
    PyRef globalDict = PyRef( PyModule_GetDict(PyImport_AddModule("__main__")));
    // import the uno module
    PyRef module( PyImport_ImportModule( "uno" ), SAL_NO_ACQUIRE );
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

/*-------------------------------------------------------------------
 RuntimeImpl implementations
 *-------------------------------------------------------------------*/
PyRef stRuntimeImpl::create( const Reference< XComponentContext > &ctx )
    throw( com::sun::star::uno::RuntimeException )
{
    RuntimeImpl *me = PyObject_NEW (RuntimeImpl, &RuntimeImpl_Type);
    if( ! me )
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "cannot instantiate pyuno::RuntimeImpl" ) ),
            Reference< XInterface > () );
    me->cargo = 0;

    // must use a different struct here, as the PyObject_NEW macro
    // makes C++ unusable
    RuntimeCargo *c = new RuntimeCargo();
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
    delete me->cargo;
    PyMem_DEL (self);
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
        sal_Bool b;
        a >>= b;
        if (b)
            return Py_True;
        else
            return Py_False;
    }
    case typelib_TypeClass_BYTE:
    case typelib_TypeClass_SHORT:
    case typelib_TypeClass_UNSIGNED_SHORT:
    case typelib_TypeClass_LONG:
    {
        long l;
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
        long l = *(long *) a.getValue();
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

        if( typelib_TypeClass_EXCEPTION == a.getValueTypeClass() )
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
            int i;
            bool cont = true;
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
                return ((Adapter*)that)->getWrappedObject();
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

    PyRef method( PyObject_GetAttrString( o , "getTypes" ), SAL_NO_ACQUIRE );
    raiseInvocationTargetExceptionWhenNeeded( r );
    if( method.is() && PyCallable_Check( method.get() ) )
    {
        PyRef types( PyObject_CallObject( method.get(), 0 ) , SAL_NO_ACQUIRE );
        raiseInvocationTargetExceptionWhenNeeded( r );
        if( types.is() && PyTuple_Check( types.get() ) )
        {
            int size = PyTuple_Size( types.get() );
            ret.realloc( size );
            for( int i = 0 ; i < size ; i ++ )
            {
                Any a = r.pyObject2Any(PyTuple_GetItem(types.get(),i));
                a >>= ret[i];
            }
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
        else if( l <= 0x7fffffff && l >= -0x80000000 )
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
    {
        // needed, if ByteSequence becomes a string
//         Runtime runtime;
//         if( PyObject_IsInstance( o, getByteSequenceClass( runtime ).get() ) )
//         {
//             // is it the byte sequence ?
//             Sequence< sal_Int8 > seq;
//             seq = Sequence<sal_Int8 > ((sal_Int8*) PyString_AsString(o) , PyString_Size(o));
//             a <<= seq;
//         }
//         else
//         {
        a <<= OUString(PyString_AsString (o), strlen( PyString_AsString(o)),
                       osl_getThreadTextEncoding());
//         }
    }
    else if( PyUnicode_Check( o ) )
    {
        OUString s;
        if( sizeof( Py_UNICODE ) == 2 )
        {
            s = OUString( (sal_Unicode *) PyUnicode_AsUnicode( o ), PyUnicode_GetSize( o ) );
        }
        else if( sizeof( Py_UNICODE ) == 4 )
        {
            // fixed for 0.9.2: OUString ctor expects the length of the byte array !
            s = OUString( (sal_Char * ) PyUnicode_AsUnicode( o ),
                          PyUnicode_GetSize( o ) * sizeof(Py_UNICODE), RTL_TEXTENCODING_UCS4 );
        }
        else
        {
            OUStringBuffer buf;
            buf.appendAscii( "pyuno string conversion routines can't deal with sizeof(Py_UNICODE) ==" );
            buf.append( (sal_Int32) sizeof( Py_UNICODE ) );
            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface > ( ) );
        }
        a <<= s;
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
            Type t = PyType2Type( o , runtime );
            a <<= t;
        }
        else if( PyObject_IsInstance( o, getEnumClass( runtime ).get() ) )
        {
            a = PyEnum2Enum( o, runtime );
        }
        else if( isInstanceOfStructOrException( runtime,  o ) )
        {
            PyRef struc(PyObject_GetAttrString( o , "value" ),SAL_NO_ACQUIRE);
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
            sal_Unicode c = PyChar2Unicode( o,runtime );
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

            // can be improved in the 643 source tree with an improved
            // invocation adapter factory
            if( ! mappedObject.is() )
            {
                Sequence< Type > interfaces = invokeGetTypes( *this, o );
                //implementsInterfaces( *this, o );
                if( interfaces.getLength() )
                {
                    Adapter *pAdapter = new Adapter( o , *this);
                    mappedObject =
                        getImpl()->cargo->xAdapterFactory->createAdapter(
                            pAdapter, interfaces );
                    pAdapter->setUnoAdapter( mappedObject );
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


PyThreadAttach::PyThreadAttach( PyInterpreterState *interp)
    throw ( com::sun::star::uno::RuntimeException )
{
    PYUNO_DEBUG_1( "PyThreadAttach ctor\n" );
    tstate = PyThreadState_New( interp );
    if( !tstate  )
        throw RuntimeException(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "Couldn't create a pythreadstate" ) ),
            Reference< XInterface > () );
    PyEval_AcquireThread( tstate);
}

PyThreadAttach::~PyThreadAttach()
{
    PYUNO_DEBUG_1( "PyThreadAttach dtor\n" );
    PyThreadState_Clear( tstate );
    PyEval_ReleaseThread( tstate );
    PyThreadState_Delete( tstate );
}

PyThreadDetach::PyThreadDetach() throw ( com::sun::star::uno::RuntimeException )
{
    PYUNO_DEBUG_1( "PyThreadDeattach ctor\n" );
    tstate = PyThreadState_Get();
    PyEval_ReleaseThread( tstate );
}

    /** Acquires the global interpreter lock again

    */
PyThreadDetach::~PyThreadDetach()
{
    PYUNO_DEBUG_1( "PyThreadDeattach dtor\n" );
    PyEval_AcquireThread( tstate );
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
