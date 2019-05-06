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
#ifndef INCLUDED_CUI_SOURCE_INC_TIPOFTHEDAYDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_TIPOFTHEDAYDLG_HXX

#include <vcl/weld.hxx>

class TipOfTheDayDialog : public weld::GenericDialogController
{
private:
    std::unique_ptr<weld::Image> m_pImage;
    std::unique_ptr<weld::Label> m_pText;
    std::unique_ptr<weld::CheckButton> m_pShowTip;
    std::unique_ptr<weld::Button> m_pNext;
    std::unique_ptr<weld::LinkButton> m_pLink;

    sal_uInt32 nCurrentTip;
    sal_uInt32 nNumberOfTips;
    OUString aLink;
    void UpdateTip();
    DECL_STATIC_LINK(TipOfTheDayDialog, OnShowTipToggled, weld::ToggleButton&, void);
    DECL_LINK(OnNextClick, weld::Button&, void);
    DECL_LINK(OnLinkClick, weld::LinkButton&, void);

public:
    TipOfTheDayDialog(weld::Window* pWindow);
    virtual ~TipOfTheDayDialog() override;
};

#endif // INCLUDED_CUI_SOURCE_INC_TIPOFTHEDAYDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
