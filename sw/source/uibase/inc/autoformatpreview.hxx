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
#include <tools/link.hxx>
#include <rtl/ustring.hxx>
#include <vcl/outdev.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <vcl/font.hxx>

#include "wrtsh.hxx"
#include <tblafmt.hxx>

class AutoFormatPreview
{
public:
    AutoFormatPreview(weld::DrawingArea* pDrawingArea);

    void NotifyChange(const SwTableAutoFormat& rNewData);

    void DetectRTL(SwWrtShell const* pWrtShell);

private:
    std::unique_ptr<weld::DrawingArea> mxDrawingArea;
    SwTableAutoFormat aCurData;
    svx::frame::Array maArray; /// Implementation to draw the frame borders.
    bool bFitWidth;
    bool mbRTL;
    Size aPrvSize;
    long nLabelColWidth;
    long nDataColWidth1;
    long nDataColWidth2;
    long nRowHeight;
    const OUString aStrJan;
    const OUString aStrFeb;
    const OUString aStrMar;
    const OUString aStrNorth;
    const OUString aStrMid;
    const OUString aStrSouth;
    const OUString aStrSum;
    std::unique_ptr<SvNumberFormatter> mxNumFormat;

    uno::Reference<i18n::XBreakIterator> m_xBreak;

    void Init();
    DECL_LINK(DoPaint, vcl::RenderContext&, void);
    DECL_LINK(DoResize, const Size& rSize, void);
    void CalcCellArray(bool bFitWidth);
    void CalcLineMap();
    void PaintCells(vcl::RenderContext& rRenderContext);

    sal_uInt8 GetFormatIndex(size_t nCol, size_t nRow) const;

    void DrawString(vcl::RenderContext& rRenderContext, size_t nCol, size_t nRow);
    void DrawBackground(vcl::RenderContext& rRenderContext);

    void MakeFonts(vcl::RenderContext& rRenderContext, sal_uInt8 nIndex, vcl::Font& rFont,
                   vcl::Font& rCJKFont, vcl::Font& rCTLFont);
};

class AutoFormatPreviewWindow : public vcl::Window
{
public:
    AutoFormatPreviewWindow(vcl::Window* pParent, WinBits nStyle);
    virtual ~AutoFormatPreviewWindow() override;
    virtual void dispose() override;

    void NotifyChange(const SwTableAutoFormat& rNewData);

    void DetectRTL(SwWrtShell const* pWrtShell);

    virtual void Resize() override;

protected:
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

private:
    SwTableAutoFormat aCurData;
    ScopedVclPtr<VirtualDevice> aVD;
    svx::frame::Array maArray; /// Implementation to draw the frame borders.
    bool bFitWidth;
    bool mbRTL;
    Size aPrvSize;
    long nLabelColWidth;
    long nDataColWidth1;
    long nDataColWidth2;
    long nRowHeight;
    const OUString aStrJan;
    const OUString aStrFeb;
    const OUString aStrMar;
    const OUString aStrNorth;
    const OUString aStrMid;
    const OUString aStrSouth;
    const OUString aStrSum;
    SvNumberFormatter* pNumFormat;

    uno::Reference<i18n::XBreakIterator> m_xBreak;

    void Init();
    void DoPaint(vcl::RenderContext& rRenderContext);
    void CalcCellArray(bool bFitWidth);
    void CalcLineMap();
    void PaintCells(vcl::RenderContext& rRenderContext);

    sal_uInt8 GetFormatIndex(size_t nCol, size_t nRow) const;

    void DrawString(vcl::RenderContext& rRenderContext, size_t nCol, size_t nRow);
    void DrawBackground(vcl::RenderContext& rRenderContext);

    void MakeFonts(sal_uInt8 nIndex, vcl::Font& rFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
