/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportHelperDefines.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:17:26 $
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
#ifndef INCLUDED_REPORTHELPERDEFINES_HXX
#define INCLUDED_REPORTHELPERDEFINES_HXX

#define REPORTCONTROLMODEL_HEADER()  \
    virtual ::rtl::OUString SAL_CALL getDataField() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setDataField(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Bool SAL_CALL getPrintWhenGroupChange() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setPrintWhenGroupChange(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::rtl::OUString SAL_CALL getConditionalPrintExpression() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setConditionalPrintExpression(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormatCondition > SAL_CALL createFormatCondition() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::uno::Exception);

#define SHAPE_HEADER()  \
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition() throw (::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL setPosition(const ::com::sun::star::awt::Point & aPosition) throw (::com::sun::star::uno::RuntimeException); \
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw (::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL setSize(const ::com::sun::star::awt::Size & aSize) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::PropertyVetoException);

#define REPORTCOMPONENT_HEADER()  \
    virtual ::rtl::OUString SAL_CALL getName() throw (::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL setName(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::PropertyVetoException); \
    virtual ::sal_Int32 SAL_CALL getHeight() throw (::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL setHeight(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException); \
    virtual ::sal_Int32 SAL_CALL getPositionX() throw (::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL setPositionX(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException); \
    virtual ::sal_Int32 SAL_CALL getPositionY() throw (::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL setPositionY(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException); \
    virtual ::sal_Int32 SAL_CALL getWidth() throw (::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL setWidth(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException); \
    virtual ::sal_Int16 SAL_CALL getControlBorder() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setControlBorder(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int32 SAL_CALL getControlBorderColor() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setControlBorderColor(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Bool SAL_CALL getPrintRepeatedValues() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setPrintRepeatedValues(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getMasterFields() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL setMasterFields( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _masterfields ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException); \
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getDetailFields() throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL setDetailFields( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _detailfields ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException); \
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > SAL_CALL getSection() throw (::com::sun::star::uno::RuntimeException);

#define REPORTCONTROLFORMAT_HEADER()  \
    virtual ::sal_Int32 SAL_CALL getControlBackground() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setControlBackground(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Bool SAL_CALL getControlBackgroundTransparent() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setControlBackgroundTransparent(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getParaAdjust() throw (::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL setParaAdjust(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException); \
    virtual ::com::sun::star::awt::FontDescriptor SAL_CALL getFontDescriptor() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setFontDescriptor(const ::com::sun::star::awt::FontDescriptor & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getControlTextEmphasis() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setControlTextEmphasis(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getCharEmphasis() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharEmphasis(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Bool SAL_CALL getCharCombineIsOn() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharCombineIsOn(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::rtl::OUString SAL_CALL getCharCombinePrefix() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharCombinePrefix(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::rtl::OUString SAL_CALL getCharCombineSuffix() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharCombineSuffix(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Bool SAL_CALL getCharHidden() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharHidden(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Bool SAL_CALL getCharShadowed() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharShadowed(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Bool SAL_CALL getCharContoured() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharContoured(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getCharCaseMap() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharCaseMap(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::com::sun::star::lang::Locale SAL_CALL getCharLocale() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharLocale(const ::com::sun::star::lang::Locale & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getCharEscapement() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharEscapement(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int8 SAL_CALL getCharEscapementHeight() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharEscapementHeight(::sal_Int8 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Bool SAL_CALL getCharAutoKerning() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharAutoKerning(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Bool SAL_CALL getCharFlash() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharFlash(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getCharRelief() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharRelief(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::rtl::OUString SAL_CALL getCharFontName() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharFontName(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::rtl::OUString SAL_CALL getCharFontStyleName() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharFontStyleName(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getCharFontFamily() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharFontFamily(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getCharFontCharSet() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharFontCharSet(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getCharFontPitch() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharFontPitch(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int32 SAL_CALL getCharColor() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharColor(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int32 SAL_CALL getCharUnderlineColor() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharUnderlineColor(::sal_Int32 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual float SAL_CALL getCharHeight() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharHeight(float the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getCharUnderline() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharUnderline(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual float SAL_CALL getCharWeight() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharWeight(float the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::com::sun::star::awt::FontSlant SAL_CALL getCharPosture() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharPosture(::com::sun::star::awt::FontSlant the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getCharStrikeout() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharStrikeout(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Bool SAL_CALL getCharWordMode() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharWordMode(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getCharRotation() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharRotation(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getCharScaleWidth() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setCharScaleWidth(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::sal_Int16 SAL_CALL getParaVertAlignment() throw (::com::sun::star::uno::RuntimeException); \
    virtual void SAL_CALL setParaVertAlignment(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException);\
    virtual ::rtl::OUString SAL_CALL getHyperLinkURL() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setHyperLinkURL(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::rtl::OUString SAL_CALL getHyperLinkTarget() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setHyperLinkTarget(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::rtl::OUString SAL_CALL getHyperLinkName() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setHyperLinkName(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::rtl::OUString SAL_CALL getVisitedCharStyleName() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setVisitedCharStyleName(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual ::rtl::OUString SAL_CALL getUnvisitedCharStyleName() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException); \
    virtual void SAL_CALL setUnvisitedCharStyleName(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException);\
    virtual ::sal_Int16 SAL_CALL getCharKerning() throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException);\
    virtual void SAL_CALL setCharKerning(::sal_Int16 the_value) throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::beans::UnknownPropertyException);


#endif // INCLUDED_REPORTHELPERDEFINES_HXX

