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

#ifndef SC_TPSORT_HXX
#define SC_TPSORT_HXX


#include <sfx2/tabdlg.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/langbox.hxx>


#include "global.hxx"
#include "address.hxx"

//------------------------------------------------------------------------

// +1 because one field is reserved for the "- undefined -" entry
#define SC_MAXFIELDS    MAXCOLCOUNT+1

class ScViewData;
class ScSortDlg;
struct ScSortParam;

//========================================================================
// Kriterien (Sort Criteria)

class ScTabPageSortFields : public SfxTabPage
{
public:
                ScTabPageSortFields( Window*             pParent,
                                     const SfxItemSet&   rArgSet );
                ~ScTabPageSortFields();

    static  SfxTabPage* Create      ( Window*               pParent,
                                      const SfxItemSet&     rArgSet );
    static  sal_uInt16*     GetRanges   ();
    virtual sal_Bool        FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );

protected:
// fuer Datenaustausch (sollte noch umgestellt werden!)
//  virtual void        ActivatePage    ( const SfxItemSet& rSet );
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage    ();
    virtual int         DeactivatePage  ( SfxItemSet* pSet = 0);

private:
    FixedLine       aFlSort1;
    ListBox         aLbSort1;
    RadioButton     aBtnUp1;
    RadioButton     aBtnDown1;

    FixedLine       aFlSort2;
    ListBox         aLbSort2;
    RadioButton     aBtnUp2;
    RadioButton     aBtnDown2;

    FixedLine       aFlSort3;
    ListBox         aLbSort3;
    RadioButton     aBtnUp3;
    RadioButton     aBtnDown3;

    String          aStrUndefined;
    String          aStrColumn;
    String          aStrRow;

    const sal_uInt16        nWhichSort;
    ScSortDlg*          pDlg;
    ScViewData*         pViewData;
    const ScSortParam&  rSortData;
    SCCOLROW            nFieldArr[SC_MAXFIELDS];
    sal_uInt16              nFieldCount;
    SCCOL               nFirstCol;
    SCROW               nFirstRow;
    sal_Bool                bHasHeader;
    sal_Bool                bSortByRows;

    ListBox*            aSortLbArr[3];
    RadioButton*        aDirBtnArr[3][2];
    FixedLine*          aFlArr[3];

#ifdef _TPSORT_CXX
private:
    void    Init            ();
    void    DisableField    ( sal_uInt16 nField );
    void    EnableField     ( sal_uInt16 nField );
    void    FillFieldLists  ();
    sal_uInt16  GetFieldSelPos  ( SCCOLROW nField );

    // Handler ------------------------
    DECL_LINK( SelectHdl, ListBox * );
#endif
};

//========================================================================
// Sortieroptionen (Sort Options)

class ScDocument;
class ScRangeData;
class CollatorRessource;
class CollatorWrapper;

#if ENABLE_LAYOUT_EXPERIMENTAL
#include <sfx2/layout.hxx>
#include <layout/layout-pre.hxx>
#else /* !ENABLE_LAYOUT_EXPERIMENTAL */
#define LocalizedString String
#endif /* !ENABLE_LAYOUT_EXPERIMENTAL */

class ScTabPageSortOptions : public SfxTabPage
{
public:
                ScTabPageSortOptions( Window*            pParent,
                                      const SfxItemSet&  rArgSet );
                ~ScTabPageSortOptions();

#undef SfxTabPage
#define SfxTabPage ::SfxTabPage
    static  SfxTabPage* Create      ( Window*               pParent,
                                      const SfxItemSet&     rArgSet );
    static  sal_uInt16*     GetRanges   ();
    virtual sal_Bool        FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );

protected:
// fuer Datenaustausch (sollte noch umgestellt werden!)
//  virtual void        ActivatePage    ( const SfxItemSet& rSet );
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage    ();
    virtual int         DeactivatePage  ( SfxItemSet* pSet = 0);

private:

    CheckBox            aBtnCase;
    CheckBox            aBtnHeader;
    CheckBox            aBtnFormats;
    CheckBox            aBtnNaturalSort;

    CheckBox            aBtnCopyResult;
    ListBox             aLbOutPos;
    Edit                aEdOutPos;

    CheckBox            aBtnSortUser;
    ListBox             aLbSortUser;

    FixedText           aFtLanguage;
    SvxLanguageBox      aLbLanguage;
    FixedText           aFtAlgorithm;
    ListBox             aLbAlgorithm;

    FixedLine           aLineDirection;
    RadioButton         aBtnTopDown;
    RadioButton         aBtnLeftRight;

//     FixedText           aFtAreaLabel;
//  FixedInfo           aFtArea;
    LocalizedString aStrRowLabel;
    LocalizedString aStrColLabel;
    LocalizedString aStrUndefined;
    String              aStrAreaLabel;

    const sal_uInt16        nWhichSort;
    const ScSortParam&  rSortData;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    ScSortDlg*          pDlg;
    ScAddress           theOutPos;

    CollatorRessource*  pColRes;
    CollatorWrapper*    pColWrap;

#ifdef _TPSORT_CXX
private:
    void Init                   ();
    void FillUserSortListBox    ();
    void FillOutPosList         ();

    // Handler ------------------------
    DECL_LINK( EnableHdl, CheckBox * );
    DECL_LINK( SelOutPosHdl, ListBox * );
    void EdOutPosModHdl ( Edit* pEd );
    DECL_LINK( SortDirHdl, RadioButton * );
    DECL_LINK( FillAlgorHdl, void * );
#endif
};

#if ENABLE_LAYOUT_EXPERIMENTAL
#include <layout/layout-post.hxx>
#endif /* ENABLE_LAYOUT_EXPERIMENTAL */

#endif // SC_TPSORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
