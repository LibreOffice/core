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

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>

#include <cppuhelper/typeprovider.hxx>


using com::sun::star::beans::XIntrospectionAccess;
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

static cppu::OImplementationId g_id( false );

Sequence<sal_Int8> Adapter::getUnoTunnelImplementationId()
{
    return g_id.getImplementationId();
}

sal_Int64 Adapter::getSomething( const Sequence< sal_Int8 > &id)
{
    if( id == g_id.getImplementationId() )
        return reinterpret_cast<sal_Int64>(this);
    return 0;
}

void raiseInvocationTargetExceptionWhenNeeded( const Runtime &runtime )
{
    if( !Py_IsInitialized() )
        throw InvocationTargetException();

    if( PyErr_Occurred() )
    {
        PyRef excType, excValue, excTraceback;
        PyErr_Fetch(reinterpret_cast<PyObject **>(&excType), reinterpret_cast<PyObject**>(&excValue), reinterpret_cast<PyObject**>(&excTraceback));
        Any unoExc( runtime.extractUnoException( excType, excValue, excTraceback ) );
        throw InvocationTargetException(
            o3tl::doAccess<css::uno::Exception>(unoExc)->Message,
            Reference<XInterface>(), unoExc );
    }
}

Reference< XIntrospectionAccess > Adapter::getIntrospection()
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
                    "pyuno bridge: Couldn't inspect uno adapter ( the python class must implement com.sun.star.lang.XTypeProvider !)" );
            }

            Reference< XIdlMethod > method = introspection->getMethod(
                functionName, css::beans::MethodConcept::ALL );
            if( ! method.is( ) )
            {
                throw RuntimeException(
                    "pyuno bridge: Couldn't get reflection for method " + functionName );
            }

            Sequence< ParamInfo > seqInfo = method->getParameterInfos();
            int i;
            int nOuts = 0;
            for( i = 0 ; i < seqInfo.getLength() ; i ++ )
            {
                if( seqInfo[i].aMode == css::reflection::ParamMode_OUT ||
                    seqInfo[i].aMode == css::reflection::ParamMode_INOUT )
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
                    if( seqInfo[i].aMode == css::reflection::ParamMode_OUT ||
                        seqInfo[i].aMode == css::reflection::ParamMode_INOUT )
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
{
    Any ret;

    // special hack for the uno object identity concept. The XUnoTunnel.getSomething() call is
    // always handled by the adapter directly.
    if( aParams.getLength() == 1 && aFunctionName == "getSomething" )
    {
        Sequence< sal_Int8 > id;
        if( aParams[0] >>= id )
            return css::uno::makeAny( getSomething( id ) );

    }

    RuntimeCargo *cargo = nullptr;
    try
    {
    PyThreadAttach guard( mInterpreter );
    {
        if( !Py_IsInitialized() )
            throw InvocationTargetException();

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
        PyRef argsTuple(PyTuple_New( size ), SAL_NO_ACQUIRE, NOT_NULL );
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

            // any2PyObject() can release the GIL
            if( !Py_IsInitialized() )
                throw InvocationTargetException();

            PyTuple_SetItem( argsTuple.get(), i, val.getAcquired() );
        }

        // get callable
        PyRef method(PyObject_GetAttrString( mWrappedObject.get(), TO_ASCII(aFunctionName)),
                     SAL_NO_ACQUIRE);

        raiseInvocationTargetExceptionWhenNeeded( runtime);
        if( !method.is() )
        {
            OUStringBuffer buf;
            buf.append( "pyuno::Adapter: Method " ).append( aFunctionName );
            buf.append( " is not implemented at object " );
            PyRef str( PyObject_Repr( mWrappedObject.get() ), SAL_NO_ACQUIRE );
            buf.append(pyString2ustring(str.get()));
            throw IllegalArgumentException( buf.makeStringAndClear(), Reference< XInterface > (),0 );
        }

        PyRef pyRet( PyObject_CallObject( method.get(), argsTuple.get() ), SAL_NO_ACQUIRE );
        raiseInvocationTargetExceptionWhenNeeded( runtime);
        if( pyRet.is() )
        {
            ret = runtime.pyObject2Any( pyRet );

            if( ret.hasValue() &&
                ret.getValueTypeClass() == css::uno::TypeClass_SEQUENCE &&
                aFunctionName != "getTypes" &&  // needed by introspection itself !
                aFunctionName != "getImplementationId" ) // needed by introspection itself !
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
                            "pyuno bridge: Couldn't extract out parameters for method " + aFunctionName );
                    }

                    if( aOutParamIndex.getLength() +1 != seq.getLength() )
                    {
                        OUStringBuffer buf;
                        buf.append( "pyuno bridge: expected for method " );
                        buf.append( aFunctionName );
                        buf.append( " one return value and " );
                        buf.append( (sal_Int32) aOutParamIndex.getLength() );
                        buf.append( " out parameters, got a sequence of " );
                        buf.append( seq.getLength() );
                        buf.append( " elements as return value." );
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
    catch( const InvocationTargetException & e )
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
    catch( const IllegalArgumentException & e )
    {
        if( isLog( cargo, LogLevel::CALL ) )
        {
            logException(
                cargo,  "except  uno->py[0x" ,
                mWrappedObject.get(), aFunctionName, &e,cppu::UnoType<decltype(e)>::get() );
        }
        throw;
    }
    catch( const RuntimeException & e )
    {
        if( cargo && isLog( cargo, LogLevel::CALL ) )
        {
            logException(
                cargo, "except  uno->py[0x" ,
                mWrappedObject.get(), aFunctionName, &e,cppu::UnoType<decltype(e)>::get() );
        }
        throw;
    }
    catch( const CannotConvertException & e )
    {
        if( isLog( cargo, LogLevel::CALL ) )
        {
            logException(
                cargo, "except  uno->py[0x" ,
                mWrappedObject.get(), aFunctionName, &e,cppu::UnoType<decltype(e)>::get() );
        }
        throw;
    }
    return ret;
}

void Adapter::setValue( const OUString & aPropertyName, const Any & value )
{
    if( !hasProperty( aPropertyName ) )
    {
        OUStringBuffer buf;
        buf.append( "pyuno::Adapter: Property " ).append( aPropertyName );
        buf.append( " is unknown." );
        throw UnknownPropertyException( buf.makeStringAndClear() );
    }

    PyThreadAttach guard( mInterpreter );
    try
    {
        if( !Py_IsInitialized() )
            throw InvocationTargetException();

        Runtime runtime;
        PyRef obj = runtime.any2PyObject( value );

        // any2PyObject() can release the GIL
        if( !Py_IsInitialized() )
            throw InvocationTargetException();

        PyObject_SetAttrString(
            mWrappedObject.get(), TO_ASCII(aPropertyName), obj.get() );
        raiseInvocationTargetExceptionWhenNeeded( runtime);

    }
    catch( const IllegalArgumentException & exc )
    {
        throw InvocationTargetException( exc.Message, *this, css::uno::makeAny( exc ) );
    }
}

Any Adapter::getValue( const OUString & aPropertyName )
{
    Any ret;
    PyThreadAttach guard( mInterpreter );
    {
        // Should probably be InvocationTargetException, but the interface doesn't allow it
        if( !Py_IsInitialized() )
            throw RuntimeException();

        Runtime runtime;
        PyRef pyRef(
            PyObject_GetAttrString( mWrappedObject.get(), TO_ASCII(aPropertyName) ),
            SAL_NO_ACQUIRE );

        if (!pyRef.is() || PyErr_Occurred())
        {
            OUStringBuffer buf;
            buf.append( "pyuno::Adapter: Property " ).append( aPropertyName );
            buf.append( " is unknown." );
            throw UnknownPropertyException( buf.makeStringAndClear() );
        }
        ret = runtime.pyObject2Any( pyRef );
    }
    return ret;
}

sal_Bool Adapter::hasMethod( const OUString & aMethodName )
{
    return hasProperty( aMethodName );
}

sal_Bool Adapter::hasProperty( const OUString & aPropertyName )
{
    bool bRet = false;
    PyThreadAttach guard( mInterpreter );
    {
        // Should probably be InvocationTargetException, but the interface doesn't allow it
        if( !Py_IsInitialized() )
            throw RuntimeException();

        bRet = PyObject_HasAttrString(
            mWrappedObject.get() , TO_ASCII( aPropertyName ));
    }
    return bRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
