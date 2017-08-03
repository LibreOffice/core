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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_TAUTOFMT_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_TAUTOFMT_HXX

#include <sfx2/basedlgs.hxx>
#include <svx/framelinkarray.hxx>
#include <vcl/hackery.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/virdev.hxx>
#include "tblafmt.hxx"

class SwTableAutoFormat;
class SwTableAutoFormatTable;
class SwWrtShell;

class AutoFormatPreview
{
public:
    AutoFormatPreview(Hackery::DrawingArea* pDrawingArea);
    ~AutoFormatPreview();

    void NotifyChange( const SwTableAutoFormat& rNewData );

    void DetectRTL(SwWrtShell const * pWrtShell);

private:
    std::unique_ptr<Hackery::DrawingArea> mxDrawingArea;
    SwTableAutoFormat          aCurData;
    svx::frame::Array       maArray;            /// Implementation to draw the frame borders.
    bool                    bFitWidth;
    bool                    mbRTL;
    Size                    aPrvSize;
    long                    nLabelColWidth;
    long                    nDataColWidth1;
    long                    nDataColWidth2;
    long                    nRowHeight;
    const OUString          aStrJan;
    const OUString          aStrFeb;
    const OUString          aStrMar;
    const OUString          aStrNorth;
    const OUString          aStrMid;
    const OUString          aStrSouth;
    const OUString          aStrSum;
    SvNumberFormatter*      pNumFormat;

    uno::Reference<i18n::XBreakIterator> m_xBreak;

    void    Init();
    DECL_LINK(DoPaint, VirtualDevice&, void);
    DECL_LINK(DoResize, const Size& rSize, void);
    void    CalcCellArray(bool bFitWidth);
    void    CalcLineMap();
    void    PaintCells(vcl::RenderContext& rRenderContext);

    sal_uInt8           GetFormatIndex( size_t nCol, size_t nRow ) const;

    void DrawString(vcl::RenderContext& rRenderContext, size_t nCol, size_t nRow);
    void DrawBackground(vcl::RenderContext& rRenderContext);

    void MakeFonts(vcl::RenderContext& rRenderContext, sal_uInt8 nIndex, vcl::Font& rFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont);
};

enum AutoFormatLine { TOP_LINE, BOTTOM_LINE, LEFT_LINE, RIGHT_LINE };

class SwAutoFormatDlg
{
    std::unique_ptr<Hackery::Builder> m_xBuilder;
    std::unique_ptr<Hackery::Dialog> m_xDialog;
    std::unique_ptr<Hackery::TreeView> m_xLbFormat;
    std::unique_ptr<Hackery::CheckButton> m_xBtnNumFormat;
    std::unique_ptr<Hackery::CheckButton> m_xBtnBorder;
    std::unique_ptr<Hackery::CheckButton> m_xBtnFont;
    std::unique_ptr<Hackery::CheckButton> m_xBtnPattern;
    std::unique_ptr<Hackery::CheckButton> m_xBtnAlignment;
    std::unique_ptr<Hackery::Button> m_xBtnOk;
    std::unique_ptr<Hackery::Button> m_xBtnCancel;
    std::unique_ptr<Hackery::Button> m_xBtnAdd;
    std::unique_ptr<Hackery::Button> m_xBtnRemove;
    std::unique_ptr<Hackery::Button> m_xBtnRename;
    AutoFormatPreview m_aWndPreview;
    OUString        aStrTitle;
    OUString        aStrLabel;
    OUString        aStrClose;
    OUString        aStrDelTitle;
    OUString        aStrDelMsg;
    OUString        aStrRenameTitle;
    OUString        aStrInvalidFormat;

    SwWrtShell*             pShell;
    SwTableAutoFormatTable*      pTableTable;
    sal_uInt8                   nIndex;
    sal_uInt8                   nDfltStylePos;
    bool                    bCoreDataChanged : 1;
    bool                    bSetAutoFormat : 1;

    void Init( const SwTableAutoFormat* pSelFormat );
    void UpdateChecks( const SwTableAutoFormat&, bool bEnableBtn );

    DECL_LINK(CheckHdl, Hackery::CheckButton&, void);
    DECL_LINK(OkHdl, Hackery::Button&, void);
    DECL_LINK(AddHdl, Hackery::Button&, void);
    DECL_LINK(RemoveHdl, Hackery::Button&, void);
    DECL_LINK(RenameHdl, Hackery::Button&, void);
    DECL_LINK(SelFormatHdl, Hackery::TreeView&, void);

public:
    SwAutoFormatDlg(Hackery::Window* pParent, SwWrtShell* pShell,
                    bool bSetAutoFormat,
                    const SwTableAutoFormat* pSelFormat);
    short Execute() { return m_xDialog->run(); }
    ~SwAutoFormatDlg();

    SwTableAutoFormat* FillAutoFormatOfIndex() const;
};

#endif // SW_AUTOFMT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
