/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fontitem.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2006-06-02 12:50:21 $
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
#ifndef _SVX_FONTITEM_HXX
#define _SVX_FONTITEM_HXX

// include ---------------------------------------------------------------

#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _FONT_HXX //autogen
#include <vcl/font.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SvXMLUnitConverter;
namespace rtl
{
    class OUString;
}

// class SvxFontItem -----------------------------------------------------

/*
    [Beschreibung]
    Dieses Item beschreibt einen Font.
*/

class SVX_DLLPUBLIC SvxFontItem : public SfxPoolItem
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
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

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

SVX_DLLPUBLIC void GetDefaultFonts( SvxFontItem& rLatin, SvxFontItem& rAsian,
                        SvxFontItem& rComplex );

#endif // #ifndef _SVX_FONTITEM_HXX


