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

#include <svtools/toolbarmenu.hxx>

namespace svx {
#define SPACING_NOCUSTOM                0
#define SPACING_CLOSE_BY_CLICK_ICON     -1
#define SPACING_CLOSE_BY_CUS_EDIT       1

#define SIDEBAR_SPACING_GLOBAL_VALUE   "PopupPanel_Spacing"

class TextCharacterSpacingPopup;

class TextCharacterSpacingControl final : public WeldToolbarPopup
{
public:
    explicit TextCharacterSpacingControl(TextCharacterSpacingPopup* pControl, weld::Widget* pParent);

    virtual void GrabFocus() override;

    virtual ~TextCharacterSpacingControl() override;

private:
    tools::Long                mnCustomKern;
    short               mnLastCus;

    std::unique_ptr<weld::MetricSpinButton> mxEditKerning;
    std::unique_ptr<weld::Button> mxTight;
    std::unique_ptr<weld::Button> mxVeryTight;
    std::unique_ptr<weld::Button> mxNormal;
    std::unique_ptr<weld::Button> mxLoose;
    std::unique_ptr<weld::Button> mxVeryLoose;
    std::unique_ptr<weld::Button> mxLastCustom;

    rtl::Reference<TextCharacterSpacingPopup> mxControl;

    void Initialize();
    void ExecuteCharacterSpacing(tools::Long nValue, bool bClose = true);

    DECL_LINK(PredefinedValuesHdl, weld::Button&, void);
    DECL_LINK(KerningModifyHdl, weld::MetricSpinButton&, void);

    static MapUnit GetCoreMetric();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
