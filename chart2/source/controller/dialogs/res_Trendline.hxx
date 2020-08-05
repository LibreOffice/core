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

#include <tools/link.hxx>
#include <svl/itemset.hxx>
#include <svx/chrtitem.hxx>

namespace weld { class Builder; }
namespace weld { class CheckButton; }
namespace weld { class Entry; }
namespace weld { class FormattedSpinButton; }
namespace weld { class Image; }
namespace weld { class RadioButton; }
namespace weld { class SpinButton; }
namespace weld { class ToggleButton; }

class SvNumberFormatter;

namespace chart
{

class TrendlineResources final
{
public:
    TrendlineResources(weld::Builder& rParent, const SfxItemSet& rInAttrs);
    ~TrendlineResources();

    void Reset(const SfxItemSet& rInAttrs);
    void FillItemSet(SfxItemSet* rOutAttrs) const;

    void FillValueSets();

    void SetNumFormatter( SvNumberFormatter* pFormatter );
    void SetNbPoints( sal_Int32 nNbPoints );

private:
    SvxChartRegress     m_eTrendLineType;

    bool                m_bTrendLineUnique;

    SvNumberFormatter*  m_pNumFormatter;
    sal_Int32           m_nNbPoints;

    std::unique_ptr<weld::RadioButton> m_xRB_Linear;
    std::unique_ptr<weld::RadioButton> m_xRB_Logarithmic;
    std::unique_ptr<weld::RadioButton> m_xRB_Exponential;
    std::unique_ptr<weld::RadioButton> m_xRB_Power;
    std::unique_ptr<weld::RadioButton> m_xRB_Polynomial;
    std::unique_ptr<weld::RadioButton> m_xRB_MovingAverage;

    std::unique_ptr<weld::Image> m_xFI_Linear;
    std::unique_ptr<weld::Image> m_xFI_Logarithmic;
    std::unique_ptr<weld::Image> m_xFI_Exponential;
    std::unique_ptr<weld::Image> m_xFI_Power;
    std::unique_ptr<weld::Image> m_xFI_Polynomial;
    std::unique_ptr<weld::Image> m_xFI_MovingAverage;

    std::unique_ptr<weld::SpinButton> m_xNF_Degree;
    std::unique_ptr<weld::SpinButton> m_xNF_Period;
    std::unique_ptr<weld::Entry> m_xEE_Name;
    std::unique_ptr<weld::FormattedSpinButton> m_xFmtFld_ExtrapolateForward;
    std::unique_ptr<weld::FormattedSpinButton> m_xFmtFld_ExtrapolateBackward;
    std::unique_ptr<weld::CheckButton> m_xCB_SetIntercept;
    std::unique_ptr<weld::FormattedSpinButton> m_xFmtFld_InterceptValue;
    std::unique_ptr<weld::CheckButton> m_xCB_ShowEquation;
    std::unique_ptr<weld::Entry> m_xEE_XName;
    std::unique_ptr<weld::Entry> m_xEE_YName;
    std::unique_ptr<weld::CheckButton> m_xCB_ShowCorrelationCoeff;

    void UpdateControlStates();
    DECL_LINK(SelectTrendLine, weld::ToggleButton&, void);
    DECL_LINK(ChangeSpinValue, weld::SpinButton&, void);
    DECL_LINK(ChangeFormattedValue, weld::FormattedSpinButton&, void);
    DECL_LINK(ShowEquation, weld::ToggleButton&, void);
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
