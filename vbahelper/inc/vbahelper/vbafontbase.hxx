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
#ifndef VBAHELPER_FONTBASE_HXX
#define VBAHELPER_FONTBASE_HXX

#include <cppuhelper/implbase1.hxx>

#include <ooo/vba/XFontBase.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbahelperinterface.hxx>

// use local constants there is no need to expose these constants
// externally. Looking at the Format->Character dialog it seem that
// these may infact be even be calculated. Leave hardcoded for now
// #FIXEME #TBD investigate the code for dialog mentioned above

// The font baseline is not specified.
const short NORMAL = 0;

// specifies a superscripted.
const short SUPERSCRIPT = 33;

// specifies a subscripted.
const short SUBSCRIPT = -33;

// specifies a hight of superscripted font
 const sal_Int8 SUPERSCRIPTHEIGHT = 58;

// specifies a hight of subscripted font
const sal_Int8 SUBSCRIPTHEIGHT = 58;

// specifies a hight of normal font
const short NORMALHEIGHT = 100;

typedef  InheritedHelperInterfaceImpl1< ov::XFontBase > VbaFontBase_BASE;

class VBAHELPER_DLLPUBLIC VbaFontBase : public VbaFontBase_BASE
{
protected:
    css::uno::Reference< css::beans::XPropertySet > mxFont;
    css::uno::Reference< css::container::XIndexAccess > mxPalette;
    bool mbFormControl;

public:
    VbaFontBase(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::container::XIndexAccess >& xPalette,
        const css::uno::Reference< css::beans::XPropertySet >& xPropertySet,
        bool bFormControl = false ) throw ( css::uno::RuntimeException );
    virtual ~VbaFontBase();// {}

    // Attributes
    virtual css::uno::Any SAL_CALL getSize() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSize( const css::uno::Any& _size ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getColorIndex() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getBold() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setBold( const css::uno::Any& _bold ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getUnderline() throw (css::uno::RuntimeException) = 0;
    virtual void SAL_CALL setUnderline( const css::uno::Any& _underline ) throw (css::uno::RuntimeException) = 0;
    virtual css::uno::Any SAL_CALL getStrikethrough() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStrikethrough( const css::uno::Any& _strikethrough ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getShadow() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setShadow( const css::uno::Any& _shadow ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getItalic() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setItalic( const css::uno::Any& _italic ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getSubscript() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSubscript( const css::uno::Any& _subscript ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getSuperscript() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSuperscript( const css::uno::Any& _superscript ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getName() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setName( const css::uno::Any& _name ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL getColor() throw (css::uno::RuntimeException) ;
    virtual void SAL_CALL setColor( const css::uno::Any& _color ) throw (css::uno::RuntimeException) ;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
