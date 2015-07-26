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

#include "pyuno_impl.hxx"

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <osl/thread.h>

#include <typelib/typedescription.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>

using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Type;
using com::sun::star::uno::TypeClass;
using com::sun::star::uno::TypeDescription;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Exception;
using com::sun::star::uno::XComponentContext;
using com::sun::star::lang::XSingleServiceFactory;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::lang::XTypeProvider;
using com::sun::star::lang::XUnoTunnel;
using com::sun::star::script::XTypeConverter;
using com::sun::star::script::XInvocation2;
using com::sun::star::beans::XMaterialHolder;
using com::sun::star::container::XElementAccess;
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

PyObject *PyUNO_str( PyObject * self );

void PyUNO_del (PyObject* self)
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
        return OUString("void");

    OUStringBuffer buf( 64 );
    buf.append( '(' );
    buf.append( pTypeRef->pTypeName );
    buf.append( ')' );

    switch (pTypeRef->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE:
    {
        buf.append( "0x" );
        buf.append( reinterpret_cast< sal_IntPtr >(*static_cast<void * const *>(pVal)), 16 );
        if( VAL2STR_MODE_DEEP == mode )
        {
            buf.append( "{" );        Reference< XInterface > r = *static_cast<Reference< XInterface > const *>(pVal);
            Reference< XServiceInfo > serviceInfo( r, UNO_QUERY);
            Reference< XTypeProvider > typeProvider(r,UNO_QUERY);
            if( serviceInfo.is() )
            {
                buf.append("implementationName=" );
                buf.append(serviceInfo->getImplementationName() );
                buf.append(", supportedServices={" );
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
        typelib_TypeDescription * pTypeDescr = 0;
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
            buf.append( ppMemberNames[nPos] );
            buf.append( " = " );
            typelib_TypeDescription * pMemberType = 0;
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
        typelib_TypeDescription * pTypeDescr = 0;
        TYPELIB_DANGER_GET( &pTypeDescr, pTypeRef );

        uno_Sequence * pSequence = *static_cast<uno_Sequence * const *>(pVal);
        typelib_TypeDescription * pElementTypeDescr = 0;
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
        buf.append( "{ " );
        buf.append( val2str( static_cast<uno_Any const *>(pVal)->pData,
                             static_cast<uno_Any const *>(pVal)->pType ,
                             mode) );
        buf.append( " }" );
        break;
    case typelib_TypeClass_TYPE:
        buf.append( (*static_cast<typelib_TypeDescriptionReference * const *>(pVal))->pTypeName );
        break;
    case typelib_TypeClass_STRING:
        buf.append( '\"' );
        buf.append( *static_cast<rtl_uString * const *>(pVal) );
        buf.append( '\"' );
        break;
    case typelib_TypeClass_ENUM:
    {
        typelib_TypeDescription * pTypeDescr = 0;
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
        buf.append( '\'' );
        buf.append( *static_cast<sal_Unicode const *>(pVal) );
        buf.append( '\'' );
        break;
    case typelib_TypeClass_FLOAT:
        buf.append( *static_cast<float const *>(pVal) );
        break;
    case typelib_TypeClass_DOUBLE:
        buf.append( *static_cast<double const *>(pVal) );
        break;
    case typelib_TypeClass_BYTE:
        buf.append( "0x" );
        buf.append( (sal_Int32)*static_cast<sal_Int8 const *>(pVal), 16 );
        break;
    case typelib_TypeClass_SHORT:
        buf.append( "0x" );
        buf.append( (sal_Int32)*static_cast<sal_Int16 const *>(pVal), 16 );
        break;
    case typelib_TypeClass_UNSIGNED_SHORT:
        buf.append( "0x" );
        buf.append( (sal_Int32)*static_cast<sal_uInt16 const *>(pVal), 16 );
        break;
    case typelib_TypeClass_LONG:
        buf.append( "0x" );
        buf.append( *static_cast<sal_Int32 const *>(pVal), 16 );
        break;
    case typelib_TypeClass_UNSIGNED_LONG:
        buf.append( "0x" );
        buf.append( (sal_Int64)*static_cast<sal_uInt32 const *>(pVal), 16 );
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

sal_Int32 lcl_PyNumber_AsSal_Int32( PyObject *pObj )
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

int lcl_PySlice_GetIndicesEx( PyObject *pObject, sal_Int32 nLen, sal_Int32 *nStart, sal_Int32 *nStop, sal_Int32 *nStep, sal_Int32 *nSliceLength )
{
    Py_ssize_t nStart_ssize, nStop_ssize, nStep_ssize, nSliceLength_ssize;

    int nResult = PySlice_GetIndicesEx(
#if PY_VERSION_HEX >= 0x030200f0
        pObject,
#else
        reinterpret_cast<PySliceObject*>(pObject),
#endif
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

    *nStart = (sal_Int32)nStart_ssize;
    *nStop = (sal_Int32)nStop_ssize;
    *nStep = (sal_Int32)nStep_ssize;
    *nSliceLength = (sal_Int32)nSliceLength_ssize;
    return 0;
}

bool lcl_hasInterfaceByName( Any const &object, OUString const & interfaceName )
{
    Reference< XInterface > xInterface( object, UNO_QUERY );
    TypeDescription typeDesc( interfaceName );
    Any aInterface = xInterface->queryInterface( typeDesc.get()->pWeakRef );

    return aInterface.hasValue();
}

PyObject *PyUNO_repr( PyObject  * self )
{
    return PyUNO_str( self );
}

Py_hash_t PyUNO_hash( PyObject *self )
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
                OUStringBuffer buf;
                buf.append( "Attribute " );
                buf.append( attrName );
                buf.append( " unknown" );
                throw RuntimeException( buf.makeStringAndClear() );
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
                return 0;
        }
        ret = PyRef( PyObject_CallObject( callable.get(), paras.get() ), SAL_NO_ACQUIRE );
    }
    catch (const ::com::sun::star::lang::IllegalArgumentException &e)
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch (const ::com::sun::star::script::CannotConvertException &e)
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch (const ::com::sun::star::uno::RuntimeException &e)
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch (const ::com::sun::star::uno::Exception &e)
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }

    return ret.getAcquired();
}

PyObject *PyUNO_str( PyObject * self )
{
    PyUNO *me = reinterpret_cast<PyUNO *>(self);

    OStringBuffer buf;

    {
        PyThreadDetach antiguard;
        buf.append( "pyuno object " );

        OUString s = val2str( me->members->wrappedObject.getValue(),
                              me->members->wrappedObject.getValueType().getTypeLibType() );
        buf.append( OUStringToOString(s,RTL_TEXTENCODING_ASCII_US) );
    }

    return PyStr_FromString( buf.getStr() );
}

PyObject* PyUNO_dir (PyObject* self)
{
    PyUNO* me = reinterpret_cast<PyUNO*>(self);

    PyObject* member_list = NULL;
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
        raisePyExceptionWithAny( makeAny(e) );
    }

    return member_list;
}

sal_Int32 lcl_detach_getLength( PyUNO *me )
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

int PyUNO_bool( PyObject* self )
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
    catch( const ::com::sun::star::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }

    return -1;
}

Py_ssize_t PyUNO_len( PyObject* self )
{
    PyUNO* me = reinterpret_cast<PyUNO*>(self);

    try
    {
        int nLen = lcl_detach_getLength( me );
        if (nLen >= 0)
            return nLen;

        PyErr_SetString( PyExc_TypeError, "object has no len()" );
    }
    catch( const ::com::sun::star::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }

    return -1;
}

void lcl_getRowsColumns( PyUNO* me, sal_Int32& nRows, sal_Int32& nColumns )
{
    Sequence<short> aOutParamIndex;
    Sequence<Any> aOutParam;
    Sequence<Any> aParams;
    Any aRet;

    aRet = me->members->xInvocation->invoke ( "getRows", aParams, aOutParamIndex, aOutParam );
    Reference< XIndexAccess > xIndexAccessRows( aRet, UNO_QUERY );
    nRows = xIndexAccessRows->getCount();
    aRet = me->members->xInvocation->invoke ( "getColumns", aParams, aOutParamIndex, aOutParam );
    Reference< XIndexAccess > xIndexAccessCols( aRet, UNO_QUERY );
    nColumns = xIndexAccessCols->getCount();
}

PyRef lcl_indexToSlice( PyRef rIndex )
{
    Py_ssize_t nIndex = PyNumber_AsSsize_t( rIndex.get(), PyExc_IndexError );
    if (nIndex == -1 && PyErr_Occurred())
        return NULL;
    PyRef rStart( PyLong_FromSsize_t( nIndex ), SAL_NO_ACQUIRE );
    PyRef rStop( PyLong_FromSsize_t( nIndex+1 ), SAL_NO_ACQUIRE );
    PyRef rStep( PyLong_FromLong( 1 ), SAL_NO_ACQUIRE );
    PyRef rSlice( PySlice_New( rStart.get(), rStop.get(), rStep.get() ), SAL_NO_ACQUIRE );

    return rSlice;
}

PyObject* lcl_getitem_XCellRange( PyUNO* me, PyObject* pKey )
{
    Runtime runtime;

    Sequence<short> aOutParamIndex;
    Sequence<Any> aOutParam;
    Sequence<Any> aParams;
    Any aRet;

    // Single string key is sugar for getCellRangeByName()
    if ( PyStr_Check( pKey ) ) {

        aParams.realloc (1);
        aParams[0] <<= pyString2ustring( pKey );
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
        rKey1 = PySlice_New( NULL, NULL, NULL );
    }
    else if ( PyTuple_Check( pKey ) && (PyTuple_Size( pKey ) == 2) )
    {
        rKey0 = PyTuple_GetItem( pKey, 0 );
        rKey1 = PyTuple_GetItem( pKey, 1 );
    }
    else
    {
        PyErr_SetString( PyExc_KeyError, "invalid subscript" );
        return NULL;
    }

    // If both keys are indices, return the corresponding cell
    if ( PyIndex_Check( rKey0.get() ) && PyIndex_Check( rKey1.get() ))
    {
        sal_Int32 nKey0_s = lcl_PyNumber_AsSal_Int32( rKey0.get() );
        sal_Int32 nKey1_s = lcl_PyNumber_AsSal_Int32( rKey1.get() );

        if ( ((nKey0_s == -1) || (nKey1_s == -1)) && PyErr_Occurred() )
            return NULL;

        aParams.realloc( 2 );
        aParams[0] <<= nKey1_s;
        aParams[1] <<= nKey0_s;
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
            return NULL;

        if ( nSliceLength0 <= 0 || nSliceLength1 <= 0 )
        {
            PyErr_SetString( PyExc_KeyError, "invalid number of rows or columns" );
            return NULL;
        }

        if ( nStep0 == 1 && nStep1 == 1 )
        {
            aParams.realloc (4);
            aParams[0] <<= nStart1;
            aParams[1] <<= nStart0;
            aParams[2] <<= nStop1 - 1;
            aParams[3] <<= nStop0 - 1;
            {
                PyThreadDetach antiguard;
                aRet = me->members->xInvocation->invoke (
                    "getCellRangeByPosition", aParams, aOutParamIndex, aOutParam );
            }
            PyRef rRet = runtime.any2PyObject( aRet );
            return rRet.getAcquired();
        }

        PyErr_SetString( PyExc_KeyError, "step != 1 not supported" );
        return NULL;
    }

    PyErr_SetString( PyExc_KeyError, "invalid subscript" );
    return NULL;
}

PyObject* lcl_getitem_index( PyUNO *me, PyObject *pKey, Runtime& runtime )
{
    Any aRet;
    sal_Int32 nIndex;

    nIndex = lcl_PyNumber_AsSal_Int32( pKey );
    if (nIndex == -1 && PyErr_Occurred())
        return NULL;

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

    return NULL;
}

PyObject* lcl_getitem_slice( PyUNO *me, PyObject *pKey )
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

    if ( xIndexAccess.is() )
    {
        sal_Int32 nStart = 0, nStop = 0, nStep = 0, nSliceLength = 0;
        int nSuccess = lcl_PySlice_GetIndicesEx(pKey, nLen, &nStart, &nStop, &nStep, &nSliceLength);
        if ( nSuccess == -1 && PyErr_Occurred() )
            return NULL;

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

    return NULL;
}

PyObject* lcl_getitem_string( PyUNO *me, PyObject *pKey, Runtime& runtime )
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

    return NULL;
}

PyObject* PyUNO_getitem( PyObject *self, PyObject *pKey )
{
    PyUNO* me = reinterpret_cast<PyUNO*>(self);
    Runtime runtime;

    try
    {
        // XIndexAccess access by index
        if ( PyIndex_Check( pKey ) )
        {
            PyObject* pRet = lcl_getitem_index( me, pKey, runtime );
            if ( pRet != NULL || PyErr_Occurred() )
                return pRet;
        }

        // XIndexAccess access by slice
        if ( PySlice_Check( pKey ) )
        {
            PyObject* pRet = lcl_getitem_slice( me, pKey );
            if ( pRet != NULL || PyErr_Occurred() )
                return pRet;
        }

        // XNameAccess access by key
        if ( PyStr_Check( pKey ) )
        {
            PyObject* pRet = lcl_getitem_string( me, pKey, runtime );
            if ( pRet != NULL )
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
            return NULL;
        }

        PyErr_SetString( PyExc_TypeError, "object is not subscriptable" );
    }
    catch( const ::com::sun::star::lang::IndexOutOfBoundsException )
    {
        PyErr_SetString( PyExc_IndexError, "index out of range" );
    }
    catch( const ::com::sun::star::container::NoSuchElementException )
    {
        PyErr_SetString( PyExc_KeyError, "key not found" );
    }
    catch( const com::sun::star::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( const com::sun::star::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( const ::com::sun::star::lang::WrappedTargetException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( const ::com::sun::star::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }

    return NULL;
}

int lcl_setitem_index( PyUNO *me, PyObject *pKey, PyObject *pValue )
{
    Runtime runtime;

    Reference< XIndexContainer > xIndexContainer;
    Reference< XIndexReplace > xIndexReplace;
    sal_Int32 nIndex = lcl_PyNumber_AsSal_Int32( pKey );
    if ( nIndex == -1 && PyErr_Occurred() )
        return 0;

    bool isTuple = false;

    Any aValue;
    if ( pValue != NULL )
    {
        isTuple = PyTuple_Check( pValue );

        try
        {
            aValue <<= runtime.pyObject2Any( pValue );
        }
        catch ( const ::com::sun::star::uno::RuntimeException )
        {
            // TODO pyObject2Any can't convert e.g. dicts but only throws
            // RuntimeException on failure. Fixing this will require an audit of
            // all the rest of PyUNO
            throw ::com::sun::star::script::CannotConvertException();
        }
    }

    {
        PyThreadDetach antiguard;

        xIndexContainer.set( me->members->xInvocation, UNO_QUERY );
        if ( xIndexContainer.is() )
            xIndexReplace.set( xIndexContainer, UNO_QUERY );
        else
            xIndexReplace.set( me->members->xInvocation, UNO_QUERY );

        if ( xIndexReplace.is() && nIndex < 0 )
            nIndex += xIndexReplace->getCount();

        // XIndexReplace replace by index
        if ( (pValue != NULL) && xIndexReplace.is() )
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
        if ( (pValue == NULL) && xIndexContainer.is() )
        {
            xIndexContainer->removeByIndex( nIndex );
            return 0;
        }
    }

    PyErr_SetString( PyExc_TypeError, "cannot assign to object" );
    return 1;
}

int lcl_setitem_slice( PyUNO *me, PyObject *pKey, PyObject *pValue )
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
            xIndexReplace.set( xIndexContainer, UNO_QUERY );
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

        if ( pValue == NULL )
        {
            pValue = PyTuple_New( 0 );
        }

        if ( !PyTuple_Check (pValue) )
        {
            PyErr_SetString( PyExc_TypeError, "value is not a tuple" );
            return 1;
        }

        Py_ssize_t nTupleLength_ssize = PyTuple_Size( pValue );
        if ( nTupleLength_ssize > SAL_MAX_INT32 )
        {
            PyErr_SetString( PyExc_ValueError, "tuple too large" );
            return 1;
        }
        sal_Int32 nTupleLength = (sal_Int32)nTupleLength_ssize;

        if ( (nTupleLength != nSliceLength) && (nStep != 1) )
        {
            PyErr_SetString( PyExc_ValueError, "number of items assigned must be equal" );
            return 1;
        }

        if ( (nTupleLength != nSliceLength) && !xIndexContainer.is() )
        {
            PyErr_SetString( PyExc_ValueError, "cannot change length" );
            return 1;
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
                    aItem <<= runtime.pyObject2Any( rItem.get() );
                }
                catch ( const ::com::sun::star::uno::RuntimeException )
                {
                    // TODO pyObject2Any can't convert e.g. dicts but only throws
                    // RuntimeException on failure. Fixing this will require an audit of
                    // all the rest of PyUNO
                    throw ::com::sun::star::script::CannotConvertException();
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
    return 1;
}

int lcl_setitem_string( PyUNO *me, PyObject *pKey, PyObject *pValue )
{
    Runtime runtime;

    OUString sKey = pyString2ustring( pKey );
    bool isTuple = false;

    Any aValue;
    if ( pValue != NULL)
    {
        isTuple = PyTuple_Check( pValue );
        try
        {
            aValue <<= runtime.pyObject2Any( pValue );
        }
        catch( const ::com::sun::star::uno::RuntimeException )
        {
            // TODO pyObject2Any can't convert e.g. dicts but only throws
            // RuntimeException on failure. Fixing this will require an audit of
            // all the rest of PyUNO
            throw ::com::sun::star::script::CannotConvertException();
        }
    }

    {
        PyThreadDetach antiguard;

        Reference< XNameContainer > xNameContainer( me->members->xInvocation, UNO_QUERY );
        Reference< XNameReplace > xNameReplace;
        if ( xNameContainer.is() )
            xNameReplace.set( xNameContainer, UNO_QUERY );
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
                    catch( com::sun::star::container::ElementExistException )
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
    return 1;
}

int PyUNO_setitem( PyObject *self, PyObject *pKey, PyObject *pValue )
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
        else if ( PyStr_Check( pKey ) )
        {
            return lcl_setitem_string( me, pKey, pValue );
        }

        PyErr_SetString( PyExc_TypeError, "list index has invalid type" );
    }
    catch( const ::com::sun::star::lang::IndexOutOfBoundsException )
    {
        PyErr_SetString( PyExc_IndexError, "list index out of range" );
    }
    catch( const ::com::sun::star::container::NoSuchElementException )
    {
        PyErr_SetString( PyExc_KeyError, "key not found" );
    }
    catch( const ::com::sun::star::lang::IllegalArgumentException )
    {
        PyErr_SetString( PyExc_TypeError, "value has invalid type" );
    }
    catch( com::sun::star::script::CannotConvertException )
    {
        PyErr_SetString( PyExc_TypeError, "value has invalid type" );
    }
    catch( const ::com::sun::star::container::ElementExistException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( const::com::sun::star::lang::WrappedTargetException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( const ::com::sun::star::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }

    return 1;
}

PyObject* PyUNO_iter( PyObject *self )
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
    catch( com::sun::star::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( com::sun::star::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( const ::com::sun::star::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }

    return NULL;
}

int PyUNO_contains( PyObject *self, PyObject *pKey )
{
    PyUNO* me = reinterpret_cast<PyUNO*>(self);

    Runtime runtime;

    try
    {
        Any aValue;
        try
        {
            aValue <<= runtime.pyObject2Any( pKey );
        }
        catch( const ::com::sun::star::uno::RuntimeException )
        {
            // TODO pyObject2Any can't convert e.g. dicts but only throws
            // RuntimeException on failure. Fixing this will require an audit of
            // all the rest of PyUNO
            throw ::com::sun::star::script::CannotConvertException();
        }

        // XNameAccess is tried first, because checking key presence is much more
        // useful for objects which implement both XIndexAccess and XNameAccess

        // For XNameAccess
        if ( PyStr_Check( pKey ) )
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
            PyObject* pItem;
            while ( (pItem = PyIter_Next( rIterator.get() )) )
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
    catch( const com::sun::star::script::CannotConvertException )
    {
        PyErr_SetString( PyExc_TypeError, "invalid type passed as left argument to 'in'" );
    }
    catch( const ::com::sun::star::container::NoSuchElementException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( const ::com::sun::star::lang::IndexOutOfBoundsException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( const com::sun::star::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( const ::com::sun::star::lang::WrappedTargetException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }
    catch( const ::com::sun::star::uno::RuntimeException &e )
    {
        raisePyExceptionWithAny( com::sun::star::uno::makeAny( e ) );
    }

    return -1;
}

PyObject* PyUNO_getattr (PyObject* self, char* name)
{
    PyUNO* me;

    try
    {

        Runtime runtime;

        me = reinterpret_cast<PyUNO*>(self);
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
        if (me->members->xInvocation->hasMethod (attrName))
        {
            //Create a callable object to invoke this...
            PyRef ret = PyUNO_callable_new (
                me->members->xInvocation,
                attrName);
            Py_XINCREF( ret.get() );
            return ret.get();

        }

        //or a property
        if (me->members->xInvocation->hasProperty ( attrName))
        {
            //Return the value of the property
            Any anyRet;
            {
                PyThreadDetach antiguard;
                anyRet = me->members->xInvocation->getValue (attrName);
            }
            PyRef ret = runtime.any2PyObject(anyRet);
            Py_XINCREF( ret.get() );
            return ret.get();
        }

        //or else...
        PyErr_SetString (PyExc_AttributeError, name);
    }
    catch( const com::sun::star::reflection::InvocationTargetException & e )
    {
        raisePyExceptionWithAny( e.TargetException );
    }
    catch( const com::sun::star::beans::UnknownPropertyException & e )
    {
        raisePyExceptionWithAny( makeAny(e) );
    }
    catch( const com::sun::star::lang::IllegalArgumentException &e )
    {
        raisePyExceptionWithAny( makeAny(e) );
    }
    catch( const com::sun::star::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( makeAny(e) );
    }
    catch( const RuntimeException &e )
    {
        raisePyExceptionWithAny( makeAny(e) );
    }

    return NULL;
}

int PyUNO_setattr (PyObject* self, char* name, PyObject* value)
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
    catch( const com::sun::star::reflection::InvocationTargetException & e )
    {
        raisePyExceptionWithAny( e.TargetException );
        return 1;
    }
    catch( const com::sun::star::beans::UnknownPropertyException & e )
    {
        raisePyExceptionWithAny( makeAny(e) );
        return 1;
    }
    catch( const com::sun::star::script::CannotConvertException &e )
    {
        raisePyExceptionWithAny( makeAny(e) );
        return 1;
    }
    catch( const RuntimeException & e )
    {
        raisePyExceptionWithAny( makeAny( e ) );
        return 1;
    }
    PyErr_SetString (PyExc_AttributeError, name);
    return 1; //as above.
}

static PyObject* PyUNO_cmp( PyObject *self, PyObject *that, int op )
{
    PyObject *result;

    if(op != Py_EQ && op != Py_NE)
    {
        PyErr_SetString(PyExc_TypeError, "only '==' and '!=' comparisons are defined");
        return 0;
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
            com::sun::star::uno::TypeClass tcMe = me->members->wrappedObject.getValueTypeClass();
            com::sun::star::uno::TypeClass tcOther = other->members->wrappedObject.getValueTypeClass();

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
    catch( const com::sun::star::uno::RuntimeException & e)
    {
        raisePyExceptionWithAny( makeAny( e ) );
    }

    result = (op == Py_EQ ? Py_False : Py_True);
    Py_INCREF(result);
    return result;
}

static PyMethodDef PyUNOMethods[] =
{
    {"__dir__",    reinterpret_cast<PyCFunction>(PyUNO_dir),    METH_NOARGS,  NULL},
    {NULL,         NULL,                                        0,            NULL}
};

static PyNumberMethods PyUNONumberMethods[] =
{
    nullptr,                                         /* nb_add */
    nullptr,                                         /* nb_subtract */
    nullptr,                                         /* nb_multiply */
#if PY_MAJOR_VERSION < 3
    nullptr,                                         /* nb_divide */
#endif
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
#if PY_MAJOR_VERSION < 3
    nullptr,                                         /* nb_coerce */
#endif
    nullptr,                                         /* nb_int */
    nullptr,                                         /* nb_reserved */
    nullptr,                                         /* nb_float */
#if PY_MAJOR_VERSION < 3
    nullptr,                                         /* nb_oct */
    nullptr,                                         /* nb_hex */
#endif
    nullptr,                                         /* nb_inplace_add */
    nullptr,                                         /* nb_inplace_subtract */
    nullptr,                                         /* nb_inplace_multiply */
#if PY_MAJOR_VERSION < 3
    nullptr,                                         /* nb_inplace_divide */
#endif
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
    nullptr,                                         /* nb_matrix_multiply */
    nullptr                                          /* nb_inplace_matrix_multiply */
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
    nullptr,
    PyUNO_getattr,
    PyUNO_setattr,
    /* this type does not exist in Python 3: (cmpfunc) */ 0,
    PyUNO_repr,
    PyUNONumberMethods,
    PyUNOSequenceMethods,
    PyUNOMappingMethods,
    PyUNO_hash,
    nullptr,
    PyUNO_str,
    nullptr,
    nullptr,
    NULL,
    Py_TPFLAGS_HAVE_ITER | Py_TPFLAGS_HAVE_RICHCOMPARE | Py_TPFLAGS_HAVE_SEQUENCE_IN,
    NULL,
    nullptr,
    nullptr,
    PyUNO_cmp,
    0,
    PyUNO_iter,
    nullptr,
    PyUNOMethods,
    NULL,
    NULL,
    NULL,
    NULL,
    nullptr,
    nullptr,
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    nullptr
#if PY_VERSION_HEX >= 0x02060000
    , 0
#endif
#if PY_VERSION_HEX >= 0x03040000
    , 0
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
            ssf->createInstanceWithArguments( Sequence<Any>( &targetInterface, 1 ) ), UNO_QUERY );
        if( !xInvocation.is() )
            throw RuntimeException("XInvocation2 not implemented, cannot interact with object");

        Reference<XUnoTunnel> xUnoTunnel (
            xInvocation->getIntrospection()->queryAdapter(cppu::UnoType<XUnoTunnel>::get()), UNO_QUERY );
        if( xUnoTunnel.is() )
        {
            sal_Int64 that = xUnoTunnel->getSomething( ::pyuno::Adapter::getUnoTunnelImplementationId() );
            if( that )
                return PyRef( reinterpret_cast<Adapter*>(that)->getWrappedObject() );
        }
    }
    if( !Py_IsInitialized() )
        throw RuntimeException();

    PyUNO* self = PyObject_New (PyUNO, &PyUNOType);
    if (self == NULL)
        return PyRef(); // == error
    self->members = new PyUNOInternals();
    self->members->xInvocation = xInvocation;
    self->members->wrappedObject = targetInterface;
    return PyRef( reinterpret_cast<PyObject*>(self), SAL_NO_ACQUIRE );

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
