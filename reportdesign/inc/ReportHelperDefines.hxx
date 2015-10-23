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
    virtual OUString SAL_CALL getDataField() throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setDataField(const OUString & the_value) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual sal_Bool SAL_CALL getPrintWhenGroupChange() throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setPrintWhenGroupChange(sal_Bool the_value) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual OUString SAL_CALL getConditionalPrintExpression() throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setConditionalPrintExpression(const OUString & the_value) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual css::uno::Reference< css::report::XFormatCondition > SAL_CALL createFormatCondition() throw (css::uno::RuntimeException, css::uno::Exception, std::exception) override;

#define SHAPE_HEADER()  \
    virtual css::awt::Point SAL_CALL getPosition() throw (css::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setPosition(const css::awt::Point & aPosition) throw (css::uno::RuntimeException, std::exception) override; \
    virtual css::awt::Size SAL_CALL getSize() throw (css::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setSize(const css::awt::Size & aSize) throw (css::uno::RuntimeException, css::beans::PropertyVetoException, std::exception) override;

#define REPORTCOMPONENT_HEADER()  \
    virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setName(const OUString & the_value) throw (css::uno::RuntimeException, css::beans::PropertyVetoException, std::exception) override; \
    virtual ::sal_Int32 SAL_CALL getHeight() throw (css::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setHeight(::sal_Int32 the_value) throw (css::uno::RuntimeException, css::beans::PropertyVetoException, std::exception) override; \
    virtual ::sal_Int32 SAL_CALL getPositionX() throw (css::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setPositionX(::sal_Int32 the_value) throw (css::uno::RuntimeException, std::exception) override; \
    virtual ::sal_Int32 SAL_CALL getPositionY() throw (css::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setPositionY(::sal_Int32 the_value) throw (css::uno::RuntimeException, std::exception) override; \
    virtual ::sal_Int32 SAL_CALL getWidth() throw (css::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setWidth(::sal_Int32 the_value) throw (css::uno::RuntimeException, css::beans::PropertyVetoException, std::exception) override; \
    virtual ::sal_Int16 SAL_CALL getControlBorder() throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setControlBorder(::sal_Int16 the_value) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual ::sal_Int32 SAL_CALL getControlBorderColor() throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setControlBorderColor(::sal_Int32 the_value) throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual sal_Bool SAL_CALL getPrintRepeatedValues() throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setPrintRepeatedValues(sal_Bool the_value) throw (css::uno::RuntimeException, css::beans::UnknownPropertyException, std::exception) override; \
    virtual css::uno::Sequence< OUString > SAL_CALL getMasterFields() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setMasterFields( const css::uno::Sequence< OUString >& _masterfields ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override; \
    virtual css::uno::Sequence< OUString > SAL_CALL getDetailFields() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setDetailFields( const css::uno::Sequence< OUString >& _detailfields ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override; \
    virtual css::uno::Reference< css::report::XSection > SAL_CALL getSection() throw (css::uno::RuntimeException, std::exception) override;

#define REPORTCONTROLFORMAT_HEADER()  \
    virtual ::sal_Int32 SAL_CALL getControlBackground() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setControlBackground( ::sal_Int32 _controlbackground ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getControlBackgroundTransparent() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setControlBackgroundTransparent( sal_Bool _controlbackgroundtransparent ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getParaAdjust() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setParaAdjust( ::sal_Int16 _paraadjust ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual css::awt::FontDescriptor SAL_CALL getFontDescriptor() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setFontDescriptor( const css::awt::FontDescriptor& _fontdescriptor ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual css::awt::FontDescriptor SAL_CALL getFontDescriptorAsian() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setFontDescriptorAsian( const css::awt::FontDescriptor& _fontdescriptor ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual css::awt::FontDescriptor SAL_CALL getFontDescriptorComplex() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setFontDescriptorComplex( const css::awt::FontDescriptor& _fontdescriptor ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getControlTextEmphasis() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setControlTextEmphasis( ::sal_Int16 _controltextemphasis ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharEmphasis() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharEmphasis( ::sal_Int16 _charemphasis ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharCombineIsOn() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharCombineIsOn( sal_Bool _charcombineison ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharCombinePrefix() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharCombinePrefix( const OUString& _charcombineprefix ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharCombineSuffix() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharCombineSuffix( const OUString& _charcombinesuffix ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharHidden() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharHidden( sal_Bool _charhidden ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharShadowed() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharShadowed( sal_Bool _charshadowed ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharContoured() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharContoured( sal_Bool _charcontoured ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharCaseMap() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharCaseMap( ::sal_Int16 _charcasemap ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual css::lang::Locale SAL_CALL getCharLocale() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharLocale( const css::lang::Locale& _charlocale ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharEscapement() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharEscapement( ::sal_Int16 _charescapement ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int8 SAL_CALL getCharEscapementHeight() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharEscapementHeight( ::sal_Int8 _charescapementheight ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharAutoKerning() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharAutoKerning( sal_Bool _charautokerning ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharKerning() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharKerning( ::sal_Int16 _charkerning ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharFlash() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFlash( sal_Bool _charflash ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharRelief() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharRelief( ::sal_Int16 _charrelief ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontName() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontName( const OUString& _charfontname ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontStyleName() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontStyleName( const OUString& _charfontstylename ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontFamily() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontFamily( ::sal_Int16 _charfontfamily ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontCharSet() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontCharSet( ::sal_Int16 _charfontcharset ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontPitch() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontPitch( ::sal_Int16 _charfontpitch ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int32 SAL_CALL getCharColor() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharColor( ::sal_Int32 _charcolor ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int32 SAL_CALL getCharUnderlineColor() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharUnderlineColor( ::sal_Int32 _charunderlinecolor ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharHeight() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharHeight( float _charheight ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharUnderline() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharUnderline( ::sal_Int16 _charunderline ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharWeight() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharWeight( float _charweight ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual css::awt::FontSlant SAL_CALL getCharPosture() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharPosture( css::awt::FontSlant _charposture ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharStrikeout() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharStrikeout( ::sal_Int16 _charstrikeout ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharWordMode() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharWordMode( sal_Bool _charwordmode ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharRotation() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharRotation( ::sal_Int16 _charrotation ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharScaleWidth() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharScaleWidth( ::sal_Int16 _charscalewidth ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual css::style::VerticalAlignment SAL_CALL getVerticalAlign() throw (css::beans::UnknownPropertyException,css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setVerticalAlign( css::style::VerticalAlignment _paravertalignment ) throw (css::lang::IllegalArgumentException, css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getHyperLinkURL() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setHyperLinkURL( const OUString& _hyperlinkurl ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getHyperLinkTarget() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setHyperLinkTarget( const OUString& _hyperlinktarget ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getHyperLinkName() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setHyperLinkName( const OUString& _hyperlinkname ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getVisitedCharStyleName() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setVisitedCharStyleName( const OUString& _visitedcharstylename ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getUnvisitedCharStyleName() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setUnvisitedCharStyleName( const OUString& _unvisitedcharstylename ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharHeightAsian() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharHeightAsian( float _charheightasian ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharWeightAsian() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharWeightAsian( float _charweightasian ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontNameAsian() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontNameAsian( const OUString& _charfontnameasian ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontStyleNameAsian() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontStyleNameAsian( const OUString& _charfontstylenameasian ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontFamilyAsian() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontFamilyAsian( ::sal_Int16 _charfontfamilyasian ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontCharSetAsian() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontCharSetAsian( ::sal_Int16 _charfontcharsetasian ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontPitchAsian() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontPitchAsian( ::sal_Int16 _charfontpitchasian ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual css::awt::FontSlant SAL_CALL getCharPostureAsian() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharPostureAsian( css::awt::FontSlant _charpostureasian ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual css::lang::Locale SAL_CALL getCharLocaleAsian() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharLocaleAsian( const css::lang::Locale& _charlocaleasian ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharHeightComplex() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharHeightComplex( float _charheightcomplex ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharWeightComplex() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharWeightComplex( float _charweightcomplex ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontNameComplex() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontNameComplex( const OUString& _charfontnamecomplex ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontStyleNameComplex() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontStyleNameComplex( const OUString& _charfontstylenamecomplex ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontFamilyComplex() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontFamilyComplex( ::sal_Int16 _charfontfamilycomplex ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontCharSetComplex() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontCharSetComplex( ::sal_Int16 _charfontcharsetcomplex ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontPitchComplex() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontPitchComplex( ::sal_Int16 _charfontpitchcomplex ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual css::awt::FontSlant SAL_CALL getCharPostureComplex() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharPostureComplex( css::awt::FontSlant _charposturecomplex ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual css::lang::Locale SAL_CALL getCharLocaleComplex() throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharLocaleComplex( const css::lang::Locale& _charlocalecomplex ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException, std::exception) override;



#endif // INCLUDED_REPORTDESIGN_INC_REPORTHELPERDEFINES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
