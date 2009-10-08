/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: valueconverter.cxx,v $
 * $Revision: 1.23 $
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

#include "valuetypeconverter.hxx"
#include "typeconverter.hxx"

inline sal_Bool rtl_ascii_isWhitespace( sal_Unicode ch )
{
    return ch <= 0x20 && ch;
}

namespace configmgr
{
// -----------------------------------------------------------------------------
static
void throwConversionError(sal_Char const* pErrorMsg) SAL_THROW((script::CannotConvertException))
{
    OSL_ENSURE(false, pErrorMsg);

    script::CannotConvertException error;
    error.Message = rtl::OUString::createFromAscii(pErrorMsg);
    throw error;
}
// -----------------------------------------------------------------------------
template <class Char>
inline
bool charInRange(Char ch, char from, char to) throw()
{
    return Char(from) <= ch && ch <= Char(to);
}

// -----------------------------------------------------------------------------
static
inline
unsigned makeHexNibble(unsigned char ch) SAL_THROW((script::CannotConvertException))
{
    unsigned nRet = 0;

         if (charInRange(ch, '0', '9')) nRet = ch - unsigned('0');

    else if (charInRange(ch, 'a', 'f')) nRet = ch - unsigned('a' - 10u);

    else if (charInRange(ch, 'A', 'F')) nRet = ch - unsigned('A' - 10u);

    else throwConversionError("Invalid Hex Character in binary value");

    return nRet;
}

// -----------------------------------------------------------------------------
static
inline
unsigned readHexNibble(sal_Unicode ch) SAL_THROW((script::CannotConvertException))
{
    if (!charInRange(ch, 0, 127)) throwConversionError("Non-Ascii Character in binary value");

    return makeHexNibble(static_cast<unsigned char>(ch));
}

// -----------------------------------------------------------------------------
static
inline
unsigned int readHexByte(sal_Unicode const*& pStr) SAL_THROW((script::CannotConvertException))
{
    register unsigned int nHigh = readHexNibble(*pStr++);
    register unsigned int nLow =  readHexNibble(*pStr++);
    return (nHigh << 4) | nLow;
}

// -----------------------------------------------------------------------------
static
void parseHexBinary(rtl::OUString const& aHexString_, uno::Sequence<sal_Int8>& rBinarySeq_)
        SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException))
{
    // PRE: aBinaryString with HexCode
    // POST: rBinarySeq with the to Hex converted String

    sal_uInt32 nCount = aHexString_.getLength();
    sal_Unicode const * pHex = aHexString_.getStr();

    if (nCount % 2) throwConversionError("Hex string has odd number of characters");
    nCount /= 2;

    rBinarySeq_.realloc(nCount);
    sal_Int8 * pBinary = rBinarySeq_.getArray();

    while (nCount--)
    {
            *pBinary++ = static_cast<sal_Int8>(readHexByte(pHex));
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
uno::Sequence<sal_Int8> ValueConverter::parseBinary(rtl::OUString const& aBinaryString_) const
        SAL_THROW((script::CannotConvertException, com::sun::star::uno::RuntimeException))
{
    uno::Sequence<sal_Int8> aResultSeq;

    parseHexBinary(aBinaryString_,aResultSeq);

    return aResultSeq;
}

// -----------------------------------------------------------------------------
static inline
uno::Type getBinaryType()
{
    uno::Sequence<sal_Int8> const * const for_binary = 0;
    return ::getCppuType(for_binary);
}

// -----------------------------------------------------------------------------
bool ValueConverter::isList() const
{
    return  m_aType.getTypeClass() == uno::TypeClass_SEQUENCE &&
            m_aType != getBinaryType();
}

// -----------------------------------------------------------------------------
uno::Any ValueConverter::convertToAny(rtl::OUString const& aContent) const
        SAL_THROW((script::CannotConvertException, com::sun::star::uno::RuntimeException))
{
    uno::Any aValue;

    if (this->isNull())
    {
        OSL_ENSURE(aContent.trim().getLength() == 0, "ValueConverter: Non-empty Null Value - ignoring content");
        OSL_ASSERT(!aValue.hasValue());
    }

    else if (this->isList())
    {
        std::vector< rtl::OUString > aContentList;
        splitListData(aContent, aContentList);
        convertListToAny(aContentList, aValue);
    }

    else
    {
        convertScalarToAny(aContent, aValue);
    }

    return aValue;
}

// -----------------------------------------------------------------------------
bool ValueConverter::convertScalarToAny(rtl::OUString const& aContent, uno::Any& rValue) const
        SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException))
{
    OSL_PRECOND(!this->isNull(),"ValueConverter::convertScalarToAny - check for NULL before calling");
    OSL_ENSURE(m_aType.getTypeClass() != uno::TypeClass_ANY,"'Any' values must be NULL");

    // check for Binary
    if (m_aType == getBinaryType())
    {
        com::sun::star::uno::Sequence<sal_Int8> aBinarySeq = parseBinary(aContent);
        rValue <<= aBinarySeq;
    }

    else
    {
        rValue = toAny(m_xTypeConverter, aContent, m_aType.getTypeClass());
    }

    return !! rValue.hasValue();
}

// -----------------------------------------------------------------------------
template <class T>
bool convertListToSequence(std::vector< rtl::OUString > const& aStringList, uno::Sequence< T >& rSequence, uno::TypeClass aElementTypeClass, ValueConverter const& rConverter)
        SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException))
{
    OSL_ASSERT(aElementTypeClass == ::getCppuType(static_cast<T const*>(0)).getTypeClass());

    rSequence.realloc(aStringList.size());

    sal_uInt32 nPos = 0;

    for(std::vector< rtl::OUString >::const_iterator it = aStringList.begin();
        it != aStringList.end();
        ++it)
    {
        uno::Any aValueAny = toAny(rConverter.getTypeConverter(), *it, aElementTypeClass);

        if (aValueAny >>= rSequence[nPos])
            ++nPos;

        else if (!aValueAny.hasValue())
            OSL_ENSURE(false,"UNEXPECTED: Found NULL value in List - ignoring value !");

        else
            OSL_ENSURE(false,"ERROR: Cannot extract converted value into List - skipping value !");
    }

    bool bOK = (nPos == aStringList.size());

    if (!bOK)
    {
        OSL_ASSERT(nPos < aStringList.size());
        rSequence.realloc(nPos);
    }
    return bOK;
}

// -----------------------------------------------------------------------------
// special conversion for string sequence

static
inline
void stringListToSequence(uno::Sequence< rtl::OUString > & rSequence, std::vector< rtl::OUString > const & aStringList)
{
    rSequence .realloc( aStringList.size() );

    std::copy( aStringList.begin(), aStringList.end(), rSequence.getArray() );
}
// -----------------------------------------------------------------------------

static
inline
std::vector< rtl::OUString > sequenceToStringList(uno::Sequence< rtl::OUString > const & aSequence)
{
    rtl::OUString const * const pBegin = aSequence.getConstArray();
    rtl::OUString const * const pEnd = pBegin + aSequence.getLength();

    return std::vector< rtl::OUString >(pBegin,pEnd);
}
// -----------------------------------------------------------------------------

uno::Sequence< rtl::OUString > ValueConverter::splitStringList(rtl::OUString const& aContent) const
{
    std::vector< rtl::OUString > aList;
    splitListData(aContent, aList);

    uno::Sequence< rtl::OUString > aResult;
    stringListToSequence(aResult,aList);

    return aResult;
}
// -----------------------------------------------------------------------------

uno::Any ValueConverter::convertListToAny(uno::Sequence< rtl::OUString > const& aContentList) const
    SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException))
{
    uno::Any aResult;
    std::vector< rtl::OUString > const aStringList = sequenceToStringList(aContentList);
    convertListToAny(aStringList,aResult);
    return aResult;
}
// -----------------------------------------------------------------------------
// special overload for binary sequence

// template<> // use an explicit specialization
bool convertListToSequence(std::vector< rtl::OUString > const& aStringList, uno::Sequence< uno::Sequence<sal_Int8> >& rSequence, uno::TypeClass aElementTypeClass, ValueConverter const& rParser )
        SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException))
{
    { (void)aElementTypeClass; }
    OSL_ASSERT(aElementTypeClass == uno::TypeClass_SEQUENCE);

    rSequence.realloc(aStringList.size());

    sal_uInt32 nPos = 0;

    for(std::vector< rtl::OUString >::const_iterator it = aStringList.begin();
        it != aStringList.end();
        ++it)
    {
        rSequence[nPos++] = rParser.parseBinary(*it);
    }
    return true;
}

// -----------------------------------------------------------------------------
// special overload for string sequence

// template<> // use an explicit specialization
bool convertListToSequence(std::vector< rtl::OUString > const& aStringList, uno::Sequence< rtl::OUString >& rSequence, uno::TypeClass aElementTypeClass, ValueConverter const& /*rParser*/ )
        SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException))
{
    { (void)aElementTypeClass; }
    OSL_ASSERT(aElementTypeClass == uno::TypeClass_STRING);

    stringListToSequence(rSequence, aStringList);

    return true;
}

// -----------------------------------------------------------------------------

#define MAYBE_EXTRACT_SEQUENCE( type ) \
    if (aElementType == ::getCppuType( (type const *)0))    \
    {                                                       \
        com::sun::star::uno::Sequence< type > aSequence;                         \
        convertListToSequence(aContentList,aSequence,aElementTypeClass, *this); \
        rValue <<= aSequence;                               \
    }

bool ValueConverter::convertListToAny(std::vector< rtl::OUString > const& aContentList, uno::Any& rValue) const
        SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException))
{
    OSL_PRECOND(!this->isNull(),"ValueConverter::convertListToAny - check for NULL before calling");
    OSL_ENSURE(m_aType.getTypeClass() == uno::TypeClass_SEQUENCE,"'Any' not allowed for lists");

    uno::Type       aElementType        = getSequenceElementType(m_aType);
    uno::TypeClass  aElementTypeClass   = aElementType.getTypeClass();

    OSL_ENSURE(aElementTypeClass != uno::TypeClass_ANY,"'Any' not allowed for list elements");

    MAYBE_EXTRACT_SEQUENCE( rtl::OUString )
    else
    MAYBE_EXTRACT_SEQUENCE( sal_Bool )
    else
    MAYBE_EXTRACT_SEQUENCE( sal_Int16 )
    else
    MAYBE_EXTRACT_SEQUENCE( sal_Int32 )
    else
    MAYBE_EXTRACT_SEQUENCE( sal_Int64 )
    else
    MAYBE_EXTRACT_SEQUENCE( double )
    else
    MAYBE_EXTRACT_SEQUENCE( com::sun::star::uno::Sequence<sal_Int8> )
    else
    {
        OSL_ENSURE(false, "Unknown element type in list");
        throwConversionError("Invalid value-type found in list value");
    }

    return !! rValue.hasValue();
}
#undef MAYBE_EXTRACT_SEQUENCE

// -----------------------------------------------------------------------------
namespace
{
    sal_Int32 const NO_MORE_TOKENS = -1;
    struct OTokenizeByWhitespace
    {

        static inline bool isWhitespace(sal_Unicode ch)
        {
            // note: for definition of whitescape see also
            //   canUseWhitespace(rtl::OUString const&)
            // in xmlformater.cxx
            // -----------------------------------------------------------------------------
            return rtl_ascii_isWhitespace(ch) ? true : false;
        }

        sal_Int32 findFirstTokenStart(rtl::OUString const& sText) const SAL_THROW(())
        {
            return findNextTokenStart(sText,0);
        }

        sal_Int32 findNextTokenStart(rtl::OUString const& sText, sal_Int32 nPrevTokenEnd) const  SAL_THROW(())
        {
            sal_Int32 const nEnd = sText.getLength();
            sal_Int32 nPos = nPrevTokenEnd;

            OSL_PRECOND( nPos == 0 || (0 < nPos && nPos < nEnd && isWhitespace(sText[nPos])) || nPos == nEnd,
                         "Invalid nPrevTokenEnd");

            while (nPos < nEnd && isWhitespace(sText[nPos]))
            {
                ++nPos;
            }

            if (nPos < nEnd)
                return nPos;
            else
                return NO_MORE_TOKENS;
        }

        sal_Int32 findTokenEnd(rtl::OUString const& sText, sal_Int32 nTokenStart) const SAL_THROW(())
        {
            sal_Int32 const nEnd = sText.getLength();
            sal_Int32 nPos = nTokenStart;

            OSL_PRECOND( 0 <= nPos && nPos < nEnd && !isWhitespace(sText[nPos]),
                         "Invalid nTokenStart");

            while (nPos < nEnd && !isWhitespace(sText[nPos]))
            {
                ++nPos;
            }

            return nPos;
        }
    };
// -----------------------------------------------------------------------------
    struct OTokenizeBySeparator
    {
        rtl::OUString const sSeparator;
        OTokenizeBySeparator(rtl::OUString const& _sSeparator) SAL_THROW(())
            : sSeparator(_sSeparator)
        {
            OSL_PRECOND(sSeparator.trim().getLength() > 0, "Invalid empty separator string");
        }

            sal_Int32 findFirstTokenStart(rtl::OUString const& /*sText*/) const SAL_THROW(())
        {
            return 0;
        }
        sal_Int32 findNextTokenStart(rtl::OUString const& sText, sal_Int32 nPrevTokenEnd) const SAL_THROW(())
        {
            sal_Int32 const nEnd = sText.getLength();
            sal_Int32 nPos = nPrevTokenEnd;
            OSL_PRECOND( nPos == nEnd || (0 <= nPos && nPos < nEnd && sText.indexOf(sSeparator, nPos) == nPos),
                         "Invalid nPrevTokenEnd");

            if (nPos < nEnd)
                return nPos + sSeparator.getLength();
            else
                return NO_MORE_TOKENS;
        }
        sal_Int32 findTokenEnd(rtl::OUString const& sText, sal_Int32 nTokenStart) const SAL_THROW(())
        {
            sal_Int32 const nEnd = sText.getLength();
            OSL_PRECOND( 0 <= nTokenStart && nTokenStart <= nEnd ,
                         "Invalid nTokenStart");

            sal_Int32 nPos = sText.indexOf(sSeparator,nTokenStart);

            if (nPos >= 0)
                return nPos;
            else
                return nEnd;
        }
    };
// -----------------------------------------------------------------------------
    template <class Tokenizer>
    void tokenizeListData(Tokenizer const& aTokenizer, rtl::OUString const& aContent, std::vector< rtl::OUString >& rContentList)
            SAL_THROW(())
    {
        sal_Int32 nTokenPos = aTokenizer.findFirstTokenStart(aContent);

        while(nTokenPos != NO_MORE_TOKENS)
        {
            sal_Int32 nTokenEnd = aTokenizer.findTokenEnd(aContent, nTokenPos);

            // this is what the tokenizer must provide
            OSL_ASSERT(0 <= nTokenPos && nTokenPos <= nTokenEnd && nTokenEnd <= aContent.getLength());

            rContentList.push_back( aContent.copy(nTokenPos, nTokenEnd-nTokenPos) );

            nTokenPos= aTokenizer.findNextTokenStart(aContent, nTokenEnd);
        }
    }
// -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------
void ValueConverter::splitListData(rtl::OUString const& aContent, std::vector< rtl::OUString >& rContentList) const
    SAL_THROW(())
{
    rtl::OUString sSeparator = m_sSeparator;

    bool bSeparateByWhitespace = (sSeparator.trim().getLength() == 0);

    if (bSeparateByWhitespace)
    {
        OSL_ENSURE( sSeparator.getLength()==0 || sSeparator.equalsAscii(" "),
                    "Unexpected whitespace-only separator");

        tokenizeListData( OTokenizeByWhitespace(), aContent, rContentList );
    }
    else
    {
        OSL_ENSURE( sSeparator.trim()==sSeparator,
                    "Unexpected whitespace in separator");

        tokenizeListData( OTokenizeBySeparator(sSeparator), aContent, rContentList );
    }
}
// -----------------------------------------------------------------------------

} // namespace
