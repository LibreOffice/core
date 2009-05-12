/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sequence.cxx,v $
 * $Revision: 1.9 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "datalock.hxx"
#include "sequence.hxx"
#include "flags.hxx"

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

#ifndef INCLUDED_STRING_H
#include <string.h>
#define INCLUDED_STRING_H
#endif

#include "utility.hxx"

#ifndef SIMPLE_REFERENCE_FAST
#  include <stdio.h>
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace sharable
    {
//-----------------------------------------------------------------------------
        namespace Type = data::Type;
        namespace uno = ::com::sun::star::uno;
//-----------------------------------------------------------------------------
static
sal_uInt32 implGetElementSize(sal_uInt8 _aElementType)
{
    OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));
    switch (_aElementType)
    {

    case Type::value_boolean:   return sizeof(sal_Bool);

    case Type::value_short:     return sizeof(sal_Int16);

    case Type::value_int:       return sizeof(sal_Int32);

    case Type::value_long:      return sizeof(sal_Int64);

    case Type::value_double:    return sizeof(double);

    case Type::value_string:    return sizeof(rtl_uString *);

    case Type::value_binary:    return sizeof(sal_uInt8 *);

    case Type::value_any: // results from value_invalid
    default:
        OSL_ENSURE( false, "Invalid type code" );
        return 0;
    }
}

//-----------------------------------------------------------------------------
static inline
sal_uInt32 implGetHeaderSize(sal_uInt32 _nElemSize)
{
    // pad header to elem size, if larger (for safe alignment)
    OSL_ASSERT(_nElemSize > sizeof(sal_Int32) || sizeof(sal_Int32)  % _nElemSize== 0);
    return _nElemSize > sizeof(sal_Int32) ? _nElemSize : sizeof(sal_Int32);
}

//-----------------------------------------------------------------------------
static
inline
sal_Int32& implGetSize(sal_uInt8 * _aSeq)
{
    return * (sal_Int32 *) _aSeq;
}

//-----------------------------------------------------------------------------
static
sal_uInt8 * implSeqAlloc(sal_Int32 _nElements, sal_uInt32 _nElemSize)
{
    sal_uInt32 nTotalSize = implGetHeaderSize(_nElemSize) + _nElements * _nElemSize;

    sal_uInt8 * aResult = (sal_uInt8 *) (new sal_uInt8[nTotalSize]);

    implGetSize(aResult) = _nElements;

    return aResult;
}

//-----------------------------------------------------------------------------

static
void allocSeqData(sal_uInt8 *_pDestAddr,
                 sal_uInt8 _aElementType,
                 sal_Int32 _nElements, sal_uInt32 _nElementSize,
                 void const * _pSourceData)
{
    OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));
    OSL_ASSERT(_nElementSize == implGetElementSize(_aElementType));
    switch (_aElementType)
    {
    case Type::value_boolean:
    case Type::value_short:
    case Type::value_int:
    case Type::value_long:
    case Type::value_double:
        ::memcpy(_pDestAddr,_pSourceData,_nElements * _nElementSize);
        break;

    case Type::value_string:
        {
            OSL_ASSERT(_nElementSize == sizeof(rtl_uString *));

            rtl::OUString const * pSource = static_cast<rtl::OUString const *>(_pSourceData);

            while (--_nElements >= 0)
            {
                rtl_uString * aElement = acquireString(*pSource);

                rtl_uString * * pDest = reinterpret_cast<rtl_uString **>(_pDestAddr);
                *pDest = aElement;

                ++pSource;
                _pDestAddr += sizeof *pDest;
            }
        }
        break;

    case Type::value_binary:
        {
            OSL_ASSERT(_nElementSize == sizeof(sal_uInt8 *));

            uno::Sequence< sal_Int8 > const * pSource = static_cast<uno::Sequence< sal_Int8 > const *>(_pSourceData);

            while (--_nElements >= 0)
            {
                sal_uInt8 * aElement = allocBinary(*pSource);

                sal_uInt8 * * pDest = (sal_uInt8 * *) _pDestAddr;
                *pDest = aElement;

                ++pSource;
                _pDestAddr += sizeof *pDest;
            }
        }
        break;

    case Type::value_any:
    default:
        OSL_ENSURE(false, "Invalid element type");
        break;
    }
}

//-----------------------------------------------------------------------------
sal_uInt8 * allocSequence(sal_uInt8 _aElementType, ::sal_Sequence const * _pSeqData)
{
    OSL_ENSURE(_aElementType == (_aElementType & Type::mask_valuetype), "Invalid type code");

    OSL_ENSURE(_pSeqData, "ERROR: Trying to allocate from a NULL sequence");
    if (_pSeqData == NULL) return 0;

    // OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));
    _aElementType &= Type::mask_basetype;

    sal_uInt32 const nElementSize = implGetElementSize(_aElementType);
    sal_Int32  const nElements = _pSeqData->nElements;

    sal_uInt8 * aResult = implSeqAlloc(nElements,nElementSize);

    if (aResult)
        allocSeqData( aResult + implGetHeaderSize(nElementSize),
                        _aElementType, nElements, nElementSize,
                        _pSeqData->elements);

    return aResult;
}

//-----------------------------------------------------------------------------
sal_uInt8 * allocBinary(uno::Sequence<sal_Int8> const & _aBinaryValue)
{
    sal_uInt32 const nElementSize = 1;
    sal_Int32  const nLength = _aBinaryValue.getLength();

    sal_uInt8 * aResult = implSeqAlloc(nLength,nElementSize);

    if (aResult)
    {
        sal_uInt8 *pElementBaseAddr = aResult + implGetHeaderSize(nElementSize);
        ::memcpy(pElementBaseAddr, _aBinaryValue.getConstArray(), nLength);
    }

    return aResult;
}

//-----------------------------------------------------------------------------
static
void freeSeqData(sal_uInt8 *_pDataAddr,
                 sal_uInt8 _aElementType, sal_Int32 _nElements)
{
    OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));

    switch (_aElementType)
    {
    case Type::value_boolean:
    case Type::value_short:
    case Type::value_int:
    case Type::value_long:
    case Type::value_double:
        // nothing to do
        break;

    case Type::value_string:
        {
            rtl_uString * * pElements = reinterpret_cast<rtl_uString **>( _pDataAddr );

            for (sal_Int32 i = 0; i < _nElements; ++i)
            {
                rtl_uString_release(pElements[i]);
            }
        }
        break;

    case Type::value_binary:
        {
            sal_uInt8 * * pElements = reinterpret_cast<sal_uInt8 **>( _pDataAddr );

            for (sal_Int32 i = 0; i < _nElements; ++i)
            {
                freeBinary(pElements[i]);
            }
        }
        break;

    case Type::value_any:
    default:
        OSL_ENSURE(false, "Invalid element type");
        break;
    }
}

//-----------------------------------------------------------------------------
void freeSequence(sal_uInt8 _aElementType, sal_uInt8 * _aSeq)
{
    OSL_ENSURE(_aElementType == (_aElementType & Type::mask_valuetype), "Invalid type code");

    OSL_ENSURE(_aSeq, "ERROR: Trying to free a NULL sequence");
    if (_aSeq == 0) return;

    // OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));
    _aElementType &= Type::mask_basetype;

    sal_uInt32 nHeaderSize = implGetHeaderSize( implGetElementSize( _aElementType ) );

    freeSeqData(_aSeq + nHeaderSize, _aElementType, implGetSize(_aSeq));

    delete[] (sal_uInt8 *)_aSeq;
}

//-----------------------------------------------------------------------------
void freeBinary(sal_uInt8 * _aSeq)
{
    OSL_ENSURE(_aSeq, "ERROR: Trying to free a NULL sequence");
    if (_aSeq == 0) return;

    delete[] (sal_uInt8 *)_aSeq;
}

//-----------------------------------------------------------------------------

static inline
sal_Sequence * implCreateSequence(void const * _pElements, sal_uInt8 _aElementType, sal_Int32 _nElements)
{
    uno::Type aUnoType = getUnoType( sal_uInt8( _aElementType | Type::flag_sequence ));

    sal_Sequence * pResult = NULL;
    ::uno_type_sequence_construct( &pResult, aUnoType.getTypeLibType(),
                                    const_cast< void * >( _pElements ),
                                    _nElements, NULL );

    OSL_ASSERT(pResult->nRefCount == 1);
    return pResult;
}

//-----------------------------------------------------------------------------
static
sal_Sequence * readSeqData(sal_uInt8 *_pDataAddr, sal_uInt8 _aElementType, sal_Int32 _nElements)
{
    OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));

    void const * pElementData = (void const *)_pDataAddr;
    switch (_aElementType)
    {
    case Type::value_boolean:
    case Type::value_short:
    case Type::value_int:
    case Type::value_long:
    case Type::value_double:
        return implCreateSequence(pElementData,_aElementType,_nElements);

    case Type::value_string:
        {
            uno::Sequence<rtl::OUString> aResult(_nElements);
            rtl::OUString * pResult = aResult.getArray();

            rtl_uString * const * pElements = static_cast<rtl_uString * const *>( pElementData );

            for (sal_Int32 i = 0; i < _nElements; ++i)
            {
                pResult[i] = rtl::OUString(pElements[i]);
            }

            sal_Sequence * pRet = aResult.get();
            ++pRet->nRefCount;
            return pRet;
        }

    case Type::value_binary:
        {
            uno::Sequence< uno::Sequence< sal_Int8 > > aResult(_nElements);
            uno::Sequence< sal_Int8 > * pResult = aResult.getArray();

            sal_uInt8 * const * pElements = static_cast<sal_uInt8 * const *>( pElementData );

            for (sal_Int32 i = 0; i < _nElements; ++i)
            {
                pResult[i] = readBinary(pElements[i]);
            }

            sal_Sequence * pRet = aResult.get();
            ++pRet->nRefCount;
            return pRet;
        }

    case Type::value_any:
    default:
        OSL_ENSURE(false, "Invalid element type");
        return NULL;
    }
}

//-----------------------------------------------------------------------------

::sal_Sequence * readSequence(sal_uInt8 _aElementType, sal_uInt8 * _aSeq)
{
    OSL_ENSURE(_aElementType == (_aElementType & Type::mask_valuetype), "Invalid type code");

    OSL_ENSURE(_aSeq, "ERROR: Trying to read from a NULL sequence");
    if (_aSeq == 0) return NULL;

    // OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));
    _aElementType &= Type::mask_basetype;

    sal_uInt32 nHeaderSize = implGetHeaderSize( implGetElementSize( _aElementType ) );

    return readSeqData(_aSeq + nHeaderSize, _aElementType, implGetSize(_aSeq));
}

//-----------------------------------------------------------------------------
uno::Any readAnySequence(sal_uInt8 _aElementType, sal_uInt8 * _aSeq)
{
    sal_Sequence * pRawSequence = readSequence(_aElementType, _aSeq);

    uno::Any aResult;

    if (pRawSequence != NULL)
        switch (_aElementType & Type::mask_basetype)
        {
        case Type::value_string:
            {
                uno::Sequence< rtl::OUString > aSequence(pRawSequence,SAL_NO_ACQUIRE);
                aResult <<=aSequence ;
            }
            break;

        case Type::value_boolean:
            {
                uno::Sequence< sal_Bool > aSequence(pRawSequence,SAL_NO_ACQUIRE);
                aResult <<=aSequence ;
            }
            break;

        case Type::value_short:
            {
                uno::Sequence< sal_Int16 > aSequence(pRawSequence,SAL_NO_ACQUIRE);
                aResult <<=aSequence ;
            }
            break;

        case Type::value_int:
            {
                uno::Sequence< sal_Int32 > aSequence(pRawSequence,SAL_NO_ACQUIRE);
                aResult <<=aSequence ;
            }
            break;

        case Type::value_long:
            {
                uno::Sequence< sal_Int64 > aSequence(pRawSequence,SAL_NO_ACQUIRE);
                aResult <<=aSequence ;
            }
            break;

        case Type::value_double:
            {
                uno::Sequence< double > aSequence(pRawSequence,SAL_NO_ACQUIRE);
                aResult <<=aSequence ;
            }
            break;

        case Type::value_binary:
            {
                uno::Sequence< uno::Sequence< sal_Int8 > > aSequence(pRawSequence,SAL_NO_ACQUIRE);
                aResult <<=aSequence ;
            }
            break;

        case Type::value_any: // from value_invalid ??
        default:
            OSL_ENSURE( false, "Invalid type code" );
            break;
        }

    OSL_ASSERT(!aResult.hasValue() || aResult.getValueType() == getUnoType(sal_uInt8(_aElementType | Type::flag_sequence)));

    return aResult;
}

//-----------------------------------------------------------------------------
uno::Sequence<sal_Int8> readBinary(sal_uInt8 * _aSeq)
{
    OSL_ENSURE(_aSeq, "ERROR: Trying to read from a NULL sequence");
    if (_aSeq == 0) return uno::Sequence<sal_Int8>();

    return uno::Sequence< sal_Int8 >((const sal_Int8 *)(_aSeq + implGetHeaderSize(1)),
                     implGetSize(_aSeq));
}

//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
} // namespace

// Remaining 'global' mutex bits - should move to api2 ...
namespace configmgr
{
    osl::Mutex UnoApiLock::aCoreLock;
    volatile oslInterlockedCount UnoApiLock::nHeld = 0;

    UnoApiLockReleaser::UnoApiLockReleaser()
    {
        mnCount = UnoApiLock::nHeld;
        for (oslInterlockedCount i = 0; i < mnCount; i++)
            UnoApiLock::release();
    }

    UnoApiLockReleaser::~UnoApiLockReleaser()
    {
        for (oslInterlockedCount i = 0; i < mnCount; i++)
            UnoApiLock::acquire();
    }
} // namespace configmgr
