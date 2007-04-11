/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mnemonic.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:01:06 $
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

#ifndef _SV_MNEMONIC_HXX
#define _SV_MNEMONIC_HXX

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_I18N_XCHARACTERCLASSIFICATION_HPP_
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#endif

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
#define MNEMONIC_INDEX_NOTFOUND     ((USHORT)0xFFFF)

// -------------------------
// - MnemonicGenerator -
// -------------------------

class VCL_DLLPUBLIC MnemonicGenerator
{
private:
    // 0 == Mnemonic; >0 == count of characters
    BYTE                maMnemonics[MAX_MNEMONICS];
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCharacterClassification > mxCharClass;

protected:
    SAL_DLLPRIVATE USHORT       ImplGetMnemonicIndex( sal_Unicode c );
    SAL_DLLPRIVATE sal_Unicode  ImplFindMnemonic( const XubString& rKey );

public:
                        MnemonicGenerator();

    void                RegisterMnemonic( const XubString& rKey );
    BOOL                CreateMnemonic( XubString& rKey );
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XCharacterClassification > GetCharClass();

    // returns a string where all '~'-characters and CJK mnemonics of the form (~A) are completely removed
    static String EraseAllMnemonicChars( const String& rStr );
};

#endif // _SV_MNEMONIC_HXX
