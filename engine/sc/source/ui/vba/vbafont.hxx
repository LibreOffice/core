/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    /// @throws cpo::uno::RuntimeException
    ScVbaFont(const cpo::uno::Reference<ov::XHelperInterface>& xParent,
              const cpo::uno::Reference<cpo::uno::XComponentContext>& xContext,
              const ScVbaPalette& dPalette,
              const cpo::uno::Reference<css::beans::XPropertySet>& xPropertySet,
              ScCellRangeObj* pRangeObj = nullptr, bool bFormControl = false);
    virtual ~ScVbaFont() override; // {}

    // Attributes
    virtual cpo::uno::Any getSize() override;
    virtual cpo::uno::Any getStandardFontSize() override;
    virtual void setStandardFontSize(const cpo::uno::Any& _standardfontsize) override;
    virtual cpo::uno::Any getStandardFont() override;
    virtual void setStandardFont(const cpo::uno::Any& _standardfont) override;
    virtual cpo::uno::Any getFontStyle() override;
    virtual void setFontStyle(const cpo::uno::Any& _fontstyle) override;
    virtual cpo::uno::Any getColorIndex() override;
    virtual void setColorIndex(const cpo::uno::Any& _colorindex) override;
    virtual cpo::uno::Any getBold() override;
    virtual cpo::uno::Any getUnderline() override;
    virtual void setUnderline(const cpo::uno::Any& _underline) override;
    virtual cpo::uno::Any getStrikethrough() override;
    virtual cpo::uno::Any getShadow() override;
    virtual cpo::uno::Any getItalic() override;
    virtual cpo::uno::Any getName() override;
    virtual cpo::uno::Any getColor() override;
    virtual cpo::uno::Any getOutlineFont() override;
    virtual void setOutlineFont(const cpo::uno::Any& _outlinefont) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
