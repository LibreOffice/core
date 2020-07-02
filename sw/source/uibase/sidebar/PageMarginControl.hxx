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
#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEMARGINCONTROL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGEMARGINCONTROL_HXX

#include <svtools/toolbarmenu.hxx>

#define SWPAGE_NARROW_VALUE    720
#define SWPAGE_NORMAL_VALUE    1136
#define SWPAGE_WIDE_VALUE1     1440
#define SWPAGE_WIDE_VALUE2     2880
#define SWPAGE_WIDE_VALUE3     1800

// #i19922# - tdf#126051 see cui/source/tabpages/page.cxx and svx/source/dialog/hdft.cxx
const long MINBODY = 56;  // 1mm in twips rounded

class PageMarginPopup;

namespace sw::sidebar {

class PageMarginControl final : public WeldToolbarPopup
{
public:
    explicit PageMarginControl(PageMarginPopup* pControl, weld::Widget* pParent);
    virtual void GrabFocus() override;
    virtual ~PageMarginControl() override;

private:
    std::unique_ptr<weld::Button> m_xNarrow;
    std::unique_ptr<weld::Button> m_xNormal;
    std::unique_ptr<weld::Button> m_xWide;
    std::unique_ptr<weld::Button> m_xMirrored;
    std::unique_ptr<weld::Button> m_xLast;

    std::unique_ptr<weld::Label> m_xLeft;
    std::unique_ptr<weld::Label> m_xRight;
    std::unique_ptr<weld::Label> m_xInner;
    std::unique_ptr<weld::Label> m_xOuter;

    std::unique_ptr<weld::MetricSpinButton> m_xLeftMarginEdit;
    std::unique_ptr<weld::MetricSpinButton> m_xRightMarginEdit;
    std::unique_ptr<weld::MetricSpinButton> m_xTopMarginEdit;
    std::unique_ptr<weld::MetricSpinButton> m_xBottomMarginEdit;

    // hidden metric field
    std::unique_ptr<weld::MetricSpinButton> m_xWidthHeightField;

    rtl::Reference<PageMarginPopup> m_xControl;

    long m_nPageLeftMargin;
    long m_nPageRightMargin;
    long m_nPageTopMargin;
    long m_nPageBottomMargin;
    bool m_bMirrored;

    const MapUnit m_eUnit;

    Size m_aPageSize;

    bool m_bUserCustomValuesAvailable;
    long m_nUserCustomPageLeftMargin;
    long m_nUserCustomPageRightMargin;
    long m_nUserCustomPageTopMargin;
    long m_nUserCustomPageBottomMargin;
    bool m_bUserCustomMirrored;

    bool m_bCustomValuesUsed;

    DECL_LINK( SelectMarginHdl, weld::Button&, void );
    DECL_LINK( ModifyLRMarginHdl, weld::MetricSpinButton&, void );
    DECL_LINK( ModifyULMarginHdl, weld::MetricSpinButton&, void );

    static void ExecuteMarginLRChange(
        const long nPageLeftMargin,
        const long nPageRightMargin );
    static void ExecuteMarginULChange(
        const long nPageTopMargin,
        const long nPageBottomMargin );
    static void ExecutePageLayoutChange( const bool bMirrored );

    void SetMetricFieldMaxValues(const Size& rPageSize);

    bool GetUserCustomValues();
    void StoreUserCustomValues();

    void FillHelpText( const bool bUserCustomValuesAvailable );
};

} // end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
