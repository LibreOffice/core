/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: test_cuno.c,v $
 * $Revision: 1.4 $
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

#include <stdio.h>
#include <osl/diagnose.h>
#include <osl/time.h>
#include <osl/interlck.h>
#include <rtl/alloc.h>
//
/*
#include <uno/dispatcher.h>
#include <uno/environment.h>
#include <uno/mapping.hxx>
*/
#include <uno/data.h>
#include <uno/any2.h>
#include <uno/sequence2.h>

#include <test/XLanguageBindingTest.h>


typedef struct _InstanceData
{
    void const * m_XInterface[2];
    void const * m_XLBTestBase[2];
    void const * m_XLanguageBindingTest[2];

    sal_Int32                m_refCount;
    typelib_TypeDescription* m_pTDXInterface;
    typelib_TypeDescription* m_pTDSeqTestElement;
    typelib_TypeDescription* m_pTDTestDataElements;
    test_TestDataElements    m_data, m_structData;
} InstanceData;

#define GET_THIS( p ) (InstanceData *)((void **)p)[1]

//==================================================================================================
static void SAL_CALL c_acquire( void * p )
    SAL_THROW_EXTERN_C( )
{
    CUNO_CALL( ((com_sun_star_uno_XInterface *)p) )->acquire( (com_sun_star_uno_XInterface *)p );
}
//==================================================================================================
static  void SAL_CALL c_release( void * p )
    SAL_THROW_EXTERN_C( )
{
    CUNO_CALL( ((com_sun_star_uno_XInterface *)p) )->release( (com_sun_star_uno_XInterface *)p );
}
//==================================================================================================
static void * SAL_CALL c_queryInterface( void * p, typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C( )
{
    uno_Any aExc;
    com_sun_star_uno_XInterface * pRet = NULL;

    if (CUNO_EXCEPTION_OCCURED( CUNO_CALL( ((com_sun_star_uno_XInterface *)p) )->queryInterface( (com_sun_star_uno_XInterface *)p, &aExc, &pRet, pType ) ))
    {
        uno_any_destruct( &aExc, c_release );
        return NULL;
    }
    else
    {
        return pRet;
    }
}

void defaultConstructData(test_TestDataElements* pData, typelib_TypeDescriptionReference * pElemType)
{
    pData->_Base._Base.Bool = sal_False;
    pData->_Base._Base.Char = 0;
    pData->_Base._Base.Byte = 0;
    pData->_Base._Base.Short = 0;
    pData->_Base._Base.UShort = 0;
    pData->_Base._Base.Long = 0;
    pData->_Base._Base.ULong = 0;
    pData->_Base._Base.Hyper = 0;
    pData->_Base._Base.UHyper = 0;
    pData->_Base._Base.Float = 0;
    pData->_Base._Base.Double = 0;
    pData->_Base._Base.Enum = test_TestEnum_TEST;
    pData->_Base.String = 0;
    rtl_uString_new(&pData->_Base.String);
    pData->_Base.Interface = 0;
    uno_any_construct(&pData->_Base.Any, 0, 0, 0);
/*  pData->Sequence = 0; */
    uno_type_sequence_construct(
        &pData->Sequence, pElemType, 0, 0, c_acquire );
}

void assign1( test_TestSimple* rData,
             sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
             sal_Int16 nShort, sal_uInt16 nUShort,
             sal_Int32 nLong, sal_uInt32 nULong,
             sal_Int64 nHyper, sal_uInt64 nUHyper,
             float fFloat, double fDouble,
             test_TestEnum eEnum)
{
    rData->Bool = bBool;
    rData->Char = cChar;
    rData->Byte = nByte;
    rData->Short = nShort;
    rData->UShort = nUShort;
    rData->Long = nLong;
    rData->ULong = nULong;
    rData->Hyper = nHyper;
    rData->UHyper = nUHyper;
    rData->Float = fFloat;
    rData->Double = fDouble;
    rData->Enum = eEnum;
}

void assign2( test_TestElement* rData,
             sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
             sal_Int16 nShort, sal_uInt16 nUShort,
             sal_Int32 nLong, sal_uInt32 nULong,
             sal_Int64 nHyper, sal_uInt64 nUHyper,
             float fFloat, double fDouble,
             test_TestEnum eEnum, rtl_uString* rStr,
             com_sun_star_uno_XInterface* xTest,
             uno_Any* rAny,
             typelib_TypeDescription* pTDIface)
{
    assign1( (test_TestSimple *)rData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum );
    rtl_uString_assign(&rData->String, rStr);
/*  uno_assignData(&rData->Interface, pTDIface, &xTest, pTDIface, c_queryInterface, c_acquire, c_release); */
    if ( rData->Interface )
         CUNO_CALL(rData->Interface)->release(rData->Interface);

    if ( xTest )
    {
        CUNO_CALL(xTest)->acquire(xTest);
        rData->Interface = xTest;
    } else
    {
        rData->Interface = 0;
    }

    uno_type_any_assign(&rData->Any, rAny->pData, rAny->pType, c_acquire, c_release);
}

void assign3( test_TestDataElements* rData,
             sal_Bool bBool, sal_Unicode cChar, sal_Int8 nByte,
             sal_Int16 nShort, sal_uInt16 nUShort,
             sal_Int32 nLong, sal_uInt32 nULong,
             sal_Int64 nHyper, sal_uInt64 nUHyper,
             float fFloat, double fDouble,
             test_TestEnum eEnum, rtl_uString* rStr,
             com_sun_star_uno_XInterface* xTest,
             uno_Any* rAny,
             /* sequence< test_TestElement >*/uno_Sequence* rSequence,
             typelib_TypeDescription* pTDIface,
             typelib_TypeDescription* pTDSeqElem)
{
    assign2( (test_TestElement *)rData,
            bBool, cChar, nByte, nShort, nUShort, nLong, nULong, nHyper, nUHyper, fFloat, fDouble,
            eEnum,rStr, xTest, rAny, pTDIface );
    uno_sequence_assign(&rData->Sequence, rSequence, pTDSeqElem, c_release);
}

/* XInterface =============================================================================== */

/* XInterface::acquire */
cuno_ErrorCode SAL_CALL XInterface_acquire( com_sun_star_uno_XInterface* pIFace)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    osl_incrementInterlockedCount( &pImpl->m_refCount );
    return CUNO_ERROR_NONE;
}

/* XInterface::release */
cuno_ErrorCode SAL_CALL XInterface_release( com_sun_star_uno_XInterface * pIFace )
{
    InstanceData * pImpl = GET_THIS( pIFace );
    if( osl_decrementInterlockedCount( &pImpl->m_refCount ) == 0)
    {
        uno_destructData(&pImpl->m_data, pImpl->m_pTDTestDataElements, c_release);
        uno_destructData(&pImpl->m_structData, pImpl->m_pTDTestDataElements, c_release);
        typelib_typedescription_release(pImpl->m_pTDXInterface);
        typelib_typedescription_release(pImpl->m_pTDSeqTestElement);
        typelib_typedescription_release(pImpl->m_pTDTestDataElements);
        rtl_freeMemory( pImpl );
    }
    return CUNO_ERROR_NONE;
}

/* XInterface::queryInterface */
cuno_ErrorCode SAL_CALL XInterface_queryInterface( com_sun_star_uno_XInterface * pIFace, uno_Any * pExc, com_sun_star_uno_XInterface ** pRet, typelib_TypeDescriptionReference * pTypeRef)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    com_sun_star_uno_XInterface * pIFaceRet = 0;
    if ( pTypeRef )
    {
        if( ! rtl_ustr_ascii_compare_WithLength( pTypeRef->pTypeName->buffer, pTypeRef->pTypeName->length,
                                                 "com.sun.star.uno.XInterface" ) )
               pIFaceRet = (com_sun_star_uno_XInterface *)&pImpl->m_XInterface;
        else if( !rtl_ustr_ascii_compare_WithLength( pTypeRef->pTypeName->buffer, pTypeRef->pTypeName->length,
                                                        "test.XLBTestBase" ) )
               pIFaceRet = (com_sun_star_uno_XInterface *)&pImpl->m_XLBTestBase;
        else if( !rtl_ustr_ascii_compare_WithLength( pTypeRef->pTypeName->buffer, pTypeRef->pTypeName->length,
                                                        "test.XLanguageBindingTest" ) )
               pIFaceRet = (com_sun_star_uno_XInterface *)&pImpl->m_XLanguageBindingTest;

        if( pIFaceRet )
        {
            CUNO_CALL(pIFaceRet)->acquire( pIFaceRet );
            *pRet = pIFaceRet;
        } else
        {
            *pRet = 0;
        }
    }
    return CUNO_ERROR_NONE;
}

/* XLBTestBase =============================================================================== */

/* XLBTestBase::getBool */
cuno_ErrorCode SAL_CALL XLBTestBase_getBool( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Bool *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.Bool;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setBool */
cuno_ErrorCode SAL_CALL XLBTestBase_setBool( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Bool value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.Bool = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getByte */
cuno_ErrorCode SAL_CALL XLBTestBase_getByte( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Int8 *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.Byte;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setByte */
cuno_ErrorCode SAL_CALL XLBTestBase_setByte( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Int8 value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.Byte = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getChar */
cuno_ErrorCode SAL_CALL XLBTestBase_getChar( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Unicode *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.Char;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setChar */
cuno_ErrorCode SAL_CALL XLBTestBase_setChar( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Unicode value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.Char = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getShort */
cuno_ErrorCode SAL_CALL XLBTestBase_getShort( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Int16 *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.Short;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setShort */
cuno_ErrorCode SAL_CALL XLBTestBase_setShort( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Int16 value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.Short = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getUShort */
cuno_ErrorCode SAL_CALL XLBTestBase_getUShort( test_XLBTestBase * pIFace, uno_Any * pExc, sal_uInt16 *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.UShort;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setUShort */
cuno_ErrorCode SAL_CALL XLBTestBase_setUShort( test_XLBTestBase * pIFace, uno_Any * pExc, sal_uInt16 value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.UShort = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getLong */
cuno_ErrorCode SAL_CALL XLBTestBase_getLong( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Int32 *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.Long;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setLong */
cuno_ErrorCode SAL_CALL XLBTestBase_setLong( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Int32 value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.Long = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getULong */
cuno_ErrorCode SAL_CALL XLBTestBase_getULong( test_XLBTestBase * pIFace, uno_Any * pExc, sal_uInt32 *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.ULong;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setULong */
cuno_ErrorCode SAL_CALL XLBTestBase_setULong( test_XLBTestBase * pIFace, uno_Any * pExc, sal_uInt32 value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.ULong = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getHyper */
cuno_ErrorCode SAL_CALL XLBTestBase_getHyper( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Int64 *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.Hyper;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setHyper */
cuno_ErrorCode SAL_CALL XLBTestBase_setHyper( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Int64 value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.Hyper = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getUHyper */
cuno_ErrorCode SAL_CALL XLBTestBase_getUHyper( test_XLBTestBase * pIFace, uno_Any * pExc, sal_uInt64 *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.UHyper;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setUHyper */
cuno_ErrorCode SAL_CALL XLBTestBase_setUHyper( test_XLBTestBase * pIFace, uno_Any * pExc, sal_uInt64 value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.UHyper = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getFloat */
cuno_ErrorCode SAL_CALL XLBTestBase_getFloat( test_XLBTestBase * pIFace, uno_Any * pExc, float *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.Float;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setFloat */
cuno_ErrorCode SAL_CALL XLBTestBase_setFloat( test_XLBTestBase * pIFace, uno_Any * pExc, float value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.Float = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getDouble */
cuno_ErrorCode SAL_CALL XLBTestBase_getDouble( test_XLBTestBase * pIFace, uno_Any * pExc, double *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.Double;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setDouble */
cuno_ErrorCode SAL_CALL XLBTestBase_setDouble( test_XLBTestBase * pIFace, uno_Any * pExc, double value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.Double = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getEnum */
cuno_ErrorCode SAL_CALL XLBTestBase_getEnum( test_XLBTestBase * pIFace, uno_Any * pExc, test_TestEnum *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = pImpl->m_data._Base._Base.Enum;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setEnum */
cuno_ErrorCode SAL_CALL XLBTestBase_setEnum( test_XLBTestBase * pIFace, uno_Any * pExc, test_TestEnum value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    pImpl->m_data._Base._Base.Enum = value;
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getString */
cuno_ErrorCode SAL_CALL XLBTestBase_getString( test_XLBTestBase * pIFace, uno_Any * pExc, rtl_uString **pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    *pRet = 0;
    rtl_uString_newFromString(pRet, pImpl->m_data._Base.String);
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setString */
cuno_ErrorCode SAL_CALL XLBTestBase_setString( test_XLBTestBase * pIFace, uno_Any * pExc, rtl_uString *value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    rtl_uString_assign(&pImpl->m_data._Base.String, value);
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getInterface */
cuno_ErrorCode SAL_CALL XLBTestBase_getInterface( test_XLBTestBase * pIFace, uno_Any * pExc, com_sun_star_uno_XInterface **pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
/*  uno_copyData(pRet, &pImpl->m_data._Base.Interface, pImpl->m_pTDXInterface, c_acquire); */
    if ( pImpl->m_data._Base.Interface )
    {
        CUNO_CALL(pImpl->m_data._Base.Interface)->acquire(pImpl->m_data._Base.Interface);
        *pRet = pImpl->m_data._Base.Interface;
    } else
    {
        *pRet = 0;
    }
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setInterface */
cuno_ErrorCode SAL_CALL XLBTestBase_setInterface( test_XLBTestBase * pIFace, uno_Any * pExc, com_sun_star_uno_XInterface *value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
/*  uno_assignData(&pImpl->m_data._Base.Interface, pImpl->m_pTDXInterface, &value, pImpl->m_pTDXInterface, c_queryInterface, c_acquire, c_release); */
    if ( pImpl->m_data._Base.Interface )
         CUNO_CALL(pImpl->m_data._Base.Interface)->release(pImpl->m_data._Base.Interface);

    if ( value )
    {
        CUNO_CALL(value)->acquire(value);
        pImpl->m_data._Base.Interface = value;
    } else
    {
        pImpl->m_data._Base.Interface = 0;
    }
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getAny */
cuno_ErrorCode SAL_CALL XLBTestBase_getAny( test_XLBTestBase * pIFace, uno_Any * pExc, uno_Any *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    typelib_TypeDescription* pTD = 0;
    typelib_typedescriptionreference_getDescription(&pTD, pImpl->m_data._Base.Any.pType);
    uno_any_construct(pRet, pImpl->m_data._Base.Any.pData, pTD, c_acquire);
    typelib_typedescription_release(pTD);
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setAny */
cuno_ErrorCode SAL_CALL XLBTestBase_setAny( test_XLBTestBase * pIFace, uno_Any * pExc, uno_Any *value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    typelib_TypeDescription* pTD = 0;
    typelib_typedescriptionreference_getDescription(&pTD, value->pType);
    uno_any_assign(&pImpl->m_data._Base.Any, value->pData, pTD, c_acquire, c_release);
    typelib_typedescription_release(pTD);
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getSequence */
cuno_ErrorCode SAL_CALL XLBTestBase_getSequence( test_XLBTestBase * pIFace, uno_Any * pExc, /*sequence< test.TestElement >*/uno_Sequence **pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    uno_sequence_construct(pRet, pImpl->m_pTDSeqTestElement, pImpl->m_data.Sequence->elements, pImpl->m_data.Sequence->nElements, c_acquire);
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setSequence */
cuno_ErrorCode SAL_CALL XLBTestBase_setSequence( test_XLBTestBase * pIFace, uno_Any * pExc, /*sequence< test.TestElement >*/uno_Sequence *value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    uno_sequence_assign(&pImpl->m_data.Sequence, value, pImpl->m_pTDSeqTestElement, c_release);
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getStruct */
cuno_ErrorCode SAL_CALL XLBTestBase_getStruct( test_XLBTestBase * pIFace, uno_Any * pExc, test_TestDataElements *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    uno_copyData(pRet, &pImpl->m_structData, pImpl->m_pTDTestDataElements, c_acquire);
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setStruct */
cuno_ErrorCode SAL_CALL XLBTestBase_setStruct( test_XLBTestBase * pIFace, uno_Any * pExc, test_TestDataElements *value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    uno_assignData(&pImpl->m_structData, pImpl->m_pTDTestDataElements, value, pImpl->m_pTDTestDataElements, c_queryInterface, c_acquire, c_release);
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setValues */
cuno_ErrorCode SAL_CALL XLBTestBase_setValues( test_XLBTestBase * pIFace, uno_Any * pExc, sal_Bool aBool, sal_Unicode aChar, sal_Int8 aByte, sal_Int16 aShort, sal_uInt16 aUShort, sal_Int32 aLong, sal_uInt32 aULong, sal_Int64 aHyper, sal_uInt64 aUHyper, float aFloat, double aDouble, test_TestEnum aEnum, rtl_uString* aString, com_sun_star_uno_XInterface *aInterface, uno_Any * aAny, /*sequence< test.TestElement >*/ uno_Sequence * aSequence, test_TestDataElements *aStruct)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    assign3(&pImpl->m_data, aBool, aChar, aByte, aShort, aUShort, aLong, aULong, aHyper, aUHyper, aFloat, aDouble,
            aEnum, aString, aInterface, aAny,aSequence, pImpl->m_pTDXInterface, pImpl->m_pTDSeqTestElement);
    uno_assignData(&pImpl->m_structData, pImpl->m_pTDTestDataElements, aStruct, pImpl->m_pTDTestDataElements, c_queryInterface, c_acquire, c_release);
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::setValues2 */
cuno_ErrorCode SAL_CALL XLBTestBase_setValues2( test_XLBTestBase * pIFace, uno_Any * pExc, test_TestDataElements* pRet, sal_Bool* aBool, sal_Unicode* aChar, sal_Int8* aByte, sal_Int16* aShort, sal_uInt16* aUShort, sal_Int32* aLong, sal_uInt32* aULong, sal_Int64* aHyper, sal_uInt64* aUHyper, float* aFloat, double* aDouble, test_TestEnum* aEnum, rtl_uString** aString, com_sun_star_uno_XInterface **aInterface, uno_Any * aAny, /*sequence< test.TestElement >*/ uno_Sequence ** aSequence, test_TestDataElements * aStruct)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    assign3(&pImpl->m_data, *aBool, *aChar, *aByte, *aShort, *aUShort, *aLong, *aULong, *aHyper, *aUHyper, *aFloat, *aDouble,
            *aEnum, *aString, *aInterface, aAny, *aSequence, pImpl->m_pTDXInterface, pImpl->m_pTDSeqTestElement);
    uno_assignData(&pImpl->m_structData, pImpl->m_pTDTestDataElements, aStruct, pImpl->m_pTDTestDataElements, c_queryInterface, c_acquire, c_release);
    uno_copyData(pRet, &pImpl->m_structData, pImpl->m_pTDTestDataElements, c_acquire);
    return CUNO_ERROR_NONE;
}
/* XLBTestBase::getValues */
cuno_ErrorCode SAL_CALL XLBTestBase_getValues( test_XLBTestBase * pIFace, uno_Any * pExc, test_TestDataElements* pRet, sal_Bool* aBool, sal_Unicode* aChar, sal_Int8* aByte, sal_Int16* aShort, sal_uInt16* aUShort, sal_Int32* aLong, sal_uInt32* aULong, sal_Int64* aHyper, sal_uInt64* aUHyper, float* aFloat, double* aDouble, test_TestEnum* aEnum, rtl_uString** aString, com_sun_star_uno_XInterface **aInterface, uno_Any * aAny, /*sequence< test.TestElement >*/ uno_Sequence ** aSequence, test_TestDataElements * aStruct)
{
    typelib_TypeDescription* pTD = 0;
    InstanceData * pImpl = GET_THIS( pIFace );
    *aBool = pImpl->m_data._Base._Base.Bool;
    *aChar = pImpl->m_data._Base._Base.Char;
    *aByte = pImpl->m_data._Base._Base.Byte;
    *aShort = pImpl->m_data._Base._Base.Short;
    *aUShort = pImpl->m_data._Base._Base.UShort;
    *aLong = pImpl->m_data._Base._Base.Long;
    *aULong = pImpl->m_data._Base._Base.ULong;
    *aHyper = pImpl->m_data._Base._Base.Hyper;
    *aUHyper = pImpl->m_data._Base._Base.UHyper;
    *aFloat = pImpl->m_data._Base._Base.Float;
    *aDouble = pImpl->m_data._Base._Base.Double;
    *aEnum = pImpl->m_data._Base._Base.Enum;
    *aString = 0;
    rtl_uString_newFromString(aString, pImpl->m_data._Base.String);
/*  uno_copyData(aInterface, &pImpl->m_data._Base.Interface, pImpl->m_pTDXInterface, c_acquire); */
    if ( pImpl->m_data._Base.Interface )
    {
        CUNO_CALL(pImpl->m_data._Base.Interface)->acquire(pImpl->m_data._Base.Interface);
        *aInterface = pImpl->m_data._Base.Interface;
    } else
    {
        *aInterface = 0;
    }
    typelib_typedescriptionreference_getDescription(&pTD, pImpl->m_data._Base.Any.pType);
    uno_any_construct(aAny, pImpl->m_data._Base.Any.pData, pTD, c_acquire);
    typelib_typedescription_release(pTD);
    uno_sequence_construct(aSequence, pImpl->m_pTDSeqTestElement, pImpl->m_data.Sequence->elements, pImpl->m_data.Sequence->nElements, c_acquire);
    uno_copyData(aStruct, &pImpl->m_structData, pImpl->m_pTDTestDataElements, c_acquire);
    uno_copyData(pRet, &pImpl->m_structData, pImpl->m_pTDTestDataElements, c_acquire);
    return CUNO_ERROR_NONE;
}

/* XLanguageBindingTest =============================================================================== */

/* XLanguageBindingTest::getRuntimeException */
cuno_ErrorCode SAL_CALL XLanguageBindingTest_getRuntimeException( test_XLanguageBindingTest * pIFace, uno_Any * pExc, sal_Int32 *pRet)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    com_sun_star_uno_RuntimeException aExc;
    typelib_TypeDescription * pTD = 0;
    rtl_uString * pTypeName = 0;
    uno_Any excp;

    rtl_uString_newFromAscii( &pTypeName, "com.sun.star.uno.RuntimeException");
    typelib_typedescription_getByName(&pTD, pTypeName);

    aExc._Base.Message = 0;
    rtl_uString_newFromAscii(&aExc._Base.Message, "dum dum dum ich tanz im kreis herum...");
    aExc._Base.Context = 0;
    if (CUNO_EXCEPTION_OCCURED( CUNO_CALL(pIFace)->getInterface( (test_XLBTestBase *)pIFace, &excp, &aExc._Base.Context) ))
    {
        /* ... */
        uno_any_destruct( &excp, 0 );
    }

    uno_any_construct(pExc, &aExc, pTD, c_acquire);
    uno_destructData(&aExc, pTD, c_release);
    typelib_typedescription_release(pTD);
    rtl_uString_release(pTypeName);

    return CUNO_ERROR_EXCEPTION;
}
/* XLanguageBindingTest::setRuntimeException */
cuno_ErrorCode SAL_CALL XLanguageBindingTest_setRuntimeException( test_XLanguageBindingTest * pIFace, uno_Any * pExc, sal_Int32 value)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    com_sun_star_uno_RuntimeException aExc;
    typelib_TypeDescription * pTD = 0;
    rtl_uString * pTypeName = 0;
    uno_Any excp;

    rtl_uString_newFromAscii( &pTypeName, "com.sun.star.uno.RuntimeException");
    typelib_typedescription_getByName(&pTD, pTypeName);

    aExc._Base.Message = 0;
    rtl_uString_newFromAscii(&aExc._Base.Message, "dum dum dum ich tanz im kreis herum...");
    aExc._Base.Context = 0;
    if (CUNO_EXCEPTION_OCCURED( CUNO_CALL(pIFace)->getInterface( (test_XLBTestBase *)pIFace, &excp, &aExc._Base.Context) ))
    {
        /* ... */
        uno_any_destruct( &excp, 0 );
    }

    uno_any_construct(pExc, &aExc, pTD, c_acquire);
    uno_destructData(&aExc, pTD, c_release);
    typelib_typedescription_release(pTD);
    rtl_uString_release(pTypeName);

    return CUNO_ERROR_EXCEPTION;
}
/* XLanguageBindingTest::raiseException */
cuno_ErrorCode SAL_CALL XLanguageBindingTest_raiseException( test_XLanguageBindingTest * pIFace, uno_Any * pExc, test_TestDataElements* pRet, sal_Bool* aBool, sal_Unicode* aChar, sal_Int8* aByte, sal_Int16* aShort, sal_uInt16* aUShort, sal_Int32* aLong, sal_uInt32* aULong, sal_Int64* aHyper, sal_uInt64* aUHyper, float* aFloat, double* aDouble, test_TestEnum* aEnum, rtl_uString ** aString, com_sun_star_uno_XInterface ** aInterface, uno_Any* aAny, /*sequence< test.TestElement >*/ uno_Sequence ** aSequence, test_TestDataElements* AStruct)
{
    InstanceData * pImpl = GET_THIS( pIFace );
    com_sun_star_lang_IllegalArgumentException aExc;
    typelib_TypeDescription * pTD = 0;
    rtl_uString * pTypeName = 0;
    uno_Any excp;

    rtl_uString_newFromAscii( &pTypeName, "com.sun.star.lang.IllegalArgumentException");
    typelib_typedescription_getByName(&pTD, pTypeName);

    aExc.ArgumentPosition = 5;
    aExc._Base.Message = 0;
    rtl_uString_newFromAscii(&aExc._Base.Message, "dum dum dum ich tanz im kreis herum...");
    aExc._Base.Context = 0;
    if (CUNO_EXCEPTION_OCCURED( CUNO_CALL(pIFace)->getInterface( (test_XLBTestBase *)pIFace, &excp, &aExc._Base.Context) ))
    {
        /* ... */
        uno_any_destruct( &excp, 0 );
    }

    uno_any_construct(pExc, &aExc, pTD, c_acquire);
    uno_destructData(&aExc, pTD, c_release);
    typelib_typedescription_release(pTD);
    rtl_uString_release(pTypeName);

    return CUNO_ERROR_EXCEPTION;
}


static const com_sun_star_uno_XInterface_ftab s_XInterface_ftab={
                           XInterface_queryInterface,
                           XInterface_acquire,
                           XInterface_release,
                          };
static const test_XLBTestBase_ftab s_XLBTestBase_ftab={
                           XInterface_queryInterface,
                           XInterface_acquire,
                           XInterface_release,
                           XLBTestBase_getBool,
                           XLBTestBase_setBool,
                           XLBTestBase_getByte,
                           XLBTestBase_setByte,
                           XLBTestBase_getChar,
                           XLBTestBase_setChar,
                           XLBTestBase_getShort,
                           XLBTestBase_setShort,
                           XLBTestBase_getUShort,
                           XLBTestBase_setUShort,
                           XLBTestBase_getLong,
                           XLBTestBase_setLong,
                           XLBTestBase_getULong,
                           XLBTestBase_setULong,
                           XLBTestBase_getHyper,
                           XLBTestBase_setHyper,
                           XLBTestBase_getUHyper,
                           XLBTestBase_setUHyper,
                           XLBTestBase_getFloat,
                           XLBTestBase_setFloat,
                           XLBTestBase_getDouble,
                           XLBTestBase_setDouble,
                           XLBTestBase_getEnum,
                           XLBTestBase_setEnum,
                           XLBTestBase_getString,
                           XLBTestBase_setString,
                           XLBTestBase_getInterface,
                           XLBTestBase_setInterface,
                           XLBTestBase_getAny,
                           XLBTestBase_setAny,
                           XLBTestBase_getSequence,
                           XLBTestBase_setSequence,
                           XLBTestBase_getStruct,
                           XLBTestBase_setStruct,
                           XLBTestBase_setValues,
                           XLBTestBase_setValues2,
                           XLBTestBase_getValues
                          };
static const test_XLanguageBindingTest_ftab s_XLanguageBindingTest_ftab={
                           XInterface_queryInterface,
                           XInterface_acquire,
                           XInterface_release,
                           XLBTestBase_getBool,
                           XLBTestBase_setBool,
                           XLBTestBase_getByte,
                           XLBTestBase_setByte,
                           XLBTestBase_getChar,
                           XLBTestBase_setChar,
                           XLBTestBase_getShort,
                           XLBTestBase_setShort,
                           XLBTestBase_getUShort,
                           XLBTestBase_setUShort,
                           XLBTestBase_getLong,
                           XLBTestBase_setLong,
                           XLBTestBase_getULong,
                           XLBTestBase_setULong,
                           XLBTestBase_getHyper,
                           XLBTestBase_setHyper,
                           XLBTestBase_getUHyper,
                           XLBTestBase_setUHyper,
                           XLBTestBase_getFloat,
                           XLBTestBase_setFloat,
                           XLBTestBase_getDouble,
                           XLBTestBase_setDouble,
                           XLBTestBase_getEnum,
                           XLBTestBase_setEnum,
                           XLBTestBase_getString,
                           XLBTestBase_setString,
                           XLBTestBase_getInterface,
                           XLBTestBase_setInterface,
                           XLBTestBase_getAny,
                           XLBTestBase_setAny,
                           XLBTestBase_getSequence,
                           XLBTestBase_setSequence,
                           XLBTestBase_getStruct,
                           XLBTestBase_setStruct,
                           XLBTestBase_setValues,
                           XLBTestBase_setValues2,
                           XLBTestBase_getValues,
                           XLanguageBindingTest_getRuntimeException,
                           XLanguageBindingTest_setRuntimeException,
                           XLanguageBindingTest_raiseException
                          };

com_sun_star_uno_XInterface* SAL_CALL createTestObject()
{
    InstanceData *pObj;
    rtl_uString* usXInterface = 0;
    rtl_uString* usSeqTestElement = 0;
    rtl_uString* usTestDataElements = 0;

    /* Create a data instance of the component */
    pObj= (InstanceData*)rtl_allocateMemory( sizeof( InstanceData) );
    pObj->m_XInterface[0] = &s_XInterface_ftab;
    pObj->m_XInterface[1] = pObj;
    pObj->m_XLBTestBase[0] = &s_XLBTestBase_ftab;
    pObj->m_XLBTestBase[1] = pObj;
    pObj->m_XLanguageBindingTest[0] = &s_XLanguageBindingTest_ftab;
    pObj->m_XLanguageBindingTest[1] = pObj;

    /* Initalize the reference counter member and other component data */
    pObj->m_refCount= 1;

    pObj->m_pTDXInterface = 0;
    rtl_uString_newFromAscii( &usXInterface, "com.sun.star.uno.XInterface");
    typelib_typedescription_getByName(&pObj->m_pTDXInterface, usXInterface);

    pObj->m_pTDSeqTestElement = 0;
    rtl_uString_newFromAscii( &usSeqTestElement, "[]test.TestElement");
    typelib_typedescription_getByName(&pObj->m_pTDSeqTestElement, usSeqTestElement);

    pObj->m_pTDTestDataElements = 0;
    rtl_uString_newFromAscii( &usTestDataElements, "test.TestDataElements");
    typelib_typedescription_getByName(&pObj->m_pTDTestDataElements, usTestDataElements);

    defaultConstructData(&pObj->m_data, pObj->m_pTDSeqTestElement->pWeakRef);
    defaultConstructData(&pObj->m_structData, pObj->m_pTDSeqTestElement->pWeakRef);

    rtl_uString_release(usXInterface);
    rtl_uString_release(usSeqTestElement);
    rtl_uString_release(usTestDataElements);
    return (com_sun_star_uno_XInterface *)&pObj->m_XInterface;
}

