/*************************************************************************
 *
 *  $RCSfile: transliteration_body.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:36:40 $
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
#ifndef TRANSLITERATION_BODY_H
#define TRANSLITERATION_BODY_H

#include <transliteration_commonclass.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#define MappingTypeLowerToUpper     1 << 0  // Upper to Lower mapping
#define MappingTypeUpperToLower     1 << 1  // Lower to Upper mapping
#define MappingTypeToUpper          1 << 2  // to Upper mapping
#define MappingTypeToLower          1 << 3  // to Lower mapping
#define MappingTypeToTitle          1 << 4  // to Title mapping
#define MappingTypeSimpleFolding    1 << 5  // Simple Case Folding
#define MappingTypeFullFolding      1 << 6  // Full Case Folding
#define MappingTypeMask MappingTypeLowerToUpper|MappingTypeUpperToLower|\
            MappingTypeToUpper|MappingTypeToLower|MappingTypeToTitle|\
            MappingTypeSimpleFolding|MappingTypeFullFolding

#define ValueTypeNotValue       1 << 7 // Value field is an address

#define CasedLetter     MappingTypeMask  // for final sigmar

typedef struct _Value{
    sal_uInt8   type;
    sal_uInt16  value;  // value or address, depend on the type
} Value;

typedef struct _Mapping{
    sal_uInt8   type;
    sal_Int8    nmap;
#define NMAPPINGMAX 3
    sal_Unicode map[NMAPPINGMAX];
} Mapping;      // for Unconditional mapping

typedef class _MappingElement {
public:
    _MappingElement() {element.nmap = current = 0;};
    Mapping element;
    sal_Int8 current;
} MappingElement;

class Transliteration_body : public transliteration_commonclass
{
public:
    Transliteration_body();

    // Methods which are shared.
    sal_Int16 SAL_CALL getType() throw(com::sun::star::uno::RuntimeException);

    rtl::OUString SAL_CALL transliterate(const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        com::sun::star::uno::Sequence< sal_Int32 >& offset) throw(com::sun::star::uno::RuntimeException);

    rtl::OUString SAL_CALL folding(const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        com::sun::star::uno::Sequence< sal_Int32 >& offset) throw(com::sun::star::uno::RuntimeException);

    sal_Bool SAL_CALL equals(
        const rtl::OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1,
        const rtl::OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(com::sun::star::uno::RuntimeException);

    com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL transliterateRange( const rtl::OUString& str1,
        const rtl::OUString& str2 ) throw(com::sun::star::uno::RuntimeException);

protected:
    sal_uInt8 aMappingType;
    Mapping& getValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len) throw(com::sun::star::uno::RuntimeException);
    Mapping& getConditionalValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len) throw(com::sun::star::uno::RuntimeException);
};

#if defined( TRANSLITERATION_UPPER_LOWER ) || defined( TRANSLITERATION_ALL )
class Transliteration_u2l : public Transliteration_body
{
public:
    Transliteration_u2l();
};

class Transliteration_l2u : public Transliteration_body
{
public:
    Transliteration_l2u();
};
#endif

#if defined( TRANSLITERATION_casemapping ) || defined( TRANSLITERATION_ALL )
class Transliteration_casemapping : public Transliteration_body
{
public:
    Transliteration_casemapping();
    void SAL_CALL setMappingType(const sal_uInt8 rMappingType, const com::sun::star::lang::Locale& rLocale );
};
#endif

} } } }

#endif
