/*************************************************************************
 *
 *  $RCSfile: pyuno_adapter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2003-04-06 17:10:40 $
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

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <com/sun/star/beans/MethodConcept.hpp>

#include <cppuhelper/typeprovider.hxx>

using rtl::OUStringToOString;
using rtl::OUString;
using rtl::OUStringBuffer;
using rtl::OString;
using rtl::OStringBuffer;

using com::sun::star::beans::XIntrospectionAccess;
using com::sun::star::beans::XIntrospection;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::XInterface;
using com::sun::star::lang::XUnoTunnel;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::beans::UnknownPropertyException;
using com::sun::star::script::CannotConvertException;
using com::sun::star::reflection::InvocationTargetException;
using com::sun::star::reflection::XIdlMethod;
using com::sun::star::reflection::ParamInfo;
using com::sun::star::reflection::XIdlClass;

#define TO_ASCII(x) OUStringToOString( x , RTL_TEXTENCODING_ASCII_US).getStr()

namespace pyuno
{

Adapter::Adapter( const PyRef & ref, const Runtime & runtime):
    mWrappedObject( ref ), mInterpreter( (PyThreadState_Get()->interp) )
{}

Adapter::~Adapter()
{
    // Problem: We don't know, if we have the python interpreter lock
    //       There is no runtime function to get to know this.
    decreaseRefCount( mInterpreter, mWrappedObject.get() );
    mWrappedObject.scratch();
}

static cppu::OImplementationId g_id( sal_False );

Sequence<sal_Int8> Adapter::getUnoTunnelImplementationId()
{
    return g_id.getImplementationId();
}

sal_Int64 Adapter::getSomething( const Sequence< sal_Int8 > &id) throw (RuntimeException)
{
    if( id == g_id.getImplementationId() )
        return (sal_Int64)this;
    return 0;
}

void raiseInvocationTargetExceptionWhenNeeded( const Runtime &runtime )
    throw ( InvocationTargetException )
{
    if( PyErr_Occurred() )
    {
        PyRef excType, excValue, excTraceback;
        PyErr_Fetch( (PyObject **)&excType, (PyObject**)&excValue,(PyObject**)&excTraceback);
        Any unoExc;
        PyRef extractTraceback(
            PyDict_GetItemString(
                runtime.getImpl()->cargo->dictUnoModule.get(), "_uno_extract_printable_stacktrace" ) );
        PyRef str;
        if( extractTraceback.is() )
        {
            PyRef args( PyTuple_New( 1), SAL_NO_ACQUIRE );
            PyTuple_SetItem( args.get(), 0, excTraceback.getAcquired() );
            str = PyRef( PyObject_CallObject( extractTraceback.get(),args.get() ), SAL_NO_ACQUIRE);
        }
        else
        {
            str = PyRef( PyString_FromString( "Couldn't find uno._uno_extract_printable_stacktrace" ),
                         SAL_NO_ACQUIRE );
        }
        if( isInstanceOfStructOrException( runtime, excValue.get() ) )
        {
            unoExc = runtime.pyObject2Any( excValue );
        }
        else
        {
            OUStringBuffer buf;
            PyRef typeName( PyObject_Str( excType.get() ), SAL_NO_ACQUIRE );
            buf.appendAscii( PyString_AsString( typeName.get() ) );
            buf.appendAscii( ": " );
            PyRef valueRep( PyObject_Str( excValue.get() ), SAL_NO_ACQUIRE );
            buf.appendAscii( PyString_AsString( valueRep.get())).appendAscii( ", traceback follows\n" );
            buf.appendAscii( PyString_AsString( str.get() ) );
            RuntimeException e;
            e.Message = buf.makeStringAndClear();
            unoExc = makeAny( e );

        }
        com::sun::star::uno::Exception e;
        unoExc >>= e;
        throw InvocationTargetException(((com::sun::star::uno::Exception*)unoExc.getValue())->Message,
                                        Reference<XInterface>(), unoExc );
    }
}

Reference< XIntrospectionAccess > Adapter::getIntrospection()
    throw ( RuntimeException )
{
    // not supported
    return Reference< XIntrospectionAccess > ();
}

Any Adapter::invoke( const OUString &aFunctionName,
                     const Sequence< Any >& aParams,
                     Sequence< sal_Int16 > &aOutParamIndex,
                     Sequence< Any > &aOutParam)
    throw (IllegalArgumentException,CannotConvertException,InvocationTargetException,RuntimeException)
{
    Any ret;
    PyThreadAttach guard( mInterpreter );
    {
        // convert parameters to python args
        // TODO: Out parameter
        Runtime runtime;

        sal_Int32 size = aParams.getLength();
        PyRef argsTuple(PyTuple_New( size ), SAL_NO_ACQUIRE );
        int i;
        // fill tuple with default values in case of exceptions
        for(  i = 0 ;i < size ; i ++ )
        {
            Py_INCREF( Py_None );
            PyTuple_SetItem( argsTuple.get(), i, Py_None );
        }

        // convert args to python
        for( i = 0; i < size ; i ++  )
        {
            PyRef val = runtime.any2PyObject( aParams[i] );
            PyTuple_SetItem( argsTuple.get(), i, val.getAcquired() );
        }

        // get callable
        PyRef method(PyObject_GetAttrString( mWrappedObject.get(), (char*)TO_ASCII(aFunctionName)),
                     SAL_NO_ACQUIRE);
        raiseInvocationTargetExceptionWhenNeeded( runtime);
        if( !method.is() )
        {
            OUStringBuffer buf;
            buf.appendAscii( "pyuno::Adapater: Method " ).append( aFunctionName );
            buf.appendAscii( " is not implemented at object " );
            PyRef str( PyObject_Repr( mWrappedObject.get() ), SAL_NO_ACQUIRE );
            buf.appendAscii( PyString_AsString( str.get() ));
            throw IllegalArgumentException( buf.makeStringAndClear(), Reference< XInterface > (),0 );
        }

        PYUNO_DEBUG_2( "entering python method %s\n" , (char*)TO_ASCII(aFunctionName) );
        PyRef pyRet( PyObject_CallObject( method.get(), argsTuple.get() ), SAL_NO_ACQUIRE );
        PYUNO_DEBUG_3( "leaving python method %s %d\n" , (char*)TO_ASCII(aFunctionName) , pyRet.is() );
        raiseInvocationTargetExceptionWhenNeeded( runtime);
        if( pyRet.is() )
        {
            ret = runtime.pyObject2Any( pyRet );

            if( ret.hasValue() &&
                ret.getValueTypeClass() == com::sun::star::uno::TypeClass_SEQUENCE &&
                0 != aFunctionName.compareToAscii( "getTypes" ) &&  // needed by introspection itself !
                0 != aFunctionName.compareToAscii( "getImplementationId" ) ) // needed by introspection itself !
            {
                // the sequence can either be
                // 1)  a simple sequence return value
                // 2)  a sequence, where the first element is the return value
                //     and the following elements are interpreted as the outparameter
                // I can only decide for one solution by checking the method signature,
                // so I need the reflection of the adapter !
//                 if( ! mReflection.is() )
//                 {
//                     Reference< XInterface > unoAdapter = mWeakUnoAdapter;
//                     if( ! unoAdapter.is() )
//                         throw RuntimeException(
//                             OUString(RTL_CONSTASCII_USTRINGPARAM(
//                                           "pyuno bridge: Couldn't retrieve uno adapter reference")),*this);
//                     mReflection = runtime.getImpl()->cargo->xCoreReflection->getType(
//                         makeAny( unoAdapter  ));
//                     mWeakUnoAdapter = com::sun::star::uno::WeakReference< XInterface > ();
//                 }
                if( ! mIntrospectionAccess.is() )
                    throw RuntimeException(
                        OUString( RTL_CONSTASCII_USTRINGPARAM( "pyuno bridge: Couldn't inspect uno adapter ( the python class must implement com.sun.star.lang.XTypeProvider !)" ) ),
                        Reference< XInterface > () );

                {
                    PyThreadDetach antiguard;
                    Reference< XIdlMethod > method = mIntrospectionAccess->getMethod(
                        aFunctionName, com::sun::star::beans::MethodConcept::ALL );
                    if( ! method.is( ) )
                    {
                        throw RuntimeException(
                            OUStringBuffer().appendAscii("pyuno bridge: Couldn't get reflection for method "
                                ).append( aFunctionName ).makeStringAndClear(),
                            Reference< XInterface > () );
                    }

                    Sequence< ParamInfo > seqInfo = method->getParameterInfos();
                    int i;
                    int nOuts = 0;
                    for( i = 0 ; i < seqInfo.getLength() ; i ++ )
                    {
                        if( seqInfo[i].aMode == com::sun::star::reflection::ParamMode_OUT ||
                            seqInfo[i].aMode == com::sun::star::reflection::ParamMode_INOUT )
                        {
                            // sequence must be interpreted as return value/outparameter tuple !
                            nOuts ++;
                            break;
                        }
                    }

                    if( nOuts )
                    {
                        Sequence< Any  > seq;
                        if( ! ( ret >>= seq ) )
                        {
                            throw RuntimeException(
                                OUStringBuffer().appendAscii(
                                    "pyuno bridge: Couldn't extract out parameters for method "
                                    ).append( aFunctionName ).makeStringAndClear(),
                                Reference< XInterface > () );
                        }
                        if( nOuts +1 != seq.getLength() )
                        {
                            OUStringBuffer buf;
                            buf.appendAscii( "pyuno bridge: expected for method " );
                            buf.append( aFunctionName );
                            buf.appendAscii( " one return value and " );
                            buf.append( (sal_Int32) nOuts );
                            buf.appendAscii( " out parameters, got a sequence of " );
                            buf.append( seq.getLength() );
                            buf.appendAscii( " elements as return value." );
                            throw RuntimeException(buf.makeStringAndClear(), *this );
                        }
                        aOutParamIndex.realloc( nOuts );
                        aOutParam.realloc( nOuts );
                        ret = seq[0];
                        sal_Int32 nOutsAssigned = 0;
                        for( i = 0 ; i < seqInfo.getLength() ; i ++ )
                        {
                            if( seqInfo[i].aMode == com::sun::star::reflection::ParamMode_OUT ||
                                seqInfo[i].aMode == com::sun::star::reflection::ParamMode_INOUT )
                            {
                                aOutParamIndex[nOutsAssigned] = (sal_Int16) i;
                                aOutParam[nOutsAssigned] = seq[1+nOutsAssigned];
                                nOutsAssigned ++;
                            }
                        }
                    }
                }
                // else { sequence is a return value !}
            }
        }

    }
    PYUNO_DEBUG_1( "leaving Adapter::invoke normally\n" );
    return ret;
}

void Adapter::setUnoAdapter( const Reference< XInterface > & unoAdapter )
{
    // A performance nightmare. Every UNO object implemented in python needs to be introspected
    // to get the knowledge about parameters. Must be solved differently

    // Even worse, the adapters do not support XWeak !
//     mWeakUnoAdapter = unoAdapter;

    Runtime runtime;
    Reference< XIntrospection > xIntrospection =runtime.getImpl()->cargo->xIntrospection;
    {
        PyThreadDetach antiguard;
        mIntrospectionAccess = xIntrospection->inspect(makeAny( unoAdapter ));
    }
}

void Adapter::setValue( const OUString & aPropertyName, const Any & value )
    throw( UnknownPropertyException, CannotConvertException, InvocationTargetException,RuntimeException)
{
    PyThreadAttach guard( mInterpreter );
    try
    {
        Runtime runtime;
        PyRef obj = runtime.any2PyObject( value );

        if( !hasProperty( aPropertyName ) )
        {
            OUStringBuffer buf;
            buf.appendAscii( "pyuno::Adapater: Property " ).append( aPropertyName );
            buf.appendAscii( " is unknown." );
            throw UnknownPropertyException( buf.makeStringAndClear(), Reference< XInterface > () );
        }

        PyObject_SetAttrString(
            mWrappedObject.get(), (char*)TO_ASCII(aPropertyName), obj.get() );
        raiseInvocationTargetExceptionWhenNeeded( runtime);

    }
    catch( IllegalArgumentException & exc )
    {
        throw InvocationTargetException( exc.Message, *this, com::sun::star::uno::makeAny( exc ) );
    }
}

Any Adapter::getValue( const OUString & aPropertyName )
    throw ( UnknownPropertyException, RuntimeException )
{
    Any ret;
    PyThreadAttach guard( mInterpreter );
    {
        Runtime runtime;
        PyRef pyRef(
            PyObject_GetAttrString( mWrappedObject.get(), (char*)TO_ASCII(aPropertyName) ),
            SAL_NO_ACQUIRE );

        raiseInvocationTargetExceptionWhenNeeded( runtime);
        if( !pyRef.is() )
        {
            OUStringBuffer buf;
            buf.appendAscii( "pyuno::Adapater: Property " ).append( aPropertyName );
            buf.appendAscii( " is unknown." );
            throw UnknownPropertyException( buf.makeStringAndClear(), Reference< XInterface > () );
        }
        ret = runtime.pyObject2Any( pyRef );
    }
    return ret;
}

sal_Bool Adapter::hasMethod( const OUString & aMethodName )
    throw ( RuntimeException )
{
    return hasProperty( aMethodName );
}

sal_Bool Adapter::hasProperty( const OUString & aPropertyName )
    throw ( RuntimeException )
{
    sal_Bool bRet = sal_False;
    PyThreadAttach guard( mInterpreter );
    {
        bRet = PyObject_HasAttrString(
            mWrappedObject.get() , (char*) TO_ASCII( aPropertyName ));
    }
    return bRet;
}

}
