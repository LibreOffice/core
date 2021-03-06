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
#pragma once

#include <cppuhelper/implbase.hxx>

#include <ooo/vba/excel/XFont.hpp>
#include <vbahelper/vbafontbase.hxx>

namespace com::sun::star::beans
{
class XPropertySet;
}

class ScCellRangeObj;
class SfxItemSet;
class ScVbaPalette;

typedef cppu::ImplInheritanceHelper<VbaFontBase, ov::excel::XFont> ScVbaFont_BASE;

class ScVbaFont : public ScVbaFont_BASE
{
    ScCellRangeObj* mpRangeObj;
    SfxItemSet* GetDataSet();

public:
    /// @throws css::uno::RuntimeException
    ScVbaFont(const css::uno::Reference<ov::XHelperInterface>& xParent,
              const css::uno::Reference<css::uno::XComponentContext>& xContext,
              const ScVbaPalette& dPalette,
              const css::uno::Reference<css::beans::XPropertySet>& xPropertySet,
              ScCellRangeObj* pRangeObj = nullptr, bool bFormControl = false);
    virtual ~ScVbaFont() override; // {}

    // Attributes
    virtual css::uno::Any SAL_CALL getSize() override;
    virtual css::uno::Any SAL_CALL getStandardFontSize() override;
    virtual void SAL_CALL setStandardFontSize(const css::uno::Any& _standardfontsize) override;
    virtual css::uno::Any SAL_CALL getStandardFont() override;
    virtual void SAL_CALL setStandardFont(const css::uno::Any& _standardfont) override;
    virtual css::uno::Any SAL_CALL getFontStyle() override;
    virtual void SAL_CALL setFontStyle(const css::uno::Any& _fontstyle) override;
    virtual css::uno::Any SAL_CALL getColorIndex() override;
    virtual void SAL_CALL setColorIndex(const css::uno::Any& _colorindex) override;
    virtual css::uno::Any SAL_CALL getBold() override;
    virtual css::uno::Any SAL_CALL getUnderline() override;
    virtual void SAL_CALL setUnderline(const css::uno::Any& _underline) override;
    virtual css::uno::Any SAL_CALL getStrikethrough() override;
    virtual css::uno::Any SAL_CALL getShadow() override;
    virtual css::uno::Any SAL_CALL getItalic() override;
    virtual css::uno::Any SAL_CALL getName() override;
    virtual css::uno::Any SAL_CALL getColor() override;
    virtual css::uno::Any SAL_CALL getOutlineFont() override;
    virtual void SAL_CALL setOutlineFont(const css::uno::Any& _outlinefont) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
