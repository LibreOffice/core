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

#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAFONT_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAFONT_HXX

#include <vbahelper/vbafontbase.hxx>
#include <ooo/vba/word/XFont.hpp>
#include <cppuhelper/implbase.hxx>

typedef cppu::ImplInheritanceHelper< VbaFontBase, ov::word::XFont > SwVbaFont_BASE;

class SwVbaFont : public SwVbaFont_BASE
{
public:
    /// @throws css::uno::RuntimeException
    SwVbaFont( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xPalette, css::uno::Reference< css::beans::XPropertySet > const & xPropertySet );

    // Attributes
    virtual void SAL_CALL setColorIndex( const css::uno::Any& _colorindex ) override;
    virtual css::uno::Any SAL_CALL getColorIndex() override;
    virtual css::uno::Any SAL_CALL getUnderline() override;
    virtual void SAL_CALL setUnderline( const css::uno::Any& _underline ) override;
    virtual css::uno::Any SAL_CALL getSubscript() override;
    virtual css::uno::Any SAL_CALL getSuperscript() override;

    virtual css::uno::Any SAL_CALL getBold() override;
    virtual css::uno::Any SAL_CALL getItalic() override;
    virtual css::uno::Any SAL_CALL getStrikethrough() override;
    virtual css::uno::Any SAL_CALL getShadow() override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
