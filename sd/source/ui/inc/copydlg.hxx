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

#include <sfx2/basedlgs.hxx>
#include <tools/fract.hxx>

class ColorListBox;

namespace sd
{
class View;

/**
 * dialog to adjust screen
 */
class CopyDlg : public SfxDialogController
{
public:
    CopyDlg(weld::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView);
    virtual ~CopyDlg() override;

    void GetAttr(SfxItemSet& rOutAttrs);
    void Reset();

private:
    const SfxItemSet& mrOutAttrs;
    Fraction maUIScale;
    ::sd::View* mpView;

    std::unique_ptr<weld::SpinButton> m_xNumFldCopies;
    std::unique_ptr<weld::Button> m_xBtnSetViewData;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldMoveX;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldMoveY;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldAngle;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldWidth;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldHeight;
    std::unique_ptr<weld::Label> m_xFtEndColor;
    std::unique_ptr<weld::Button> m_xBtnSetDefault;
    std::unique_ptr<ColorListBox> m_xLbStartColor;
    std::unique_ptr<ColorListBox> m_xLbEndColor;

    DECL_LINK(SelectColorHdl, ColorListBox&, void);
    DECL_LINK(SetViewData, weld::Button&, void);
    DECL_LINK(SetDefault, weld::Button&, void);
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
