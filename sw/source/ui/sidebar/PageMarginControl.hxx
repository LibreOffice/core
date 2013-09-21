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
#ifndef _SW_SIDEBAR_PAGE_MARGIN_CONTROL_HXX_
#define _SW_SIDEBAR_PAGE_MARGIN_CONTROL_HXX_

#include <svx/sidebar/PopupControl.hxx>

#include <tools/fldunit.hxx>
#include <svl/poolitem.hxx>
#include <svx/rulritem.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <svtools/unitconv.hxx>
#include <vector>

#define SWPAGE_NARROW_VALUE    720
#define SWPAGE_NORMAL_VALUE    1136
#define SWPAGE_WIDE_VALUE1     1440
#define SWPAGE_WIDE_VALUE2     2880
#define SWPAGE_WIDE_VALUE3     1800


namespace svx { namespace sidebar {
    class ValueSetWithTextControl;
} }

static const long MINBODY = 284; //0.5 cm in twips

namespace sw { namespace sidebar {

class PagePropertyPanel;


class PageMarginControl
    : public ::svx::sidebar::PopupControl
{
public:
    PageMarginControl(
        Window* pParent,
        PagePropertyPanel& rPanel,
        const SvxLongLRSpaceItem& aPageLRMargin,
        const SvxLongULSpaceItem& aPageULMargin,
        const bool bMirrored,
        const Size aPageSize,
        const sal_Bool bLandscape,
        const FieldUnit eFUnit,
        const SfxMapUnit eUnit );
    ~PageMarginControl(void);

private:
    ::svx::sidebar::ValueSetWithTextControl* mpMarginValueSet;

    FixedText maCustom;
    FixedText maLeft;
    FixedText maInner;
    MetricField maLeftMarginEdit;
    FixedText maRight;
    FixedText maOuter;
    MetricField maRightMarginEdit;
    FixedText maTop;
    MetricField maTopMarginEdit;
    FixedText maBottom;
    MetricField maBottomMarginEdit;

    // hidden metric field
    MetricField maWidthHeightField;

    long mnPageLeftMargin;
    long mnPageRightMargin;
    long mnPageTopMargin;
    long mnPageBottomMargin;
    bool mbMirrored;

    const SfxMapUnit meUnit;

    bool mbUserCustomValuesAvailable;
    long mnUserCustomPageLeftMargin;
    long mnUserCustomPageRightMargin;
    long mnUserCustomPageTopMargin;
    long mnUserCustomPageBottomMargin;
    bool mbUserCustomMirrored;

    bool mbCustomValuesUsed;

    PagePropertyPanel& mrPagePropPanel;

    DECL_LINK( ImplMarginHdl, void* );
    DECL_LINK( ModifyLRMarginHdl, MetricField* );
    DECL_LINK( ModifyULMarginHdl, MetricField* );

    void SetMetricFieldMaxValues( const Size aPageSize );

    bool GetUserCustomValues();
    void StoreUserCustomValues();

    void FillValueSet(
        const bool bLandscape,
        const bool bUserCustomValuesAvailable );
    void SelectValueSetItem();
};

} } // end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
