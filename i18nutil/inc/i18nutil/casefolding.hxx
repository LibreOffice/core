/*************************************************************************
 *
 *  $RCSfile: casefolding.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 12:25:58 $
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
