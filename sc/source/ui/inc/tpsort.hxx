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
    virtual void        dispose() override;
    static  VclPtr<SfxTabPage> Create      ( vcl::Window*               pParent,
                                      const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;

    virtual void SetPosSizePixel(const Point& rAllocPos, const Size& rAllocation) override;
    virtual void SetSizePixel(const Size& rAllocation) override;
    virtual void SetPosPixel(const Point& rAllocPos) override;

protected:
    virtual void        ActivatePage    ( const SfxItemSet& rSet ) override;
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual DeactivateRC   DeactivatePage  ( SfxItemSet* pSet ) override;

private:
    OUString            aStrUndefined;
    OUString            aStrColumn;
    OUString            aStrRow;

    const sal_uInt16    nWhichSort;
    VclPtr<ScSortDlg>   pDlg;
    ScViewData*         pViewData;
    ScSortParam         aSortData;
    std::vector<SCCOLROW>  nFieldArr;
    sal_uInt16          nFieldCount;
    sal_uInt16          nSortKeyCount;

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
    DECL_LINK_TYPED( SelectHdl, ListBox&, void );
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
    virtual void dispose() override;

#undef SfxTabPage
#define SfxTabPage ::SfxTabPage
    static  VclPtr<SfxTabPage> Create      ( vcl::Window*               pParent,
                                      const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;

protected:
    virtual void        ActivatePage    ( const SfxItemSet& rSet ) override;
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual DeactivateRC   DeactivatePage  ( SfxItemSet* pSet ) override;

private:

    VclPtr<CheckBox>           m_pBtnCase;
    VclPtr<CheckBox>           m_pBtnHeader;
    VclPtr<CheckBox>           m_pBtnFormats;
    VclPtr<CheckBox>           m_pBtnNaturalSort;

    VclPtr<CheckBox>           m_pBtnCopyResult;
    VclPtr<ListBox>            m_pLbOutPos;
    VclPtr<Edit>               m_pEdOutPos;

    VclPtr<CheckBox>           m_pBtnSortUser;
    VclPtr<ListBox>            m_pLbSortUser;

    VclPtr<SvxLanguageBox>     m_pLbLanguage;
    VclPtr<FixedText>          m_pFtAlgorithm;
    VclPtr<ListBox>            m_pLbAlgorithm;

    VclPtr<RadioButton>        m_pBtnTopDown;
    VclPtr<RadioButton>        m_pBtnLeftRight;

    OUString            aStrRowLabel;
    OUString            aStrColLabel;
    OUString            aStrUndefined;

    const sal_uInt16    nWhichSort;
    ScSortParam         aSortData;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    VclPtr<ScSortDlg>          pDlg;
    ScAddress           theOutPos;

    CollatorResource*  pColRes;
    CollatorWrapper*    pColWrap;

private:
    void Init                   ();
    void FillUserSortListBox    ();

    // Handler ------------------------
    DECL_LINK_TYPED( EnableHdl, Button*, void );
    DECL_LINK_TYPED( SelOutPosHdl, ListBox&, void );
    void EdOutPosModHdl ( Edit* pEd );
    DECL_LINK_TYPED( SortDirHdl, Button *, void );
    DECL_LINK_TYPED( FillAlgorHdl, ListBox&, void );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_TPSORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
