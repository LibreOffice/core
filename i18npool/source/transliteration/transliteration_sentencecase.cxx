/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: halfwidthToFullwidth.cxx,v $
 * $Revision: 1.12 $
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
#include "precompiled_i18npool.hxx"

// prevent internal compiler error with MSVC6SP3
#include <utility>

#include <i18nutil/widthfolding.hxx>
#include <i18nutil/casefolding.hxx>
#define TRANSLITERATION_sentencecase
#include <transliteration_sentencecase.hxx>
#include <rtl/ustring.hxx>
#include <i18nutil/x_rtl_ustring.h>
#include <i18nutil/unicode.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {


Transliteration_sentencecase::Transliteration_sentencecase()
{
    transliterationName = "sentenceCase";
    implementationName = "com.sun.star.i18n.Transliteration.SENTENCE_CASE";
}

OUString SAL_CALL
Transliteration_sentencecase::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset )
  throw(RuntimeException)
{
    // inspired from Transliteration_body::transliterate
    sal_Int32 nOffCount = 0, i;
    bool bPoint = true;
    if (useOffset)
    {
        for( i = 0; i < nCount; ++i ) {
            sal_Unicode c = inStr.getStr()[ i + startPos ];
            if( sal_Unicode('.') == c || sal_Unicode('!') == c || sal_Unicode('?') == c ) {
                bPoint = true;
                nOffCount++;
            }
            else if( unicode::isAlpha( c ) || unicode::isDigit( c ) )
            {
                const Mapping* map = 0;
                if( bPoint && unicode::isLower( c ))
                {
                    map = &casefolding::getValue(&c, 0, 1, aLocale, MappingTypeLowerToUpper);
                }
                bPoint = false;

                if(map == 0)
                {
                    nOffCount++;
                }
                else
                {
                    nOffCount += map->nmap;
                }
            }
            else
            {
                nOffCount++;
            }
        }
    }

    bPoint = true;
    rtl::OUStringBuffer result;

    if (useOffset)
    {
        result.ensureCapacity(nOffCount);
        if ( nOffCount != offset.getLength() )
            offset.realloc( nOffCount );
    }


    sal_Int32 j = 0;
    sal_Int32 * pArr = offset.getArray();
    for(  i = 0; i < nCount; ++i ) {
        sal_Unicode c = inStr.getStr()[ i + startPos ];
        if( sal_Unicode('.') == c || sal_Unicode('!') == c || sal_Unicode('?') == c ) {
            bPoint = true;
            result.append(c);
            pArr[j++] = i + startPos;
        }
        else if( unicode::isAlpha( c ) || unicode::isDigit( c ) )
        {
            const Mapping* map = 0;
            if( bPoint && unicode::isLower( c ))
            {
                map = &casefolding::getValue(&c, 0, 1, aLocale, MappingTypeLowerToUpper);
            }
            bPoint = false;

            if(map == 0)
            {
                result.append( c );
                pArr[j++] = i + startPos;
            }
            else
            {
                for (sal_Int32 k = 0; k < map->nmap; k++)
                {
                    result.append( map->map[k] );
                    pArr[j++] = i + startPos;
                }
            }
        }
        else
        {
            result.append( c );
            pArr[j++] = i + startPos;
        }
    }
    return result.makeStringAndClear();
}

sal_Unicode SAL_CALL
Transliteration_sentencecase::transliterateChar2Char( sal_Unicode inChar)
  throw(RuntimeException, MultipleCharsOutputException)
{
    return inChar;
}

sal_Int16 SAL_CALL Transliteration_sentencecase::getType() throw(RuntimeException)
{
    return TransliterationType::IGNORE;
}

OUString SAL_CALL
Transliteration_sentencecase::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset) throw(RuntimeException)
{
    return this->transliterate(inStr, startPos, nCount, offset);
}

sal_Bool SAL_CALL Transliteration_sentencecase::equals(
    const OUString& /*str1*/, sal_Int32 /*pos1*/, sal_Int32 /*nCount1*/, sal_Int32& /*nMatch1*/,
    const OUString& /*str2*/, sal_Int32 /*pos2*/, sal_Int32 /*nCount2*/, sal_Int32& /*nMatch2*/)
    throw(RuntimeException)
{
    throw RuntimeException();
}

Sequence< OUString > SAL_CALL
Transliteration_sentencecase::transliterateRange( const OUString& str1, const OUString& str2 )
    throw( RuntimeException)
{
    Sequence< OUString > ostr(2);
    ostr[0] = str1;
    ostr[1] = str2;
    return ostr;
}

} } } }

