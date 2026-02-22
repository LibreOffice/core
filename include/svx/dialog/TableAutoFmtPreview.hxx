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

#include <com/sun/star/i18n/XBreakIterator.hdl>
#include <svl/numformat.hxx>
#include <svx/framelinkarray.hxx>
#include <svx/TableAutoFmt.hxx>
#include <vcl/weld/customweld.hxx>

class SVX_DLLPUBLIC SvxAutoFmtPreview : public weld::CustomWidgetController
{
public:
    SvxAutoFmtPreview(bool bRTL);

    void NotifyChange(const SvxAutoFormatData* rNewData);

private:
    static const tools::Long FRAME_OFFSET = 4;
    static const size_t GRID_SIZE = 5;

    const SvxAutoFormatData* mpCurrentData;
    svx::frame::Array maArray; /// Implementation to draw the frame borders.
    bool mbFitWidth;
    bool mbRTL;
    Size maPreviousSize;
    tools::Long mnLabelColWidth;
    tools::Long mnDataColWidth1;
    tools::Long mnDataColWidth2;
    tools::Long mnRowHeight;
    const OUString maStrJan;
    const OUString maStrFeb;
    const OUString maStrMar;
    const OUString maStrNorth;
    const OUString maStrMid;
    const OUString maStrSouth;
    const OUString maStrSum;
    std::unique_ptr<SvNumberFormatter> mxNumFormat;

    css::uno::Reference<css::i18n::XBreakIterator> mxBreakIter;

    void Init();
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void Resize() override;
    void CalcCellArray(bool bFitWidth);
    void CalcLineMap();
    void PaintCells(vcl::RenderContext& rRenderContext);

    sal_uInt8 GetFormatIndex(size_t nCol, size_t nRow);
    const SvxBoxItem& GetBoxItem(size_t nCol, size_t nRow) const;
    const SvxLineItem& GetDiagItem(size_t nCol, size_t nRow, bool bTLBR) const;

    void DrawString(vcl::RenderContext& rRenderContext, size_t nCol, size_t nRow);
    void DrawBackground(vcl::RenderContext& rRenderContext);

    void MakeFonts(vcl::RenderContext const& rRenderContext, sal_uInt8 nIndex, vcl::Font& rFont,
                   vcl::Font& rCJKFont, vcl::Font& rCTLFont);

    void setStyleFromBorderPriority(size_t nCol, size_t nRow);
    void setStyleFromBorder(sal_uInt8 nElement, size_t nCol, size_t nRow, bool reset);
    bool compareTextAttr(int nIndex);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
