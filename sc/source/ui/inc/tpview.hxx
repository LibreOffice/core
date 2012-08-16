/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    FixedLine       aLinesGB;
    FixedText       aGridFT;
    ListBox         aGridLB;
    FixedText       aColorFT;
    ColorListBox    aColorLB;
    CheckBox        aBreakCB;
    CheckBox        aGuideLineCB;

    FixedLine       aSeparator1FL;

    FixedLine       aDisplayGB;
    CheckBox        aFormulaCB;
    CheckBox        aNilCB;
    CheckBox        aAnnotCB;
    CheckBox        aValueCB;
    CheckBox        aAnchorCB;
    CheckBox        aClipMarkCB;
    CheckBox        aRangeFindCB;

    FixedLine       aObjectGB;
    FixedText       aObjGrfFT;
    ListBox         aObjGrfLB;
    FixedText       aDiagramFT;
    ListBox         aDiagramLB;
    FixedText       aDrawFT;
    ListBox         aDrawLB;

    FixedLine       aZoomGB;
    CheckBox        aSyncZoomCB;

    FixedLine       aSeparator2FL;

    FixedLine       aWindowGB;
    CheckBox        aRowColHeaderCB;
    CheckBox        aHScrollCB;
    CheckBox        aVScrollCB;
    CheckBox        aTblRegCB;
    CheckBox        aOutlineCB;

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
    FixedLine       aUnitGB;
    FixedText       aUnitFT;
    ListBox         aUnitLB;
    FixedText       aTabFT;
    MetricField     aTabMF;

    FixedLine       aSeparatorFL;
    FixedLine       aLinkGB;
    FixedText       aLinkFT;
    RadioButton     aAlwaysRB;
    RadioButton     aRequestRB;
    RadioButton     aNeverRB;

    FixedLine       aOptionsGB;
    CheckBox        aAlignCB;
    ListBox         aAlignLB;
    CheckBox        aEditModeCB;
    CheckBox        aFormatCB;
    CheckBox        aExpRefCB;
    CheckBox        aMarkHdrCB;
    CheckBox        aTextFmtCB;
    CheckBox        aReplWarnCB;

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
