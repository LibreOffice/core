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
    virtual OUString SAL_CALL getDataField() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setDataField(const OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual sal_Bool SAL_CALL getPrintWhenGroupChange() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setPrintWhenGroupChange(sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual OUString SAL_CALL getConditionalPrintExpression() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setConditionalPrintExpression(const OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormatCondition > SAL_CALL createFormatCondition() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::uno::Exception, std::exception) override;

#define SHAPE_HEADER()  \
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw (::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setPosition(const ::com::sun::star::awt::Point & aPosition) throw (::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw (::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setSize(const ::com::sun::star::awt::Size & aSize) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::PropertyVetoException, std::exception) override;

#define REPORTCOMPONENT_HEADER()  \
    virtual OUString SAL_CALL getName() throw (::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setName(const OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::PropertyVetoException, std::exception) override; \
    virtual ::sal_Int32 SAL_CALL getHeight() throw (::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setHeight(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::PropertyVetoException, std::exception) override; \
    virtual ::sal_Int32 SAL_CALL getPositionX() throw (::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setPositionX(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual ::sal_Int32 SAL_CALL getPositionY() throw (::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setPositionY(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual ::sal_Int32 SAL_CALL getWidth() throw (::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setWidth(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::PropertyVetoException, std::exception) override; \
    virtual ::sal_Int16 SAL_CALL getControlBorder() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setControlBorder(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual ::sal_Int32 SAL_CALL getControlBorderColor() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setControlBorderColor(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual sal_Bool SAL_CALL getPrintRepeatedValues() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual void SAL_CALL setPrintRepeatedValues(sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException, std::exception) override; \
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getMasterFields() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setMasterFields( const ::com::sun::star::uno::Sequence< OUString >& _masterfields ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getDetailFields() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual void SAL_CALL setDetailFields( const ::com::sun::star::uno::Sequence< OUString >& _detailfields ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override; \
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > SAL_CALL getSection() throw (::com::sun::star::uno::RuntimeException, std::exception) override;

#define REPORTCONTROLFORMAT_HEADER()  \
    virtual ::sal_Int32 SAL_CALL getControlBackground() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setControlBackground( ::sal_Int32 _controlbackground ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getControlBackgroundTransparent() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setControlBackgroundTransparent( sal_Bool _controlbackgroundtransparent ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getParaAdjust() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setParaAdjust( ::sal_Int16 _paraadjust ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getFontDescriptor() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setFontDescriptor( const ::com::sun::star::awt::FontDescriptor& _fontdescriptor ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getFontDescriptorAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setFontDescriptorAsian( const ::com::sun::star::awt::FontDescriptor& _fontdescriptor ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getFontDescriptorComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setFontDescriptorComplex( const ::com::sun::star::awt::FontDescriptor& _fontdescriptor ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getControlTextEmphasis() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setControlTextEmphasis( ::sal_Int16 _controltextemphasis ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharEmphasis() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharEmphasis( ::sal_Int16 _charemphasis ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharCombineIsOn() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharCombineIsOn( sal_Bool _charcombineison ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharCombinePrefix() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharCombinePrefix( const OUString& _charcombineprefix ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharCombineSuffix() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharCombineSuffix( const OUString& _charcombinesuffix ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharHidden() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharHidden( sal_Bool _charhidden ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharShadowed() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharShadowed( sal_Bool _charshadowed ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharContoured() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharContoured( sal_Bool _charcontoured ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharCaseMap() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharCaseMap( ::sal_Int16 _charcasemap ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::com::sun::star::lang::Locale SAL_CALL getCharLocale() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharLocale( const ::com::sun::star::lang::Locale& _charlocale ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharEscapement() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharEscapement( ::sal_Int16 _charescapement ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int8 SAL_CALL getCharEscapementHeight() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharEscapementHeight( ::sal_Int8 _charescapementheight ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharAutoKerning() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharAutoKerning( sal_Bool _charautokerning ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharKerning() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharKerning( ::sal_Int16 _charkerning ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharFlash() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFlash( sal_Bool _charflash ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharRelief() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharRelief( ::sal_Int16 _charrelief ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontName() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontName( const OUString& _charfontname ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontStyleName() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontStyleName( const OUString& _charfontstylename ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontFamily() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontFamily( ::sal_Int16 _charfontfamily ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontCharSet() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontCharSet( ::sal_Int16 _charfontcharset ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontPitch() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontPitch( ::sal_Int16 _charfontpitch ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int32 SAL_CALL getCharColor() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharColor( ::sal_Int32 _charcolor ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int32 SAL_CALL getCharUnderlineColor() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharUnderlineColor( ::sal_Int32 _charunderlinecolor ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharHeight() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharHeight( float _charheight ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharUnderline() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharUnderline( ::sal_Int16 _charunderline ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharWeight() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharWeight( float _charweight ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::com::sun::star::awt::FontSlant SAL_CALL getCharPosture() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharPosture( ::com::sun::star::awt::FontSlant _charposture ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharStrikeout() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharStrikeout( ::sal_Int16 _charstrikeout ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual sal_Bool SAL_CALL getCharWordMode() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharWordMode( sal_Bool _charwordmode ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharRotation() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharRotation( ::sal_Int16 _charrotation ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharScaleWidth() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharScaleWidth( ::sal_Int16 _charscalewidth ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::com::sun::star::style::VerticalAlignment SAL_CALL getVerticalAlign() throw (::com::sun::star::beans::UnknownPropertyException,::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setVerticalAlign( ::com::sun::star::style::VerticalAlignment _paravertalignment ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getHyperLinkURL() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setHyperLinkURL( const OUString& _hyperlinkurl ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getHyperLinkTarget() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setHyperLinkTarget( const OUString& _hyperlinktarget ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getHyperLinkName() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setHyperLinkName( const OUString& _hyperlinkname ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getVisitedCharStyleName() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setVisitedCharStyleName( const OUString& _visitedcharstylename ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getUnvisitedCharStyleName() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setUnvisitedCharStyleName( const OUString& _unvisitedcharstylename ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharHeightAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharHeightAsian( float _charheightasian ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharWeightAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharWeightAsian( float _charweightasian ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontNameAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontNameAsian( const OUString& _charfontnameasian ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontStyleNameAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontStyleNameAsian( const OUString& _charfontstylenameasian ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontFamilyAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontFamilyAsian( ::sal_Int16 _charfontfamilyasian ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontCharSetAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontCharSetAsian( ::sal_Int16 _charfontcharsetasian ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontPitchAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontPitchAsian( ::sal_Int16 _charfontpitchasian ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::com::sun::star::awt::FontSlant SAL_CALL getCharPostureAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharPostureAsian( ::com::sun::star::awt::FontSlant _charpostureasian ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::com::sun::star::lang::Locale SAL_CALL getCharLocaleAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharLocaleAsian( const ::com::sun::star::lang::Locale& _charlocaleasian ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharHeightComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharHeightComplex( float _charheightcomplex ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual float SAL_CALL getCharWeightComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharWeightComplex( float _charweightcomplex ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontNameComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontNameComplex( const OUString& _charfontnamecomplex ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual OUString SAL_CALL getCharFontStyleNameComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontStyleNameComplex( const OUString& _charfontstylenamecomplex ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontFamilyComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontFamilyComplex( ::sal_Int16 _charfontfamilycomplex ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontCharSetComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontCharSetComplex( ::sal_Int16 _charfontcharsetcomplex ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::sal_Int16 SAL_CALL getCharFontPitchComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharFontPitchComplex( ::sal_Int16 _charfontpitchcomplex ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::com::sun::star::awt::FontSlant SAL_CALL getCharPostureComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharPostureComplex( ::com::sun::star::awt::FontSlant _charposturecomplex ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual ::com::sun::star::lang::Locale SAL_CALL getCharLocaleComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;\
    virtual void SAL_CALL setCharLocaleComplex( const ::com::sun::star::lang::Locale& _charlocalecomplex ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;



#endif // INCLUDED_REPORTDESIGN_INC_REPORTHELPERDEFINES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
