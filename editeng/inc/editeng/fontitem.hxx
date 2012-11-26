/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

/*
    [Beschreibung]
    Dieses Item beschreibt einen Font.
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
    POOLITEM_FACTORY()
    SvxFontItem( const sal_uInt16 nId = 0 );
    SvxFontItem( const FontFamily eFam, const String& rFamilyName,
        const String& rStyleName,
        const FontPitch eFontPitch /*= PITCH_DONTKNOW*/,
        const rtl_TextEncoding eFontTextEncoding /*= RTL_TEXTENCODING_DONTKNOW*/,
        const sal_uInt16 nId  );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, sal_uInt16) const;
    virtual SvStream&        Store(SvStream &, sal_uInt16 nItemVersion) const;
    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    // ZugriffsMethoden:
    void SetFamilyName( const String& rFamilyName ) { aFamilyName = rFamilyName; }
    inline const String &GetFamilyName() const { return aFamilyName; }

    void SetStyleName(const String &rStyleName ) { aStyleName = rStyleName; }
    inline const String &GetStyleName() const { return aStyleName; }

    void SetFamily( FontFamily _eFamily ) { eFamily = _eFamily; }
    inline FontFamily GetFamily() const { return eFamily; }

    void SetPitch(FontPitch _ePitch ) { ePitch = _ePitch; }
    inline FontPitch GetPitch() const { return ePitch; }

    void SetCharSet(rtl_TextEncoding _eEncoding) { eTextEncoding = _eEncoding; }

    inline rtl_TextEncoding GetCharSet() const { return eTextEncoding; }

    SvxFontItem& operator=(const SvxFontItem& rFont);

    static void EnableStoreUnicodeNames( sal_Bool bEnable );

};

EDITENG_DLLPUBLIC void GetDefaultFonts( SvxFontItem& rLatin, SvxFontItem& rAsian,
                        SvxFontItem& rComplex );

#endif // #ifndef _SVX_FONTITEM_HXX


