/*************************************************************************
 *
 *  $RCSfile: transliteration_body.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:23:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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
#define TRANSLITERATION_ALL
#include "transliteration_body.hxx"
#include "data/transliteration_casemapping.h"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

Transliteration_body::Transliteration_body()
{
    aMappingType = 0;
    transliterationName = "Transliteration_body";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_body";
}

sal_Int16 SAL_CALL Transliteration_body::getType() throw(RuntimeException)
{
    return TransliterationType::ONE_TO_ONE;
}

sal_Bool SAL_CALL Transliteration_body::equals(
    const OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
    const OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2)
    throw(RuntimeException)
{
    throw RuntimeException();
}

Sequence< OUString > SAL_CALL
Transliteration_body::transliterateRange( const OUString& str1, const OUString& str2 )
    throw( RuntimeException)
{
    Sequence< OUString > ostr(2);
    ostr[0] = str1;
    ostr[1] = str2;
    return ostr;
}

OUString SAL_CALL
Transliteration_body::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset) throw(RuntimeException)
{
    // Allocate the max possible buffer. Try to use stack instead of heap which
    // would have to be reallocated most times anyway.
    const sal_Int32 nLocalBuf = 512 * NMAPPINGMAX;
    sal_Unicode aLocalBuf[nLocalBuf], *out = aLocalBuf, *aHeapBuf = NULL;
    sal_Unicode *in = (sal_Unicode*) inStr.getStr() + startPos;

    if (nCount > 512)
        out = aHeapBuf =  (sal_Unicode*) malloc((nCount * NMAPPINGMAX) * sizeof(sal_Unicode));

    offset.realloc(nCount * NMAPPINGMAX);
    sal_Int32 j = 0;
    for (sal_Int32 i = 0; i < nCount; i++) {
        Mapping &map = getValue(in, i, nCount);
        for (sal_Int32 k = 0; k < map.nmap; k++) {
        out[j] = map.map[k];
        offset[j++] = i + startPos;
        }
    }
    offset.realloc(j);

    OUString r(out, j);

    if (aHeapBuf)
        free(aHeapBuf);

    return r;
}

OUString SAL_CALL
Transliteration_body::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset) throw(RuntimeException)
{
    return this->transliterate(inStr, startPos, nCount, offset);
}

static Mapping mapping_03a3[] = {{0, 1, 0x03c2, 0, 0 },{0, 1, 0x03c3, 0, 0}};
static Mapping mapping_0307[] = {{0, 0, 0, 0, 0 },{0, 1, 0x0307, 0, 0}};
static Mapping mapping_0049[] = {{0, 2, 0x0069, 0x0307, 0},{0, 1, 0x0131, 0, 0},{0, 1, 0x0069, 0, 0}};
static Mapping mapping_004a[] = {{0, 2, 0x006a, 0x0307, 0},{0, 1, 0x006a, 0, 0}};
static Mapping mapping_012e[] = {{0, 2, 0x012f, 0x0307, 0},{0, 1, 0x012f, 0, 0}};
static Mapping mapping_00cc[] = {{0, 3, 0x0069, 0x0307, 0x0300},{0, 1, 0x00ec, 0, 0}};
static Mapping mapping_00cd[] = {{0, 3, 0x0069, 0x0307, 0x0301},{0, 1, 0x00ed, 0, 0}};
static Mapping mapping_0128[] = {{0, 3, 0x0069, 0x0307, 0x0303},{0, 1, 0x0129, 0, 0}};
static Mapping mapping_0069[] = {{0, 1, 0x0130, 0, 0},{0, 1, 0x0049, 0, 0}};

#define langIs(lang) (aLocale.Language.compareToAscii(lang) == 0)

// only check simple case, there is more complicated case need to be checked.
#define type_i(ch) (ch == 0x0069 || ch == 0x006a)

#define cased_letter(ch) (CaseMappingIndex[ch>>8] >= 0 && (CaseMappingValue[(CaseMappingIndex[ch>>8] << 8) + (ch&0xff)].type & CasedLetter))

Mapping& Transliteration_body::getConditionalValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len) throw(RuntimeException)
{
    switch(str[pos]) {
    case 0x03a3:
        // final_sigma (not followed by cased and preceded by cased character)
        // DOES NOT check ignorable sequence yet (more complicated implementation).
        return !(pos < len && cased_letter(str[pos+1])) && (pos > 0 && cased_letter(str[pos-1])) ?
        mapping_03a3[0] : mapping_03a3[1];
    case 0x0307:
        return ((aMappingType == MappingTypeLowerToUpper && langIs("lt") ||
        aMappingType == MappingTypeUpperToLower && (langIs("tr") || langIs("az"))) &&
        (pos > 0 && type_i(str[pos-1]))) ?  // after_i
            mapping_0307[0] : mapping_0307[1];
    case 0x0069:
        return (langIs("tr") || langIs("az")) ? mapping_0069[0] : mapping_0069[1];
    case 0x0049: return langIs("lt") ? mapping_0049[0] :
            (langIs("tr") || langIs("az")) ? mapping_0049[1] : mapping_0049[2];
    case 0x004a: return langIs("lt") ? mapping_004a[0] : mapping_004a[1];
    case 0x012e: return langIs("lt") ? mapping_012e[0] : mapping_012e[1];
    case 0x00cc: return langIs("lt") ? mapping_00cc[0] : mapping_00cc[1];
    case 0x00cd: return langIs("lt") ? mapping_00cd[0] : mapping_00cd[1];
    case 0x0128: return langIs("lt") ? mapping_0128[0] : mapping_0128[1];
    }
}

Mapping& Transliteration_body::getValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len)  throw(RuntimeException)
{
    static Mapping dummy = { 0, 1, 0, 0, 0 };
    sal_Int16 address = CaseMappingIndex[str[pos] >> 8] << 8;

    dummy.map[0] = str[pos];

    if (address >= 0 && (CaseMappingValue[address += (str[pos] & 0xFF)].type & aMappingType)) {
        sal_uInt8 type = CaseMappingValue[address].type;
        if (type & ValueTypeNotValue) {
        if (CaseMappingValue[address].value == 0)
            return getConditionalValue(str, pos, len);
        else {
            for (int map = CaseMappingValue[address].value;
                map < CaseMappingValue[address].value + MaxCaseMappingExtras; map++) {
            if (CaseMappingExtra[map].type & aMappingType) {
                if (CaseMappingExtra[map].type & ValueTypeNotValue)
                return getConditionalValue(str, pos, len);
                else
                return CaseMappingExtra[map];
            }
            }
            // Should not come here
            throw RuntimeException();
        }
        } else
        dummy.map[0] = CaseMappingValue[address].value;
    }
    return dummy;
}

Transliteration_casemapping::Transliteration_casemapping()
{
    aMappingType = 0;
    transliterationName = "casemapping(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_casemapping";
}

void SAL_CALL
Transliteration_casemapping::setMappingType( const sal_uInt8 rMappingType, const Locale& rLocale )
{
    aMappingType = rMappingType;
    aLocale = rLocale;
}

Transliteration_u2l::Transliteration_u2l()
{
    aMappingType = MappingTypeUpperToLower;
    transliterationName = "upper_to_lower(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_u2l";
}

Transliteration_l2u::Transliteration_l2u()
{
    aMappingType = MappingTypeLowerToUpper;
    transliterationName = "lower_to_upper(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_l2u";
}

} } } }
