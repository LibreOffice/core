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
#ifndef INCLUDED_SW_SOURCE_UI_FLDUI_FLDVAR_HXX
#define INCLUDED_SW_SOURCE_UI_FLDUI_FLDVAR_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/edit.hxx>

#include "fldpage.hxx"
#include "condedit.hxx"
#include "numfmtlb.hxx"

class SwFldVarPage;

class SelectionListBox : public ListBox
{
    bool            bCallAddSelection;

    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

public:
    SelectionListBox(vcl::Window* pParent, WinBits nStyle);

    //  detect selection via Ctrl or Alt and evaluate with SelectHdl
    bool            IsCallAddSelection() const {return bCallAddSelection;}
    void            ResetCallAddSelection() {bCallAddSelection = false;}
};

class SwFldVarPage : public SwFldPage
{
    friend class SelectionListBox;

    ListBox*            m_pTypeLB;
    VclContainer*       m_pSelection;
    SelectionListBox*   m_pSelectionLB;
    FixedText*          m_pNameFT;
    Edit*               m_pNameED;
    FixedText*          m_pValueFT;
    ConditionEdit*      m_pValueED;
    VclContainer*       m_pFormat;
    NumFormatListBox*   m_pNumFormatLB;
    ListBox*            m_pFormatLB;
    VclContainer*       m_pChapterFrame;
    ListBox*            m_pChapterLevelLB;
    CheckBox*           m_pInvisibleCB;
    FixedText*          m_pSeparatorFT;
    Edit*               m_pSeparatorED;
    ToolBox*            m_pNewDelTBX;
    sal_uInt16          m_nApplyId;
    sal_uInt16          m_nDeleteId;

    OUString            sOldValueFT;
    OUString            sOldNameFT;

    sal_uLong           nOldFormat;
    bool                bInit;

    DECL_LINK(TypeHdl, void *);
    DECL_LINK( SubTypeHdl, ListBox* pLB = 0 );
    DECL_LINK(ModifyHdl, void * = 0);
    DECL_LINK( TBClickHdl, ToolBox *pTB = 0);
    DECL_LINK(ChapterHdl, void * = 0);
    DECL_LINK(SeparatorHdl, void * = 0);

    void                UpdateSubType();
    sal_Int32           FillFormatLB(sal_uInt16 nTypeId);

protected:
    virtual sal_uInt16      GetGroup() SAL_OVERRIDE;

public:
                        SwFldVarPage(vcl::Window* pParent, const SfxItemSet& rSet);

                        virtual ~SwFldVarPage();

    static SfxTabPage*  Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;

    virtual void        FillUserData() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
