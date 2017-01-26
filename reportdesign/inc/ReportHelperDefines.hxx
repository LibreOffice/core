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
#ifndef INCLUDED_REPORTDESIGN_INC_REPORTHELPERDEFINES_HXX
#define INCLUDED_REPORTDESIGN_INC_REPORTHELPERDEFINES_HXX

#define REPORTCONTROLMODEL_HEADER()  \
    virtual OUString SAL_CALL getDataField() override; \
    virtual void SAL_CALL setDataField(const OUString & the_value) override; \
    virtual sal_Bool SAL_CALL getPrintWhenGroupChange() override; \
    virtual void SAL_CALL setPrintWhenGroupChange(sal_Bool the_value) override; \
    virtual OUString SAL_CALL getConditionalPrintExpression() override; \
    virtual void SAL_CALL setConditionalPrintExpression(const OUString & the_value) override; \
    virtual css::uno::Reference< css::report::XFormatCondition > SAL_CALL createFormatCondition() override;

#define SHAPE_HEADER()  \
    virtual css::awt::Point SAL_CALL getPosition() override; \
    virtual void SAL_CALL setPosition(const css::awt::Point & aPosition) override; \
    virtual css::awt::Size SAL_CALL getSize() override; \
    virtual void SAL_CALL setSize(const css::awt::Size & aSize) override;

#define REPORTCOMPONENT_HEADER()  \
    virtual OUString SAL_CALL getName() override; \
    virtual void SAL_CALL setName(const OUString & the_value) override; \
    virtual ::sal_Int32 SAL_CALL getHeight() override; \
    virtual void SAL_CALL setHeight(::sal_Int32 the_value) override; \
    virtual ::sal_Int32 SAL_CALL getPositionX() override; \
    virtual void SAL_CALL setPositionX(::sal_Int32 the_value) override; \
    virtual ::sal_Int32 SAL_CALL getPositionY() override; \
    virtual void SAL_CALL setPositionY(::sal_Int32 the_value) override; \
    virtual ::sal_Int32 SAL_CALL getWidth() override; \
    virtual void SAL_CALL setWidth(::sal_Int32 the_value) override; \
    virtual ::sal_Int16 SAL_CALL getControlBorder() override; \
    virtual void SAL_CALL setControlBorder(::sal_Int16 the_value) override; \
    virtual ::sal_Int32 SAL_CALL getControlBorderColor() override; \
    virtual void SAL_CALL setControlBorderColor(::sal_Int32 the_value) override; \
    virtual sal_Bool SAL_CALL getPrintRepeatedValues() override; \
    virtual void SAL_CALL setPrintRepeatedValues(sal_Bool the_value) override; \
    virtual css::uno::Sequence< OUString > SAL_CALL getMasterFields() override; \
    virtual void SAL_CALL setMasterFields( const css::uno::Sequence< OUString >& _masterfields ) override; \
    virtual css::uno::Sequence< OUString > SAL_CALL getDetailFields() override; \
    virtual void SAL_CALL setDetailFields( const css::uno::Sequence< OUString >& _detailfields ) override; \
    virtual css::uno::Reference< css::report::XSection > SAL_CALL getSection() override;

#define REPORTCONTROLFORMAT_HEADER()  \
    virtual ::sal_Int32 SAL_CALL getControlBackground() override;\
    virtual void SAL_CALL setControlBackground( ::sal_Int32 _controlbackground ) override;\
    virtual sal_Bool SAL_CALL getControlBackgroundTransparent() override;\
    virtual void SAL_CALL setControlBackgroundTransparent( sal_Bool _controlbackgroundtransparent ) override;\
    virtual ::sal_Int16 SAL_CALL getParaAdjust() override;\
    virtual void SAL_CALL setParaAdjust( ::sal_Int16 _paraadjust ) override;\
    virtual css::awt::FontDescriptor SAL_CALL getFontDescriptor() override;\
    virtual void SAL_CALL setFontDescriptor( const css::awt::FontDescriptor& _fontdescriptor ) override;\
    virtual css::awt::FontDescriptor SAL_CALL getFontDescriptorAsian() override;\
    virtual void SAL_CALL setFontDescriptorAsian( const css::awt::FontDescriptor& _fontdescriptor ) override;\
    virtual css::awt::FontDescriptor SAL_CALL getFontDescriptorComplex() override;\
    virtual void SAL_CALL setFontDescriptorComplex( const css::awt::FontDescriptor& _fontdescriptor ) override;\
    virtual ::sal_Int16 SAL_CALL getControlTextEmphasis() override;\
    virtual void SAL_CALL setControlTextEmphasis( ::sal_Int16 _controltextemphasis ) override;\
    virtual ::sal_Int16 SAL_CALL getCharEmphasis() override;\
    virtual void SAL_CALL setCharEmphasis( ::sal_Int16 _charemphasis ) override;\
    virtual sal_Bool SAL_CALL getCharCombineIsOn() override;\
    virtual void SAL_CALL setCharCombineIsOn( sal_Bool _charcombineison ) override;\
    virtual OUString SAL_CALL getCharCombinePrefix() override;\
    virtual void SAL_CALL setCharCombinePrefix( const OUString& _charcombineprefix ) override;\
    virtual OUString SAL_CALL getCharCombineSuffix() override;\
    virtual void SAL_CALL setCharCombineSuffix( const OUString& _charcombinesuffix ) override;\
    virtual sal_Bool SAL_CALL getCharHidden() override;\
    virtual void SAL_CALL setCharHidden( sal_Bool _charhidden ) override;\
    virtual sal_Bool SAL_CALL getCharShadowed() override;\
    virtual void SAL_CALL setCharShadowed( sal_Bool _charshadowed ) override;\
    virtual sal_Bool SAL_CALL getCharContoured() override;\
    virtual void SAL_CALL setCharContoured( sal_Bool _charcontoured ) override;\
    virtual ::sal_Int16 SAL_CALL getCharCaseMap() override;\
    virtual void SAL_CALL setCharCaseMap( ::sal_Int16 _charcasemap ) override;\
    virtual css::lang::Locale SAL_CALL getCharLocale() override;\
    virtual void SAL_CALL setCharLocale( const css::lang::Locale& _charlocale ) override;\
    virtual ::sal_Int16 SAL_CALL getCharEscapement() override;\
    virtual void SAL_CALL setCharEscapement( ::sal_Int16 _charescapement ) override;\
    virtual ::sal_Int8 SAL_CALL getCharEscapementHeight() override;\
    virtual void SAL_CALL setCharEscapementHeight( ::sal_Int8 _charescapementheight ) override;\
    virtual sal_Bool SAL_CALL getCharAutoKerning() override;\
    virtual void SAL_CALL setCharAutoKerning( sal_Bool _charautokerning ) override;\
    virtual ::sal_Int16 SAL_CALL getCharKerning() override;\
    virtual void SAL_CALL setCharKerning( ::sal_Int16 _charkerning ) override;\
    virtual sal_Bool SAL_CALL getCharFlash() override;\
    virtual void SAL_CALL setCharFlash( sal_Bool _charflash ) override;\
    virtual ::sal_Int16 SAL_CALL getCharRelief() override;\
    virtual void SAL_CALL setCharRelief( ::sal_Int16 _charrelief ) override;\
    virtual OUString SAL_CALL getCharFontName() override;\
    virtual void SAL_CALL setCharFontName( const OUString& _charfontname ) override;\
    virtual OUString SAL_CALL getCharFontStyleName() override;\
    virtual void SAL_CALL setCharFontStyleName( const OUString& _charfontstylename ) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontFamily() override;\
    virtual void SAL_CALL setCharFontFamily( ::sal_Int16 _charfontfamily ) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontCharSet() override;\
    virtual void SAL_CALL setCharFontCharSet( ::sal_Int16 _charfontcharset ) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontPitch() override;\
    virtual void SAL_CALL setCharFontPitch( ::sal_Int16 _charfontpitch ) override;\
    virtual ::sal_Int32 SAL_CALL getCharColor() override;\
    virtual void SAL_CALL setCharColor( ::sal_Int32 _charcolor ) override;\
    virtual ::sal_Int32 SAL_CALL getCharUnderlineColor() override;\
    virtual void SAL_CALL setCharUnderlineColor( ::sal_Int32 _charunderlinecolor ) override;\
    virtual float SAL_CALL getCharHeight() override;\
    virtual void SAL_CALL setCharHeight( float _charheight ) override;\
    virtual ::sal_Int16 SAL_CALL getCharUnderline() override;\
    virtual void SAL_CALL setCharUnderline( ::sal_Int16 _charunderline ) override;\
    virtual float SAL_CALL getCharWeight() override;\
    virtual void SAL_CALL setCharWeight( float _charweight ) override;\
    virtual css::awt::FontSlant SAL_CALL getCharPosture() override;\
    virtual void SAL_CALL setCharPosture( css::awt::FontSlant _charposture ) override;\
    virtual ::sal_Int16 SAL_CALL getCharStrikeout() override;\
    virtual void SAL_CALL setCharStrikeout( ::sal_Int16 _charstrikeout ) override;\
    virtual sal_Bool SAL_CALL getCharWordMode() override;\
    virtual void SAL_CALL setCharWordMode( sal_Bool _charwordmode ) override;\
    virtual ::sal_Int16 SAL_CALL getCharRotation() override;\
    virtual void SAL_CALL setCharRotation( ::sal_Int16 _charrotation ) override;\
    virtual ::sal_Int16 SAL_CALL getCharScaleWidth() override;\
    virtual void SAL_CALL setCharScaleWidth( ::sal_Int16 _charscalewidth ) override;\
    virtual css::style::VerticalAlignment SAL_CALL getVerticalAlign() override;\
    virtual void SAL_CALL setVerticalAlign( css::style::VerticalAlignment _paravertalignment ) override;\
    virtual OUString SAL_CALL getHyperLinkURL() override;\
    virtual void SAL_CALL setHyperLinkURL( const OUString& _hyperlinkurl ) override;\
    virtual OUString SAL_CALL getHyperLinkTarget() override;\
    virtual void SAL_CALL setHyperLinkTarget( const OUString& _hyperlinktarget ) override;\
    virtual OUString SAL_CALL getHyperLinkName() override;\
    virtual void SAL_CALL setHyperLinkName( const OUString& _hyperlinkname ) override;\
    virtual OUString SAL_CALL getVisitedCharStyleName() override;\
    virtual void SAL_CALL setVisitedCharStyleName( const OUString& _visitedcharstylename ) override;\
    virtual OUString SAL_CALL getUnvisitedCharStyleName() override;\
    virtual void SAL_CALL setUnvisitedCharStyleName( const OUString& _unvisitedcharstylename ) override;\
    virtual float SAL_CALL getCharHeightAsian() override;\
    virtual void SAL_CALL setCharHeightAsian( float _charheightasian ) override;\
    virtual float SAL_CALL getCharWeightAsian() override;\
    virtual void SAL_CALL setCharWeightAsian( float _charweightasian ) override;\
    virtual OUString SAL_CALL getCharFontNameAsian() override;\
    virtual void SAL_CALL setCharFontNameAsian( const OUString& _charfontnameasian ) override;\
    virtual OUString SAL_CALL getCharFontStyleNameAsian() override;\
    virtual void SAL_CALL setCharFontStyleNameAsian( const OUString& _charfontstylenameasian ) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontFamilyAsian() override;\
    virtual void SAL_CALL setCharFontFamilyAsian( ::sal_Int16 _charfontfamilyasian ) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontCharSetAsian() override;\
    virtual void SAL_CALL setCharFontCharSetAsian( ::sal_Int16 _charfontcharsetasian ) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontPitchAsian() override;\
    virtual void SAL_CALL setCharFontPitchAsian( ::sal_Int16 _charfontpitchasian ) override;\
    virtual css::awt::FontSlant SAL_CALL getCharPostureAsian() override;\
    virtual void SAL_CALL setCharPostureAsian( css::awt::FontSlant _charpostureasian ) override;\
    virtual css::lang::Locale SAL_CALL getCharLocaleAsian() override;\
    virtual void SAL_CALL setCharLocaleAsian( const css::lang::Locale& _charlocaleasian ) override;\
    virtual float SAL_CALL getCharHeightComplex() override;\
    virtual void SAL_CALL setCharHeightComplex( float _charheightcomplex ) override;\
    virtual float SAL_CALL getCharWeightComplex() override;\
    virtual void SAL_CALL setCharWeightComplex( float _charweightcomplex ) override;\
    virtual OUString SAL_CALL getCharFontNameComplex() override;\
    virtual void SAL_CALL setCharFontNameComplex( const OUString& _charfontnamecomplex ) override;\
    virtual OUString SAL_CALL getCharFontStyleNameComplex() override;\
    virtual void SAL_CALL setCharFontStyleNameComplex( const OUString& _charfontstylenamecomplex ) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontFamilyComplex() override;\
    virtual void SAL_CALL setCharFontFamilyComplex( ::sal_Int16 _charfontfamilycomplex ) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontCharSetComplex() override;\
    virtual void SAL_CALL setCharFontCharSetComplex( ::sal_Int16 _charfontcharsetcomplex ) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontPitchComplex() override;\
    virtual void SAL_CALL setCharFontPitchComplex( ::sal_Int16 _charfontpitchcomplex ) override;\
    virtual css::awt::FontSlant SAL_CALL getCharPostureComplex() override;\
    virtual void SAL_CALL setCharPostureComplex( css::awt::FontSlant _charposturecomplex ) override;\
    virtual css::lang::Locale SAL_CALL getCharLocaleComplex() override;\
    virtual void SAL_CALL setCharLocaleComplex( const css::lang::Locale& _charlocalecomplex ) override;


#endif // INCLUDED_REPORTDESIGN_INC_REPORTHELPERDEFINES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
