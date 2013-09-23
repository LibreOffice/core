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
#ifndef _SVX_FONTITEM_HXX
#define _SVX_FONTITEM_HXX

#include <editeng/editengdllapi.h>
#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include <vcl/font.hxx>

class SvXMLUnitConverter;

// class SvxFontItem -----------------------------------------------------

/*  [Description]

    This item describes a Font.
*/

class EDITENG_DLLPUBLIC SvxFontItem : public SfxPoolItem
{
    OUString aFamilyName;
    OUString aStyleName;
    FontFamily eFamily;
    FontPitch ePitch;
    rtl_TextEncoding eTextEncoding;

    static sal_Bool bEnableStoreUnicodeNames;

public:
    TYPEINFO();

    explicit SvxFontItem( const sal_uInt16 nId  );
    SvxFontItem( const FontFamily eFam, const OUString& rFamilyName,
        const OUString& rStyleName,
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
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    // Access methods:
    void SetFamilyName( const OUString& rFamilyName ) { aFamilyName = rFamilyName; }
    const OUString &GetFamilyName() const { return aFamilyName; }

    void SetStyleName(const OUString &rStyleName ) { aStyleName = rStyleName; }
    const OUString &GetStyleName() const { return aStyleName; }

    void SetFamily( FontFamily _eFamily ) { eFamily = _eFamily; }
    FontFamily GetFamily() const { return eFamily; }

    void SetPitch(FontPitch _ePitch ) { ePitch = _ePitch; }
    FontPitch GetPitch() const { return ePitch; }

    void SetCharSet(rtl_TextEncoding _eEncoding) { eTextEncoding = _eEncoding; }

    rtl_TextEncoding GetCharSet() const { return eTextEncoding; }

    SvxFontItem& operator=(const SvxFontItem& rFont);

    static void EnableStoreUnicodeNames( sal_Bool bEnable );

};

EDITENG_DLLPUBLIC void GetDefaultFonts( SvxFontItem& rLatin, SvxFontItem& rAsian,
                        SvxFontItem& rComplex );

#endif // #ifndef _SVX_FONTITEM_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
