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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DRPCPS_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DRPCPS_HXX

#include <sfx2/basedlgs.hxx>
#include <sfx2/tabdlg.hxx>

#include <vcl/button.hxx>

#include <vcl/fixed.hxx>

#include <vcl/field.hxx>

#include <vcl/edit.hxx>

#include <vcl/lstbox.hxx>

class SwWrtShell;

class SwDropCapsDlg : public SfxSingleTabDialog
{
public:
    SwDropCapsDlg(vcl::Window *pParent, const SfxItemSet &rSet );
};

class SwDropCapsPict;

class SwDropCapsPage : public SfxTabPage
{
friend class SwDropCapsPict;
    CheckBox*        m_pDropCapsBox;
    CheckBox*        m_pWholeWordCB;
    FixedText*       m_pSwitchText;
    NumericField*    m_pDropCapsField;
    FixedText*       m_pLinesText;
    NumericField*    m_pLinesField;
    FixedText*       m_pDistanceText;
    MetricField*     m_pDistanceField;
    FixedText*       m_pTextText;
    Edit*            m_pTextEdit;
    FixedText*       m_pTemplateText;
    ListBox*         m_pTemplateBox;

    SwDropCapsPict*  m_pPict;

    bool          bModified;
    bool          bFormat;
    bool          bHtmlMode;

    SwWrtShell &rSh;

     SwDropCapsPage(vcl::Window *pParent, const SfxItemSet &rSet);
    virtual ~SwDropCapsPage();

    virtual int     DeactivatePage(SfxItemSet *pSet) SAL_OVERRIDE;
    void    FillSet( SfxItemSet &rSet );

    DECL_LINK(ClickHdl, void *);
    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK(SelectHdl, void *);
    DECL_LINK(WholeWordHdl, void *);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage *Create(vcl::Window *pParent, const SfxItemSet *rSet);
    static const sal_uInt16* GetRanges();

    virtual bool FillItemSet(      SfxItemSet *rSet) SAL_OVERRIDE;
    virtual void Reset      (const SfxItemSet *rSet) SAL_OVERRIDE;

    void    SetFormat(bool bSet){bFormat = bSet;}
protected:
    void aSwitchText(bool bChecked);
    //void SW_RES(int arg1);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
