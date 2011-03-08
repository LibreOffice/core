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
#ifndef SC_VBA_FONT_HXX
#define SC_VBA_FONT_HXX

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XFont.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbafontbase.hxx>
#include "vbapalette.hxx"

class ScTableSheetsObj;
class ScCellRangeObj;

typedef cppu::ImplInheritanceHelper1< VbaFontBase, ov::excel::XFont > ScVbaFont_BASE;

class ScVbaFont : public ScVbaFont_BASE
{
    ScVbaPalette mPalette;
    ScCellRangeObj* mpRangeObj;
    SfxItemSet*  GetDataSet();
public:
    ScVbaFont(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const ScVbaPalette& dPalette,
        const css::uno::Reference< css::beans::XPropertySet >& xPropertySet,
        ScCellRangeObj* pRangeObj = 0, bool bFormControl = false ) throw ( css::uno::RuntimeException );
    virtual ~ScVbaFont();// {}

    // Attributes
    virtual css::uno::Any SAL_CALL getSize() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStandardFontSize() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStandardFontSize( const css::uno::Any& _standardfontsize ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStandardFont() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStandardFont( const css::uno::Any& _standardfont ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getFontStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFontStyle( const css::uno::Any& _fontstyle ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getColorIndex() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getBold() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getUnderline() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setUnderline( const css::uno::Any& _underline ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getStrikethrough() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getShadow() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getItalic() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getSubscript() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSubscript( const css::uno::Any& _subscript ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getSuperscript() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSuperscript( const css::uno::Any& _superscript ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getColor() throw (css::uno::RuntimeException) ;
    virtual css::uno::Any SAL_CALL getOutlineFont() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setOutlineFont( const css::uno::Any& _outlinefont ) throw (css::uno::RuntimeException) ;
    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

};



#endif /* SC_VBA_FONT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
