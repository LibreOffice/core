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
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <i18nutil/i18nutildllapi.h>
#include <o3tl/typed_flags_set.hxx>

enum class TransliterationFlags;

enum class MappingType {
    NONE             = 0x00,
    LowerToUpper     = 0x01,  // Upper to Lower mapping
    UpperToLower     = 0x02,  // Lower to Upper mapping
    ToUpper          = 0x04,  // to Upper mapping
    ToLower          = 0x08,  // to Lower mapping
    ToTitle          = 0x10,  // to Title mapping
    SimpleFolding    = 0x20,  // Simple Case Folding
    FullFolding      = 0x40,  // Full Case Folding
    CasedLetterMask  = LowerToUpper | UpperToLower | ToUpper | ToLower | ToTitle | SimpleFolding | FullFolding, // for final sigmar
    NotValue         = 0x80,  // Value field is an address
};
namespace o3tl {
    template<> struct typed_flags<MappingType> : is_typed_flags<MappingType, 0xff> {};
}

namespace i18nutil {

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
    /// @throws css::uno::RuntimeException
    static Mapping& getValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len, css::lang::Locale const & aLocale, MappingType nMappingType);
    /// @throws css::uno::RuntimeException
    static Mapping& getConditionalValue(const sal_Unicode* str, sal_Int32 pos, sal_Int32 len, css::lang::Locale const & aLocale, MappingType nMappingType);
    /// @throws css::uno::RuntimeException
    static sal_Unicode getNextChar(const sal_Unicode *str, sal_Int32& idx, sal_Int32 len, MappingElement& e, css::lang::Locale const & aLocale, MappingType nMappingtype, TransliterationFlags moduleLoaded);

};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
