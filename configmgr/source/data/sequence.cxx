/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sequence.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:22:33 $
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

#include "sequence.hxx"

#ifndef INCLUDED_DATA_FLAGS_HXX
#include "flags.hxx"
#endif
#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

#ifndef INCLUDED_STRING_H
#include <string.h>
#define INCLUDED_STRING_H
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
        using memory::Allocator;
        using memory::Accessor;
        using memory::Pointer;
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
sal_Int32& implGetSize(Allocator const& _anAllocator, Sequence _aSeq)
{
    OSL_ASSERT(_aSeq != 0);
    void * pBase = _anAllocator.access(_aSeq);
    return * static_cast<sal_Int32 *>(pBase);
}

//-----------------------------------------------------------------------------
static
inline
sal_Int32 implGetSize(Accessor const& _anAccessor, Sequence _aSeq)
{
    OSL_ASSERT(_aSeq != 0);
    void const * pBase = _anAccessor.access( Pointer(_aSeq) );
    return * static_cast<sal_Int32 const *>(pBase);
}

//-----------------------------------------------------------------------------
static
Sequence implSeqAlloc(Allocator const& _anAllocator, sal_Int32 _nElements, sal_uInt32 _nElemSize)
{
    sal_uInt32 nTotalSize = implGetHeaderSize(_nElemSize) + _nElements * _nElemSize;

    Sequence aResult = _anAllocator.allocate(nTotalSize);

    implGetSize(_anAllocator,aResult) = _nElements;

    return aResult;
}

//-----------------------------------------------------------------------------

static
void allocSeqData(Allocator const& _anAllocator, Address _aDestAddr,
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
        ::memcpy(_anAllocator.access(_aDestAddr),_pSourceData,_nElements * _nElementSize);
        break;

    case Type::value_string:
        {
            OSL_ASSERT(_nElementSize == sizeof(String));

            rtl::OUString const * pSource = static_cast<rtl::OUString const *>(_pSourceData);

            while (--_nElements >= 0)
            {
                String aElement = allocString(_anAllocator,*pSource);

                String * pDest = static_cast<String*>( _anAllocator.access(_aDestAddr) );
                *pDest = aElement;

                ++pSource;
                _aDestAddr += sizeof *pDest;
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
                Vector aElement = allocBinary(_anAllocator,*pSource);

                Vector * pDest = static_cast<Vector*>( _anAllocator.access(_aDestAddr) );
                *pDest = aElement;

                ++pSource;
                _aDestAddr += sizeof *pDest;
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
Sequence allocSequence(Allocator const& _anAllocator, TypeCode _aElementType, ::sal_Sequence const * _pSeqData)
{
    OSL_ENSURE(_aElementType == (_aElementType & Type::mask_valuetype), "Invalid type code");

    OSL_ENSURE(_pSeqData, "ERROR: Trying to allocate from a NULL sequence");
    if (_pSeqData == NULL) return 0;

    // OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));
    _aElementType &= Type::mask_basetype;

    sal_uInt32 const nElementSize = implGetElementSize(_aElementType);
    sal_Int32  const nElements = _pSeqData->nElements;

    Sequence aResult = implSeqAlloc(_anAllocator,nElements,nElementSize);

    if (aResult)
        allocSeqData( _anAllocator, aResult + implGetHeaderSize(nElementSize),
                        _aElementType, nElements, nElementSize,
                        _pSeqData->elements);

    return aResult;
}

//-----------------------------------------------------------------------------
Sequence allocBinary(Allocator const& _anAllocator, uno::Sequence<sal_Int8> const & _aBinaryValue)
{
    sal_uInt32 const nElementSize = 1;
    sal_Int32  const nLength = _aBinaryValue.getLength();

    Sequence aResult = implSeqAlloc(_anAllocator,nLength,nElementSize);

    if (aResult)
    {
        Address aElementBaseAddr = aResult + implGetHeaderSize(nElementSize);
        ::memcpy(_anAllocator.access(aElementBaseAddr), _aBinaryValue.getConstArray(), nLength);
    }

    return aResult;
}

//-----------------------------------------------------------------------------
// Sequence copySequence(Allocator const& _anAllocator, TypeCode _aElementType, Sequence _aSeq)

//-----------------------------------------------------------------------------
// Sequence copyBinary(Allocator const& _anAllocator, Sequence _aSeq)

//-----------------------------------------------------------------------------
static
void freeSeqData(Allocator const& _anAllocator, Address _aDataAddr,
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
            String * pElements = static_cast<String*>( _anAllocator.access(_aDataAddr) );

            for (sal_Int32 i = 0; i < _nElements; ++i)
            {
                freeString(_anAllocator,pElements[i]);
            }
        }
        break;

    case Type::value_binary:
        {
            Vector * pElements = static_cast<Vector*>( _anAllocator.access(_aDataAddr) );

            for (sal_Int32 i = 0; i < _nElements; ++i)
            {
                freeBinary(_anAllocator,pElements[i]);
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
void freeSequence(Allocator const& _anAllocator, TypeCode _aElementType, Sequence _aSeq)
{
    OSL_ENSURE(_aElementType == (_aElementType & Type::mask_valuetype), "Invalid type code");

    OSL_ENSURE(_aSeq, "ERROR: Trying to free a NULL sequence");
    if (_aSeq == 0) return;

    // OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));
    _aElementType &= Type::mask_basetype;

    sal_uInt32 nHeaderSize = implGetHeaderSize( implGetElementSize( _aElementType ) );

    freeSeqData(_anAllocator,_aSeq + nHeaderSize, _aElementType, implGetSize(_anAllocator,_aSeq));

    _anAllocator.deallocate(_aSeq);
}

//-----------------------------------------------------------------------------
void freeBinary(memory::Allocator const& _anAllocator, Sequence _aSeq)
{
    OSL_ENSURE(_aSeq, "ERROR: Trying to free a NULL sequence");
    if (_aSeq == 0) return;

    _anAllocator.deallocate(_aSeq);
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
sal_Sequence * readSeqData(Accessor const & _anAccessor, Address _aDataAddr, TypeCode _aElementType, sal_Int32 _nElements)
{
    OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));

    void const * pElementData = _anAccessor.validate( Pointer(_aDataAddr) );
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
                pResult[i] = readString(_anAccessor,pElements[i]);
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
                pResult[i] = readBinary(_anAccessor,pElements[i]);
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

::sal_Sequence * readSequence(Accessor const& _anAccessor, TypeCode _aElementType, Sequence _aSeq)
{
    OSL_ENSURE(_aElementType == (_aElementType & Type::mask_valuetype), "Invalid type code");

    OSL_ENSURE(_aSeq, "ERROR: Trying to read from a NULL sequence");
    if (_aSeq == 0) return NULL;

    // OSL_ASSERT(_aElementType == (_aElementType & Type::mask_basetype));
    _aElementType &= Type::mask_basetype;

    sal_uInt32 nHeaderSize = implGetHeaderSize( implGetElementSize( _aElementType ) );

    return readSeqData(_anAccessor,_aSeq + nHeaderSize, _aElementType, implGetSize(_anAccessor,_aSeq));
}

//-----------------------------------------------------------------------------
uno::Any readAnySequence(Accessor const& _anAccessor, TypeCode _aElementType, Sequence _aSeq)
{
    sal_Sequence * pRawSequence = readSequence(_anAccessor, _aElementType, _aSeq);

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
uno::Sequence<sal_Int8> readBinary(Accessor const& _anAccessor, Sequence _aSeq)
{
    OSL_ENSURE(_aSeq, "ERROR: Trying to read from a NULL sequence");
    if (_aSeq == 0) return uno::Sequence<sal_Int8>();

    sal_Int32 const nElements = implGetSize(_anAccessor,_aSeq);

    void const * const pElementData = _anAccessor.validate( Pointer(_aSeq + implGetHeaderSize(1)) );

    sal_Int8 const * const pBinaryData = static_cast<sal_Int8 const *>(pElementData);

    uno::Sequence< sal_Int8 > aSequence(pBinaryData,nElements);

    return aSequence;
}

//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
} // namespace

