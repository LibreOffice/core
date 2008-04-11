/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pyuno_adapter.cxx,v $
 * $Revision: 1.10 $
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
using com::sun::star::uno::Type;
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

Adapter::Adapter( const PyRef & ref, const Sequence< Type > &types )
    : mWrappedObject( ref ),
      mInterpreter( (PyThreadState_Get()->interp) ),
      mTypes( types )
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
        return reinterpret_cast<sal_Int64>(this);
    return 0;
}

void raiseInvocationTargetExceptionWhenNeeded( const Runtime &runtime )
    throw ( InvocationTargetException )
{
    if( PyErr_Occurred() )
    {
        PyRef excType, excValue, excTraceback;
        PyErr_Fetch( (PyObject **)&excType, (PyObject**)&excValue,(PyObject**)&excTraceback);
        Any unoExc( runtime.extractUnoException( excType, excValue, excTraceback ) );
        throw InvocationTargetException(
            ((com::sun::star::uno::Exception*)unoExc.getValue())->Message,
            Reference<XInterface>(), unoExc );
    }
}

Reference< XIntrospectionAccess > Adapter::getIntrospection()
    throw ( RuntimeException )
{
    // not supported
    return Reference< XIntrospectionAccess > ();
}

Sequence< sal_Int16 > Adapter::getOutIndexes( const OUString & functionName )
{
    Sequence< sal_Int16 > ret;
    MethodOutIndexMap::const_iterator ii = m_methodOutIndexMap.find( functionName );
    if( ii == m_methodOutIndexMap.end() )
    {

        Runtime runtime;
        {
            PyThreadDetach antiguard;

            // retrieve the adapter object again. It will be the same instance as before,
            // (the adapter factory keeps a weak map inside, which I couldn't have outside)
            Reference< XInterface > unoAdapterObject =
                runtime.getImpl()->cargo->xAdapterFactory->createAdapter( this, mTypes );

            // uuuh, that's really expensive. The alternative would have been, to store
            // an instance of the introspection at (this), but this results in a cyclic
            // reference, which is never broken (as it is up to OOo1.1.0).
            Reference< XIntrospectionAccess > introspection =
                runtime.getImpl()->cargo->xIntrospection->inspect( makeAny( unoAdapterObject ) );

            if( !introspection.is() )
            {
                throw RuntimeException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "pyuno bridge: Couldn't inspect uno adapter ( the python class must implement com.sun.star.lang.XTypeProvider !)" ) ),
                    Reference< XInterface > () );
            }

            Reference< XIdlMethod > method = introspection->getMethod(
                functionName, com::sun::star::beans::MethodConcept::ALL );
            if( ! method.is( ) )
            {
                throw RuntimeException(
                    (OUString(
                        RTL_CONSTASCII_USTRINGPARAM(
                            "pyuno bridge: Couldn't get reflection for method "))
                     + functionName),
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
                }
            }

            if( nOuts )
            {
                ret.realloc( nOuts );
                sal_Int32 nOutsAssigned = 0;
                for( i = 0 ; i < seqInfo.getLength() ; i ++ )
                {
                    if( seqInfo[i].aMode == com::sun::star::reflection::ParamMode_OUT ||
                        seqInfo[i].aMode == com::sun::star::reflection::ParamMode_INOUT )
                    {
                        ret[nOutsAssigned] = (sal_Int16) i;
                        nOutsAssigned ++;
                    }
                }
            }
        }
        // guard active again !
        m_methodOutIndexMap[ functionName ] = ret;
    }
    else
    {
        ret = ii->second;
    }
    return ret;
}

Any Adapter::invoke( const OUString &aFunctionName,
                     const Sequence< Any >& aParams,
                     Sequence< sal_Int16 > &aOutParamIndex,
                     Sequence< Any > &aOutParam)
    throw (IllegalArgumentException,CannotConvertException,InvocationTargetException,RuntimeException)
{
    Any ret;

    // special hack for the uno object identity concept. The XUnoTunnel.getSomething() call is
    // always handled by the adapter directly.
    if( aParams.getLength() == 1 && 0 == aFunctionName.compareToAscii( "getSomething" ) )
    {
        Sequence< sal_Int8 > id;
        if( aParams[0] >>= id )
            return com::sun::star::uno::makeAny( getSomething( id ) );

    }

    RuntimeCargo *cargo = 0;
    try
    {
    PyThreadAttach guard( mInterpreter );
    {
        // convert parameters to python args
        // TODO: Out parameter
        Runtime runtime;
        cargo = runtime.getImpl()->cargo;
        if( isLog( cargo, LogLevel::CALL ) )
        {
            logCall( cargo, "try     uno->py[0x",
                     mWrappedObject.get(), aFunctionName, aParams );
        }

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

        PyRef pyRet( PyObject_CallObject( method.get(), argsTuple.get() ), SAL_NO_ACQUIRE );
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
                aOutParamIndex = getOutIndexes( aFunctionName );
                if( aOutParamIndex.getLength() )
                {
                    // out parameters exist, extract the sequence
                    Sequence< Any  > seq;
                    if( ! ( ret >>= seq ) )
                    {
                        throw RuntimeException(
                            (OUString(
                                RTL_CONSTASCII_USTRINGPARAM(
                                    "pyuno bridge: Couldn't extract out"
                                    " parameters for method "))
                             + aFunctionName),
                            Reference< XInterface > () );
                    }

                    if( aOutParamIndex.getLength() +1 != seq.getLength() )
                    {
                        OUStringBuffer buf;
                        buf.appendAscii( "pyuno bridge: expected for method " );
                        buf.append( aFunctionName );
                        buf.appendAscii( " one return value and " );
                        buf.append( (sal_Int32) aOutParamIndex.getLength() );
                        buf.appendAscii( " out parameters, got a sequence of " );
                        buf.append( seq.getLength() );
                        buf.appendAscii( " elements as return value." );
                        throw RuntimeException(buf.makeStringAndClear(), *this );
                    }

                    aOutParam.realloc( aOutParamIndex.getLength() );
                    ret = seq[0];
                    for( i = 0 ; i < aOutParamIndex.getLength() ; i ++ )
                    {
                        aOutParam[i] = seq[1+i];
                    }
                }
                // else { sequence is a return value !}
            }
        }

        // log the reply, if desired
        if( isLog( cargo, LogLevel::CALL ) )
        {
            logReply( cargo, "success uno->py[0x" ,
                      mWrappedObject.get(), aFunctionName, ret, aOutParam );
        }
    }

    }
    catch(InvocationTargetException & e )
    {
        if( isLog( cargo, LogLevel::CALL ) )
        {
            logException(
                cargo, "except  uno->py[0x" ,
                mWrappedObject.get(), aFunctionName,
                e.TargetException.getValue(),e.TargetException.getValueType() );
        }
        throw;
    }
    catch( RuntimeException & e )
    {
        if( cargo && isLog( cargo, LogLevel::CALL ) )
        {
            logException(
                cargo, "except  uno->py[0x" ,
                mWrappedObject.get(), aFunctionName, &e,getCppuType(&e) );
        }
        throw;
    }
    catch( CannotConvertException & e )
    {
        if( isLog( cargo, LogLevel::CALL ) )
        {
            logException(
                cargo, "except  uno->py[0x" ,
                mWrappedObject.get(), aFunctionName, &e,getCppuType(&e) );
        }
        throw;
    }
    catch(  IllegalArgumentException & e )
    {
        if( isLog( cargo, LogLevel::CALL ) )
        {
            logException(
                cargo,  "except  uno->py[0x" ,
                mWrappedObject.get(), aFunctionName, &e,getCppuType(&e) );
        }
        throw;
    }
    return ret;
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
    bool bRet = false;
    PyThreadAttach guard( mInterpreter );
    {
        bRet = PyObject_HasAttrString(
            mWrappedObject.get() , (char*) TO_ASCII( aPropertyName ));
    }
    return bRet;
}

}
