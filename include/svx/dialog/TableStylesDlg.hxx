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

#include <svtools/ctrlbox.hxx>
#include <svx/colorbox.hxx>
#include <vcl/weld/DialogController.hxx>
#include <vcl/weld/MetricSpinButton.hxx>
#include <vcl/weld/TreeView.hxx>
#include <svx/dialog/TableAutoFmtPreview.hxx>

class SVX_DLLPUBLIC SvxTableStylesDlg : public weld::GenericDialogController
{
private:
    enum BorderWidth
    {
        HAIRLINE = 1,
        VERY_THIN = 10,
        THIN = 15,
        MEDIUM = 30,
        THICK = 45,
        EXTRA_THICK = 90
    };

    enum BorderLayout
    {
        TABLE_NO_BORDERS = 1,
        TABLE_LEFT_BORDER,
        TABLE_RIGHT_BORDER,
        TABLE_LEFT_RIGHT_BORDERS,
        TABLE_TOP_BORDER,
        TABLE_BOTTOM_BORDER,
        TABLE_TOP_BOTTOM_BORDERS,
        TABLE_ALL_BORDERS
    };

    SvxAutoFmtPreview maWndPreview;
    SvxAutoFormat& mpFormat;
    SvxAutoFormatData& mpOriginalData;
    SvxAutoFormatData& mpData;
    SvxAutoFormatDataField* mpCurrField;
    OUString msParentName;
    bool mbNewStyle;
    int mnField;

    std::unique_ptr<weld::Toolbar> mpTextStyle;
    std::unique_ptr<weld::Toolbar> mpNumberFormat;
    std::unique_ptr<weld::Toolbar> mpHorAlign;
    std::unique_ptr<weld::Toolbar> mpVerAlign;
    std::unique_ptr<weld::MetricSpinButton> m_xCellPadding;
    std::unique_ptr<weld::ComboBox> m_xLinkedWith;
    std::unique_ptr<weld::ComboBox> m_xBorderWidth;
    std::unique_ptr<weld::Container> m_xNumberFormat;
    std::unique_ptr<weld::ComboBox> m_xTextFont;
    std::unique_ptr<FontSizeBox> m_xTextSize;
    std::unique_ptr<ColorListBox> m_xCellColor;
    std::unique_ptr<ColorListBox> m_xBorderColor;
    std::unique_ptr<ColorListBox> m_xTextColor;
    std::unique_ptr<SvtLineListBox> m_xBorderStyle;
    std::unique_ptr<ValueSet> m_xBorderSelector;
    std::unique_ptr<weld::CustomWeld> m_xBorderSelectorWin;
    std::unique_ptr<weld::CustomWeld> mxWndPreview;
    std::unique_ptr<weld::ComboBox> m_xElementBox;
    std::unique_ptr<weld::Entry> m_xNameEntry;
    std::unique_ptr<weld::Button> m_xOkayBtn;
    std::unique_ptr<weld::Button> m_xResetBtn;
    std::unique_ptr<weld::Button> m_xCancelBtn;

    void InitLineStyles();
    void InitBorderSelector();

    DECL_LINK(ElementSelHdl, weld::ComboBox&, void);
    DECL_LINK(LinkedWithSelHdl, weld::ComboBox&, void);
    DECL_LINK(BorderWidthSelHdl, weld::ComboBox&, void);
    DECL_LINK(TextFontSelHdl, weld::ComboBox&, void);
    DECL_LINK(TextSizeSelHdl, weld::ComboBox&, void);
    DECL_LINK(CellColorSelHdl, ColorListBox&, void);
    DECL_LINK(BorderColorSelHdl, ColorListBox&, void);
    DECL_LINK(TextColorSelHdl, ColorListBox&, void);
    DECL_LINK(TextStyleHdl, const OUString&, void);
    DECL_LINK(HorAlignHdl, const OUString&, void);
    DECL_LINK(VerAlignHdl, const OUString&, void);
    DECL_LINK(BorderLayoutSelHdl, ValueSet*, void);
    DECL_LINK(BorderStyleHdl, SvtLineListBox&, void);
    DECL_LINK(PaddingHdl, weld::MetricSpinButton&, void);
    DECL_LINK(OkayHdl, weld::Button&, void);
    DECL_LINK(ResetHdl, weld::Button&, void);
    DECL_LINK(CancelHdl, weld::Button&, void);

    void UpdateWidth(tools::Long nWidth);
    SvxBoxItem GetValidBox();
    OUString GetValidFont(size_t nField);

public:
    SvxTableStylesDlg(weld::Window* pWindow, bool bNewStyle, SvxAutoFormat& pFormat,
                      SvxAutoFormatData& pData, bool bRTL);
    virtual ~SvxTableStylesDlg() override;
    SvxAutoFormatData& GetFormat() { return mpData; };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
