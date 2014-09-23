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
#ifndef INCLUDED_SW_SOURCE_UI_FLDUI_FLDFUNC_HXX
#define INCLUDED_SW_SOURCE_UI_FLDUI_FLDFUNC_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/edit.hxx>

#include "condedit.hxx"
#include "fldpage.hxx"
#include <actctrl.hxx>

class SwFldFuncPage : public SwFldPage
{
    ListBox*        m_pTypeLB;
    ListBox*        m_pSelectionLB;
    VclContainer*   m_pFormat;
    ListBox*        m_pFormatLB;
    FixedText*      m_pNameFT;
    ConditionEdit*  m_pNameED;
    VclContainer*   m_pValueGroup;
    FixedText*      m_pValueFT;
    Edit*           m_pValueED;
    FixedText*      m_pCond1FT;
    ConditionEdit*  m_pCond1ED;
    FixedText*      m_pCond2FT;
    ConditionEdit*  m_pCond2ED;
    PushButton*     m_pMacroBT;

    //controls of "Input list"
    VclContainer*   m_pListGroup;
    FixedText*      m_pListItemFT;
    ReturnActionEdit* m_pListItemED;
    PushButton*     m_pListAddPB;
    FixedText*      m_pListItemsFT;
    ListBox*        m_pListItemsLB;
    PushButton*     m_pListRemovePB;
    PushButton*     m_pListUpPB;
    PushButton*     m_pListDownPB;
    FixedText*      m_pListNameFT;
    Edit*           m_pListNameED;

    OUString        m_sOldValueFT;
    OUString        m_sOldNameFT;

    sal_uLong           nOldFormat;
    bool            bDropDownLBChanged;

    DECL_LINK(TypeHdl, void *);
    DECL_LINK(SelectHdl, void * = 0);
    DECL_LINK(InsertMacroHdl, void *);
    DECL_LINK(ModifyHdl, void * = 0);
    DECL_LINK( ListModifyHdl, Control*);
    DECL_LINK( ListEnableHdl, void*);

    // select Macro
    DECL_LINK( MacroHdl, Button * );

    void                UpdateSubType();
    OUString            TurnMacroString(const OUString &rMacro);

protected:
    virtual sal_uInt16      GetGroup() SAL_OVERRIDE;

public:
                        SwFldFuncPage(vcl::Window* pParent, const SfxItemSet& rSet);

                        virtual ~SwFldFuncPage();

    static SfxTabPage*  Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;

    virtual void        FillUserData() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
