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

#ifndef _SV_MNEMONIC_HXX
#define _SV_MNEMONIC_HXX

#include <vcl/dllapi.h>
#include <tools/string.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/i18n/XCharacterClassification.hpp>

// ---------------------
// - ImplMnemonicTypes -
// ---------------------

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

// -------------------------
// - MnemonicGenerator -
// -------------------------

class VCL_DLLPUBLIC MnemonicGenerator
{
private:
    // 0 == Mnemonic; >0 == count of characters
    sal_uInt8               maMnemonics[MAX_MNEMONICS];
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCharacterClassification > mxCharClass;

protected:
    SAL_DLLPRIVATE sal_uInt16       ImplGetMnemonicIndex( sal_Unicode c );
    SAL_DLLPRIVATE sal_Unicode  ImplFindMnemonic( const XubString& rKey );

public:
                        MnemonicGenerator();

    void                RegisterMnemonic( const XubString& rKey );
    sal_Bool                CreateMnemonic( XubString& rKey );
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCharacterClassification > GetCharClass();

    // returns a string where all '~'-characters and CJK mnemonics of the form (~A) are completely removed
    static String EraseAllMnemonicChars( const String& rStr );
};

#endif // _SV_MNEMONIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
