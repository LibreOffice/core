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

class OfaMSFilterTabPage : public SfxTabPage
{
    std::unique_ptr<weld::CheckButton> m_xWBasicCodeCB;
    std::unique_ptr<weld::CheckButton> m_xWBasicWbctblCB;
    std::unique_ptr<weld::CheckButton> m_xWBasicStgCB;
    std::unique_ptr<weld::CheckButton> m_xEBasicCodeCB;
    std::unique_ptr<weld::CheckButton> m_xEBasicExectblCB;
    std::unique_ptr<weld::CheckButton> m_xEBasicStgCB;
    std::unique_ptr<weld::CheckButton> m_xPBasicCodeCB;
    std::unique_ptr<weld::CheckButton> m_xPBasicStgCB;

    DECL_LINK(LoadWordBasicCheckHdl_Impl, weld::Button&, void);
    DECL_LINK(LoadExcelBasicCheckHdl_Impl, weld::Button&, void);
public:
    OfaMSFilterTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet );
    virtual ~OfaMSFilterTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController,
                                const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

enum class MSFltrPg2_CheckBoxEntries;

class OfaMSFilterTabPage2 : public SfxTabPage
{
    OUString sChgToFromMath,
           sChgToFromWriter,
           sChgToFromCalc,
           sChgToFromImpress,
           sChgToFromSmartArt,
           sChgToFromVisio,
           sChgToFromPDF;

    std::unique_ptr<weld::TreeView> m_xCheckLB;
    std::unique_ptr<weld::RadioButton> m_xHighlightingRB;
    std::unique_ptr<weld::RadioButton> m_xShadingRB;
    std::unique_ptr<weld::CheckButton> m_xMSOLockFileCB;

    void                InsertEntry( const OUString& _rTxt, MSFltrPg2_CheckBoxEntries _nType );
    void                InsertEntry( const OUString& _rTxt, MSFltrPg2_CheckBoxEntries _nType,
                                     bool saveEnabled );
    int                 GetEntry4Type( MSFltrPg2_CheckBoxEntries _nType ) const;

public:
    OfaMSFilterTabPage2(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet );
    virtual ~OfaMSFilterTabPage2() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
