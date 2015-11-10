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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAFONT_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAFONT_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <ooo/vba/excel/XFont.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbafontbase.hxx>
#include "vbapalette.hxx"

class ScCellRangeObj;

typedef cppu::ImplInheritanceHelper< VbaFontBase, ov::excel::XFont > ScVbaFont_BASE;

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
        ScCellRangeObj* pRangeObj = nullptr, bool bFormControl = false ) throw ( css::uno::RuntimeException );
    virtual ~ScVbaFont();// {}

    // Attributes
    virtual css::uno::Any SAL_CALL getSize() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getStandardFontSize() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStandardFontSize( const css::uno::Any& _standardfontsize ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getStandardFont() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStandardFont( const css::uno::Any& _standardfont ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getFontStyle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFontStyle( const css::uno::Any& _fontstyle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getColorIndex() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getBold() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getUnderline() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setUnderline( const css::uno::Any& _underline ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getStrikethrough() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getShadow() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getItalic() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getSubscript() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSubscript( const css::uno::Any& _subscript ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getSuperscript() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSuperscript( const css::uno::Any& _superscript ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getColor() throw (css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Any SAL_CALL getOutlineFont() throw (css::uno::RuntimeException, std::exception) override ;
    virtual void SAL_CALL setOutlineFont( const css::uno::Any& _outlinefont ) throw (css::uno::RuntimeException, std::exception) override ;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
