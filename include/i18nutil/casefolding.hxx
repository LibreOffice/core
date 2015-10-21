/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_I18NUTIL_CASEFOLDING_HXX
#define INCLUDED_I18NUTIL_CASEFOLDING_HXX

#include <sal/types.h>
#include <com/sun/star/i18n/TransliterationModules.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <i18nutil/i18nutildllapi.h>

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
    MappingElement()
        : current(0)
    {
        element.type = element.nmap = 0;
    }
    Mapping element;
    sal_Int8 current;
};

class I18NUTIL_DLLPUBLIC casefolding
{
public:
    static Mapping& getValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len, css::lang::Locale& aLocale, sal_uInt8 nMappingType) throw (css::uno::RuntimeException);
    static Mapping& getConditionalValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len, css::lang::Locale& aLocale, sal_uInt8 nMappingType) throw (css::uno::RuntimeException);
    static sal_Unicode getNextChar(const sal_Unicode *str, sal_Int32& idx, sal_Int32 len, MappingElement& e, css::lang::Locale& aLocale,sal_uInt8 nMappingtype, TransliterationModules moduleLoaded) throw (css::uno::RuntimeException);

};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
