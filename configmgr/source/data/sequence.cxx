/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sequence.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:13:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "sequence.hxx"

#ifndef INCLUDED_DATA_FLAGS_HXX
#include "flags.hxx"
#endif

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
        typedef AnyData::TypeCode TypeCode;
//-----------------------------------------------------------------------------
static
sal_uInt32 implGetElementSize(TypeCode _aElementType)
{
    OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));
    switch (_aElementType)
    {

    case Type::value_boolean:   return sizeof(sal_Bool);

    case Type::value_short:     return sizeof(sal_Int16);

    case Type::value_int:       return sizeof(sal_Int32);

    case Type::value_long:      return sizeof(sal_Int64);

    case Type::value_double:    return sizeof(double);

    case Type::value_string:    return sizeof(String);

    case Type::value_binary:    return sizeof(Vector);

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
sal_Int32& implGetSize(Sequence _aSeq)
{
    return * (sal_Int32 *) _aSeq;
}

//-----------------------------------------------------------------------------
static
Sequence implSeqAlloc(sal_Int32 _nElements, sal_uInt32 _nElemSize)
{
    sal_uInt32 nTotalSize = implGetHeaderSize(_nElemSize) + _nElements * _nElemSize;

    Sequence aResult = (Sequence) (new sal_uInt8[nTotalSize]);

    implGetSize(aResult) = _nElements;

    return aResult;
}

//-----------------------------------------------------------------------------

static
void allocSeqData(sal_uInt8 *_pDestAddr,
                 TypeCode _aElementType,
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
            OSL_ASSERT(_nElementSize == sizeof(String));

            rtl::OUString const * pSource = static_cast<rtl::OUString const *>(_pSourceData);

            while (--_nElements >= 0)
            {
                String aElement = allocString(*pSource);

                String * pDest = reinterpret_cast<String*>(_pDestAddr);
                *pDest = aElement;

                ++pSource;
                _pDestAddr += sizeof *pDest;
            }
        }
        break;

    case Type::value_binary:
        {
            OSL_ASSERT(_nElementSize == sizeof(Vector));

            typedef uno::Sequence< sal_Int8 > BinSequence;
            BinSequence const * pSource = static_cast<BinSequence const *>(_pSourceData);

            while (--_nElements >= 0)
            {
                Vector aElement = allocBinary(*pSource);

                Vector * pDest = (Vector *) _pDestAddr;
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
Sequence allocSequence(TypeCode _aElementType, ::sal_Sequence const * _pSeqData)
{
    OSL_ENSURE(_aElementType == (_aElementType & Type::mask_valuetype), "Invalid type code");

    OSL_ENSURE(_pSeqData, "ERROR: Trying to allocate from a NULL sequence");
    if (_pSeqData == NULL) return 0;

    // OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));
    _aElementType &= Type::mask_basetype;

    sal_uInt32 const nElementSize = implGetElementSize(_aElementType);
    sal_Int32  const nElements = _pSeqData->nElements;

    Sequence aResult = implSeqAlloc(nElements,nElementSize);

    if (aResult)
        allocSeqData( aResult + implGetHeaderSize(nElementSize),
                        _aElementType, nElements, nElementSize,
                        _pSeqData->elements);

    return aResult;
}

//-----------------------------------------------------------------------------
Sequence allocBinary(uno::Sequence<sal_Int8> const & _aBinaryValue)
{
    sal_uInt32 const nElementSize = 1;
    sal_Int32  const nLength = _aBinaryValue.getLength();

    Sequence aResult = implSeqAlloc(nLength,nElementSize);

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
                 TypeCode _aElementType, sal_Int32 _nElements)
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
            String * pElements = reinterpret_cast<String*>( _pDataAddr );

            for (sal_Int32 i = 0; i < _nElements; ++i)
            {
                freeString(pElements[i]);
            }
        }
        break;

    case Type::value_binary:
        {
            Vector * pElements = reinterpret_cast<Vector*>( _pDataAddr );

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
void freeSequence(TypeCode _aElementType, Sequence _aSeq)
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
void freeBinary(Sequence _aSeq)
{
    OSL_ENSURE(_aSeq, "ERROR: Trying to free a NULL sequence");
    if (_aSeq == 0) return;

    delete[] (sal_uInt8 *)_aSeq;
}

//-----------------------------------------------------------------------------

static inline
sal_Sequence * implCreateSequence(void const * _pElements, TypeCode _aElementType, sal_Int32 _nElements)
{
    uno::Type aUnoType = getUnoType( TypeCode( _aElementType | Type::flag_sequence ));

    sal_Sequence * pResult = NULL;
    ::uno_type_sequence_construct( &pResult, aUnoType.getTypeLibType(),
                                    const_cast< void * >( _pElements ),
                                    _nElements, NULL );

    OSL_ASSERT(pResult->nRefCount == 1);
    return pResult;
}

//-----------------------------------------------------------------------------
static
sal_Sequence * readSeqData(sal_uInt8 *_pDataAddr, TypeCode _aElementType, sal_Int32 _nElements)
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

            String const * pElements = static_cast<String const *>( pElementData );

            for (sal_Int32 i = 0; i < _nElements; ++i)
            {
                pResult[i] = readString(pElements[i]);
            }

            sal_Sequence * pRet = aResult.get();
            ++pRet->nRefCount;
            return pRet;
        }

    case Type::value_binary:
        {
            typedef uno::Sequence< sal_Int8 > BinSequence;

            uno::Sequence<BinSequence> aResult(_nElements);
            BinSequence * pResult = aResult.getArray();

            Vector const * pElements = static_cast<Vector const *>( pElementData );

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

::sal_Sequence * readSequence(TypeCode _aElementType, Sequence _aSeq)
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
uno::Any readAnySequence(TypeCode _aElementType, Sequence _aSeq)
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

    OSL_ASSERT(!aResult.hasValue() || aResult.getValueType() == getUnoType(TypeCode(_aElementType | Type::flag_sequence)));

    return aResult;
}

//-----------------------------------------------------------------------------
uno::Sequence<sal_Int8> readBinary(Sequence _aSeq)
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

#ifndef SIMPLE_REFERENCE_FAST
    void SimpleReferenceObject::acquire() SAL_THROW(())
    {
        if (!UnoApiLock::isHeld())
        {
            fprintf (stderr, "Locking disaster\n");
            fscanf (stdin, "");
        }
        m_nCount++;
    }
    void SimpleReferenceObject::release() SAL_THROW(())
    {
        if (!UnoApiLock::isHeld())
        {
            fprintf (stderr, "Locking disaster\n");
            fscanf (stdin, "");
        }
        if (--m_nCount == 0)
            delete this;
    }
#endif
    SimpleReferenceObject::~SimpleReferenceObject() SAL_THROW(())
    {
        OSL_ASSERT(m_nCount == 0);
    }
} // namespace configmgr
