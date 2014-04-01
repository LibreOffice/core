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
#ifndef INCLUDED_SW_SOURCE_UI_INC_NUMPARA_HXX
#define INCLUDED_SW_SOURCE_UI_INC_NUMPARA_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>

/* --- class SwParagraphNumTabPage ----------------------------------------
    with this TabPage numbering settings at the paragraph / paragraph style
    are performed.

 ---------------------------------------------------------------------------*/
class SwParagraphNumTabPage : public SfxTabPage
{
    VclHBox*                 m_pOutlineStartBX;
    ListBox*                 m_pOutlineLvLB;

    VclHBox*                 m_pNumberStyleBX;
    ListBox*                 m_pNumberStyleLB;

    TriStateBox*             m_pNewStartCB;
    VclHBox*                 m_pNewStartBX;
    TriStateBox*             m_pNewStartNumberCB;
    NumericField*            m_pNewStartNF;

    VclFrame*                m_pCountParaFram;
    TriStateBox*             m_pCountParaCB;
    TriStateBox*             m_pRestartParaCountCB;

    VclHBox*                 m_pRestartBX;
    NumericField*            m_pRestartNF;

    // --> OD 2008-04-14 #outlinelevel#
    const OUString msOutlineNumbering;

    sal_Bool                    bModified : 1;
    sal_Bool                    bCurNumrule : 1;

    DECL_LINK(NewStartHdl_Impl, void *);
    DECL_LINK( StyleHdl_Impl, ListBox* );
    DECL_LINK(LineCountHdl_Impl, void *);

protected:
        SwParagraphNumTabPage(Window* pParent, const SfxItemSet& rSet );
    void aCountParaFL();

public:
        virtual ~SwParagraphNumTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rSet );
    static sal_uInt16*      GetRanges();

    virtual bool        FillItemSet( SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet& rSet ) SAL_OVERRIDE;

    void                EnableNewStart();
    void                DisableOutline();
    void                DisableNumbering();

    ListBox&            GetStyleBox() {return *m_pNumberStyleLB;};
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
