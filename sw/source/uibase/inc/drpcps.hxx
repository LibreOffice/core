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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DRPCPS_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DRPCPS_HXX

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <editeng/svxfont.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/customweld.hxx>
#include <vcl/print.hxx>

/// Dedicated drop caps dialog, opened by the .uno:FormatDropcap UNO command, which is not in the
/// default menus.
class SwDropCapsDlg final : public SfxSingleTabDialogController
{
public:
    SwDropCapsDlg(weld::Window *pParent, const SfxItemSet &rSet);
};

class SwDropCapsPage;

class SwDropCapsPict final : public weld::CustomWidgetController
{
    SwDropCapsPage* mpPage;
    OUString        maText;
    OUString        maScriptText;
    Color           maBackColor;
    Color           maTextLineColor;
    sal_uInt8       mnLines;
    tools::Long            mnTotLineH;
    tools::Long            mnLineH;
    tools::Long            mnTextH;
    sal_uInt16      mnDistance;
    VclPtr<Printer> mpPrinter;
    bool            mbDelPrinter;
    /// The ScriptInfo structure holds information on where we change from one
    /// script to another.
    struct ScriptInfo
    {
        sal_uLong  textWidth;   ///< Physical width of this segment.
        sal_uInt16 scriptType;  ///< Script type (e.g. Latin, Asian, Complex)
        sal_Int32 changePos;   ///< Character position where the script changes.
        ScriptInfo(sal_uInt16 scrptType, sal_Int32 position)
            : textWidth(0), scriptType(scrptType), changePos(position) {}
    };
    std::vector<ScriptInfo> maScriptChanges;
    SvxFont         maFont;
    SvxFont         maCJKFont;
    SvxFont         maCTLFont;
    Size            maTextSize;
    css::uno::Reference< css::i18n::XBreakIterator >   m_xBreak;

    virtual void    Paint(vcl::RenderContext& /*rRenderContext*/, const tools::Rectangle &rRect) override;
    void            CheckScript();
    Size            CalcTextSize();
    inline void     InitPrinter();
    void            InitPrinter_();
    static void     GetFontSettings( vcl::Font& _rFont, sal_uInt16 _nWhich );
    void            GetFirstScriptSegment(sal_Int32 &start, sal_Int32 &end, sal_uInt16 &scriptType);
    bool            GetNextScriptSegment(size_t &nIdx, sal_Int32 &start, sal_Int32 &end, sal_uInt16 &scriptType);

public:

    SwDropCapsPict()
        : mpPage(nullptr)
        , mnLines(0)
        , mnTotLineH(0)
        , mnLineH(0)
        , mnTextH(0)
        , mnDistance(0)
        , mpPrinter(nullptr)
        , mbDelPrinter(false)
    {}

    void SetDropCapsPage(SwDropCapsPage* pPage) { mpPage = pPage; }

    virtual ~SwDropCapsPict() override;

    void UpdatePaintSettings();       // also invalidates control!

    virtual void Resize() override;

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    void SetText( const OUString& rT );
    void SetLines( sal_uInt8 nL );
    void SetDistance( sal_uInt16 nD );
    void SetValues( const OUString& rText, sal_uInt8 nLines, sal_uInt16 nDistance );

    void DrawPrev(vcl::RenderContext& rRenderContext, const Point& rPt);
};

class SwDropCapsPage final : public SfxTabPage
{
friend class SwDropCapsPict;
    SwDropCapsPict  m_aPict;

    bool          m_bModified;
    bool          m_bFormat;
    bool          m_bHtmlMode;

    std::unique_ptr<weld::CheckButton> m_xDropCapsBox;
    std::unique_ptr<weld::CheckButton> m_xWholeWordCB;
    std::unique_ptr<weld::Label> m_xSwitchText;
    std::unique_ptr<weld::SpinButton> m_xDropCapsField;
    std::unique_ptr<weld::Label> m_xLinesText;
    std::unique_ptr<weld::SpinButton> m_xLinesField;
    std::unique_ptr<weld::Label> m_xDistanceText;
    std::unique_ptr<weld::MetricSpinButton> m_xDistanceField;
    std::unique_ptr<weld::Label> m_xTextText;
    std::unique_ptr<weld::Entry> m_xTextEdit;
    std::unique_ptr<weld::Label> m_xTemplateText;
    std::unique_ptr<weld::ComboBox> m_xTemplateBox;
    std::unique_ptr<weld::CustomWeld> m_xPict;

    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;
    void    FillSet( SfxItemSet &rSet );

    void ModifyEntry(const weld::Entry& rEdit);

    DECL_LINK(ClickHdl, weld::Toggleable&, void);
    DECL_LINK(MetricValueChangedHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ValueChangedHdl, weld::SpinButton&, void);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(WholeWordHdl, weld::Toggleable&, void);

    static const WhichRangesContainer s_aPageRg;

public:
    SwDropCapsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet);
    virtual ~SwDropCapsPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);
    static const WhichRangesContainer & GetRanges() { return s_aPageRg; }


    virtual bool FillItemSet(      SfxItemSet *rSet) override;
    virtual void Reset      (const SfxItemSet *rSet) override;

    void    SetFormat(bool bSet){m_bFormat = bSet;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
