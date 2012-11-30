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
#ifndef INCLUDED_REPORTHELPERIMPL_HXX
#define INCLUDED_REPORTHELPERIMPL_HXX
// ::com::sun::star::report::XReportComponent:
#define REPORTCOMPONENT_IMPL3(clazz,arg) \
::rtl::OUString SAL_CALL clazz::getName() throw (uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return (arg).m_sName; \
} \
void SAL_CALL clazz::setName( const ::rtl::OUString& _name ) throw (uno::RuntimeException,beans::PropertyVetoException) \
{ \
    set(PROPERTY_NAME,_name,(arg).m_sName); \
} \
::sal_Int32 SAL_CALL clazz::getHeight() throw (uno::RuntimeException) \
{ \
    return getSize().Height; \
} \
void SAL_CALL clazz::setHeight( ::sal_Int32 _height ) throw (uno::RuntimeException) \
{ \
    awt::Size aSize = getSize(); \
    aSize.Height = _height; \
    try \
    { \
        setSize(aSize); \
    } \
    catch (const beans::PropertyVetoException &)      \
    { \
        throw uno::RuntimeException("Unacceptable height", static_cast<cppu::OWeakObject*>(this)); \
    } \
} \
::sal_Int32 SAL_CALL clazz::getPositionX() throw (uno::RuntimeException) \
{ \
    return getPosition().X; \
} \
void SAL_CALL clazz::setPositionX( ::sal_Int32 _positionx ) throw (uno::RuntimeException) \
{ \
    awt::Point aPos = getPosition(); \
    aPos.X = _positionx; \
    setPosition(aPos); \
} \
::sal_Int32 SAL_CALL clazz::getPositionY() throw (uno::RuntimeException) \
{ \
    return getPosition().Y; \
} \
void SAL_CALL clazz::setPositionY( ::sal_Int32 _positiony ) throw (uno::RuntimeException) \
{ \
    awt::Point aPos = getPosition(); \
    aPos.Y = _positiony; \
    setPosition(aPos); \
} \
::sal_Int32 SAL_CALL clazz::getWidth() throw (uno::RuntimeException) \
{ \
    return getSize().Width; \
} \
void SAL_CALL clazz::setWidth( ::sal_Int32 _width ) throw (uno::RuntimeException) \
{ \
    awt::Size aSize = getSize(); \
    aSize.Width = _width; \
    try \
    { \
        setSize(aSize); \
    } \
    catch (const beans::PropertyVetoException &)      \
    { \
        throw uno::RuntimeException("Unacceptable width", static_cast<cppu::OWeakObject*>(this)); \
    } \
} \
uno::Reference< report::XSection > SAL_CALL clazz::getSection() throw (uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    uno::Reference< container::XChild > xParent(getParent(  ),uno::UNO_QUERY); \
    return lcl_getSection(xParent); \
}

#define REPORTCOMPONENT_IMPL(clazz,arg) \
REPORTCOMPONENT_IMPL3(clazz,arg)\
::sal_Bool SAL_CALL clazz::getPrintRepeatedValues() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return (arg).m_bPrintRepeatedValues; \
} \
void SAL_CALL clazz::setPrintRepeatedValues( ::sal_Bool _printrepeatedvalues ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_PRINTREPEATEDVALUES,_printrepeatedvalues,(arg).m_bPrintRepeatedValues); \
}

#define REPORTCOMPONENT_IMPL2(clazz,arg) \
::sal_Int16  SAL_CALL clazz::getControlBorder() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return (arg).m_nBorder; \
} \
void SAL_CALL clazz::setControlBorder( ::sal_Int16 _border ) throw (uno::RuntimeException,lang::IllegalArgumentException,beans::UnknownPropertyException)\
{ \
    set(PROPERTY_CONTROLBORDER,_border,(arg).m_nBorder); \
} \
::sal_Int32 SAL_CALL clazz::getControlBorderColor() throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return (arg).m_nBorderColor; \
} \
void SAL_CALL clazz::setControlBorderColor( ::sal_Int32 _bordercolor ) throw (uno::RuntimeException, lang::IllegalArgumentException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CONTROLBORDERCOLOR,_bordercolor,(arg).m_nBorderColor); \
}

#define REPORTCOMPONENT_MASTERDETAIL(clazz,arg) \
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL clazz::getMasterFields() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return (arg).m_aMasterFields; \
} \
void SAL_CALL clazz::setMasterFields( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _masterfields ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    set(PROPERTY_MASTERFIELDS,_masterfields,(arg).m_aMasterFields); \
} \
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL clazz::getDetailFields() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return (arg).m_aDetailFields; \
} \
void SAL_CALL clazz::setDetailFields( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _detailfields ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    set(PROPERTY_DETAILFIELDS,_detailfields,(arg).m_aDetailFields); \
}

#define REPORTCOMPONENT_NOMASTERDETAIL(clazz) \
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL clazz::getMasterFields() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException) \
{ \
    throw ::com::sun::star::beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setMasterFields( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    throw ::com::sun::star::beans::UnknownPropertyException();\
} \
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL clazz::getDetailFields() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    throw ::com::sun::star::beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setDetailFields( const ::com::sun::star::uno::Sequence< ::rtl::OUString >&  ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    throw ::com::sun::star::beans::UnknownPropertyException();\
}

// ::com::sun::star::report::XReportControlFormat:
#define REPORTCONTROLFORMAT_IMPL1(clazz,varName)  \
::sal_Int32 SAL_CALL clazz::getControlBackground() throw (::com::sun::star::beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.m_bBackgroundTransparent ? COL_TRANSPARENT : varName.nBackgroundColor; \
} \
 \
void SAL_CALL clazz::setControlBackground( ::sal_Int32 _backgroundcolor ) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    sal_Bool bTransparent = _backgroundcolor == static_cast<sal_Int32>(COL_TRANSPARENT);\
    setControlBackgroundTransparent(bTransparent);\
    if ( !bTransparent )\
        set(PROPERTY_CONTROLBACKGROUND,_backgroundcolor,varName.nBackgroundColor);\
} \
 \
::sal_Bool SAL_CALL clazz::getControlBackgroundTransparent() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.m_bBackgroundTransparent; \
} \
 \
void SAL_CALL clazz::setControlBackgroundTransparent( ::sal_Bool _controlbackgroundtransparent ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_CONTROLBACKGROUNDTRANSPARENT,_controlbackgroundtransparent,varName.m_bBackgroundTransparent);\
    if ( _controlbackgroundtransparent )\
        set(PROPERTY_CONTROLBACKGROUND,static_cast<sal_Int32>(COL_TRANSPARENT),varName.nBackgroundColor);\
}

#define REPORTCONTROLFORMAT_IMPL2(clazz,varName)  \
::sal_Int16 SAL_CALL clazz::getCharStrikeout() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor.Strikeout; \
} \
 \
void SAL_CALL clazz::setCharStrikeout(::sal_Int16 the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CHARSTRIKEOUT,the_value,varName.aFontDescriptor.Strikeout); \
} \
 \
::sal_Bool SAL_CALL clazz::getCharWordMode() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor.WordLineMode; \
} \
 \
void SAL_CALL clazz::setCharWordMode(::sal_Bool the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CHARWORDMODE,the_value,varName.aFontDescriptor.WordLineMode); \
} \
 \
::sal_Int16 SAL_CALL clazz::getCharRotation() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return static_cast<sal_Int16>(varName.aFontDescriptor.Orientation); \
} \
 \
void SAL_CALL clazz::setCharRotation(::sal_Int16 the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    float newValue = the_value; \
    set(PROPERTY_CHARROTATION,newValue,varName.aFontDescriptor.Orientation); \
} \
 \
::sal_Int16 SAL_CALL clazz::getCharScaleWidth() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return static_cast<sal_Int16>(varName.aFontDescriptor.CharacterWidth); \
} \
 \
void SAL_CALL clazz::setCharScaleWidth(::sal_Int16 the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    float newValue = the_value; \
    set(PROPERTY_CHARSCALEWIDTH,newValue,varName.aFontDescriptor.CharacterWidth); \
} \
sal_Int16 SAL_CALL clazz::getParaAdjust() throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.nAlign; \
} \
 \
void SAL_CALL clazz::setParaAdjust( sal_Int16 _align ) throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    set(PROPERTY_PARAADJUST,_align,varName.nAlign); \
} \
 \
awt::FontDescriptor SAL_CALL clazz::getFontDescriptor() throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor; \
} \
 \
void SAL_CALL clazz::setFontDescriptor( const awt::FontDescriptor& _fontdescriptor ) throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    set(PROPERTY_FONTDESCRIPTOR,_fontdescriptor,varName.aFontDescriptor); \
} \
awt::FontDescriptor SAL_CALL clazz::getFontDescriptorAsian() throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aAsianFontDescriptor; \
} \
 \
void SAL_CALL clazz::setFontDescriptorAsian( const awt::FontDescriptor& _fontdescriptor ) throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    set(PROPERTY_FONTDESCRIPTORASIAN,_fontdescriptor,varName.aAsianFontDescriptor); \
} \
awt::FontDescriptor SAL_CALL clazz::getFontDescriptorComplex() throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aComplexFontDescriptor; \
} \
 \
void SAL_CALL clazz::setFontDescriptorComplex( const awt::FontDescriptor& _fontdescriptor ) throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    set(PROPERTY_FONTDESCRIPTORCOMPLEX,_fontdescriptor,varName.aComplexFontDescriptor); \
} \
 \
::sal_Int16 SAL_CALL clazz::getControlTextEmphasis() throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.nFontEmphasisMark; \
} \
 \
void SAL_CALL clazz::setControlTextEmphasis( ::sal_Int16 _fontemphasismark ) throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    set(PROPERTY_CONTROLTEXTEMPHASISMARK,_fontemphasismark,varName.nFontEmphasisMark); \
} \
 \
::sal_Int16 SAL_CALL clazz::getCharRelief() throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.nFontRelief; \
} \
 \
void SAL_CALL clazz::setCharRelief( ::sal_Int16 _fontrelief ) throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    set(PROPERTY_CHARRELIEF,_fontrelief,varName.nFontRelief); \
} \
 \
::sal_Int32 SAL_CALL clazz::getCharColor() throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.nTextColor; \
} \
 \
void SAL_CALL clazz::setCharColor( ::sal_Int32 _textcolor ) throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    set(PROPERTY_CHARCOLOR,_textcolor,varName.nTextColor); \
} \
 \
::sal_Int32 SAL_CALL clazz::getCharUnderlineColor() throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.nTextLineColor; \
} \
 \
void SAL_CALL clazz::setCharUnderlineColor( ::sal_Int32 _textlinecolor ) throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    set(PROPERTY_CHARUNDERLINECOLOR,_textlinecolor,varName.nTextLineColor); \
} \
 \
style::VerticalAlignment SAL_CALL clazz::getVerticalAlign() throw (beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aVerticalAlignment; \
} \
 \
void SAL_CALL clazz::setVerticalAlign( style::VerticalAlignment _verticalalign ) throw (lang::IllegalArgumentException,beans::UnknownPropertyException,uno::RuntimeException) \
{ \
    set(PROPERTY_VERTICALALIGN,_verticalalign,varName.aVerticalAlignment); \
} \
 \
::sal_Int16 SAL_CALL clazz::getCharEmphasis() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    return getControlTextEmphasis(); \
} \
 \
void SAL_CALL clazz::setCharEmphasis( ::sal_Int16 _charemphasis ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_CHAREMPHASIS,_charemphasis,varName.nFontEmphasisMark); \
} \
 \
::rtl::OUString SAL_CALL clazz::getCharFontName() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor.Name; \
} \
 \
void SAL_CALL clazz::setCharFontName( const ::rtl::OUString& _charfontname ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_CHARFONTNAME,_charfontname,varName.aFontDescriptor.Name); \
} \
 \
::rtl::OUString SAL_CALL clazz::getCharFontStyleName() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor.StyleName; \
} \
 \
void SAL_CALL clazz::setCharFontStyleName( const ::rtl::OUString& _charfontstylename ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_CHARFONTSTYLENAME,_charfontstylename,varName.aFontDescriptor.StyleName); \
} \
 \
::sal_Int16 SAL_CALL clazz::getCharFontFamily() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor.Family; \
} \
 \
void SAL_CALL clazz::setCharFontFamily( ::sal_Int16 _charfontfamily ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_CHARFONTFAMILY,_charfontfamily,varName.aFontDescriptor.Family); \
} \
 \
::sal_Int16 SAL_CALL clazz::getCharFontCharSet() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor.CharSet; \
} \
 \
void SAL_CALL clazz::setCharFontCharSet( ::sal_Int16 _charfontcharset ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_CHARFONTCHARSET,_charfontcharset,varName.aFontDescriptor.CharSet); \
} \
 \
::sal_Int16 SAL_CALL clazz::getCharFontPitch() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor.Pitch; \
} \
 \
void SAL_CALL clazz::setCharFontPitch( ::sal_Int16 _charfontpitch ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_CHARFONTPITCH,_charfontpitch,varName.aFontDescriptor.Pitch); \
} \
 \
float SAL_CALL clazz::getCharHeight() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor.Height; \
} \
 \
void SAL_CALL clazz::setCharHeight( float _charheight ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_CHARHEIGHT,static_cast<sal_Int16>(_charheight),varName.aFontDescriptor.Height); \
} \
 \
::sal_Int16 SAL_CALL clazz::getCharUnderline() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor.Underline; \
} \
 \
void SAL_CALL clazz::setCharUnderline( ::sal_Int16 _charunderline ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_CHARUNDERLINE,_charunderline,varName.aFontDescriptor.Underline); \
} \
 \
float SAL_CALL clazz::getCharWeight() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor.Weight; \
} \
 \
void SAL_CALL clazz::setCharWeight( float _charweight ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_CHARWEIGHT,_charweight,varName.aFontDescriptor.Weight); \
} \
 \
awt::FontSlant SAL_CALL clazz::getCharPosture() throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aFontDescriptor.Slant; \
} \
 \
void SAL_CALL clazz::setCharPosture( awt::FontSlant _charposture ) throw (beans::UnknownPropertyException, uno::RuntimeException) \
{ \
    set(PROPERTY_CHARPOSTURE,_charposture,varName.aFontDescriptor.Slant); \
}\
::sal_Bool SAL_CALL clazz::getCharFlash() throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.bCharFlash; \
} \
void SAL_CALL clazz::setCharFlash(::sal_Bool the_value) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    set(PROPERTY_CHARFLASH,the_value,varName.bCharFlash); \
}\
::sal_Bool SAL_CALL clazz::getCharAutoKerning() throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.bCharAutoKerning; \
} \
void SAL_CALL clazz::setCharAutoKerning(::sal_Bool the_value) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    set(PROPERTY_CHARAUTOKERNING,the_value,varName.bCharAutoKerning); \
}\
::sal_Int8 SAL_CALL clazz::getCharEscapementHeight() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.nCharEscapementHeight; \
} \
void SAL_CALL clazz::setCharEscapementHeight(::sal_Int8 the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CHARESCAPEMENTHEIGHT,the_value,varName.nCharEscapementHeight); \
}\
lang::Locale SAL_CALL clazz::getCharLocale() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aCharLocale; \
} \
void SAL_CALL clazz::setCharLocale(const lang::Locale & the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    BoundListeners l; \
    { \
        ::osl::MutexGuard aGuard(m_aMutex); \
        if ( varName.aCharLocale.Language != the_value.Language  \
            || varName.aCharLocale.Country != the_value.Country  \
            || varName.aCharLocale.Variant != the_value.Variant ) \
        { \
            prepareSet(PROPERTY_CHARLOCALE, ::com::sun::star::uno::makeAny(varName.aCharLocale), ::com::sun::star::uno::makeAny(the_value), &l); \
            varName.aCharLocale = the_value; \
        } \
    } \
    l.notify(); \
}\
::sal_Int16 SAL_CALL clazz::getCharEscapement() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.nCharEscapement; \
} \
void SAL_CALL clazz::setCharEscapement(::sal_Int16 the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CHARESCAPEMENT,the_value,varName.nCharEscapement); \
}\
::sal_Int16 SAL_CALL clazz::getCharCaseMap() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.nCharCaseMap; \
} \
void SAL_CALL clazz::setCharCaseMap(::sal_Int16 the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CHARCASEMAP,the_value,varName.nCharCaseMap); \
}\
::sal_Bool SAL_CALL clazz::getCharCombineIsOn() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.bCharCombineIsOn; \
} \
void SAL_CALL clazz::setCharCombineIsOn(::sal_Bool the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CHARCOMBINEISON,the_value,varName.bCharCombineIsOn); \
}\
::rtl::OUString SAL_CALL clazz::getCharCombinePrefix() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.sCharCombinePrefix; \
} \
void SAL_CALL clazz::setCharCombinePrefix(const ::rtl::OUString & the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CHARCOMBINEPREFIX,the_value,varName.sCharCombinePrefix); \
}\
::rtl::OUString SAL_CALL clazz::getCharCombineSuffix() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.sCharCombineSuffix; \
} \
void SAL_CALL clazz::setCharCombineSuffix(const ::rtl::OUString & the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CHARCOMBINESUFFIX,the_value,varName.sCharCombineSuffix); \
}\
::sal_Bool SAL_CALL clazz::getCharHidden() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.bCharHidden; \
} \
void SAL_CALL clazz::setCharHidden(::sal_Bool the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CHARHIDDEN,the_value,varName.bCharHidden); \
}\
::sal_Bool SAL_CALL clazz::getCharShadowed() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.bCharShadowed; \
} \
void SAL_CALL clazz::setCharShadowed(::sal_Bool the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CHARSHADOWED,the_value,varName.bCharShadowed); \
}\
::sal_Bool SAL_CALL clazz::getCharContoured() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.bCharContoured; \
} \
void SAL_CALL clazz::setCharContoured(::sal_Bool the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_CHARCONTOURED,the_value,varName.bCharContoured); \
}\
::rtl::OUString SAL_CALL clazz::getHyperLinkURL() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.sHyperLinkURL; \
} \
void SAL_CALL clazz::setHyperLinkURL(const ::rtl::OUString & the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_HYPERLINKURL,the_value,varName.sHyperLinkURL); \
}\
::rtl::OUString SAL_CALL clazz::getHyperLinkTarget() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.sHyperLinkTarget; \
} \
void SAL_CALL clazz::setHyperLinkTarget(const ::rtl::OUString & the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_HYPERLINKTARGET,the_value,varName.sHyperLinkTarget); \
}\
::rtl::OUString SAL_CALL clazz::getHyperLinkName() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.sHyperLinkName; \
} \
void SAL_CALL clazz::setHyperLinkName(const ::rtl::OUString & the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_HYPERLINKNAME,the_value,varName.sHyperLinkName); \
}\
::rtl::OUString SAL_CALL clazz::getVisitedCharStyleName() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.sVisitedCharStyleName; \
} \
void SAL_CALL clazz::setVisitedCharStyleName(const ::rtl::OUString & the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_VISITEDCHARSTYLENAME,the_value,varName.sVisitedCharStyleName); \
}\
::rtl::OUString SAL_CALL clazz::getUnvisitedCharStyleName() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.sUnvisitedCharStyleName; \
} \
void SAL_CALL clazz::setUnvisitedCharStyleName(const ::rtl::OUString & the_value) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    set(PROPERTY_UNVISITEDCHARSTYLENAME,the_value,varName.sUnvisitedCharStyleName); \
}\
::sal_Int16 SAL_CALL clazz::getCharKerning() throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.nCharKerning; \
}\
void SAL_CALL clazz::setCharKerning(::sal_Int16 the_value) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    set(PROPERTY_CHARKERNING,the_value,varName.nCharKerning); \
}\
float SAL_CALL clazz::getCharHeightAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
    { \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aAsianFontDescriptor.Height; \
}\
void SAL_CALL clazz::setCharHeightAsian( float the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARHEIGHTASIAN,static_cast<sal_Int16>(the_value),varName.aAsianFontDescriptor.Height); \
}\
float SAL_CALL clazz::getCharWeightAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
    { \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aAsianFontDescriptor.Weight; \
}\
void SAL_CALL clazz::setCharWeightAsian( float the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARWEIGHTASIAN,the_value,varName.aAsianFontDescriptor.Weight); \
}\
::rtl::OUString SAL_CALL clazz::getCharFontNameAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aAsianFontDescriptor.Name; \
}\
void SAL_CALL clazz::setCharFontNameAsian( const ::rtl::OUString& the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARFONTNAMEASIAN,the_value,varName.aAsianFontDescriptor.Name); \
}\
::rtl::OUString SAL_CALL clazz::getCharFontStyleNameAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aAsianFontDescriptor.StyleName; \
}\
void SAL_CALL clazz::setCharFontStyleNameAsian( const ::rtl::OUString& the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARFONTSTYLENAMEASIAN,the_value,varName.aAsianFontDescriptor.StyleName); \
}\
::sal_Int16 SAL_CALL clazz::getCharFontFamilyAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aAsianFontDescriptor.Family; \
}\
void SAL_CALL clazz::setCharFontFamilyAsian( ::sal_Int16 the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARFONTFAMILYASIAN,the_value,varName.aAsianFontDescriptor.Family); \
}\
::sal_Int16 SAL_CALL clazz::getCharFontCharSetAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aAsianFontDescriptor.CharSet; \
}\
void SAL_CALL clazz::setCharFontCharSetAsian( ::sal_Int16 the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARFONTCHARSETASIAN,the_value,varName.aAsianFontDescriptor.CharSet); \
}\
::sal_Int16 SAL_CALL clazz::getCharFontPitchAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aAsianFontDescriptor.Pitch; \
}\
void SAL_CALL clazz::setCharFontPitchAsian( ::sal_Int16 the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARFONTPITCHASIAN,the_value,varName.aAsianFontDescriptor.Pitch); \
}\
::com::sun::star::awt::FontSlant SAL_CALL clazz::getCharPostureAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aAsianFontDescriptor.Slant; \
}\
void SAL_CALL clazz::setCharPostureAsian( ::com::sun::star::awt::FontSlant the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARPOSTUREASIAN,the_value,varName.aAsianFontDescriptor.Slant); \
}\
::com::sun::star::lang::Locale SAL_CALL clazz::getCharLocaleAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aCharLocaleAsian; \
}\
void SAL_CALL clazz::setCharLocaleAsian( const ::com::sun::star::lang::Locale& the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    BoundListeners l; \
    { \
        ::osl::MutexGuard aGuard(m_aMutex); \
        if ( varName.aCharLocaleAsian.Language != the_value.Language  \
            || varName.aCharLocaleAsian.Country != the_value.Country  \
            || varName.aCharLocaleAsian.Variant != the_value.Variant ) \
        { \
            prepareSet(PROPERTY_CHARLOCALEASIAN, ::com::sun::star::uno::makeAny(varName.aCharLocaleAsian), ::com::sun::star::uno::makeAny(the_value), &l); \
            varName.aCharLocaleAsian = the_value; \
        } \
    } \
    l.notify(); \
}\
float SAL_CALL clazz::getCharHeightComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aComplexFontDescriptor.Height; \
}\
void SAL_CALL clazz::setCharHeightComplex( float the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARHEIGHTCOMPLEX,static_cast<sal_Int16>(the_value),varName.aComplexFontDescriptor.Height); \
}\
float SAL_CALL clazz::getCharWeightComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aComplexFontDescriptor.Weight; \
}\
void SAL_CALL clazz::setCharWeightComplex( float the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARWEIGHTCOMPLEX,the_value,varName.aComplexFontDescriptor.Weight); \
}\
::rtl::OUString SAL_CALL clazz::getCharFontNameComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aComplexFontDescriptor.Name; \
}\
void SAL_CALL clazz::setCharFontNameComplex( const ::rtl::OUString& the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARFONTNAMECOMPLEX,the_value,varName.aComplexFontDescriptor.Name); \
}\
::rtl::OUString SAL_CALL clazz::getCharFontStyleNameComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aComplexFontDescriptor.StyleName; \
}\
void SAL_CALL clazz::setCharFontStyleNameComplex( const ::rtl::OUString& the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARFONTSTYLENAMECOMPLEX,the_value,varName.aComplexFontDescriptor.StyleName); \
}\
::sal_Int16 SAL_CALL clazz::getCharFontFamilyComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aComplexFontDescriptor.Family; \
}\
void SAL_CALL clazz::setCharFontFamilyComplex( ::sal_Int16 the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARFONTFAMILYCOMPLEX,the_value,varName.aComplexFontDescriptor.Family); \
}\
::sal_Int16 SAL_CALL clazz::getCharFontCharSetComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aComplexFontDescriptor.CharSet; \
}\
void SAL_CALL clazz::setCharFontCharSetComplex( ::sal_Int16 the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARFONTCHARSETCOMPLEX,the_value,varName.aComplexFontDescriptor.CharSet); \
}\
::sal_Int16 SAL_CALL clazz::getCharFontPitchComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aComplexFontDescriptor.Pitch; \
}\
void SAL_CALL clazz::setCharFontPitchComplex( ::sal_Int16 the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARFONTPITCHCOMPLEX,the_value,varName.aComplexFontDescriptor.Pitch); \
}\
::com::sun::star::awt::FontSlant SAL_CALL clazz::getCharPostureComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aComplexFontDescriptor.Slant; \
}\
void SAL_CALL clazz::setCharPostureComplex( ::com::sun::star::awt::FontSlant the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    set(PROPERTY_CHARPOSTURECOMPLEX,the_value,varName.aComplexFontDescriptor.Slant); \
}\
::com::sun::star::lang::Locale SAL_CALL clazz::getCharLocaleComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    ::osl::MutexGuard aGuard(m_aMutex); \
    return varName.aCharLocaleComplex; \
}\
void SAL_CALL clazz::setCharLocaleComplex( const ::com::sun::star::lang::Locale& the_value ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
{ \
    BoundListeners l; \
    { \
        ::osl::MutexGuard aGuard(m_aMutex); \
        if ( varName.aCharLocaleComplex.Language != the_value.Language  \
            || varName.aCharLocaleComplex.Country != the_value.Country  \
            || varName.aCharLocaleComplex.Variant != the_value.Variant ) \
        { \
            prepareSet(PROPERTY_CHARLOCALECOMPLEX, ::com::sun::star::uno::makeAny(varName.aCharLocaleComplex), ::com::sun::star::uno::makeAny(the_value), &l); \
            varName.aCharLocaleComplex = the_value; \
        } \
    } \
    l.notify(); \
}\


#define NO_REPORTCONTROLFORMAT_IMPL(clazz)  \
sal_Int16 SAL_CALL clazz::getParaAdjust() throw (uno::RuntimeException, beans::UnknownPropertyException)\
{\
    throw beans::UnknownPropertyException();\
}\
void SAL_CALL clazz::setParaAdjust(sal_Int16 /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{\
    throw beans::UnknownPropertyException();\
}\
::sal_Int16 SAL_CALL clazz::getCharStrikeout() throw (uno::RuntimeException, beans::UnknownPropertyException)\
{\
    throw beans::UnknownPropertyException();\
}\
void SAL_CALL clazz::setCharStrikeout(::sal_Int16 /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{\
    throw beans::UnknownPropertyException();\
}\
::sal_Bool SAL_CALL clazz::getCharWordMode() throw (uno::RuntimeException, beans::UnknownPropertyException)\
{\
    throw beans::UnknownPropertyException();\
}\
void SAL_CALL clazz::setCharWordMode(::sal_Bool /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{\
    throw beans::UnknownPropertyException();\
}\
::sal_Int16 SAL_CALL clazz::getCharRotation() throw (uno::RuntimeException, beans::UnknownPropertyException)\
{\
    throw beans::UnknownPropertyException();\
}\
void SAL_CALL clazz::setCharRotation(::sal_Int16 /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{\
    throw beans::UnknownPropertyException();\
}\
::sal_Int16 SAL_CALL clazz::getCharScaleWidth() throw (uno::RuntimeException, beans::UnknownPropertyException)\
{\
    throw beans::UnknownPropertyException();\
}\
void SAL_CALL clazz::setCharScaleWidth(::sal_Int16 /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{\
    throw beans::UnknownPropertyException();\
}\
::sal_Bool SAL_CALL clazz::getCharFlash() throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharFlash(::sal_Bool /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    throw beans::UnknownPropertyException();\
}\
::sal_Bool SAL_CALL clazz::getCharAutoKerning() throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharAutoKerning(::sal_Bool /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    throw beans::UnknownPropertyException();\
}\
::sal_Int8 SAL_CALL clazz::getCharEscapementHeight() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharEscapementHeight(::sal_Int8 /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
lang::Locale SAL_CALL clazz::getCharLocale() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharLocale(const lang::Locale & /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
::sal_Int16 SAL_CALL clazz::getCharEscapement() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharEscapement(::sal_Int16 /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
::sal_Int16 SAL_CALL clazz::getCharCaseMap() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharCaseMap(::sal_Int16 /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
::sal_Bool SAL_CALL clazz::getCharCombineIsOn() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharCombineIsOn(::sal_Bool /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
::rtl::OUString SAL_CALL clazz::getCharCombinePrefix() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharCombinePrefix(const ::rtl::OUString & /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
::rtl::OUString SAL_CALL clazz::getCharCombineSuffix() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharCombineSuffix(const ::rtl::OUString & /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
::sal_Bool SAL_CALL clazz::getCharHidden() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharHidden(::sal_Bool /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
::sal_Bool SAL_CALL clazz::getCharShadowed() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharShadowed(::sal_Bool /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
::sal_Bool SAL_CALL clazz::getCharContoured() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setCharContoured(::sal_Bool /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
::rtl::OUString SAL_CALL clazz::getVisitedCharStyleName() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setVisitedCharStyleName(const ::rtl::OUString & /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
::rtl::OUString SAL_CALL clazz::getUnvisitedCharStyleName() throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
} \
void SAL_CALL clazz::setUnvisitedCharStyleName(const ::rtl::OUString & /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException) \
{ \
    throw beans::UnknownPropertyException();\
}\
::sal_Int16 SAL_CALL clazz::getCharKerning() throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    throw beans::UnknownPropertyException();\
}\
void SAL_CALL clazz::setCharKerning(::sal_Int16 /*the_value*/) throw (uno::RuntimeException, beans::UnknownPropertyException)\
{ \
    throw beans::UnknownPropertyException();\
}\
\
awt::FontDescriptor SAL_CALL clazz::getFontDescriptor() throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setFontDescriptor( const awt::FontDescriptor& /*_fontdescriptor*/ ) throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
awt::FontDescriptor SAL_CALL clazz::getFontDescriptorAsian() throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setFontDescriptorAsian( const awt::FontDescriptor& /*_fontdescriptor*/ ) throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
awt::FontDescriptor SAL_CALL clazz::getFontDescriptorComplex() throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setFontDescriptorComplex( const awt::FontDescriptor& /*_fontdescriptor*/ ) throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
throw beans::UnknownPropertyException();\
}\
\
::sal_Int16 SAL_CALL clazz::getControlTextEmphasis() throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setControlTextEmphasis( ::sal_Int16 /*_fontemphasismark*/ ) throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
::sal_Int16 SAL_CALL clazz::getCharRelief() throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharRelief( ::sal_Int16 /*_fontrelief*/ ) throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
::sal_Int32 SAL_CALL clazz::getCharColor() throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharColor( ::sal_Int32 /*_textcolor*/ ) throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
::sal_Int32 SAL_CALL clazz::getCharUnderlineColor() throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharUnderlineColor( ::sal_Int32 /*_textlinecolor*/ ) throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
style::VerticalAlignment SAL_CALL clazz::getVerticalAlign() throw (beans::UnknownPropertyException,uno::RuntimeException)\
{\
    ::osl::MutexGuard aGuard(m_aMutex);\
    return m_aProps.aFormatProperties.aVerticalAlignment;\
}\
\
void SAL_CALL clazz::setVerticalAlign( style::VerticalAlignment _verticalalign ) throw (lang::IllegalArgumentException,beans::UnknownPropertyException,uno::RuntimeException)\
{\
    set(PROPERTY_VERTICALALIGN,_verticalalign,m_aProps.aFormatProperties.aVerticalAlignment);\
}\
\
::sal_Int16 SAL_CALL clazz::getCharEmphasis() throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharEmphasis( ::sal_Int16 /*_charemphasis*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
::rtl::OUString SAL_CALL clazz::getCharFontName() throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharFontName( const ::rtl::OUString& /*_charfontname*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
::rtl::OUString SAL_CALL clazz::getCharFontStyleName() throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharFontStyleName( const ::rtl::OUString& /*_charfontstylename*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
::sal_Int16 SAL_CALL clazz::getCharFontFamily() throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharFontFamily( ::sal_Int16 /*_charfontfamily*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
::sal_Int16 SAL_CALL clazz::getCharFontCharSet() throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharFontCharSet( ::sal_Int16 /*_charfontcharset*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
::sal_Int16 SAL_CALL clazz::getCharFontPitch() throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharFontPitch( ::sal_Int16 /*_charfontpitch*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
float SAL_CALL clazz::getCharHeight() throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharHeight( float /*_charheight*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
::sal_Int16 SAL_CALL clazz::getCharUnderline() throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharUnderline( ::sal_Int16 /*_charunderline*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
float SAL_CALL clazz::getCharWeight() throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharWeight( float /*_charweight*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
awt::FontSlant SAL_CALL clazz::getCharPosture() throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
\
void SAL_CALL clazz::setCharPosture( awt::FontSlant /*_charposture*/ ) throw (beans::UnknownPropertyException, uno::RuntimeException)\
{\
    throw beans::UnknownPropertyException();\
}\
 float SAL_CALL clazz::getCharHeightAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharHeightAsian( float  ) throw (beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 float SAL_CALL clazz::getCharWeightAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharWeightAsian( float  ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::rtl::OUString SAL_CALL clazz::getCharFontNameAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharFontNameAsian( const ::rtl::OUString& ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::rtl::OUString SAL_CALL clazz::getCharFontStyleNameAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharFontStyleNameAsian( const ::rtl::OUString& ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::sal_Int16 SAL_CALL clazz::getCharFontFamilyAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharFontFamilyAsian( ::sal_Int16 ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::sal_Int16 SAL_CALL clazz::getCharFontCharSetAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharFontCharSetAsian( ::sal_Int16 ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::sal_Int16 SAL_CALL clazz::getCharFontPitchAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharFontPitchAsian( ::sal_Int16 ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::com::sun::star::awt::FontSlant SAL_CALL clazz::getCharPostureAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharPostureAsian( ::com::sun::star::awt::FontSlant ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::com::sun::star::lang::Locale SAL_CALL clazz::getCharLocaleAsian() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharLocaleAsian( const ::com::sun::star::lang::Locale& ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 float SAL_CALL clazz::getCharHeightComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharHeightComplex( float ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 float SAL_CALL clazz::getCharWeightComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharWeightComplex( float ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::rtl::OUString SAL_CALL clazz::getCharFontNameComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharFontNameComplex( const ::rtl::OUString& ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::rtl::OUString SAL_CALL clazz::getCharFontStyleNameComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharFontStyleNameComplex( const ::rtl::OUString& ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::sal_Int16 SAL_CALL clazz::getCharFontFamilyComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharFontFamilyComplex( ::sal_Int16 ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::sal_Int16 SAL_CALL clazz::getCharFontCharSetComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharFontCharSetComplex( ::sal_Int16 ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::sal_Int16 SAL_CALL clazz::getCharFontPitchComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharFontPitchComplex( ::sal_Int16 ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::com::sun::star::awt::FontSlant SAL_CALL clazz::getCharPostureComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharPostureComplex( ::com::sun::star::awt::FontSlant ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 ::com::sun::star::lang::Locale SAL_CALL clazz::getCharLocaleComplex() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}\
 void SAL_CALL clazz::setCharLocaleComplex( const ::com::sun::star::lang::Locale& ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException)\
 {\
    throw beans::UnknownPropertyException();\
}


// ::com::sun::star::report::XReportControlFormat:
#define REPORTCONTROLFORMAT_IMPL(clazz,varName)  \
        REPORTCONTROLFORMAT_IMPL1(clazz,varName)  \
        REPORTCONTROLFORMAT_IMPL2(clazz,varName)


#endif //INCLUDED_REPORTHELPERIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
