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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DOCSTDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DOCSTDLG_HXX

#include <sfx2/tabdlg.hxx>
#include <docstat.hxx>

// DocInfo now as page
class SwDocStatPage final : public SfxTabPage
{
public:
    SwDocStatPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet);
    virtual ~SwDocStatPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);

private:
    virtual bool    FillItemSet(      SfxItemSet *rSet) override;
    virtual void    Reset      (const SfxItemSet *rSet) override;

    DECL_LINK(UpdateHdl, weld::Button&, void);

    std::unique_ptr<weld::Label> m_xPageNo;
    std::unique_ptr<weld::Label> m_xTableNo;
    std::unique_ptr<weld::Label> m_xGrfNo;
    std::unique_ptr<weld::Label> m_xOLENo;
    std::unique_ptr<weld::Label> m_xParaNo;
    std::unique_ptr<weld::Label> m_xWordNo;
    std::unique_ptr<weld::Label> m_xCharNo;
    std::unique_ptr<weld::Label> m_xCharExclSpacesNo;
    std::unique_ptr<weld::Label> m_xLineLbl;
    std::unique_ptr<weld::Label> m_xLineNo;
    std::unique_ptr<weld::Button> m_xUpdatePB;

    SwDocStat       m_aDocStat;

    void            Update();

    void            SetData(const SwDocStat &rStat);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
