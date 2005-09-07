/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: casefolding.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:38:52 $
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
#ifndef INCLUDED_I18NUTIL_CASEFOLDING_HXX
#define INCLUDED_I18NUTIL_CASEFOLDING_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HPP_
#include <com/sun/star/i18n/TransliterationModules.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

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

struct Value
{
    sal_uInt8   type;
    sal_uInt16  value;  // value or address, depend on the type
};

struct Mapping
{
    sal_uInt8   type;
    sal_Int8    nmap;
#define NMAPPINGMAX 3
    sal_Unicode map[NMAPPINGMAX];
};      // for Unconditional mapping

struct MappingElement
{
    MappingElement() {element.nmap = current = 0;};
    Mapping element;
    sal_Int8 current;
};

class casefolding
{
public:
    static Mapping& getValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len, com::sun::star::lang::Locale& aLocale, sal_uInt8 nMappingType) throw (com::sun::star::uno::RuntimeException);
    static Mapping& getConditionalValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len, com::sun::star::lang::Locale& aLocale, sal_uInt8 nMappingType) throw (com::sun::star::uno::RuntimeException);
    static sal_Unicode getNextChar(const sal_Unicode *str, sal_Int32& idx, sal_Int32 len, MappingElement& e, com::sun::star::lang::Locale& aLocale,sal_uInt8 nMappingtype, TransliterationModules moduleLoaded) throw (com::sun::star::uno::RuntimeException);

};

} } } }

#endif
