/*************************************************************************
 *
 *  $RCSfile: valueformatter.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:20:09 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "valueformatter.hxx"

#ifndef CONFIGMGR_XML_ELEMENTFORMATTER_HXX
#include "elementformatter.hxx"
#endif
#ifndef CONFIGMGR_XML_STRINGS_HXX_
#include "xmlstrings.hxx"
#endif
#ifndef CONFIGMGR_TYPECONVERTER_HXX
#include "typeconverter.hxx"
#endif
#ifndef CONFIGMGR_SIMPLETYPEHELPER_HXX
#include "simpletypehelper.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

namespace configmgr
{
    namespace uno = com::sun::star::uno;
    using ::rtl::OUString;

    namespace xml
    {

//==========================================================================
//= Helper
//==========================================================================

// -----------------------------------------------------------------------------
namespace
{
// -----------------------------------------------------------------------------

static
inline
bool isWhitespaceCharacter( sal_Unicode ch )
{
    return ch <= 0x20 && ch;
}
// -----------------------------------------------------------------------------

static
inline
bool isWhitespaceString(OUString const & aStr)
{
    sal_Unicode const * const pBegin = aStr.getStr();
    sal_Unicode const * const pEnd   = pBegin + aStr.getLength();

    // BACK: true, if any whitespace in string or string is empty
    if (pBegin == pEnd)
        return true;

    sal_Unicode const * const pSpace = std::find_if(pBegin,pEnd,isWhitespaceCharacter);

    return pSpace != pEnd;
}
// -----------------------------------------------------------------------------

static
bool hasWhitespaceString( uno::Sequence< OUString > const & aSeq)
{
    // BACK: true, if whitespace Separator is ok, (no whitespace in Strings, no empty strings)
    OUString const * const pBegin = aSeq.getConstArray();
    OUString const * const pEnd   = pBegin + aSeq.getLength();

    OUString const * const pSpace = std::find_if(pBegin,pEnd,isWhitespaceString);

    return pSpace != pEnd;
}
// -----------------------------------------------------------------------------

struct HasSubString
{
    HasSubString(OUString const & _aSubStr)
    : m_aSubStr(_aSubStr)
    {}

    bool operator()(OUString const & _aStr)
    { return _aStr.indexOf(m_aSubStr) >= 0; }

    OUString const m_aSubStr;
};
// -----------------------------------------------------------------------------

static
bool hasStringWithSubstring(const uno::Sequence< OUString > &aSeq, OUString const & _aSubStr)
{
    OUString const * const pBegin = aSeq.getConstArray();
    OUString const * const pEnd   = pBegin + aSeq.getLength();

    OUString const * const pSpace = std::find_if(pBegin,pEnd,HasSubString(_aSubStr));

    return pSpace != pEnd;
}
// -----------------------------------------------------------------------------


template <class Element_>
struct IsEmptySequence
{
    bool operator()(uno::Sequence<Element_> const & aSeq) const
    {
        return aSeq.getLength() == 0;
    }
};
// -----------------------------------------------------------------------------

template <class Element_>
bool hasEmptySequence(uno::Sequence< uno::Sequence<Element_> > const & aSeqSeq)
{
    // BACK: true, if whitespace Separator is ok, (no whitespace in Strings, no empty strings)
    uno::Sequence<Element_> const * const pBegin = aSeqSeq.getConstArray();
    uno::Sequence<Element_> const * const pEnd   = pBegin + aSeqSeq.getLength();

    uno::Sequence<Element_> const * const pEmpty = std::find_if(pBegin, pEnd, IsEmptySequence<Element_>() );

    return pEmpty != pEnd;
}
// -----------------------------------------------------------------------------

inline
bool canUseSeparator(uno::Sequence< OUString > const & aSeq, OUString const & aSeparator)
{
    return ! hasStringWithSubstring(aSeq,aSeparator);
}
// -----------------------------------------------------------------------------

inline
bool canUseWhitespaceSeparator(uno::Sequence< OUString > const & aSeq)
{
    return ! hasWhitespaceString(aSeq);
}
// -----------------------------------------------------------------------------

template <class Element_>
inline
bool canUseWhitespaceSeparator(const uno::Sequence< uno::Sequence<Element_> > &aSeq)
{
    return ! hasEmptySequence(aSeq);
}
// -----------------------------------------------------------------------------

class Separator
{
    OUString m_sValue;
public:
    // -----------------------------------------------------------------------------
    Separator() : m_sValue() {}
    // -----------------------------------------------------------------------------
    bool isDefault() const { return m_sValue.getLength() == 0; }
    // -----------------------------------------------------------------------------
    OUString value() const { return isDefault() ? static_cast<OUString>(SEPARATOR_WHITESPACE) : m_sValue; }
    // -----------------------------------------------------------------------------

    bool check(const uno::Sequence<rtl::OUString> &aSeq) const
    {
        return isDefault() ? canUseWhitespaceSeparator(aSeq) : canUseSeparator(aSeq, m_sValue);
    }

    // -----------------------------------------------------------------------------
    bool trySeparator(rtl::OUString const& sSep, const uno::Sequence<rtl::OUString> & aSeq)
    {
        OSL_ENSURE( ! isWhitespaceString(sSep), "There should be no spaces in non-default separators");
        // BACK: true, if Separator is ok, not in Strings
        if (!canUseSeparator(aSeq, sSep))
            return false;
        this->setSeparator(sSep);
        return true;
    }
    // -----------------------------------------------------------------------------
    void setSeparator(rtl::OUString const& sSep)
    {
        m_sValue = sSep;
    }
    // -----------------------------------------------------------------------------
};
// -----------------------------------------------------------------------------
#define ASCII( STRING_LIT_ ) ( OUString( RTL_CONSTASCII_USTRINGPARAM( STRING_LIT_ ) ) )
// -----------------------------------------------------------------------------
static
Separator createSeparator(const uno::Any& aAny)
{
    Separator aResult;

    // create a Separator which isn't in any value
    if (aAny.getValueTypeClass() == uno::TypeClass_SEQUENCE)
    {
        uno::Type aElementType = configmgr::getSequenceElementType(aAny.getValueType());
        if (aElementType.getTypeClass() == uno::TypeClass_STRING)
        {
            // only in strings we need to search a separator
            uno::Sequence<OUString> aSeq;

            OSL_VERIFY (aAny >>= aSeq);

            bool bValidSeparator =
                    canUseWhitespaceSeparator(aSeq) ||
                    aResult.trySeparator(ASCII(","), aSeq) ||
                    aResult.trySeparator(ASCII(";"), aSeq) ||
                    aResult.trySeparator(ASCII(":"), aSeq) ||
                    aResult.trySeparator(ASCII("|"), aSeq) ||
                    aResult.trySeparator(ASCII("#"), aSeq) ||
                    aResult.trySeparator(ASCII("-#*=+#-"), aSeq);

            if (!bValidSeparator)
            {
                OSL_TRACE("ERROR: configuration formatter: Could not create Separator for string list");
                OSL_ENSURE(false, "ERROR: Could not create Separator for string list");
            }
            else
            {
                // maybe the whitespace test was invalid ?
                OSL_ENSURE(aResult.check(aSeq), "Found Separator does not pass check ?!");
            }
        }
        else if (aElementType == SimpleTypeHelper::getBinaryType())
        {
            // only in strings we need to search a separator
            uno::Sequence< uno::Sequence<sal_Int8> > aSeq;
            OSL_VERIFY(aAny >>= aSeq);

            if (!canUseWhitespaceSeparator(aSeq))
            {
                aResult.setSeparator( ASCII(":") );
            }
        }
    }

    // DefaultSeparator
    return aResult;
}
#undef ASCII
// -----------------------------------------------------------------------------
static
inline
sal_Unicode hexNibble(sal_uInt8 _nNibble)
{
    OSL_ASSERT(_nNibble <= 0x0F);

    const sal_uInt8 cDecOffset = sal_uInt8('0');
    const sal_uInt8 cHexOffset = sal_uInt8('a') - 10;

    return _nNibble + (_nNibble<10 ? cDecOffset : cHexOffset);
}

// -----------------------------------------------------------------------------
static
inline
void appendHex(rtl::OUStringBuffer& rBuff, sal_uInt8 _nByte)
{
    rBuff.append( hexNibble(_nByte >> 4) );
    rBuff.append( hexNibble(_nByte & 0x0f) );
}

// -----------------------------------------------------------------------------
static
OUString binaryToHex(const uno::Sequence<sal_Int8>& _aBinarySeq)
{
    sal_Int32 const nLength = _aBinarySeq.getLength();

    rtl::OUStringBuffer sHex(2*nLength);

    for (sal_Int32 nPos = 0;nPos < nLength; ++nPos)
    {
        appendHex( sHex, _aBinarySeq[nPos] );
    }

    OSL_ASSERT(sHex.getLength() == 2*nLength);
    return sHex.makeStringAndClear();;
}
// -----------------------------------------------------------------------------
typedef ValueFormatter::TypeConverter TypeConverter;
// -----------------------------------------------------------------------------
OUString formatSimpleValue(uno::Any const & _aValue, TypeConverter const & _xTCV)
{
    OUString sResult;

    if (_aValue.hasValue())
    {
        if (_aValue .getValueType() == SimpleTypeHelper::getBinaryType())
        {
            uno::Sequence<sal_Int8> aBinarySeq;

            OSL_VERIFY(_aValue >>= aBinarySeq);

            sResult = binaryToHex(aBinarySeq);
        }
        else
        {
            // cannot have nested any
            OSL_ASSERT(_aValue.getValueTypeClass() != uno::TypeClass_ANY);

            sResult = toString(_xTCV, _aValue);
        }
    }
    return sResult;
}

// -----------------------------------------------------------------------------
template <class Element_>
OUString formatSequence(uno::Sequence< Element_ > const& aSequence, OUString const& sSeparator, TypeConverter const & _xTCV)
{
    rtl::OUStringBuffer aResult;

    if (sal_Int32 const nLength = aSequence.getLength())
    {
        Element_ const * pSeq = aSequence.getConstArray();

        aResult = formatSimpleValue( uno::makeAny(pSeq[0]),_xTCV);

        for(sal_Int32 i=1; i<nLength; ++i)
        {
            aResult.append( sSeparator );
            aResult.append( formatSimpleValue(uno::makeAny(pSeq[i]),_xTCV) );
        }
    }

    return aResult.makeStringAndClear();
}
// -----------------------------------------------------------------------------
// template <> // optimized overload for String
OUString formatSequence(uno::Sequence< OUString > const& aSequence, OUString const& sSeparator, TypeConverter const & )
{
    rtl::OUStringBuffer aResult;

    if (sal_Int32 const nLength = aSequence.getLength())
    {
        OUString const * pSeq = aSequence.getConstArray();

        aResult = pSeq[0];

        for(sal_Int32 i=1; i<nLength; ++i)
        {
            aResult.append( sSeparator ).append( pSeq[i] );
        }
    }

    return aResult.makeStringAndClear();
}

// -----------------------------------------------------------------------------

#define CASE_WRITE_SEQUENCE(TYPE_CLASS, DATA_TYPE)  \
    case TYPE_CLASS:                                \
    {                                               \
        uno::Sequence< DATA_TYPE > aData;               \
        OSL_ENSURE( ::getCppuType(static_cast< DATA_TYPE const*>(0)).getTypeClass() == (TYPE_CLASS),    \
                    "Usage Error for CASE_WRITE_SEQUENCE: Type extracted does not match type class");   \
        OSL_VERIFY( _aValue >>= aData );                                        \
        aResult = formatSequence(aData,sSeparator,xTCV);                        \
    }   break                                                                   \

OUString formatSequenceValue(uno::Any const& _aValue, OUString const& sSeparator, TypeConverter const & xTCV)
{
    OUString aResult;

    uno::Type aElementType = getSequenceElementType( _aValue.getValueType() );

    switch(aElementType.getTypeClass())
    {
        CASE_WRITE_SEQUENCE( uno::TypeClass_BOOLEAN, sal_Bool );

    CASE_WRITE_SEQUENCE( uno::TypeClass_SHORT, sal_Int16 );

    CASE_WRITE_SEQUENCE( uno::TypeClass_LONG, sal_Int32 );

    CASE_WRITE_SEQUENCE( uno::TypeClass_HYPER, sal_Int64 );

    CASE_WRITE_SEQUENCE( uno::TypeClass_DOUBLE, double );

    CASE_WRITE_SEQUENCE( uno::TypeClass_STRING, OUString );

    CASE_WRITE_SEQUENCE( uno::TypeClass_SEQUENCE, uno::Sequence<sal_Int8> );

    default:
        OSL_ENSURE(false, "Unexpected typeclass for sequence elements");
        break;
    }

    return aResult;
}

#undef CASE_WRITE_SEQUENCE
// -----------------------------------------------------------------------------
} // anonymous namspace
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

static inline bool isListVal(uno::Any const & _aValue)
{
    bool bList = false;
    if (_aValue.hasValue())
    {
        getBasicType(_aValue.getValueType(),bList);
    }
    return bList;
}
// -----------------------------------------------------------------------------
void ValueFormatter::makeSeparator()
{
    if (isListVal(m_aValue))
    {
        Separator aSeparator = createSeparator(m_aValue);

        m_sSeparator = aSeparator.value();
        m_bUseSeparator = !aSeparator.isDefault();

        OSL_POSTCOND( this->isList() , "ValueFormatter: Could not mark as list");
    }
    else
    {
        m_sSeparator = OUString();
        m_bUseSeparator = false;

        OSL_POSTCOND( !this->isList(), "ValueFormatter: Could not mark as non-list");
    }
}
// -----------------------------------------------------------------------------

OUString ValueFormatter::getContent(TypeConverter const & _xTCV) const
{
    OUString aResult;
    try
    {
        if (this->isList())
        {
            aResult = formatSequenceValue(m_aValue, m_sSeparator, _xTCV);
        }
        else
        {
            aResult = formatSimpleValue(m_aValue, _xTCV);
        }
    }
    catch (script::CannotConvertException& cce)
    {
        OUString const sMessage(RTL_CONSTASCII_USTRINGPARAM("Configuration: Could not convert value to XML representation: "));
        throw uno::RuntimeException(sMessage + cce.Message, cce.Context);
    }

    return aResult;
}
// -----------------------------------------------------------------------------

bool ValueFormatter::addValueAttributes(ElementFormatter & _rFormatter) const
{
    // do we have a NULL value
    if (!m_aValue.hasValue())
    {
        _rFormatter.addIsNull();
        return false;
    }

    // create a sequence separator
    if (m_bUseSeparator)
    {
        OSL_ASSERT(this->isList());
        _rFormatter.addSeparator(m_sSeparator);
    }

    return true;
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
} // namespace xml

// -----------------------------------------------------------------------------
} // namespace configmgr


