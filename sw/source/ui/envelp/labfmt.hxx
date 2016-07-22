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
#ifndef INCLUDED_SW_SOURCE_UI_ENVELP_LABFMT_HXX
#define INCLUDED_SW_SOURCE_UI_ENVELP_LABFMT_HXX

#include "swuilabimp.hxx"
#include "labimg.hxx"
#include <vcl/msgbox.hxx>
#include <vcl/idle.hxx>
class SwLabFormatPage;

class SwLabPreview : public vcl::Window
{
    Color m_aGrayColor;

    OUString m_aHDistStr;
    OUString m_aVDistStr;
    OUString m_aWidthStr;
    OUString m_aHeightStr;
    OUString m_aLeftStr;
    OUString m_aUpperStr;
    OUString m_aColsStr;
    OUString m_aRowsStr;
    OUString m_aPWidthStr;
    OUString m_aPHeightStr;

    long m_lHDistWidth;
    long m_lVDistWidth;
    long m_lHeightWidth;
    long m_lLeftWidth;
    long m_lUpperWidth;
    long m_lColsWidth;
    long m_lPWidthWidth;
    long m_lPHeightWidth;

    long m_lXWidth;
    long m_lXHeight;

    SwLabItem m_aItem;

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle&) override;

    virtual Size GetOptimalSize() const override;

public:

    explicit SwLabPreview(vcl::Window* pParent);

    void UpdateItem(const SwLabItem& rItem);
};

class SwLabFormatPage : public SfxTabPage
{
    VclPtr<FixedText>    m_pMakeFI;
    VclPtr<FixedText>    m_pTypeFI;
    VclPtr<SwLabPreview> m_pPreview;
    VclPtr<MetricField>  m_pHDistField;
    VclPtr<MetricField>  m_pVDistField;
    VclPtr<MetricField>  m_pWidthField;
    VclPtr<MetricField>  m_pHeightField;
    VclPtr<MetricField>  m_pLeftField;
    VclPtr<MetricField>  m_pUpperField;
    VclPtr<NumericField> m_pColsField;
    VclPtr<NumericField> m_pRowsField;
    VclPtr<MetricField>  m_pPWidthField;
    VclPtr<MetricField>  m_pPHeightField;
    VclPtr<PushButton>   m_pSavePB;

    Idle aPreviewIdle;
    bool  bModified;

    SwLabItem    aItem;

    DECL_LINK_TYPED( ModifyHdl, Edit&, void );
    DECL_LINK_TYPED( PreviewHdl, Idle *, void );
    DECL_LINK_TYPED( LoseFocusHdl, Control&, void );
    DECL_LINK_TYPED( SaveHdl, Button*, void );

    void ChangeMinMax();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwLabFormatPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwLabFormatPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
            void FillItem(SwLabItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;

    SwLabDlg* GetParentSwLabDlg() {return static_cast<SwLabDlg*>(GetParentDialog());}
};

class SwSaveLabelDlg : public ModalDialog
{
    VclPtr<ComboBox>   m_pMakeCB;
    VclPtr<Edit>       m_pTypeED;
    VclPtr<OKButton>   m_pOKPB;

    bool        bSuccess;
    VclPtr<SwLabFormatPage>   pLabPage;
    SwLabRec&       rLabRec;

    DECL_LINK_TYPED(OkHdl, Button*, void);
    DECL_LINK_TYPED(ModifyHdl, Edit&, void);

public:
    SwSaveLabelDlg(SwLabFormatPage* pParent, SwLabRec& rRec);
    virtual ~SwSaveLabelDlg();
    virtual void dispose() override;

    void SetLabel(const OUString& rMake, const OUString& rType)
    {
        m_pMakeCB->SetText(rMake);
        m_pTypeED->SetText(rType);
    }
    bool GetLabel(SwLabItem& rItem);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
