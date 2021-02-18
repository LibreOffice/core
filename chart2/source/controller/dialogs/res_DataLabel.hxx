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

#include <svl/itemset.hxx>
#include <svx/dialcontrol.hxx>

#include <map>

class SvNumberFormatter;
namespace chart { class TextDirectionListBox; }

namespace chart
{

class DataLabelResources final
{
public:
    DataLabelResources(weld::Builder* pBuilder, weld::Window* pParent, const SfxItemSet& rInAttrs);
    ~DataLabelResources();

    void FillItemSet(SfxItemSet* rOutAttrs) const;
    void Reset(const SfxItemSet& rInAttrs);

    void SetNumberFormatter( SvNumberFormatter* pFormatter );

private:
    std::map< sal_Int32, sal_uInt16 > m_aPlacementToListBoxMap;
    std::map< sal_uInt16, sal_Int32 > m_aListBoxToPlacementMap;

    SvNumberFormatter*  m_pNumberFormatter;
    bool                m_bNumberFormatMixedState;
    bool                m_bPercentFormatMixedState;
    sal_uLong           m_nNumberFormatForValue;
    sal_uLong           m_nNumberFormatForPercent;

    bool                m_bSourceFormatMixedState;
    bool                m_bPercentSourceMixedState;
    bool                m_bSourceFormatForValue;
    bool                m_bSourceFormatForPercent;

    weld::Window*       m_pWindow;
    SfxItemPool*        m_pPool;

    std::unique_ptr<weld::CheckButton> m_xCBNumber;
    std::unique_ptr<weld::Button> m_xPB_NumberFormatForValue;
    std::unique_ptr<weld::CheckButton> m_xCBPercent;
    std::unique_ptr<weld::Button> m_xPB_NumberFormatForPercent;
    std::unique_ptr<weld::Label> m_xFT_NumberFormatForPercent;
    std::unique_ptr<weld::CheckButton> m_xCBCategory;
    std::unique_ptr<weld::CheckButton> m_xCBSymbol;
    std::unique_ptr<weld::CheckButton> m_xCBDataSeries;
    std::unique_ptr<weld::CheckButton> m_xCBWrapText;

    std::unique_ptr<weld::ComboBox> m_xLB_Separator;
    std::unique_ptr<weld::ComboBox> m_xLB_LabelPlacement;

    std::unique_ptr<weld::Widget> m_xBxOrientation;
    std::unique_ptr<weld::Label> m_xFT_Dial;
    std::unique_ptr<weld::MetricSpinButton> m_xNF_Degrees;

    std::unique_ptr<weld::Widget> m_xBxTextDirection;

    std::unique_ptr<TextDirectionListBox> m_xLB_TextDirection;
    std::unique_ptr<svx::DialControl> m_xDC_Dial;
    std::unique_ptr<weld::CustomWeld> m_xDC_DialWin;

    std::unique_ptr<weld::CheckButton> m_xCBCustomLeaderLines;

    DECL_LINK(NumberFormatDialogHdl, weld::Button&, void );
    DECL_LINK(CheckHdl, weld::ToggleButton&, void );
    void EnableControls();
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
