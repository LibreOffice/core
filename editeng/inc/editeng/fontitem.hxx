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

#include <svl/poolitem.hxx>
#include <tools/string.hxx>
#include <vcl/font.hxx>
#include "editeng/editengdllapi.h"

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxFontItem -----------------------------------------------------

/*  [Description]

    This item describes a Font.
*/

class EDITENG_DLLPUBLIC SvxFontItem : public SfxPoolItem
{
    String  aFamilyName;
    String  aStyleName;
    FontFamily  eFamily;
    FontPitch ePitch;
    rtl_TextEncoding eTextEncoding;

    static sal_Bool bEnableStoreUnicodeNames;

public:
    TYPEINFO();

    SvxFontItem( const sal_uInt16 nId  );
    SvxFontItem( const FontFamily eFam, const String& rFamilyName,
        const String& rStyleName,
        const FontPitch eFontPitch /*= PITCH_DONTKNOW*/,
        const rtl_TextEncoding eFontTextEncoding /*= RTL_TEXTENCODING_DONTKNOW*/,
        const sal_uInt16 nId  );

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    // Access methods:
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

    static void EnableStoreUnicodeNames( sal_Bool bEnable );

};

EDITENG_DLLPUBLIC void GetDefaultFonts( SvxFontItem& rLatin, SvxFontItem& rAsian,
                        SvxFontItem& rComplex );

#endif // #ifndef _SVX_FONTITEM_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
