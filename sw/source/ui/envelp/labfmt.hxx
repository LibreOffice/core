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
#ifndef _LABFMT_HXX
#define _LABFMT_HXX

#include "swuilabimp.hxx"
#include "labimg.hxx"
#include <vcl/msgbox.hxx>
class SwLabFmtPage;

// class SwLabPreview -------------------------------------------------------

class SwLabPreview : public Window
{
    Color aGrayColor;

    String aHDistStr;
    String aVDistStr;
    String aWidthStr;
    String aHeightStr;
    String aLeftStr;
    String aUpperStr;
    String aColsStr;
    String aRowsStr;
    String aPWidthStr;
    String aPHeightStr;

    long lHDistWidth;
    long lVDistWidth;
    long lHeightWidth;
    long lLeftWidth;
    long lUpperWidth;
    long lColsWidth;
    long lPWidthWidth;
    long lPHeightWidth;

    long lXWidth;
    long lXHeight;

    SwLabItem aItem;

    virtual void Paint(const Rectangle&);

    virtual Size GetOptimalSize() const;

    void DrawArrow(const Point& rP1, const Point& rP2, bool bArrow);

public:

    SwLabPreview(Window* pParent);

    void UpdateItem(const SwLabItem& rItem);
};

// class SwLabFmtPage -------------------------------------------------------

class SwLabFmtPage : public SfxTabPage
{
    FixedText*    m_pMakeFI;
    FixedText*    m_pTypeFI;
    SwLabPreview* m_pPreview;
    MetricField*  m_pHDistField;
    MetricField*  m_pVDistField;
    MetricField*  m_pWidthField;
    MetricField*  m_pHeightField;
    MetricField*  m_pLeftField;
    MetricField*  m_pUpperField;
    NumericField* m_pColsField;
    NumericField* m_pRowsField;
    MetricField*  m_pPWidthField;
    MetricField*  m_pPHeightField;
    PushButton*   m_pSavePB;

    Timer aPreviewTimer;
    bool  bModified;

    SwLabItem    aItem;

     SwLabFmtPage(Window* pParent, const SfxItemSet& rSet);
    ~SwLabFmtPage();

    DECL_LINK(ModifyHdl, void *);
    DECL_LINK(PreviewHdl, void *);
    DECL_LINK( LoseFocusHdl, Control * );
    DECL_LINK(SaveHdl, void *);

    void ChangeMinMax();

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
            void FillItem(SwLabItem& rItem);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);

    SwLabDlg* GetParentSwLabDlg() {return (SwLabDlg*)GetParentDialog();}
};

class SwSaveLabelDlg : public ModalDialog
{
    ComboBox*   m_pMakeCB;
    Edit*       m_pTypeED;
    OKButton*   m_pOKPB;

    bool        bSuccess;
    SwLabFmtPage*   pLabPage;
    SwLabRec&       rLabRec;

    DECL_LINK(OkHdl, void *);
    DECL_LINK(ModifyHdl, void *);

public:
    SwSaveLabelDlg(SwLabFmtPage* pParent, SwLabRec& rRec);

    void SetLabel(const OUString& rMake, const OUString& rType)
    {
        m_pMakeCB->SetText(rMake);
        m_pTypeED->SetText(rType);
    }
    bool GetLabel(SwLabItem& rItem);
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
