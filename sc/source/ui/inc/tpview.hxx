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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPVIEW_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPVIEW_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <svtools/ctrlbox.hxx>
#include <svx/strarray.hxx>

class ScViewOptions;

class ScTpContentOptions : public SfxTabPage
{
    friend class VclPtr<ScTpContentOptions>;
    VclPtr<ListBox>         pGridLB;
    VclPtr<FixedText>       pColorFT;
    VclPtr<ColorListBox>    pColorLB;
    VclPtr<CheckBox>        pBreakCB;
    VclPtr<CheckBox>        pGuideLineCB;

    VclPtr<CheckBox>        pFormulaCB;
    VclPtr<CheckBox>        pNilCB;
    VclPtr<CheckBox>        pAnnotCB;
    VclPtr<CheckBox>        pValueCB;
    VclPtr<CheckBox>        pAnchorCB;
    VclPtr<CheckBox>        pClipMarkCB;
    VclPtr<CheckBox>        pRangeFindCB;

    VclPtr<ListBox>         pObjGrfLB;
    VclPtr<ListBox>         pDiagramLB;
    VclPtr<ListBox>         pDrawLB;

    VclPtr<CheckBox>        pSyncZoomCB;

    VclPtr<CheckBox>        pRowColHeaderCB;
    VclPtr<CheckBox>        pHScrollCB;
    VclPtr<CheckBox>        pVScrollCB;
    VclPtr<CheckBox>        pTblRegCB;
    VclPtr<CheckBox>        pOutlineCB;

    ScViewOptions*  pLocalOptions;

    void    InitGridOpt();
    DECL_LINK_TYPED( GridHdl, ListBox&, void );
    DECL_LINK_TYPED( SelLbObjHdl, ListBox&, void );
    DECL_LINK_TYPED( CBHdl, Button*, void );

            ScTpContentOptions( vcl::Window*         pParent,
                             const SfxItemSet&  rArgSet );
            virtual ~ScTpContentOptions();
    virtual void dispose() override;

public:
    static  VclPtr<SfxTabPage> Create          ( vcl::Window*               pParent,
                                          const SfxItemSet*     rCoreSet );
    virtual bool        FillItemSet     ( SfxItemSet* rCoreSet ) override;
    virtual void        Reset           ( const SfxItemSet* rCoreSet ) override;
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

};

class ScDocument;
class ScTpLayoutOptions : public SfxTabPage
{
    friend class VclPtrInstance<ScTpLayoutOptions>;
    VclPtr<ListBox>        m_pUnitLB;
    VclPtr<MetricField>    m_pTabMF;

    VclPtr<RadioButton>    m_pAlwaysRB;
    VclPtr<RadioButton>    m_pRequestRB;
    VclPtr<RadioButton>    m_pNeverRB;

    VclPtr<CheckBox>       m_pAlignCB;
    VclPtr<ListBox>        m_pAlignLB;
    VclPtr<CheckBox>       m_pEditModeCB;
    VclPtr<CheckBox>       m_pFormatCB;
    VclPtr<CheckBox>       m_pExpRefCB;
    VclPtr<CheckBox>       m_pSortRefUpdateCB;
    VclPtr<CheckBox>       m_pMarkHdrCB;
    VclPtr<CheckBox>       m_pTextFmtCB;
    VclPtr<CheckBox>       m_pReplWarnCB;
    VclPtr<CheckBox>       m_pLegacyCellSelectionCB;

    SvxStringArray  aUnitArr;

    DECL_LINK_TYPED(MetricHdl, ListBox&, void );
    DECL_LINK_TYPED( AlignHdl, Button*, void );

    ScDocument *pDoc;

            ScTpLayoutOptions( vcl::Window*          pParent,
                             const SfxItemSet&  rArgSet );
public:
    virtual ~ScTpLayoutOptions();
    virtual void        dispose() override;
    static  VclPtr<SfxTabPage> Create          ( vcl::Window*               pParent,
                                          const SfxItemSet*     rCoreSet );
    virtual bool        FillItemSet     ( SfxItemSet* rCoreSet ) override;
    virtual void        Reset           ( const SfxItemSet* rCoreSet ) override;
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

    void                SetDocument(ScDocument* pPtr){pDoc = pPtr;}

};

#endif // INCLUDED_SC_SOURCE_UI_INC_TPVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
