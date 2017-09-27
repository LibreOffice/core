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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARALINESPACINGCONTROL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARALINESPACINGCONTROL_HXX

#include <svtools/ctrlbox.hxx>
#include <sfx2/tbxctrl.hxx>

class SvxLineSpacingItem;
class Button;

namespace svx {

class ParaLineSpacingControl : public SfxPopupWindow
{
public:
    explicit ParaLineSpacingControl(sal_uInt16 nId, vcl::Window* pParent);
    virtual ~ParaLineSpacingControl() override;
    virtual void dispose() override;

private:
    MapUnit                        meLNSpaceUnit;

    VclPtr<Edit>                   mpActLineDistFld;

    VclPtr<PushButton>             mpSpacing1Button;
    VclPtr<PushButton>             mpSpacing115Button;
    VclPtr<PushButton>             mpSpacing15Button;
    VclPtr<PushButton>             mpSpacing2Button;

    VclPtr<ListBox>                mpLineDist;

    VclPtr<FixedText>              mpLineDistLabel;
    VclPtr<MetricField>            mpLineDistAtPercentBox;
    VclPtr<MetricField>            mpLineDistAtMetricBox;

private:
    /// Setup the widgets with values from the document.
    void Initialize();

    /// Take the values from the widgets, and update the paragraph accordingly.
    void ExecuteLineSpace();

    /// Set one particular value.
    static void SetLineSpace(SvxLineSpacingItem& rLineSpace, sal_Int32 eSpace, long lValue = 0);

    /// For the buttons - set the values, and close the popup.
    void ExecuteLineSpacing(sal_Int32 aEntry);

    /// Set mpActlineDistFld and visibility of mpLineDist* fields according to what is just selected.
    void UpdateMetricFields();

    /// Set the entry and update the metric fields.
    void SelectEntryPos(sal_Int32 nPos);

    DECL_LINK(LineSPDistHdl_Impl, ListBox&, void);
    DECL_LINK(LineSPDistAtHdl_Impl, Edit&, void);
    DECL_LINK(PredefinedValuesHandler, Button*, void);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
