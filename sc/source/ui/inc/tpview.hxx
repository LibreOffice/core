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

#ifndef SC_TPVIEW_HXX
#define SC_TPVIEW_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <svtools/ctrlbox.hxx>
#include <svx/strarray.hxx>

//========================================================================

class ScViewOptions;

//========================================================================

class ScTpContentOptions : public SfxTabPage
{
    ListBox*         pGridLB;
    FixedText*       pColorFT;
    ColorListBox*    pColorLB;
    CheckBox*        pBreakCB;
    CheckBox*        pGuideLineCB;

    CheckBox*        pFormulaCB;
    CheckBox*        pNilCB;
    CheckBox*        pAnnotCB;
    CheckBox*        pValueCB;
    CheckBox*        pAnchorCB;
    CheckBox*        pClipMarkCB;
    CheckBox*        pRangeFindCB;

    ListBox*         pObjGrfLB;
    ListBox*         pDiagramLB;
    ListBox*         pDrawLB;

    CheckBox*        pSyncZoomCB;

    CheckBox*        pRowColHeaderCB;
    CheckBox*        pHScrollCB;
    CheckBox*        pVScrollCB;
    CheckBox*        pTblRegCB;
    CheckBox*        pOutlineCB;

    ScViewOptions*  pLocalOptions;

    void    InitGridOpt();
    DECL_LINK( GridHdl, ListBox* );
    DECL_LINK( SelLbObjHdl, ListBox* );
    DECL_LINK( CBHdl, CheckBox* );

            ScTpContentOptions( Window*         pParent,
                             const SfxItemSet&  rArgSet );
            ~ScTpContentOptions();

public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rCoreSet );
    virtual sal_Bool        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );


};

//========================================================================

class ScDocument;
class ScTpLayoutOptions : public SfxTabPage
{
    ListBox*        m_pUnitLB;
    MetricField*    m_pTabMF;

    RadioButton*    m_pAlwaysRB;
    RadioButton*    m_pRequestRB;
    RadioButton*    m_pNeverRB;

    CheckBox*       m_pAlignCB;
    ListBox*        m_pAlignLB;
    CheckBox*       m_pEditModeCB;
    CheckBox*       m_pFormatCB;
    CheckBox*       m_pExpRefCB;
    CheckBox*       m_pMarkHdrCB;
    CheckBox*       m_pTextFmtCB;
    CheckBox*       m_pReplWarnCB;
    CheckBox*       m_pLegacyCellSelectionCB;

    SvxStringArray  aUnitArr;

    DECL_LINK( CBHdl, CheckBox* );

    DECL_LINK(MetricHdl, void *);
    DECL_LINK( AlignHdl, CheckBox* );

    ScDocument *pDoc;

    DECL_LINK(  UpdateHdl, CheckBox* );

            ScTpLayoutOptions( Window*          pParent,
                             const SfxItemSet&  rArgSet );
            ~ScTpLayoutOptions();

public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rCoreSet );
    virtual sal_Bool        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

    void                SetDocument(ScDocument* pPtr){pDoc = pPtr;}

};
//========================================================================

#endif // SC_TPUSRLST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
