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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPSORT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPSORT_HXX

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

// +1 because one field is reserved for the "- undefined -" entry
#define SC_MAXFIELDS    MAXCOLCOUNT+1

class ScViewData;
class ScSortDlg;
struct ScSortParam;

// Sort Criteria

class ScTabPageSortFields : public SfxTabPage
{
public:
    ScTabPageSortFields( vcl::Window*             pParent,
            const SfxItemSet&   rArgSet );
    virtual ~ScTabPageSortFields();

    static  SfxTabPage* Create      ( vcl::Window*               pParent,
                                      const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) SAL_OVERRIDE;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) SAL_OVERRIDE;

    virtual void SetPosSizePixel(const Point& rAllocPos, const Size& rAllocation) SAL_OVERRIDE;
    virtual void SetSizePixel(const Size& rAllocation) SAL_OVERRIDE;
    virtual void SetPosPixel(const Point& rAllocPos) SAL_OVERRIDE;

protected:
    virtual void        ActivatePage    ( const SfxItemSet& rSet ) SAL_OVERRIDE;
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = 0) SAL_OVERRIDE;

private:
    OUString            aStrUndefined;
    OUString            aStrColumn;
    OUString            aStrRow;

    const sal_uInt16    nWhichSort;
    ScSortDlg*          pDlg;
    ScViewData*         pViewData;
    ScSortParam         aSortData;
    std::vector<SCCOLROW>  nFieldArr;
    sal_uInt16          nFieldCount;
    sal_uInt16          nSortKeyCount;

    SCCOL               nFirstCol;
    SCROW               nFirstRow;
    bool                bHasHeader;
    bool                bSortByRows;

    ScSortKeyItems      maSortKeyItems;
    ScSortKeyCtrl       maSortKeyCtrl;

private:
    void    Init            ();
    void    FillFieldLists  ( sal_uInt16 nStartField );
    sal_uInt16  GetFieldSelPos  ( SCCOLROW nField );
    void    SetLastSortKey( sal_uInt16 nItem );

    // Handler ------------------------
    DECL_LINK( SelectHdl, ListBox * );
};

// Sort Options

class ScDocument;
class CollatorResource;
class CollatorWrapper;

class ScTabPageSortOptions : public SfxTabPage
{
public:
    ScTabPageSortOptions( vcl::Window*            pParent,
            const SfxItemSet&  rArgSet );
    virtual ~ScTabPageSortOptions();

#undef SfxTabPage
#define SfxTabPage ::SfxTabPage
    static  SfxTabPage* Create      ( vcl::Window*               pParent,
                                      const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) SAL_OVERRIDE;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) SAL_OVERRIDE;

protected:
    virtual void        ActivatePage    ( const SfxItemSet& rSet ) SAL_OVERRIDE;
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = 0) SAL_OVERRIDE;

private:

    CheckBox*           m_pBtnCase;
    CheckBox*           m_pBtnHeader;
    CheckBox*           m_pBtnFormats;
    CheckBox*           m_pBtnNaturalSort;

    CheckBox*           m_pBtnCopyResult;
    ListBox*            m_pLbOutPos;
    Edit*               m_pEdOutPos;

    CheckBox*           m_pBtnSortUser;
    ListBox*            m_pLbSortUser;

    SvxLanguageBox*     m_pLbLanguage;
    FixedText*          m_pFtAlgorithm;
    ListBox*            m_pLbAlgorithm;

    RadioButton*        m_pBtnTopDown;
    RadioButton*        m_pBtnLeftRight;

    OUString            aStrRowLabel;
    OUString            aStrColLabel;
    OUString            aStrUndefined;
    OUString            aStrAreaLabel;

    const sal_uInt16    nWhichSort;
    ScSortParam         aSortData;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    ScSortDlg*          pDlg;
    ScAddress           theOutPos;

    CollatorResource*  pColRes;
    CollatorWrapper*    pColWrap;

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
};

#endif // INCLUDED_SC_SOURCE_UI_INC_TPSORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
