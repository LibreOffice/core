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
#ifndef INCLUDED_SW_SOURCE_UI_INC_OPTLOAD_HXX
#define INCLUDED_SW_SOURCE_UI_INC_OPTLOAD_HXX

#include <sfx2/tabdlg.hxx>

#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/field.hxx>
#include <svx/strarray.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/checklbx.hxx>
#include <swlbox.hxx>
#include <caption.hxx>

class SwFldMgr;
class SvTreeListEntry;
class SwWrtShell;

class SwLoadOptPage : public SfxTabPage
{
private:
    RadioButton* m_pAlwaysRB;
    RadioButton* m_pRequestRB;
    RadioButton* m_pNeverRB;

    CheckBox*    m_pAutoUpdateFields;
    CheckBox*    m_pAutoUpdateCharts;

    ListBox*     m_pMetricLB;
    FixedText*   m_pTabFT;
    MetricField* m_pTabMF;
    CheckBox*    m_pUseSquaredPageMode;
    CheckBox*    m_pUseCharUnit;
    Edit*        m_pWordCountED;

    SwWrtShell*  m_pWrtShell;
    sal_uInt16   m_nLastTab;
    sal_Int32    m_nOldLinkMode;

    DECL_LINK(MetricHdl, void *);

public:
    SwLoadOptPage(Window* pParent, const SfxItemSet& rSet);

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

class SwCaptionOptDlg : public SfxSingleTabDialog
{
public:
    SwCaptionOptDlg(Window* pParent, const SfxItemSet& rSet);
};

class CaptionComboBox : public SwComboBox
{
protected:
    virtual void KeyInput( const KeyEvent& );

public:
    CaptionComboBox(Window* pParent, WinBits nStyle)
        : SwComboBox(pParent, nStyle)
    {}
};

class SwCaptionPreview : public Window
{
private:
    OUString maText;
    Point    maDrawPos;
public:
    SwCaptionPreview( Window* pParent, WinBits nStyle );
    void Init();
    void SetPreviewText( const OUString& rText );
    virtual void Paint( const Rectangle& rRect );
    virtual Size GetOptimalSize() const;
};

class SwCaptionOptPage : public SfxTabPage
{
private:
    SvxCheckListBox*  m_pCheckLB;
    ListBox*          m_pLbCaptionOrder;
    SwCaptionPreview* m_pPreview;

    VclContainer*     m_pSettingsGroup;
    CaptionComboBox*  m_pCategoryBox;
    FixedText*        m_pFormatText;
    ListBox*          m_pFormatBox;
    //#i61007# order of captions
    FixedText*        m_pNumberingSeparatorFT;
    Edit*             m_pNumberingSeparatorED;
    FixedText*        m_pTextText;
    Edit*             m_pTextEdit;
    ListBox*          m_pPosBox;

    VclContainer*     m_pNumCapt;
    ListBox*          m_pLbLevel;
    Edit*             m_pEdDelim;

    VclContainer*     m_pCategory;
    ListBox*          m_pCharStyleLB;
    CheckBox*         m_pApplyBorderCB;

    OUString        m_sSWTable;
    OUString        m_sSWFrame;
    OUString        m_sSWGraphic;
    OUString        m_sOLE;

    OUString        m_sIllustration;
    OUString        m_sTable;
    OUString        m_sText;
    OUString        m_sDrawing;

    OUString        m_sBegin;
    OUString        m_sEnd;
    OUString        m_sAbove;
    OUString        m_sBelow;

    OUString        m_sNone;

    SwFldMgr        *pMgr;
    bool            bHTMLMode;

    DECL_LINK(SelectHdl, void *);
    DECL_LINK(ModifyHdl, void * = 0);
    DECL_LINK( OrderHdl, ListBox* );
    DECL_LINK(ShowEntryHdl, void *);
    DECL_LINK(SaveEntryHdl, void *);

    void                DelUserData();
    void                SetOptions( const sal_uLong nPos,
                                    const SwCapObjType eType,
                                    const SvGlobalName *pOleId = 0);
    void                SaveEntry( SvTreeListEntry* pEntry );
    void                DrawSample();

public:
                        SwCaptionOptPage( Window* pParent,
                                         const SfxItemSet& rSet );
                        ~SwCaptionOptPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
