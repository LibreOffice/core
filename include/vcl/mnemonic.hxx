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

#ifndef INCLUDED_VCL_MNEMONIC_HXX
#define INCLUDED_VCL_MNEMONIC_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <rtl/ustring.hxx>
#include <vcl/dllapi.h>


// Mnemonic Chars, which we want support
// Latin 0-9
#define MNEMONIC_RANGE_1_START      0x30
#define MNEMONIC_RANGE_1_END        0x39
// Latin A-Z
#define MNEMONIC_RANGE_2_START      0x41
#define MNEMONIC_RANGE_2_END        0x5A
// Cyrillic
#define MNEMONIC_RANGE_3_START      0x0410
#define MNEMONIC_RANGE_3_END        0x042F
// Greek
#define MNEMONIC_RANGE_4_START      0x0391
#define MNEMONIC_RANGE_4_END        0x03AB
#define MNEMONIC_RANGES             4
#define MAX_MNEMONICS               ((MNEMONIC_RANGE_1_END-MNEMONIC_RANGE_1_START+1)+\
                                     (MNEMONIC_RANGE_2_END-MNEMONIC_RANGE_2_START+1)+\
                                     (MNEMONIC_RANGE_3_END-MNEMONIC_RANGE_3_START+1)+\
                                     (MNEMONIC_RANGE_4_END-MNEMONIC_RANGE_4_START+1))

#define MNEMONIC_CHAR               ((sal_Unicode)'~')
#define MNEMONIC_INDEX_NOTFOUND     ((sal_uInt16)0xFFFF)


class VCL_DLLPUBLIC MnemonicGenerator
{
private:
    // 0 == Mnemonic; >0 == count of characters
    sal_uInt8               maMnemonics[MAX_MNEMONICS];
    css::uno::Reference< css::i18n::XCharacterClassification > mxCharClass;

protected:
    SAL_DLLPRIVATE sal_uInt16       ImplGetMnemonicIndex( sal_Unicode c );
    SAL_DLLPRIVATE sal_Unicode  ImplFindMnemonic( const OUString& rKey );

public:
                        MnemonicGenerator();

    void                RegisterMnemonic( const OUString& rKey );
    OUString            CreateMnemonic( const OUString& rKey );
    css::uno::Reference< css::i18n::XCharacterClassification > const & GetCharClass();

    // returns a string where all '~'-characters and CJK mnemonics of the form (~A) are completely removed
    static OUString EraseAllMnemonicChars( const OUString& rStr );
};

#endif // INCLUDED_VCL_MNEMONIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
