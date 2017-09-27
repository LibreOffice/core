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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTCHARACTERSPACINGCONTROL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTCHARACTERSPACINGCONTROL_HXX

#include <sfx2/bindings.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <sfx2/tbxctrl.hxx>

namespace svx {
#define SPACING_NOCUSTOM                0
#define SPACING_CLOSE_BY_CLICK_ICON     -1
#define SPACING_CLOSE_BY_CUS_EDIT       1

#define SIDEBAR_SPACING_GLOBAL_VALUE   "PopupPanel_Spacing"

class TextCharacterSpacingControl : public SfxPopupWindow
{
public:
    explicit TextCharacterSpacingControl(sal_uInt16 nId, vcl::Window* pParent);
    virtual ~TextCharacterSpacingControl() override;
    virtual void dispose() override;

    long  GetLastCustomValue() { return mnCustomKern;}

private:
    VclPtr<MetricField> maEditKerning;

    VclPtr<PushButton> maNormal;
    VclPtr<PushButton> maVeryTight;
    VclPtr<PushButton> maTight;
    VclPtr<PushButton> maVeryLoose;
    VclPtr<PushButton> maLoose;
    VclPtr<PushButton> maLastCustom;

    sal_uInt16          mnId;
    long                mnCustomKern;
    short               mnLastCus;

    void Initialize();
    void ExecuteCharacterSpacing(long nValue, bool bClose = true);

    DECL_LINK(PredefinedValuesHdl, Button*, void);
    DECL_LINK(KerningSelectHdl, ListBox&, void);
    DECL_LINK(KerningModifyHdl, Edit&, void);

    MapUnit GetCoreMetric() const;
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
