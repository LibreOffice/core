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

#include <sal/config.h>

#include <algorithm>
#include <cassert>

#include <rtl/ustrbuf.hxx>

#include <typelib/typedescription.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/XInvocation2.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <comphelper/servicehelper.hxx>

#include "pyuno_impl.hxx"

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Type;
using com::sun::star::uno::TypeClass;
using com::sun::star::uno::TypeDescription;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Exception;
using com::sun::star::lang::XSingleServiceFactory;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::lang::XTypeProvider;
using com::sun::star::lang::XUnoTunnel;
using com::sun::star::script::XInvocation2;
using com::sun::star::container::XEnumeration;
using com::sun::star::container::XEnumerationAccess;
using com::sun::star::container::XIndexAccess;
using com::sun::star::container::XIndexContainer;
using com::sun::star::container::XIndexReplace;
using com::sun::star::container::XNameAccess;
using com::sun::star::container::XNameContainer;
using com::sun::star::container::XNameReplace;

namespace pyuno
{

static PyObject *PyUNO_str( PyObject * self );

static void PyUNO_del (PyObject* self)
{
    PyUNO* me = reinterpret_cast< PyUNO* > (self);
    {
        PyThreadDetach antiguard;
        delete me->members;
    }
    PyObject_Del (self);
}


OUString val2str( const void * pVal, typelib_TypeDescriptionReference * pTypeRef , sal_Int32 mode )
{
    assert( pVal );
    if (pTypeRef->eTypeClass == typelib_TypeClass_VOID)
        return "void";

    OUStringBuffer buf( 64 );
    buf.append( "(" + OUString::unacquired(&pTypeRef->pTypeName) + ")" );

    switch (pTypeRef->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE:
    {
        buf.append( "0x" +
            OUString::number( reinterpret_cast< sal_IntPtr >(*static_cast<void * const *>(pVal)), 16 ));
        if( VAL2STR_MODE_DEEP == mode )
        {
            buf.append( "{" );        Reference< XInterface > r = *static_cast<Reference< XInterface > const *>(pVal);
            Reference< XServiceInfo > serviceInfo( r, UNO_QUERY);
            Reference< XTypeProvider > typeProvider(r,UNO_QUERY);
            if( serviceInfo.is() )
            {
                buf.append("implementationName="
                    + serviceInfo->getImplementationName()
                    + ", supportedServices={" );
                Sequence< OUString > seq = serviceInfo->getSupportedServiceNames();
                for( int i = 0 ; i < seq.getLength() ; i ++ )
                {
                    buf.append( seq[i] );
                    if( i +1 != seq.getLength() )
                        buf.append( "," );
                }
                buf.append("}");
            }

            if( typeProvider.is() )
            {
                buf.append(", supportedInterfaces={" );
                Sequence< Type > seq (typeProvider->getTypes());
                for( int i = 0 ; i < seq.getLength() ; i ++ )
                {
                    buf.append(seq[i].getTypeName());
                    if( i +1 != seq.getLength() )
                        buf.append( "," );
                }
                buf.append("}");
            }
            buf.append( "}" );
        }

        break;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        buf.append( "{ " );
        typelib_TypeDescription * pTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );
        assert( pTypeDescr );

        typelib_CompoundTypeDescription * pCompType = reinterpret_cast<typelib_CompoundTypeDescription *>(pTypeDescr);
        sal_Int32 nDescr = pCompType->nMembers;

        if (pCompType->pBaseTypeDescription)
        {
            buf.append( val2str( pVal, pCompType->pBaseTypeDescription->aBase.pWeakRef, mode ) );
            if (nDescr)
                buf.append( ", " );
        }

        typelib_TypeDescriptionReference ** ppTypeRefs = pCompType->ppTypeRefs;
        sal_Int32 * pMemberOffsets = pCompType->pMemberOffsets;
        rtl_uString ** ppMemberNames = pCompType->ppMemberNames;

        for ( sal_Int32 nPos = 0; nPos < nDescr; ++nPos )
        {
            buf.append( OUString::unacquired(&ppMemberNames[nPos]) + " = " );
            typelib_TypeDescription * pMemberType = nullptr;
            TYPELIB_DANGER_GET( &pMemberType, ppTypeRefs[nPos] );
            buf.append( val2str( static_cast<char const *>(pVal) + pMemberOffsets[nPos], pMemberType->pWeakRef, mode ) );
            TYPELIB_DANGER_RELEASE( pMemberType );
            if (nPos < (nDescr -1))
                buf.append( ", " );
        }

        TYPELIB_DANGER_RELEASE( pTypeDescr );

        buf.append( " }" );
        break;
    }
    case typelib_TypeClass_SEQUENCE:
    {
        typelib_TypeDescription * pTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

        uno_Sequence * pSequence = *static_cast<uno_Sequence * const *>(pVal);
        typelib_TypeDescription * pElementTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pElementTypeDescr, reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType );

        sal_Int32 nElementSize = pElementTypeDescr->nSize;
        sal_Int32 nElements = pSequence->nElements;

        if (nElements)
        {
            buf.append( "{ " );
            char * pElements = pSequence->elements;
            for ( sal_Int32 nPos = 0; nPos < nElements; ++nPos )
            {
                buf.append( val2str( pElements + (nElementSize * nPos), pElementTypeDescr->pWeakRef, mode ) );
                if (nPos < (nElements -1))
                    buf.append( ", " );
            }
            buf.append( " }" );
        }
        else
        {
            buf.append( "{}" );
        }
        TYPELIB_DANGER_RELEASE( pElementTypeDescr );
        TYPELIB_DANGER_RELEASE( pTypeDescr );
        break;
    }
    case typelib_TypeClass_ANY:
        buf.append( "{ "
            + val2str( static_cast<uno_Any const *>(pVal)->pData,
                             static_cast<uno_Any const *>(pVal)->pType ,
                             mode)
            + " }" );
        break;
    case typelib_TypeClass_TYPE:
        buf.append( (*static_cast<typelib_TypeDescriptionReference * const *>(pVal))->pTypeName );
        break;
    case typelib_TypeClass_STRING:
        buf.append( "\"" +
            OUString::unacquired(&*static_cast<rtl_uString * const *>(pVal)) +
            "\"" );
        break;
    case typelib_TypeClass_ENUM:
    {
        typelib_TypeDescription * pTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

        sal_Int32 * pValues = reinterpret_cast<typelib_EnumTypeDescription *>(pTypeDescr)->pEnumValues;
        sal_Int32 nPos = reinterpret_cast<typelib_EnumTypeDescription *>(pTypeDescr)->nEnumValues;
        while (nPos--)
        {
            if (pValues[nPos] == *static_cast<int const *>(pVal))
                break;
        }
        if (nPos >= 0)
            buf.append( reinterpret_cast<typelib_EnumTypeDescription *>(pTypeDescr)->ppEnumNames[nPos] );
        else
            buf.append( '?' );

        TYPELIB_DANGER_RELEASE( pTypeDescr );
        break;
    }
    case typelib_TypeClass_BOOLEAN:
        if (*static_cast<sal_Bool const *>(pVal))
            buf.append( "true" );
        else
            buf.append( "false" );
        break;
    case typelib_TypeClass_CHAR:
        buf.append( "\'"
            + OUStringChar(*static_cast<sal_Unicode const *>(pVal) )
            + "\'" );
        break;
    case typelib_TypeClass_FLOAT:
        buf.append( *static_cast<float const *>(pVal) );
        break;
    case typelib_TypeClass_DOUBLE:
        buf.append( *static_cast<double const *>(pVal) );
        break;
    case typelib_TypeClass_BYTE:
        buf.append( "0x" +
            OUString::number( static_cast<sal_Int32>(*static_cast<sal_Int8 const *>(pVal)), 16 ));
        break;
    case typelib_TypeClass_SHORT:
        buf.append( "0x" +
            OUString::number( static_cast<sal_Int32>(*static_cast<sal_Int16 const *>(pVal)), 16 ));
        break;
    case typelib_TypeClass_UNSIGNED_SHORT:
        buf.append( "0x" +
            OUString::number( static_cast<sal_Int32>(*static_cast<sal_uInt16 const *>(pVal)), 16 ));
        break;
    case typelib_TypeClass_LONG:
        buf.append( "0x" +
            OUString::number( *static_cast<sal_Int32 const *>(pVal), 16 ));
        break;
    case typelib_TypeClass_UNSIGNED_LONG:
        buf.append( "0x" +
            OUString::number( static_cast<sal_Int64>(*static_cast<sal_uInt32 const *>(pVal)), 16 ));
        break;
    case typelib_TypeClass_HYPER:
    case typelib_TypeClass_UNSIGNED_HYPER:
        buf.append( "0x" );
#if defined(__GNUC__) && defined(SPARC)
// I guess this really should check if there are strict alignment
// requirements, not just "GCC on SPARC".
        {
            sal_Int64 aVal;
            *(sal_Int32 *)&aVal = *(sal_Int32 *)pVal;
            *((sal_Int32 *)&aVal +1)= *((sal_Int32 *)pVal +1);
            buf.append( aVal, 16 );
        }
#else
        buf.append( *static_cast<sal_Int64 const *>(pVal), 16 );
#endif
        break;

    case typelib_TypeClass_VOID:
    case typelib_TypeClass_UNKNOWN:
    case typelib_TypeClass_SERVICE:
    case typelib_TypeClass_MODULE:
    default:
        buf.append( '?' );
    }

    return buf.makeStringAndClear();
}

static sal_Int32 lcl_PyNumber_AsSal_Int32( PyObject *pObj )
{
    // Check object is an index
    PyRef rIndex( PyNumber_Index( pObj ), SAL_NO_ACQUIRE );
    if ( !rIndex.is() )
        return -1;

    // Convert Python number to platform long, then check actual value against
    // bounds of sal_Int32
    int nOverflow;
    long nResult = PyLong_AsLongAndOverflow( pObj, &nOverflow );
    if ( nOverflow || nResult > SAL_MAX_INT32 || nResult < SAL_MIN_INT32) {
        PyErr_SetString( PyExc_IndexError, "Python int too large to convert to UNO long" );
        return -1;
    }

    return nResult;
}

static int lcl_PySlice_GetIndicesEx( PyObject *pObject, sal_Int32 nLen, sal_Int32 *nStart, sal_Int32 *nStop, sal_Int32 *nStep, sal_Int32 *nSliceLength )
{
    Py_ssize_t nStart_ssize, nStop_ssize, nStep_ssize, nSliceLength_ssize;

    int nResult = PySlice_GetIndicesEx(pObject,
        nLen, &nStart_ssize, &nStop_ssize, &nStep_ssize, &nSliceLength_ssize );
    if (nResult == -1)
        return -1;

    if ( nStart_ssize > SAL_MAX_INT32 || nStart_ssize < SAL_MIN_INT32
         || nStop_ssize > SAL_MAX_INT32 || nStop_ssize < SAL_MIN_INT32
         || nStep_ssize > SAL_MAX_INT32 || nStep_ssize < SAL_MIN_INT32
         || nSliceLength_ssize > SAL_MAX_INT32 || nSliceLength_ssize < SAL_MIN_INT32 )
    {
        PyErr_SetString( PyExc_IndexError, "Python int too large to convert to UNO long" );
        return -1;
    }

    *nStart = static_cast<sal_Int32>(nStart_ssize);
    *nStop = static_cast<sal_Int32>(nStop_ssize);
    *nStep = static_cast<sal_Int32>(nStep_ssize);
    *nSliceLength = static_cast<sal_Int32>(nSliceLength_ssize);
    return 0;
}

static bool lcl_hasInterfaceByName( Any const &object, OUString const & interfaceName )
{
    Reference< XInterface > xInterface( object, UNO_QUERY );
    TypeDescription typeDesc( interfaceName );
    Any aInterface = xInterface->queryInterface( typeDesc.get()->pWeakRef );

    return aInterface.hasValue();
}

static PyObject *PyUNO_repr( PyObject  * self )
{
    return PyUNO_str( self );
}

static Py_hash_t PyUNO_hash( PyObject *self )
{

    PyUNO *me = reinterpret_cast<PyUNO *>(self);

    // Py_hash_t is not necessarily the same size as a pointer, but this is not
    // important for hashing - it just has to return the same value each time
    return sal::static_int_cast< Py_hash_t >( reinterpret_cast< sal_IntPtr > (
        *static_cast<void * const *>(me->members->wrappedObject.getValue()) ) );

}

PyObject *PyUNO_invoke( PyObject *object, const char *name , PyObject *args )
{
    PyRef ret;
    try
    {
        Runtime runtime;

        PyRef paras,callable;
        if( PyObject_IsInstance( object, getPyUnoClass().get() ) )
        {
            PyUNO* me = reinterpret_cast<PyUNO*>(object);
            OUString attrName = OUString::createFromAscii(name);
            if (! me->members->xInvocation->hasMethod (attrName))
            {
                throw RuntimeException( "Attribute " + attrName + " unknown" );
            }
            callable = PyUNO_callable_new (
                me->members->xInvocation,
                attrName,
                ACCEPT_UNO_ANY);
            paras = args;
        }
        else
        {
            // clean the tuple from uno.Any !
            int size = PyTuple_Size( args );
            { // for CC, keeping ref-count of tuple being 1
            paras = PyRef(PyTuple_New( size ), SAL_NO_ACQUIRE);
            }
            for( int i = 0 ; i < size ;i ++ )
            {
                PyObject * element = PyTuple_GetItem( args , i );
                if( PyObject_IsInstance( element , getAnyClass( runtime ).get() ) )
                {
                    element = PyObject_GetAttrString(
                        element, "value" );
                }
                else
                {
                    Py_XINCREF( element );
                }
                PyTuple_SetItem( paras.get(), i , element );
            }
            callable = PyRef( PyObject_GetAttrString( object , name ), SAL_NO_ACQUIRE );
            if( !callable.is() )
                return nullptr;
        }
        ret = PyRef( PyObject_CallObject( callable.get(), paras.get() ), SAL_NO_ACQUIRE );
    }
    catch (const css::lang::IllegalArgumentException &e)
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch (const css::script::CannotConvertException &e)
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch (const css::uno::RuntimeException &e)
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch (const css::uno::Exception &e)
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }

    return ret.getAcquired();
}

PyObject *PyUNO_str( PyObject * self )
{
    PyUNO *me = reinterpret_cast<PyUNO *>(self);

    OString buf;

    {
        PyThreadDetach antiguard;

        OUString s = val2str( me->members->wrappedObject.getValue(),
                              me->members->wrappedObject.getValueType().getTypeLibType() );
        buf = "pyuno object " + OUStringToOString(s,RTL_TEXTENCODING_ASCII_US);
    }

    return PyUnicode_FromString( buf.getStr() );
}

static PyObject* PyUNO_dir (PyObject* self)
{
    PyUNO* me = reinterpret_cast<PyUNO*>(self);

    PyObject* member_list = nullptr;
    Sequence<OUString> oo_member_list;

    try
    {
        oo_member_list = me->members->xInvocation->getMemberNames ();
        member_list = PyList_New (oo_member_list.getLength ());
        for (int i = 0; i < oo_member_list.getLength (); i++)
        {
            // setitem steals a reference
            PyList_SetItem (member_list, i, ustring2PyString(oo_member_list[i]).getAcquired() );
        }
    }
    catch( const RuntimeException &e )
    {
        raisePyExceptionWithAny( Any(e) );
    }

    return member_list;
}

static sal_Int32 lcl_detach_getLength( PyUNO const *me )
{
    PyThreadDetach antiguard;

    // If both XIndexContainer and XNameContainer are implemented, it is
    // assumed that getCount() gives the same result as the number of names
    // returned by getElementNames(), or the user may be surprised.

    // For XIndexContainer
    Reference< XIndexAccess > xIndexAccess( me->members->xInvocation, UNO_QUERY );
    if ( xIndexAccess.is() )
    {
        return xIndexAccess->getCount();
    }

    // For XNameContainer
    // Not terribly efficient - get the count of all the names
    Reference< XNameAccess > xNameAccess( me->members->xInvocation, UNO_QUERY );
    if ( xNameAccess.is() )
    {
        return xNameAccess->getElementNames().getLength();
    }

    return -1;
}

static int PyUNO_bool( PyObject* self )
{
    PyUNO* me = reinterpret_cast<PyUNO*>(self);

    try
    {
        int nLen = lcl_detach_getLength( me );
        if (nLen >= 0)
            return nLen == 0 ? 0 : 1;

        // Anything which doesn't have members is a scalar object and therefore true
        return 1;
    }
    catch( const css::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }

    return -1;
}

static Py_ssize_t PyUNO_len( PyObject* self )
{
    PyUNO* me = reinterpret_cast<PyUNO*>(self);

    try
    {
        int nLen = lcl_detach_getLength( me );
        if (nLen >= 0)
            return nLen;

        PyErr_SetString( PyExc_TypeError, "object has no len()" );
    }
    catch( const css::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }

    return -1;
}

static void lcl_getRowsColumns( PyUNO const * me, sal_Int32& nRows, sal_Int32& nColumns )
{
    Sequence<short> aOutParamIndex;
    Sequence<Any> aOutParam;
    Sequence<Any> aParams;
    Any aRet = me->members->xInvocation->invoke ( "getRows", aParams, aOutParamIndex, aOutParam );
    Reference< XIndexAccess > xIndexAccessRows( aRet, UNO_QUERY );
    nRows = xIndexAccessRows->getCount();
    aRet = me->members->xInvocation->invoke ( "getColumns", aParams, aOutParamIndex, aOutParam );
    Reference< XIndexAccess > xIndexAccessCols( aRet, UNO_QUERY );
    nColumns = xIndexAccessCols->getCount();
}

static PyRef lcl_indexToSlice( const PyRef& rIndex )
{
    Py_ssize_t nIndex = PyNumber_AsSsize_t( rIndex.get(), PyExc_IndexError );
    if (nIndex == -1 && PyErr_Occurred())
        return nullptr;
    PyRef rStart( PyLong_FromSsize_t( nIndex ), SAL_NO_ACQUIRE );
    PyRef rStop( PyLong_FromSsize_t( nIndex+1 ), SAL_NO_ACQUIRE );
    PyRef rStep( PyLong_FromLong( 1 ), SAL_NO_ACQUIRE );
    PyRef rSlice( PySlice_New( rStart.get(), rStop.get(), rStep.get() ), SAL_NO_ACQUIRE );

    return rSlice;
}

static PyObject* lcl_getitem_XCellRange( PyUNO const * me, PyObject* pKey )
{
    Runtime runtime;

    Sequence<short> aOutParamIndex;
    Sequence<Any> aOutParam;
    Sequence<Any> aParams;
    Any aRet;

    // Single string key is sugar for getCellRangeByName()
    if ( PyUnicode_Check( pKey ) ) {

        aParams = { Any(pyString2ustring( pKey )) };
        {
            PyThreadDetach antiguard;
            aRet = me->members->xInvocation->invoke (
                "getCellRangeByName", aParams, aOutParamIndex, aOutParam );
        }
        PyRef rRet = runtime.any2PyObject ( aRet );
        return rRet.getAcquired();

    }

    PyRef rKey0, rKey1;
    if ( PyIndex_Check( pKey ) )
    {
        // [0] is equivalent to [0,:]
        rKey0 = pKey;
        rKey1 = PySlice_New( nullptr, nullptr, nullptr );
    }
    else if ( PyTuple_Check( pKey ) && (PyTuple_Size( pKey ) == 2) )
    {
        rKey0 = PyTuple_GetItem( pKey, 0 );
        rKey1 = PyTuple_GetItem( pKey, 1 );
    }
    else
    {
        PyErr_SetString( PyExc_KeyError, "invalid subscript" );
        return nullptr;
    }

    // If both keys are indices, return the corresponding cell
    if ( PyIndex_Check( rKey0.get() ) && PyIndex_Check( rKey1.get() ))
    {
        sal_Int32 nKey0_s = lcl_PyNumber_AsSal_Int32( rKey0.get() );
        sal_Int32 nKey1_s = lcl_PyNumber_AsSal_Int32( rKey1.get() );

        if ( ((nKey0_s == -1) || (nKey1_s == -1)) && PyErr_Occurred() )
            return nullptr;

        aParams = { Any(nKey1_s), Any(nKey0_s) };
        {
            PyThreadDetach antiguard;
            aRet = me->members->xInvocation->invoke (
                "getCellByPosition", aParams, aOutParamIndex, aOutParam );
        }
        PyRef rRet = runtime.any2PyObject( aRet );
        return rRet.getAcquired();
    }

    // If either argument is an index, coerce it to a slice
    if ( PyIndex_Check( rKey0.get() ) )
        rKey0 = lcl_indexToSlice( rKey0 );

    if ( PyIndex_Check( rKey1.get() ) )
        rKey1 = lcl_indexToSlice( rKey1 );

    // If both arguments are slices, return the corresponding cell range
    if ( PySlice_Check( rKey0.get() ) && PySlice_Check( rKey1.get() ) )
    {
        sal_Int32 nLen0 = SAL_MAX_INT32, nLen1 = SAL_MAX_INT32;
        sal_Int32 nStart0 = 0, nStop0 = 0, nStep0 = 0, nSliceLength0 = 0;
        sal_Int32 nStart1 = 0, nStop1 = 0, nStep1 = 0, nSliceLength1 = 0;

        {
            PyThreadDetach antiguard;

            if ( lcl_hasInterfaceByName( me->members->wrappedObject, "com.sun.star.table.XColumnRowRange" ) )
            {
                lcl_getRowsColumns (me, nLen0, nLen1);
            }
        }

        int nSuccess1 = lcl_PySlice_GetIndicesEx( rKey0.get(), nLen0, &nStart0, &nStop0, &nStep0, &nSliceLength0 );
        int nSuccess2 = lcl_PySlice_GetIndicesEx( rKey1.get(), nLen1, &nStart1, &nStop1, &nStep1, &nSliceLength1 );
        if ( ((nSuccess1 == -1) || (nSuccess2 == -1)) && PyErr_Occurred() )
            return nullptr;

        if ( nSliceLength0 <= 0 || nSliceLength1 <= 0 )
        {
            PyErr_SetString( PyExc_KeyError, "invalid number of rows or columns" );
            return nullptr;
        }

        if ( nStep0 == 1 && nStep1 == 1 )
        {
            aParams = { Any(nStart1), Any(nStart0), Any(nStop1 - 1), Any(nStop0 - 1) };
            {
                PyThreadDetach antiguard;
                aRet = me->members->xInvocation->invoke (
                    "getCellRangeByPosition", aParams, aOutParamIndex, aOutParam );
            }
            PyRef rRet = runtime.any2PyObject( aRet );
            return rRet.getAcquired();
        }

        PyErr_SetString( PyExc_KeyError, "step != 1 not supported" );
        return nullptr;
    }

    PyErr_SetString( PyExc_KeyError, "invalid subscript" );
    return nullptr;
}

static PyObject* lcl_getitem_index( PyUNO const *me, PyObject *pKey, Runtime const & runtime )
{
    Any aRet;
    sal_Int32 nIndex;

    nIndex = lcl_PyNumber_AsSal_Int32( pKey );
    if (nIndex == -1 && PyErr_Occurred())
        return nullptr;

    {
        PyThreadDetach antiguard;

        Reference< XIndexAccess > xIndexAccess( me->members->xInvocation, UNO_QUERY );
        if ( xIndexAccess.is() )
        {
            if (nIndex < 0)
                nIndex += xIndexAccess->getCount();
            aRet = xIndexAccess->getByIndex( nIndex );
        }
    }
    if ( aRet.hasValue() )
    {
        PyRef rRet ( runtime.any2PyObject( aRet ) );
        return rRet.getAcquired();
    }

    return nullptr;
}

static PyObject* lcl_getitem_slice( PyUNO const *me, PyObject *pKey )
{
    Runtime runtime;

    Reference< XIndexAccess > xIndexAccess;
    sal_Int32 nLen = 0;

    {
        PyThreadDetach antiguard;

        xIndexAccess.set( me->members->xInvocation, UNO_QUERY );
        if ( xIndexAccess.is() )
            nLen = xIndexAccess->getCount();
    }

    if ( !xIndexAccess )
        return nullptr;

    sal_Int32 nStart = 0, nStop = 0, nStep = 0, nSliceLength = 0;
    int nSuccess = lcl_PySlice_GetIndicesEx(pKey, nLen, &nStart, &nStop, &nStep, &nSliceLength);
    if ( nSuccess == -1 && PyErr_Occurred() )
        return nullptr;

    PyRef rTuple( PyTuple_New( nSliceLength ), SAL_NO_ACQUIRE, NOT_NULL );
    sal_Int32 nCur, i;
    for ( nCur = nStart, i = 0; i < nSliceLength; nCur += nStep, i++ )
    {
        Any aRet;

        {
            PyThreadDetach antiguard;

            aRet = xIndexAccess->getByIndex( nCur );
        }
        PyRef rRet = runtime.any2PyObject( aRet );
        PyTuple_SetItem( rTuple.get(), i, rRet.getAcquired() );
    }

    return rTuple.getAcquired();
}

static PyObject* lcl_getitem_string( PyUNO const *me, PyObject *pKey, Runtime const & runtime )
{
    OUString sKey = pyString2ustring( pKey );
    Any aRet;

    {
        PyThreadDetach antiguard;

        Reference< XNameAccess > xNameAccess( me->members->xInvocation, UNO_QUERY );
        if ( xNameAccess.is() )
        {
            aRet = xNameAccess->getByName( sKey );
        }
    }
    if ( aRet.hasValue() )
    {
        PyRef rRet = runtime.any2PyObject( aRet );
        return rRet.getAcquired();
    }

    return nullptr;
}

static PyObject* PyUNO_getitem( PyObject *self, PyObject *pKey )
{
    PyUNO* me = reinterpret_cast<PyUNO*>(self);
    Runtime runtime;

    try
    {
        // XIndexAccess access by index
        if ( PyIndex_Check( pKey ) )
        {
            PyObject* pRet = lcl_getitem_index( me, pKey, runtime );
            if ( pRet != nullptr || PyErr_Occurred() )
                return pRet;
        }

        // XIndexAccess access by slice
        if ( PySlice_Check( pKey ) )
        {
            PyObject* pRet = lcl_getitem_slice( me, pKey );
            if ( pRet != nullptr || PyErr_Occurred() )
                return pRet;
        }

        // XNameAccess access by key
        if ( PyUnicode_Check( pKey ) )
        {
            PyObject* pRet = lcl_getitem_string( me, pKey, runtime );
            if ( pRet != nullptr )
                return pRet;
        }

        // XCellRange/XColumnRowRange specialisation
        // Uses reflection as we can't have a hard dependency on XCellRange here
        bool hasXCellRange = false;

        {
            PyThreadDetach antiguard;

            hasXCellRange = lcl_hasInterfaceByName( me->members->wrappedObject, "com.sun.star.table.XCellRange" );
        }
        if ( hasXCellRange )
        {
            return lcl_getitem_XCellRange( me, pKey );
        }


        // If the object is an XIndexAccess and/or XNameAccess, but the
        // key passed wasn't suitable, give a TypeError which specifically
        // describes this
        Reference< XIndexAccess > xIndexAccess( me->members->xInvocation, UNO_QUERY );
        Reference< XNameAccess > xNameAccess( me->members->xInvocation, UNO_QUERY );
        if ( xIndexAccess.is() || xNameAccess.is() )
        {
            PyErr_SetString( PyExc_TypeError, "subscription with invalid type" );
            return nullptr;
        }

        PyErr_SetString( PyExc_TypeError, "object is not subscriptable" );
    }
    catch( const css::lang::IndexOutOfBoundsException & )
    {
        PyErr_SetString( PyExc_IndexError, "index out of range" );
    }
    catch( const css::container::NoSuchElementException & )
    {
        PyErr_SetString( PyExc_KeyError, "key not found" );
    }
    catch( const css::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::lang::WrappedTargetException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }

    return nullptr;
}

static int lcl_setitem_index( PyUNO const *me, PyObject *pKey, PyObject *pValue )
{
    Runtime runtime;

    Reference< XIndexContainer > xIndexContainer;
    Reference< XIndexReplace > xIndexReplace;
    sal_Int32 nIndex = lcl_PyNumber_AsSal_Int32( pKey );
    if ( nIndex == -1 && PyErr_Occurred() )
        return 0;

    bool isTuple = false;

    Any aValue;
    if ( pValue != nullptr )
    {
        isTuple = PyTuple_Check( pValue );

        try
        {
            aValue = runtime.pyObject2Any( pValue );
        }
        catch ( const css::uno::RuntimeException & )
        {
            // TODO pyObject2Any can't convert e.g. dicts but only throws
            // RuntimeException on failure. Fixing this will require an audit of
            // all the rest of PyUNO
            throw css::script::CannotConvertException();
        }
    }

    {
        PyThreadDetach antiguard;

        xIndexContainer.set( me->members->xInvocation, UNO_QUERY );
        if ( xIndexContainer.is() )
            xIndexReplace = xIndexContainer;
        else
            xIndexReplace.set( me->members->xInvocation, UNO_QUERY );

        if ( xIndexReplace.is() && nIndex < 0 )
            nIndex += xIndexReplace->getCount();

        // XIndexReplace replace by index
        if ( (pValue != nullptr) && xIndexReplace.is() )
        {
            if ( isTuple )
            {
                // Apply type specialisation to ensure the correct kind of sequence is passed
                Type aType = xIndexReplace->getElementType();
                aValue = runtime.getImpl()->cargo->xTypeConverter->convertTo( aValue, aType );
            }

            xIndexReplace->replaceByIndex( nIndex, aValue );
            return 0;
        }

        // XIndexContainer remove by index
        if ( (pValue == nullptr) && xIndexContainer.is() )
        {
            xIndexContainer->removeByIndex( nIndex );
            return 0;
        }
    }

    PyErr_SetString( PyExc_TypeError, "cannot assign to object" );
    return -1;
}

static int lcl_setitem_slice( PyUNO const *me, PyObject *pKey, PyObject *pValue )
{
    // XIndexContainer insert/remove/replace by slice
    Runtime runtime;

    Reference< XIndexReplace > xIndexReplace;
    Reference< XIndexContainer > xIndexContainer;
    sal_Int32 nLen = 0;

    {
        PyThreadDetach antiguard;

        xIndexContainer.set( me->members->xInvocation, UNO_QUERY );
        if ( xIndexContainer.is() )
            xIndexReplace = xIndexContainer;
        else
            xIndexReplace.set( me->members->xInvocation, UNO_QUERY );

        if ( xIndexReplace.is() )
            nLen = xIndexReplace->getCount();
    }

    if ( xIndexReplace.is() )
    {
        sal_Int32 nStart = 0, nStop = 0, nStep = 0, nSliceLength = 0;
        int nSuccess = lcl_PySlice_GetIndicesEx( pKey, nLen, &nStart, &nStop, &nStep, &nSliceLength );
        if ( (nSuccess == -1) && PyErr_Occurred() )
            return 0;

        if ( pValue == nullptr )
        {
            pValue = PyTuple_New( 0 );
        }

        if ( !PyTuple_Check (pValue) )
        {
            PyErr_SetString( PyExc_TypeError, "value is not a tuple" );
            return -1;
        }

        Py_ssize_t nTupleLength_ssize = PyTuple_Size( pValue );
        if ( nTupleLength_ssize > SAL_MAX_INT32 )
        {
            PyErr_SetString( PyExc_ValueError, "tuple too large" );
            return -1;
        }
        sal_Int32 nTupleLength = static_cast<sal_Int32>(nTupleLength_ssize);

        if ( (nTupleLength != nSliceLength) && (nStep != 1) )
        {
            PyErr_SetString( PyExc_ValueError, "number of items assigned must be equal" );
            return -1;
        }

        if ( (nTupleLength != nSliceLength) && !xIndexContainer.is() )
        {
            PyErr_SetString( PyExc_ValueError, "cannot change length" );
            return -1;
        }

        sal_Int32 nCur, i;
        sal_Int32 nMax = ::std::max( nSliceLength, nTupleLength );
        for ( nCur = nStart, i = 0; i < nMax; nCur += nStep, i++ )
        {
            if ( i < nTupleLength )
            {
                PyRef rItem = PyTuple_GetItem( pValue, i );
                bool isTuple = PyTuple_Check( rItem.get() );

                Any aItem;
                try
                {
                    aItem = runtime.pyObject2Any( rItem.get() );
                }
                catch ( const css::uno::RuntimeException & )
                {
                    // TODO pyObject2Any can't convert e.g. dicts but only throws
                    // RuntimeException on failure. Fixing this will require an audit of
                    // all the rest of PyUNO
                    throw css::script::CannotConvertException();
                }

                {
                    PyThreadDetach antiguard;

                    if ( isTuple )
                    {
                        // Apply type specialisation to ensure the correct kind of sequence is passed
                        Type aType = xIndexReplace->getElementType();
                        aItem = runtime.getImpl()->cargo->xTypeConverter->convertTo( aItem, aType );
                    }

                    if ( i < nSliceLength )
                    {
                        xIndexReplace->replaceByIndex( nCur, aItem );
                    }
                    else
                    {
                        xIndexContainer->insertByIndex( nCur, aItem );
                    }
                }
            }
            else
            {
                PyThreadDetach antiguard;

                xIndexContainer->removeByIndex( nCur );
                nCur--;
            }
        }

        return 0;
    }

    PyErr_SetString( PyExc_TypeError, "cannot assign to object" );
    return -1;
}

static int lcl_setitem_string( PyUNO const *me, PyObject *pKey, PyObject *pValue )
{
    Runtime runtime;

    OUString sKey = pyString2ustring( pKey );
    bool isTuple = false;

    Any aValue;
    if ( pValue != nullptr)
    {
        isTuple = PyTuple_Check( pValue );
        try
        {
            aValue = runtime.pyObject2Any( pValue );
        }
        catch( const css::uno::RuntimeException & )
        {
            // TODO pyObject2Any can't convert e.g. dicts but only throws
            // RuntimeException on failure. Fixing this will require an audit of
            // all the rest of PyUNO
            throw css::script::CannotConvertException();
        }
    }

    {
        PyThreadDetach antiguard;

        Reference< XNameContainer > xNameContainer( me->members->xInvocation, UNO_QUERY );
        Reference< XNameReplace > xNameReplace;
        if ( xNameContainer.is() )
            xNameReplace = xNameContainer;
        else
            xNameReplace.set( me->members->xInvocation, UNO_QUERY );

        if ( xNameReplace.is() )
        {
            if ( isTuple && aValue.hasValue() )
            {
                // Apply type specialisation to ensure the correct kind of sequence is passed
                Type aType = xNameReplace->getElementType();
                aValue = runtime.getImpl()->cargo->xTypeConverter->convertTo( aValue, aType );
            }

            if ( aValue.hasValue() )
            {
                if ( xNameContainer.is() )
                {
                    try {
                        xNameContainer->insertByName( sKey, aValue );
                        return 0;
                    }
                    catch( const css::container::ElementExistException & )
                    {
                        // Fall through, try replace instead
                    }
                }

                xNameReplace->replaceByName( sKey, aValue );
                return 0;
            }
            else if ( xNameContainer.is() )
            {
                xNameContainer->removeByName( sKey );
                return 0;
            }
        }
    }

    PyErr_SetString( PyExc_TypeError, "cannot assign to object" );
    return -1;
}

static int PyUNO_setitem( PyObject *self, PyObject *pKey, PyObject *pValue )
{
    PyUNO* me = reinterpret_cast<PyUNO*>(self);

    try
    {
        if ( PyIndex_Check( pKey ) )
        {
            return lcl_setitem_index( me, pKey, pValue );
        }
        else if ( PySlice_Check( pKey ) )
        {
            return lcl_setitem_slice( me, pKey, pValue );
        }
        else if ( PyUnicode_Check( pKey ) )
        {
            return lcl_setitem_string( me, pKey, pValue );
        }

        PyErr_SetString( PyExc_TypeError, "list index has invalid type" );
    }
    catch( const css::lang::IndexOutOfBoundsException & )
    {
        PyErr_SetString( PyExc_IndexError, "list index out of range" );
    }
    catch( const css::container::NoSuchElementException & )
    {
        PyErr_SetString( PyExc_KeyError, "key not found" );
    }
    catch( const css::lang::IllegalArgumentException & )
    {
        PyErr_SetString( PyExc_TypeError, "value has invalid type" );
    }
    catch( const css::script::CannotConvertException & )
    {
        PyErr_SetString( PyExc_TypeError, "value has invalid type" );
    }
    catch( const css::container::ElementExistException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::lang::WrappedTargetException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }

    return -1;
}

static PyObject* PyUNO_iter( PyObject *self )
{
    PyUNO* me = reinterpret_cast<PyUNO*>(self);

    try
    {
        Reference< XEnumerationAccess > xEnumerationAccess;
        Reference< XEnumeration > xEnumeration;
        Reference< XIndexAccess > xIndexAccess;
        Reference< XNameAccess > xNameAccess;

        {
            PyThreadDetach antiguard;

            xEnumerationAccess.set( me->members->xInvocation, UNO_QUERY );
            if ( xEnumerationAccess.is() )
                xEnumeration = xEnumerationAccess->createEnumeration();
            else
                xEnumeration.set( me->members->wrappedObject, UNO_QUERY );

            if ( !xEnumeration.is() )
                xIndexAccess.set( me->members->xInvocation, UNO_QUERY );

            if ( !xIndexAccess.is() )
                xNameAccess.set( me->members->xInvocation, UNO_QUERY );
        }

        // XEnumerationAccess iterator
        // XEnumeration iterator
        if (xEnumeration.is())
        {
            return PyUNO_iterator_new( xEnumeration );
        }

        // XIndexAccess iterator
        if ( xIndexAccess.is() )
        {
            // We'd like to be able to use PySeqIter_New() here, but we're not
            // allowed to because we also implement the mapping protocol
            return PyUNO_list_iterator_new( xIndexAccess );
        }

        // XNameAccess iterator
        if (xNameAccess.is())
        {
            // There's no generic mapping iterator, but we can cobble our own
            // together using PySeqIter_New()
            Runtime runtime;
            Any aRet;

            {
                PyThreadDetach antiguard;
                aRet <<= xNameAccess->getElementNames();
            }
            PyRef rNames = runtime.any2PyObject( aRet );
            return PySeqIter_New( rNames.getAcquired() );
        }

        PyErr_SetString ( PyExc_TypeError, "object is not iterable" );
    }
    catch( css::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( css::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }

    return nullptr;
}

static int PyUNO_contains( PyObject *self, PyObject *pKey )
{
    PyUNO* me = reinterpret_cast<PyUNO*>(self);

    Runtime runtime;

    try
    {
        Any aValue;
        try
        {
            aValue = runtime.pyObject2Any( pKey );
        }
        catch( const css::uno::RuntimeException & )
        {
            // TODO pyObject2Any can't convert e.g. dicts but only throws
            // RuntimeException on failure. Fixing this will require an audit of
            // all the rest of PyUNO
            throw css::script::CannotConvertException();
        }

        // XNameAccess is tried first, because checking key presence is much more
        // useful for objects which implement both XIndexAccess and XNameAccess

        // For XNameAccess
        if ( PyUnicode_Check( pKey ) )
        {
            OUString sKey;
            aValue >>= sKey;
            Reference< XNameAccess > xNameAccess;

            {
                PyThreadDetach antiguard;

                xNameAccess.set( me->members->xInvocation, UNO_QUERY );
                if ( xNameAccess.is() )
                {
                    bool hasKey = xNameAccess->hasByName( sKey );
                    return hasKey ? 1 : 0;
                }
            }
        }

        // For any other type of PyUNO iterable: Ugly iterative search by
        // content (XIndexAccess, XEnumerationAccess, XEnumeration)
        PyRef rIterator( PyUNO_iter( self ), SAL_NO_ACQUIRE );
        if ( rIterator.is() )
        {
            while ( PyObject* pItem = PyIter_Next( rIterator.get() ) )
            {
                PyRef rItem( pItem, SAL_NO_ACQUIRE );
                if ( PyObject_RichCompareBool( pKey, rItem.get(), Py_EQ ) == 1 )
                {
                    return 1;
                }
            }
            return 0;
        }

        PyErr_SetString( PyExc_TypeError, "argument is not iterable" );
    }
    catch( const css::script::CannotConvertException& )
    {
        PyErr_SetString( PyExc_TypeError, "invalid type passed as left argument to 'in'" );
    }
    catch( const css::container::NoSuchElementException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::lang::IndexOutOfBoundsException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::lang::WrappedTargetException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }
    catch( const css::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( css::uno::Any( e ) );
    }

    return -1;
}

static PyObject* PyUNO_getattr (PyObject* self, char* name)
{
    try
    {

        Runtime runtime;

        PyUNO* me = reinterpret_cast<PyUNO*>(self);
        if (strcmp (name, "__dict__") == 0)
        {
            Py_INCREF (Py_TYPE(me)->tp_dict);
            return Py_TYPE(me)->tp_dict;
        }
        if (strcmp (name, "__class__") == 0)
        {
            Py_INCREF (Py_None);
            return Py_None;
        }

        PyObject *pRet = PyObject_GenericGetAttr( self, PyUnicode_FromString( name ) );
        if( pRet )
            return pRet;
        PyErr_Clear();

        OUString attrName( OUString::createFromAscii( name ) );
        //We need to find out if it's a method...
        bool isMethod;
        {
            PyThreadDetach antiguard;
            isMethod = me->members->xInvocation->hasMethod (attrName);
        }
        if (isMethod)
        {
            //Create a callable object to invoke this...
            PyRef ret = PyUNO_callable_new (
                me->members->xInvocation,
                attrName);
            Py_XINCREF( ret.get() );
            return ret.get();

        }

        //or a property
        bool isProperty;
        Any anyRet;
        {
            PyThreadDetach antiguard;
            isProperty = me->members->xInvocation->hasProperty ( attrName);
            if (isProperty)
            {
                //Return the value of the property
                anyRet = me->members->xInvocation->getValue (attrName);
            }
        }
        if (isProperty)
        {
            PyRef ret = runtime.any2PyObject(anyRet);
            Py_XINCREF( ret.get() );
            return ret.get();
        }

        //or else...
        PyErr_SetString (PyExc_AttributeError, name);
    }
    catch( const css::reflection::InvocationTargetException & e )
    {
        raisePyExceptionWithAny( e.TargetException );
    }
    catch( const css::beans::UnknownPropertyException & e )
    {
        raisePyExceptionWithAny( Any(e) );
    }
    catch( const css::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( Any(e) );
    }
    catch( const css::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( Any(e) );
    }
    catch( const RuntimeException &e )
    {
        raisePyExceptionWithAny( Any(e) );
    }

    return nullptr;
}

static int PyUNO_setattr (PyObject* self, char* name, PyObject* value)
{
    PyUNO* me;

    me = reinterpret_cast<PyUNO*>(self);
    try
    {
        Runtime runtime;
        Any val= runtime.pyObject2Any(value, ACCEPT_UNO_ANY);

        OUString attrName( OUString::createFromAscii( name ) );
        {
            PyThreadDetach antiguard;
            if (me->members->xInvocation->hasProperty (attrName))
            {
                me->members->xInvocation->setValue (attrName, val);
                return 0; //Keep with Python's boolean system
            }
        }
    }
    catch( const css::reflection::InvocationTargetException & e )
    {
        raisePyExceptionWithAny( e.TargetException );
        return -1;
    }
    catch( const css::beans::UnknownPropertyException & e )
    {
        raisePyExceptionWithAny( Any(e) );
        return -1;
    }
    catch( const css::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( Any(e) );
        return -1;
    }
    catch( const RuntimeException & e )
    {
        raisePyExceptionWithAny( Any( e ) );
        return -1;
    }
    PyErr_SetString (PyExc_AttributeError, name);
    return -1; //as above.
}

static PyObject* PyUNO_cmp( PyObject *self, PyObject *that, int op )
{
    PyObject *result;

    if(op != Py_EQ && op != Py_NE)
    {
        PyErr_SetString(PyExc_TypeError, "only '==' and '!=' comparisons are defined");
        return nullptr;
    }
    if( self == that )
    {
        result = (op == Py_EQ ? Py_True : Py_False);
        Py_INCREF(result);
        return result;
    }
    try
    {
        Runtime runtime;
        if( PyObject_IsInstance( that, getPyUnoClass().get() ) )
        {

            PyUNO *me = reinterpret_cast< PyUNO*> ( self );
            PyUNO *other = reinterpret_cast< PyUNO *> (that );
            css::uno::TypeClass tcMe = me->members->wrappedObject.getValueTypeClass();
            css::uno::TypeClass tcOther = other->members->wrappedObject.getValueTypeClass();

            if( tcMe == tcOther )
            {
                if( me->members->wrappedObject == other->members->wrappedObject )
                {
                    result = (op == Py_EQ ? Py_True : Py_False);
                    Py_INCREF(result);
                    return result;
                }
            }
        }
    }
    catch( const css::uno::RuntimeException & e)
    {
        raisePyExceptionWithAny( Any( e ) );
    }

    result = (op == Py_EQ ? Py_False : Py_True);
    Py_INCREF(result);
    return result;
}

static PyMethodDef PyUNOMethods[] =
{
    {"__dir__",    reinterpret_cast<PyCFunction>(PyUNO_dir),    METH_NOARGS,  nullptr},
    {nullptr,         nullptr,                                        0,            nullptr}
};

static PyNumberMethods PyUNONumberMethods[] =
{
    nullptr,                                         /* nb_add */
    nullptr,                                         /* nb_subtract */
    nullptr,                                         /* nb_multiply */
    nullptr,                                         /* nb_remainder */
    nullptr,                                         /* nb_divmod */
    nullptr,                                         /* nb_power */
    nullptr,                                         /* nb_negative */
    nullptr,                                         /* nb_positive */
    nullptr,                                         /* nb_absolute */
    PyUNO_bool,                                      /* nb_bool */
    nullptr,                                         /* nb_invert */
    nullptr,                                         /* nb_lshift */
    nullptr,                                         /* nb_rshift */
    nullptr,                                         /* nb_and */
    nullptr,                                         /* nb_xor */
    nullptr,                                         /* nb_or */
    nullptr,                                         /* nb_int */
    nullptr,                                         /* nb_reserved */
    nullptr,                                         /* nb_float */
    nullptr,                                         /* nb_inplace_add */
    nullptr,                                         /* nb_inplace_subtract */
    nullptr,                                         /* nb_inplace_multiply */
    nullptr,                                         /* nb_inplace_remainder */
    nullptr,                                         /* nb_inplace_power */
    nullptr,                                         /* nb_inplace_lshift */
    nullptr,                                         /* nb_inplace_rshift */
    nullptr,                                         /* nb_inplace_and */
    nullptr,                                         /* nb_inplace_xor */
    nullptr,                                         /* nb_inplace_or */

    nullptr,                                         /* nb_floor_divide */
    nullptr,                                         /* nb_true_divide */
    nullptr,                                         /* nb_inplace_floor_divide */
    nullptr,                                         /* nb_inplace_true_divide */

    nullptr,                                         /* nb_index */
#if PY_MAJOR_VERSION == 3 && PY_MINOR_VERSION >= 5
    nullptr,                                         /* nb_matrix_multiply */
    nullptr,                                         /* nb_inplace_matrix_multiply */
#endif
};

static PySequenceMethods PyUNOSequenceMethods[] =
{
    nullptr,                                         /* sq_length */
    nullptr,                                         /* sq_concat */
    nullptr,                                         /* sq_repeat */
    nullptr,                                         /* sq_item */
    nullptr,                                         /* sq_slice */
    nullptr,                                         /* sq_ass_item */
    nullptr,                                         /* sq_ass_slice */
    PyUNO_contains,                                  /* sq_contains */
    nullptr,                                         /* sq_inplace_concat */
    nullptr                                          /* sq_inplace_repeat */
};

static PyMappingMethods PyUNOMappingMethods[] =
{
    PyUNO_len,                                       /* mp_length */
    PyUNO_getitem,                                   /* mp_subscript */
    PyUNO_setitem,                                   /* mp_ass_subscript */
};

static PyTypeObject PyUNOType =
{
    PyVarObject_HEAD_INIT( &PyType_Type, 0 )
    "pyuno",
    sizeof (PyUNO),
    0,
    PyUNO_del,
#if PY_VERSION_HEX >= 0x03080000
    0, // Py_ssize_t tp_vectorcall_offset
#else
    nullptr, // printfunc tp_print
#endif
    PyUNO_getattr,
    PyUNO_setattr,
    /* this type does not exist in Python 3: (cmpfunc) */ nullptr,
    PyUNO_repr,
    PyUNONumberMethods,
    PyUNOSequenceMethods,
    PyUNOMappingMethods,
    PyUNO_hash,
    nullptr,
    PyUNO_str,
    nullptr,
    nullptr,
    nullptr,
    Py_TPFLAGS_HAVE_ITER | Py_TPFLAGS_HAVE_RICHCOMPARE | Py_TPFLAGS_HAVE_SEQUENCE_IN,
    nullptr,
    nullptr,
    nullptr,
    PyUNO_cmp,
    0,
    PyUNO_iter,
    nullptr,
    PyUNOMethods,
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
    , 0
#if PY_VERSION_HEX >= 0x03040000
    , nullptr
#if PY_VERSION_HEX >= 0x03080000
    , nullptr // vectorcallfunc tp_vectorcall
#if PY_VERSION_HEX < 0x03090000
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
    , nullptr // tp_print
#if defined __clang__
#pragma clang diagnostic pop
#endif
#endif
#if PY_VERSION_HEX >= 0x030C00A1
    , 0 // tp_watched
#endif
#endif
#endif
};

int PyUNO_initType()
{
    return PyType_Ready(&PyUNOType);
}

PyRef getPyUnoClass()
{
    return PyRef( reinterpret_cast< PyObject * > ( &PyUNOType ) );
}

PyRef PyUNO_new (
    const Any &targetInterface,
    const Reference<XSingleServiceFactory> &ssf )
{
    Reference<XInvocation2> xInvocation;

    {
        PyThreadDetach antiguard;
        xInvocation.set(
            ssf->createInstanceWithArguments( Sequence<Any>( &targetInterface, 1 ) ), css::uno::UNO_QUERY_THROW );

        auto that = comphelper::getFromUnoTunnel<Adapter>(
            xInvocation->getIntrospection()->queryAdapter(cppu::UnoType<XUnoTunnel>::get()));
        if( that )
            return that->getWrappedObject();
    }
    if( !Py_IsInitialized() )
        throw RuntimeException();

    PyUNO* self = PyObject_New (PyUNO, &PyUNOType);
    if (self == nullptr)
        return PyRef(); // == error
    self->members = new PyUNOInternals;
    self->members->xInvocation = xInvocation;
    self->members->wrappedObject = targetInterface;
    return PyRef( reinterpret_cast<PyObject*>(self), SAL_NO_ACQUIRE );

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
