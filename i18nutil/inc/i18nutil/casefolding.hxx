/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef INCLUDED_I18NUTIL_CASEFOLDING_HXX
#define INCLUDED_I18NUTIL_CASEFOLDING_HXX

#include <sal/types.h>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

#define MappingTypeLowerToUpper     (1 << 0)  // Upper to Lower mapping
#define MappingTypeUpperToLower     (1 << 1)  // Lower to Upper mapping
#define MappingTypeToUpper          (1 << 2)  // to Upper mapping
#define MappingTypeToLower          (1 << 3)  // to Lower mapping
#define MappingTypeToTitle          (1 << 4)  // to Title mapping
#define MappingTypeSimpleFolding    (1 << 5)  // Simple Case Folding
#define MappingTypeFullFolding      (1 << 6)  // Full Case Folding
#define MappingTypeMask (MappingTypeLowerToUpper|MappingTypeUpperToLower|\
            MappingTypeToUpper|MappingTypeToLower|MappingTypeToTitle|\
            MappingTypeSimpleFolding|MappingTypeFullFolding)

#define ValueTypeNotValue           (1 << 7)  // Value field is an address

#define CasedLetter     (MappingTypeMask)  // for final sigmar

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
    MappingElement() {element.nmap = current = 0;}
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
