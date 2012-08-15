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

#include <vector>

#include <sfx2/tabdlg.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svx/langbox.hxx>

#include "sortkeydlg.hxx"

#include "global.hxx"
#include "address.hxx"
#include "sortparam.hxx"

//------------------------------------------------------------------------

// +1 because one field is reserved for the "- undefined -" entry
#define SC_MAXFIELDS    MAXCOLCOUNT+1

class ScViewData;
class ScSortDlg;
struct ScSortParam;

//========================================================================
// Sort Criteria

class ScTabPageSortFields : public SfxTabPage
{
public:
    ScTabPageSortFields( Window*             pParent,
            const SfxItemSet&   rArgSet );
    virtual ~ScTabPageSortFields();

    static  SfxTabPage* Create      ( Window*               pParent,
                                      const SfxItemSet&     rArgSet );
    virtual sal_Bool    FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );

protected:
// for data exchange (TODO: should be changed!)
//  virtual void        ActivatePage    ( const SfxItemSet& rSet );
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage    ();
    virtual int         DeactivatePage  ( SfxItemSet* pSet = 0);

private:
    rtl::OUString          aStrUndefined;
    rtl::OUString          aStrColumn;
    rtl::OUString          aStrRow;

    const sal_uInt16    nWhichSort;
    ScSortDlg*          pDlg;
    ScViewData*         pViewData;
    ScSortParam         aSortData;
    std::vector<SCCOLROW>  nFieldArr;
    sal_uInt16          nFieldCount;
    sal_uInt16          nSortKeyCount;
    sal_uInt16          nCurrentOffset;

    SCCOL               nFirstCol;
    SCROW               nFirstRow;
    sal_Bool            bHasHeader;
    sal_Bool            bSortByRows;

    ScSortKeyItems      maSortKeyItems;
    ScSortKeyCtrl       maSortKeyCtrl;

#ifdef _TPSORT_CXX
private:
    void    Init            ();
    void    FillFieldLists  ( sal_uInt16 nStartField );
    sal_uInt16  GetFieldSelPos  ( SCCOLROW nField );

    // Handler ------------------------
    DECL_LINK( SelectHdl, ListBox * );
#endif
};

//========================================================================
// Sort Options

class ScDocument;
class CollatorResource;
class CollatorWrapper;

class ScTabPageSortOptions : public SfxTabPage
{
public:
    ScTabPageSortOptions( Window*            pParent,
            const SfxItemSet&  rArgSet );
    virtual ~ScTabPageSortOptions();

#undef SfxTabPage
#define SfxTabPage ::SfxTabPage
    static  SfxTabPage* Create      ( Window*               pParent,
                                      const SfxItemSet&     rArgSet );
    virtual sal_Bool    FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );

protected:
// for data exchange (TODO: should be changed!)
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

    rtl::OUString              aStrRowLabel;
    rtl::OUString              aStrColLabel;
    rtl::OUString              aStrUndefined;
    rtl::OUString              aStrAreaLabel;

    const sal_uInt16    nWhichSort;
    ScSortParam         aSortData;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    ScSortDlg*          pDlg;
    ScAddress           theOutPos;

    CollatorResource*  pColRes;
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

#endif // SC_TPSORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
