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
#ifndef INCLUDED_SW_SOURCE_UI_INC_DOCSTDLG_HXX
#define INCLUDED_SW_SOURCE_UI_INC_DOCSTDLG_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/fixed.hxx>
#include <svtools/stdctrl.hxx>

#include "docstat.hxx"

/*--------------------------------------------------------------------
    Description:   DocInfo now as page
 --------------------------------------------------------------------*/
class SwDocStatPage: public SfxTabPage
{
public:
    SwDocStatPage(Window *pParent, const SfxItemSet &rSet);
    ~SwDocStatPage();

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);

protected:
    virtual bool    FillItemSet(      SfxItemSet &rSet);
    virtual void    Reset      (const SfxItemSet &rSet);

    DECL_LINK(UpdateHdl, void *);

private:
    FixedText*      m_pPageNo;
    FixedText*      m_pTableNo;
    FixedText*      m_pGrfNo;
    FixedText*      m_pOLENo;
    FixedText*      m_pParaNo;
    FixedText*      m_pWordNo;
    FixedText*      m_pCharNo;
    FixedText*      m_pCharExclSpacesNo;

    FixedText*      m_pLineLbl;
    FixedText*      m_pLineNo;
    PushButton*     m_pUpdatePB;

    SwDocStat       aDocStat;

    void            Update();

    using Window::SetData;
    void            SetData(const SwDocStat &rStat);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
