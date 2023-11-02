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

#include <sfx2/tabdlg.hxx>
#include <svx/txencbox.hxx>

class OfaHtmlTabPage : public SfxTabPage
{

    std::unique_ptr<weld::SpinButton> m_xSize1NF;
    std::unique_ptr<weld::Widget> m_xSize1Img;
    std::unique_ptr<weld::SpinButton> m_xSize2NF;
    std::unique_ptr<weld::Widget> m_xSize2Img;
    std::unique_ptr<weld::SpinButton> m_xSize3NF;
    std::unique_ptr<weld::Widget> m_xSize3Img;
    std::unique_ptr<weld::SpinButton> m_xSize4NF;
    std::unique_ptr<weld::Widget> m_xSize4Img;
    std::unique_ptr<weld::SpinButton> m_xSize5NF;
    std::unique_ptr<weld::Widget> m_xSize5Img;
    std::unique_ptr<weld::SpinButton> m_xSize6NF;
    std::unique_ptr<weld::Widget> m_xSize6Img;
    std::unique_ptr<weld::SpinButton> m_xSize7NF;
    std::unique_ptr<weld::Widget> m_xSize7Img;

    std::unique_ptr<weld::CheckButton> m_xNumbersEnglishUSCB;
    std::unique_ptr<weld::Widget> m_xNumbersEnglishUSImg;
    std::unique_ptr<weld::CheckButton> m_xUnknownTagCB;
    std::unique_ptr<weld::Widget> m_xUnknownTagImg;
    std::unique_ptr<weld::CheckButton> m_xIgnoreFontNamesCB;
    std::unique_ptr<weld::Widget> m_xIgnoreFontNamesImg;

    std::unique_ptr<weld::CheckButton> m_xStarBasicCB;
    std::unique_ptr<weld::Widget> m_xStarBasicImg;
    std::unique_ptr<weld::CheckButton> m_xStarBasicWarningCB;
    std::unique_ptr<weld::Widget> m_xStarBasicWarningImg;
    std::unique_ptr<weld::CheckButton> m_xPrintExtensionCB;
    std::unique_ptr<weld::Widget> m_xPrintExtensionImg;
    std::unique_ptr<weld::CheckButton> m_xSaveGrfLocalCB;
    std::unique_ptr<weld::Widget> m_xSaveGrfLocalImg;

    DECL_LINK(CheckBoxHdl_Impl, weld::Toggleable&, void);

public:
    OfaHtmlTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~OfaHtmlTabPage() override;
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController,
                                       const SfxItemSet* rAttrSet );

    virtual OUString GetAllStrings() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

};



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
