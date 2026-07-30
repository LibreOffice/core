/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XTitle.hpp>

#include <address.hxx>
#include "anyrefdg.hxx"
#include "viewdata.hxx"

class ScDocShell;

class ScPrincipalComponentAnalysisDialog : public ScAnyRefDlgController
{
public:
    ScPrincipalComponentAnalysisDialog(SfxBindings* pB, SfxChildWindow* pCW, weld::Window* pParent,
                                       ScViewData& rViewData);

    virtual ~ScPrincipalComponentAnalysisDialog() override;

    virtual void SetReference(const ScRange& rRef, ScDocument& rDoc) override;
    virtual void SetActive() override;
    virtual void Close() override;

private:
    ScViewData& mrViewData;
    ScDocument& mrDocument;

    ScRange maInputRange;
    ScAddress::Details maAddressDetails;

    // The part of the input range that holds numbers, so the header row is
    // left out when the labels box is ticked.
    ScRange maDataRange;
    bool mbWithLabels;

    // Which of the two charts the sheet is to carry.
    bool mbVarianceChart;
    bool mbCorrelationCircleChart;

    bool mbDialogLostFocus;

    std::unique_ptr<weld::Label> mxInputRangeLabel;
    std::unique_ptr<formula::RefEdit> mxInputRangeEdit;
    std::unique_ptr<formula::RefButton> mxInputRangeButton;
    std::unique_ptr<weld::CheckButton> mxWithLabelsCheckBox;
    std::unique_ptr<weld::CheckButton> mxVarianceChartCheckBox;
    std::unique_ptr<weld::CheckButton> mxCorrelationChartCheckBox;
    std::unique_ptr<weld::Label> mxErrorMessage;
    std::unique_ptr<weld::Button> mxButtonCalculate;
    std::unique_ptr<weld::Button> mxButtonCancel;

    void Init();
    void GetRangeFromSelection();
    void ValidateDialogInput();

    /** Fills maDataRange and mbWithLabels, and puts a message in the error
        label when the input cannot be analysed. */
    bool InputIsValid();
    OUString GetOutputSheetName() const;

    /** A name for the chart that no drawing object in the document holds yet. */
    OUString GetChartName(SCTAB nOutputTab) const;
    void Calculate();
    ScRange WriteOutput(ScDocShell& rDocShell, SCTAB nOutputTab);

    /** A chart title holding rText as its one run of text. */
    static css::uno::Reference<css::chart2::XTitle> MakeChartTitle(const OUString& rText);

    /** Puts a chart of rDataRange on the sheet that range is on, at rRectangle
        in hundredths of a millimetre, and hands back its chart document. The
        first cell of every column names the series it heads. */
    css::uno::Reference<css::chart2::XChartDocument>
    CreateSheetChart(ScDocShell& rDocShell, const ScRange& rDataRange,
                     const css::awt::Rectangle& rRectangle);

    /** Draws rShareRange, whose two columns are the share of the variance and
        the running total of it, as bars with a line over them. */
    void AddVarianceChart(ScDocShell& rDocShell, const ScRange& rShareRange, SCCOL nChartColumn);

    /** Draws rDataRange as a correlation circle, nTopOffset in hundredths of a
        millimetre below the top of the anchor cell. Its last two columns are
        the two components the features are placed against, and every column
        before them is a feature. */
    void AddCorrelationCircleChart(ScDocShell& rDocShell, const ScRange& rDataRange,
                                   SCCOL nChartColumn, sal_Int32 nTopOffset);

    DECL_LINK(ButtonClicked, weld::Button&, void);
    DECL_LINK(CheckBoxToggled, weld::Toggleable&, void);
    DECL_LINK(GetEditFocusHandler, formula::RefEdit&, void);
    DECL_LINK(GetButtonFocusHandler, formula::RefButton&, void);
    DECL_LINK(LoseEditFocusHandler, formula::RefEdit&, void);
    DECL_LINK(LoseButtonFocusHandler, formula::RefButton&, void);
    DECL_LINK(RefInputModifyHandler, formula::RefEdit&, void);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
