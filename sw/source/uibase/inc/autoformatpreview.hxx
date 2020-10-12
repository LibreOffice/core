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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_AUTOFORMATPREVIEW_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_AUTOFORMATPREVIEW_HXX

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <sal/types.h>
#include <svx/framelinkarray.hxx>
#include <svl/zforlist.hxx>
#include <tools/gen.hxx>
#include <rtl/ustring.hxx>
#include <vcl/customweld.hxx>
#include <vcl/font.hxx>

#include "wrtsh.hxx"
#include <tblafmt.hxx>

class AutoFormatPreview : public weld::CustomWidgetController
{
public:
    AutoFormatPreview();

    void NotifyChange(const SwTableAutoFormat& rNewData);

    void DetectRTL(SwWrtShell const* pWrtShell);

private:
    SwTableAutoFormat maCurrentData;
    svx::frame::Array maArray; /// Implementation to draw the frame borders.
    bool mbFitWidth;
    bool mbRTL;
    Size maPreviousSize;
    tools::Long mnLabelColumnWidth;
    tools::Long mnDataColumnWidth1;
    tools::Long mnDataColumnWidth2;
    tools::Long mnRowHeight;
    const OUString maStringJan;
    const OUString maStringFeb;
    const OUString maStringMar;
    const OUString maStringNorth;
    const OUString maStringMid;
    const OUString maStringSouth;
    const OUString maStringSum;
    std::unique_ptr<SvNumberFormatter> mxNumFormat;

    uno::Reference<i18n::XBreakIterator> m_xBreak;

    void Init();
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void Resize() override;
    void CalcCellArray(bool bFitWidth);
    void CalcLineMap();
    void PaintCells(vcl::RenderContext& rRenderContext);

    sal_uInt8 GetFormatIndex(size_t nCol, size_t nRow) const;

    void DrawString(vcl::RenderContext& rRenderContext, size_t nCol, size_t nRow);
    void DrawBackground(vcl::RenderContext& rRenderContext);

    void MakeFonts(vcl::RenderContext const& rRenderContext, sal_uInt8 nIndex, vcl::Font& rFont,
                   vcl::Font& rCJKFont, vcl::Font& rCTLFont);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
