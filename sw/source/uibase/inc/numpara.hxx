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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_NUMPARA_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_NUMPARA_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>

// with this TabPage numbering settings at the paragraph / paragraph style
// are performed.
class SwParagraphNumTabPage : public SfxTabPage
{
    VclPtr<VclHBox>                 m_pOutlineStartBX;
    VclPtr<ListBox>                 m_pOutlineLvLB;
    VclPtr<VclHBox>                 m_pNumberStyleBX;
    VclPtr<ListBox>                 m_pNumberStyleLB;
    VclPtr<PushButton>              m_pEditNumStyleBtn;

    VclPtr<TriStateBox>             m_pNewStartCB;
    VclPtr<VclHBox>                 m_pNewStartBX;
    VclPtr<TriStateBox>             m_pNewStartNumberCB;
    VclPtr<NumericField>            m_pNewStartNF;

    VclPtr<VclFrame>                m_pCountParaFram;
    VclPtr<TriStateBox>             m_pCountParaCB;
    VclPtr<TriStateBox>             m_pRestartParaCountCB;

    VclPtr<VclHBox>                 m_pRestartBX;
    VclPtr<NumericField>            m_pRestartNF;

    // --> OD 2008-04-14 #outlinelevel#
    const OUString msOutlineNumbering;

    bool                    bModified : 1;
    bool                    bCurNumrule : 1;

    DECL_LINK_TYPED(NewStartHdl_Impl, Button*, void);
    DECL_LINK_TYPED( StyleHdl_Impl, ListBox&,void );
    DECL_LINK_TYPED(LineCountHdl_Impl, Button*, void);
    DECL_LINK_TYPED(EditNumStyleHdl_Impl, Button*, void);
    DECL_LINK_TYPED(EditNumStyleSelectHdl_Impl, ListBox&, void);

    static const sal_uInt16 aPageRg[];

protected:
    static bool ExecuteEditNumStyle_Impl( sal_uInt16 nId, const OUString& rStr, const OUString& rRefStr,
                          sal_uInt16 nFamily, sal_uInt16 nMask = 0,
                          const sal_uInt16* pModifier = nullptr );

public:
    SwParagraphNumTabPage(vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SwParagraphNumTabPage();
    virtual void        dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent,
                                      const SfxItemSet* rSet );
    static const sal_uInt16* GetRanges() { return aPageRg; }

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ChangesApplied() override;

    void                EnableNewStart();
    void                DisableOutline();
    void                DisableNumbering();

    ListBox&            GetStyleBox() {return *m_pNumberStyleLB;};
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
