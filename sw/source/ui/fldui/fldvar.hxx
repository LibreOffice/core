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
#ifndef _SWFLDVAR_HXX
#define _SWFLDVAR_HXX

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

    virtual long    PreNotify( NotifyEvent& rNEvt );

public:
    SelectionListBox(Window* pParent, WinBits nStyle);

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

    String              sOldValueFT;
    String              sOldNameFT;

    sal_uLong               nOldFormat;
    sal_Bool                bInit;

    DECL_LINK(TypeHdl, void *);
    DECL_LINK( SubTypeHdl, ListBox* pLB = 0 );
    DECL_LINK(ModifyHdl, void * = 0);
    DECL_LINK( TBClickHdl, ToolBox *pTB = 0);
    DECL_LINK(ChapterHdl, void * = 0);
    DECL_LINK(SeparatorHdl, void * = 0);

    void                UpdateSubType();
    sal_uInt16              FillFormatLB(sal_uInt16 nTypeId);

protected:
    virtual sal_uInt16      GetGroup();

public:
                        SwFldVarPage(Window* pParent, const SfxItemSet& rSet);

                        ~SwFldVarPage();

    static SfxTabPage*  Create(Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

    virtual void        FillUserData();
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
