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
#ifndef _WRAP_HXX
#define _WRAP_HXX

#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

class Window;
class SfxItemSet;
class SwWrtShell;

class SwWrapDlg : public SfxNoLayoutSingleTabDialog
{
    SwWrtShell*         pWrtShell;

public:
     SwWrapDlg(Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, sal_Bool bDrawMode);
    ~SwWrapDlg();

    inline SwWrtShell*  GetWrtShell()   { return pWrtShell; }
};


/*--------------------------------------------------------------------
    Description:    circulation TabPage
 --------------------------------------------------------------------*/

class SwWrapTabPage: public SfxTabPage
{
    // WRAPPING
    RadioButton*   m_pNoWrapRB;
    RadioButton*   m_pWrapLeftRB;
    RadioButton*   m_pWrapRightRB;
    RadioButton*   m_pWrapParallelRB;
    RadioButton*   m_pWrapThroughRB;
    RadioButton*   m_pIdealWrapRB;

    // MARGIN
    MetricField*   m_pLeftMarginED;
    MetricField*   m_pRightMarginED;
    MetricField*   m_pTopMarginED;
    MetricField*   m_pBottomMarginED;

    // OPTIONS
    CheckBox*      m_pWrapAnchorOnlyCB;
    CheckBox*      m_pWrapTransparentCB;
    CheckBox*      m_pWrapOutlineCB;
    CheckBox*      m_pWrapOutsideCB;

    sal_uInt16              nOldLeftMargin;
    sal_uInt16              nOldRightMargin;
    sal_uInt16              nOldUpperMargin;
    sal_uInt16              nOldLowerMargin;

    RndStdIds           nAnchorId;
    sal_uInt16              nHtmlMode;

    Size aFrmSize;
    SwWrtShell*         pWrtSh;

    sal_Bool bFormat;
    sal_Bool bNew;
    sal_Bool bHtmlMode;
    sal_Bool bDrawMode;
    sal_Bool bContourImage;

    SwWrapTabPage(Window *pParent, const SfxItemSet &rSet);
    ~SwWrapTabPage();

    void            ApplyImageList();
    void            EnableModes(const SfxItemSet& rSet);
    virtual void    ActivatePage(const SfxItemSet& rSet);
    virtual int     DeactivatePage(SfxItemSet *pSet);
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    DECL_LINK( RangeModifyHdl, MetricField * );
    DECL_LINK( WrapTypeHdl, RadioButton * );
    DECL_LINK(ContourHdl, void *);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);

    virtual sal_Bool    FillItemSet(SfxItemSet &rSet);
    virtual void    Reset(const SfxItemSet &rSet);

    static sal_uInt16*  GetRanges();
    inline void     SetNewFrame(sal_Bool bNewFrame) { bNew = bNewFrame; }
    inline void     SetFormatUsed(sal_Bool bFmt, sal_Bool bDrw) { bFormat = bFmt;
                                                            bDrawMode = bDrw; }
    inline void     SetShell(SwWrtShell* pSh) { pWrtSh = pSh; }
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
