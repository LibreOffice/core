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

#include <svtools/toolbarmenu.hxx>

class SvxLineSpacingItem;

namespace svx
{
class SvxLineSpacingToolBoxControl;

class ParaLineSpacingControl : public WeldToolbarPopup
{
public:
    explicit ParaLineSpacingControl(SvxLineSpacingToolBoxControl* pControl, weld::Widget* pParent);
    virtual ~ParaLineSpacingControl() override;

    /// Setup the widgets with values from the document.
    void Initialize();

    virtual void GrabFocus() override;

private:
    rtl::Reference<SvxLineSpacingToolBoxControl> mxControl;

    MapUnit meLNSpaceUnit;

    std::unique_ptr<weld::Button> mxSpacing1Button;
    std::unique_ptr<weld::Button> mxSpacing115Button;
    std::unique_ptr<weld::Button> mxSpacing15Button;
    std::unique_ptr<weld::Button> mxSpacing2Button;

    std::unique_ptr<weld::ComboBox> mxLineDist;

    std::unique_ptr<weld::Label> mxLineDistLabel;
    std::unique_ptr<weld::MetricSpinButton> mxLineDistAtPercentBox;
    std::unique_ptr<weld::MetricSpinButton> mxLineDistAtMetricBox;
    weld::MetricSpinButton* mpActLineDistFld;

private:
    /// Take the values from the widgets, and update the paragraph accordingly.
    void ExecuteLineSpace();

    /// Set one particular value.
    static void SetLineSpace(SvxLineSpacingItem& rLineSpace, sal_Int32 eSpace,
                             tools::Long lValue = 0);

    /// For the buttons - set the values, and close the popup.
    void ExecuteLineSpacing(sal_Int32 aEntry);

    /// Set mpActlineDistFld and visibility of mpLineDist* fields according to what is just selected.
    void UpdateMetricFields();

    /// Set the entry and update the metric fields.
    void SelectEntryPos(sal_Int32 nPos);

    DECL_LINK(LineSPDistHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(LineSPDistAtHdl_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(PredefinedValuesHandler, weld::Button&, void);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
