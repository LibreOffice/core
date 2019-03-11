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

#ifndef INCLUDED_SC_SOURCE_UI_INC_AUTOFMT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_AUTOFMT_HXX

#include <svx/framelinkarray.hxx>
#include <scdllapi.h>
#include <vcl/customweld.hxx>

namespace com { namespace sun { namespace star { namespace i18n { class XBreakIterator; } } } }

class ScAutoFormatData;
class SvxBoxItem;
class SvxLineItem;
class SvNumberFormatter;
class VirtualDevice;
class ScViewData;

class SC_DLLPUBLIC ScAutoFmtPreview : public weld::CustomWidgetController
{
public:
    ScAutoFmtPreview();
    void DetectRTL(const ScViewData *pViewData);
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual ~ScAutoFmtPreview() override;

    void NotifyChange( ScAutoFormatData* pNewData );

protected:
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void Resize() override;

private:
    ScAutoFormatData* pCurData;
    ScopedVclPtrInstance<VirtualDevice> aVD;
    css::uno::Reference<css::i18n::XBreakIterator> xBreakIter;
    bool                    bFitWidth;
    svx::frame::Array       maArray;            /// Implementation to draw the frame borders.
    bool                    mbRTL;
    Size                    aPrvSize;
    long                    mnLabelColWidth;
    long                    mnDataColWidth1;
    long                    mnDataColWidth2;
    long                    mnRowHeight;
    const OUString          aStrJan;
    const OUString          aStrFeb;
    const OUString          aStrMar;
    const OUString          aStrNorth;
    const OUString          aStrMid;
    const OUString          aStrSouth;
    const OUString          aStrSum;
    std::unique_ptr<SvNumberFormatter> pNumFmt;

    SAL_DLLPRIVATE void Init();
    SAL_DLLPRIVATE void DoPaint(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE void CalcCellArray(bool bFitWidth);
    SAL_DLLPRIVATE void CalcLineMap();
    SAL_DLLPRIVATE void PaintCells(vcl::RenderContext& rRenderContext);

/*  Usage of type size_t instead of SCCOL/SCROW is correct here - used in
    conjunction with class svx::frame::Array (svx/framelinkarray.hxx), which
    expects size_t coordinates. */

    SAL_DLLPRIVATE sal_uInt16 GetFormatIndex( size_t nCol, size_t nRow ) const;
    SAL_DLLPRIVATE const SvxBoxItem& GetBoxItem( size_t nCol, size_t nRow ) const;
    SAL_DLLPRIVATE const SvxLineItem& GetDiagItem( size_t nCol, size_t nRow, bool bTLBR ) const;

    SAL_DLLPRIVATE void DrawString(vcl::RenderContext& rRenderContext, size_t nCol, size_t nRow);
    SAL_DLLPRIVATE void DrawBackground(vcl::RenderContext& rRenderContext);

    SAL_DLLPRIVATE void MakeFonts(vcl::RenderContext const& rRenderContext, sal_uInt16 nIndex,
                                  vcl::Font& rFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont);
};

#endif // INCLUDED_SC_SOURCE_UI_INC_AUTOFMT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
