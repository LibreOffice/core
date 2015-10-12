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

#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>

#include "docstat.hxx"

// DocInfo now as page
class SwDocStatPage: public SfxTabPage
{
public:
    SwDocStatPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwDocStatPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window *pParent, const SfxItemSet *rSet);

protected:
    virtual bool    FillItemSet(      SfxItemSet *rSet) override;
    virtual void    Reset      (const SfxItemSet *rSet) override;

    DECL_LINK_TYPED(UpdateHdl, Button*, void);

private:
    VclPtr<FixedText>      m_pPageNo;
    VclPtr<FixedText>      m_pTableNo;
    VclPtr<FixedText>      m_pGrfNo;
    VclPtr<FixedText>      m_pOLENo;
    VclPtr<FixedText>      m_pParaNo;
    VclPtr<FixedText>      m_pWordNo;
    VclPtr<FixedText>      m_pCharNo;
    VclPtr<FixedText>      m_pCharExclSpacesNo;

    VclPtr<FixedText>      m_pLineLbl;
    VclPtr<FixedText>      m_pLineNo;
    VclPtr<PushButton>     m_pUpdatePB;

    SwDocStat       aDocStat;

    void            Update();

    using Window::SetData;
    void            SetData(const SwDocStat &rStat);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
