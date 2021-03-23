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

#include <tools/fract.hxx>
#include <vcl/weld.hxx>

/************************************************************************/

class SfxItemSet;
namespace sd {
    class View;
}

/**
 * dialog to adjust snap- lines and points
 */
class SdSnapLineDlg : public weld::GenericDialogController
{
private:
    int                 nXValue;
    int                 nYValue;
    Fraction            aUIScale;

    std::unique_ptr<weld::Label> m_xFtX;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldX;
    std::unique_ptr<weld::Label> m_xFtY;
    std::unique_ptr<weld::MetricSpinButton> m_xMtrFldY;
    std::unique_ptr<weld::Widget> m_xRadioGroup;
    std::unique_ptr<weld::RadioButton> m_xRbPoint;
    std::unique_ptr<weld::RadioButton> m_xRbVert;
    std::unique_ptr<weld::RadioButton> m_xRbHorz;
    std::unique_ptr<weld::Button> m_xBtnDelete;

    DECL_LINK(ClickHdl, weld::Button&, void);

public:
    SdSnapLineDlg(weld::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View const * pView);
    virtual ~SdSnapLineDlg() override;

    void GetAttr(SfxItemSet& rOutAttrs);

    void HideRadioGroup();
    void HideDeleteBtn() { m_xBtnDelete->hide(); }
    void SetInputFields(bool bEnableX, bool bEnableY);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
