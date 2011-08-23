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
#ifndef _SVX_FONTITEM_HXX
#define _SVX_FONTITEM_HXX

// include ---------------------------------------------------------------

#include <bf_svtools/poolitem.hxx>
#include <tools/string.hxx>
#include <vcl/font.hxx>

namespace rtl
{
    class OUString;
}
namespace binfilter {
class SvXMLUnitConverter;
// class SvxFontItem -----------------------------------------------------

/*
    [Beschreibung]
    Dieses Item beschreibt einen Font.
*/

class SvxFontItem : public SfxPoolItem
{
    String  aFamilyName;
    String  aStyleName;
    FontFamily  eFamily;
    FontPitch ePitch;
    rtl_TextEncoding eTextEncoding;

    static BOOL bEnableStoreUnicodeNames;

public:
    TYPEINFO();

    SvxFontItem( const USHORT nId = ITEMID_FONT );
    SvxFontItem( const FontFamily eFam, const String& rFamilyName,
        const String& rStyleName,
        const FontPitch eFontPitch = PITCH_DONTKNOW,
        const rtl_TextEncoding eFontTextEncoding = RTL_TEXTENCODING_DONTKNOW,
        const USHORT nId = ITEMID_FONT );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int 			 operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	 Create(SvStream &, USHORT) const;
    virtual SvStream&		 Store(SvStream &, USHORT nItemVersion) const;
    virtual	bool             QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual	bool             PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );


    // ZugriffsMethoden:
    inline String &GetFamilyName() { return aFamilyName; }
    inline const String &GetFamilyName() const { return aFamilyName; }

    inline String &GetStyleName() { return aStyleName; }
    inline const String &GetStyleName() const { return aStyleName; }

    inline FontFamily &GetFamily() { return eFamily; }
    inline FontFamily GetFamily() const { return eFamily; }

    inline FontPitch &GetPitch() { return ePitch; }
    inline FontPitch GetPitch() const { return ePitch; }

    inline rtl_TextEncoding &GetCharSet() { return eTextEncoding; }
    inline rtl_TextEncoding GetCharSet() const { return eTextEncoding; }

    inline SvxFontItem& operator=(const SvxFontItem& rFont)
    {
        aFamilyName =  rFont.GetFamilyName();
        aStyleName =   rFont.GetStyleName();
        eFamily =      rFont.GetFamily();
        ePitch =   rFont.GetPitch();
        eTextEncoding = rFont.GetCharSet();
        return *this;
    }

    static void EnableStoreUnicodeNames( BOOL bEnable ) { bEnableStoreUnicodeNames = bEnable; }

};

void GetDefaultFonts( SvxFontItem& rLatin, SvxFontItem& rAsian,
                        SvxFontItem& rComplex );

}//end of namespace binfilter
#endif // #ifndef _SVX_FONTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
