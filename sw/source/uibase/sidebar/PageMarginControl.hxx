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

#include <tools/fldunit.hxx>
#include <svl/poolitem.hxx>
#include <svx/rulritem.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <svtools/unitconv.hxx>
#include <vector>
#include <vcl/layout.hxx>

#include <svx/tbxctl.hxx>

#define SWPAGE_NARROW_VALUE    720
#define SWPAGE_NORMAL_VALUE    1136
#define SWPAGE_WIDE_VALUE1     1440
#define SWPAGE_WIDE_VALUE2     2880
#define SWPAGE_WIDE_VALUE3     1800

static const long MINBODY = 284; //0.5 cm in twips

namespace sw { namespace sidebar {

class PageMarginControl : public SfxPopupWindow
{
public:
    explicit PageMarginControl( sal_uInt16 nId, vcl::Window* pParent );
    virtual ~PageMarginControl() override;
    virtual void dispose() override;

private:
    VclPtr<PushButton> m_pNarrow;
    VclPtr<PushButton> m_pNormal;
    VclPtr<PushButton> m_pWide;
    VclPtr<PushButton> m_pMirrored;
    VclPtr<PushButton> m_pLast;

    VclPtr<FixedText> m_pLeft;
    VclPtr<FixedText> m_pRight;
    VclPtr<FixedText> m_pInner;
    VclPtr<FixedText> m_pOuter;

    VclPtr<VclVBox> m_pContainer;

    VclPtr<MetricField> m_pLeftMarginEdit;
    VclPtr<MetricField> m_pRightMarginEdit;
    VclPtr<MetricField> m_pTopMarginEdit;
    VclPtr<MetricField> m_pBottomMarginEdit;

    // hidden metric field
    VclPtr<MetricField> m_pWidthHeightField;

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

    DECL_LINK( SelectMarginHdl, Button*, void );
    DECL_LINK( ModifyLRMarginHdl, Edit&, void );
    DECL_LINK( ModifyULMarginHdl, Edit&, void );

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

} } // end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
