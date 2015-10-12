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
#ifndef INCLUDED_VBAHELPER_VBAFONTBASE_HXX
#define INCLUDED_VBAHELPER_VBAFONTBASE_HXX

#include <ooo/vba/XFontBase.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef  InheritedHelperInterfaceWeakImpl< ov::XFontBase > VbaFontBase_BASE;

class VBAHELPER_DLLPUBLIC VbaFontBase : public VbaFontBase_BASE
{
protected:
    css::uno::Reference< css::beans::XPropertySet > mxFont;
    css::uno::Reference< css::container::XIndexAccess > mxPalette;
    bool mbFormControl;

public:
    // use local constants there is no need to expose these constants
    // externally. Looking at the Format->Character dialog it seem that
    // these may in fact even be calculated. Leave hardcoded for now
    // #FIXEME #TBD investigate the code for dialog mentioned above

    // The font baseline is not specified.
    static const short NORMAL = 0;

    // specifies a superscripted.
    static const short SUPERSCRIPT = 33;

    // specifies a subscripted.
    static const short SUBSCRIPT = -33;

    // specifies a hight of superscripted font
    static const sal_Int8 SUPERSCRIPTHEIGHT = 58;

    // specifies a hight of subscripted font
    static const sal_Int8 SUBSCRIPTHEIGHT = 58;

    // specifies a hight of normal font
    static const short NORMALHEIGHT = 100;

    VbaFontBase(
        const css::uno::Reference< ov::XHelperInterface >& xParent,
        const css::uno::Reference< css::uno::XComponentContext >& xContext,
        const css::uno::Reference< css::container::XIndexAccess >& xPalette,
        const css::uno::Reference< css::beans::XPropertySet >& xPropertySet,
        bool bFormControl = false ) throw ( css::uno::RuntimeException );
    virtual ~VbaFontBase();// {}

    // Attributes
    virtual css::uno::Any SAL_CALL getSize() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSize( const css::uno::Any& _size ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getColorIndex() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getBold() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setBold( const css::uno::Any& _bold ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getUnderline() throw (css::uno::RuntimeException, std::exception) override = 0;
    virtual void SAL_CALL setUnderline( const css::uno::Any& _underline ) throw (css::uno::RuntimeException, std::exception) override = 0;
    virtual css::uno::Any SAL_CALL getStrikethrough() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStrikethrough( const css::uno::Any& _strikethrough ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getShadow() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setShadow( const css::uno::Any& _shadow ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getItalic() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setItalic( const css::uno::Any& _italic ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getSubscript() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSubscript( const css::uno::Any& _subscript ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getSuperscript() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSuperscript( const css::uno::Any& _superscript ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setName( const css::uno::Any& _name ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getColor() throw (css::uno::RuntimeException, std::exception) override ;
    virtual void SAL_CALL setColor( const css::uno::Any& _color ) throw (css::uno::RuntimeException, std::exception) override ;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
