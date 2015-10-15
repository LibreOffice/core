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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_OPTLOAD_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_OPTLOAD_HXX

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

class SwFieldMgr;
class SvTreeListEntry;
class SwWrtShell;

class SwLoadOptPage : public SfxTabPage
{
private:
    VclPtr<RadioButton>     m_pAlwaysRB;
    VclPtr<RadioButton>     m_pRequestRB;
    VclPtr<RadioButton>     m_pNeverRB;

    VclPtr<CheckBox>        m_pAutoUpdateFields;
    VclPtr<CheckBox>        m_pAutoUpdateCharts;

    VclPtr<ListBox>         m_pMetricLB;
    VclPtr<FixedText>       m_pTabFT;
    VclPtr<MetricField>     m_pTabMF;
    VclPtr<CheckBox>        m_pUseSquaredPageMode;
    VclPtr<CheckBox>        m_pUseCharUnit;
    VclPtr<Edit>            m_pWordCountED;
    VclPtr<CheckBox>        m_pShowStandardizedPageCount;
    VclPtr<NumericField>    m_pStandardizedPageSizeNF;

    SwWrtShell*      m_pWrtShell;
    sal_uInt16       m_nLastTab;
    sal_Int32        m_nOldLinkMode;

    DECL_LINK_TYPED(MetricHdl, ListBox&, void);
    DECL_LINK_TYPED(StandardizedPageCountCheckHdl, Button*, void);

public:
    SwLoadOptPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwLoadOptPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent,
                                      const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

class SwCaptionOptDlg : public SfxSingleTabDialog
{
public:
    SwCaptionOptDlg(vcl::Window* pParent, const SfxItemSet& rSet);
};

class CaptionComboBox : public SwComboBox
{
protected:
    virtual void KeyInput( const KeyEvent& ) override;

public:
    CaptionComboBox(vcl::Window* pParent, WinBits nStyle)
        : SwComboBox(pParent, nStyle)
    {}
};

class SwCaptionPreview : public vcl::Window
{
private:
    OUString maText;
    bool mbFontInitialized;
    vcl::Font maFont;
    Point maDrawPos;
public:
    SwCaptionPreview(vcl::Window* pParent, WinBits nStyle);
    void Init();
    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;
    void SetPreviewText( const OUString& rText );
    virtual void Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual Size GetOptimalSize() const override;
};

class SwCaptionOptPage : public SfxTabPage
{
private:
    VclPtr<SvxCheckListBox>  m_pCheckLB;
    VclPtr<ListBox>          m_pLbCaptionOrder;
    VclPtr<SwCaptionPreview> m_pPreview;

    VclPtr<VclContainer>     m_pSettingsGroup;
    VclPtr<CaptionComboBox>  m_pCategoryBox;
    VclPtr<FixedText>        m_pFormatText;
    VclPtr<ListBox>          m_pFormatBox;
    //#i61007# order of captions
    VclPtr<FixedText>        m_pNumberingSeparatorFT;
    VclPtr<Edit>             m_pNumberingSeparatorED;
    VclPtr<FixedText>        m_pTextText;
    VclPtr<Edit>             m_pTextEdit;
    VclPtr<ListBox>          m_pPosBox;

    VclPtr<VclContainer>     m_pNumCapt;
    VclPtr<ListBox>          m_pLbLevel;
    VclPtr<Edit>             m_pEdDelim;

    VclPtr<VclContainer>     m_pCategory;
    VclPtr<ListBox>          m_pCharStyleLB;
    VclPtr<CheckBox>         m_pApplyBorderCB;

    OUString m_sSWTable;
    OUString m_sSWFrame;
    OUString m_sSWGraphic;
    OUString m_sOLE;

    OUString m_sIllustration;
    OUString m_sTable;
    OUString m_sText;
    OUString m_sDrawing;

    OUString m_sBegin;
    OUString m_sEnd;
    OUString m_sAbove;
    OUString m_sBelow;

    OUString m_sNone;

    SwFieldMgr* pMgr;
    bool bHTMLMode;

    DECL_LINK_TYPED(SelectHdl, ComboBox&, void);
    DECL_LINK_TYPED(SelectListBoxHdl, ListBox&, void);
    DECL_LINK_TYPED(ModifyHdl, Edit&, void);
    DECL_LINK_TYPED( OrderHdl, ListBox&, void );
    DECL_LINK_TYPED(ShowEntryHdl, SvTreeListBox*, void);
    DECL_LINK_TYPED(SaveEntryHdl, SvTreeListBox*, void);

    void DelUserData();
    void SetOptions(const sal_uLong nPos, const SwCapObjType eType, const SvGlobalName *pOleId = 0);
    void SaveEntry(SvTreeListEntry* pEntry);
    void InvalidatePreview();

public:
                        SwCaptionOptPage( vcl::Window* pParent,
                                         const SfxItemSet& rSet );
                        virtual ~SwCaptionOptPage();
    virtual void        dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent,
                                      const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
