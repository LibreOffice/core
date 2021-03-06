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

#pragma once

#include <vector>

#include <sfx2/tabdlg.hxx>
#include <svtools/collatorres.hxx>
#include <svx/langbox.hxx>
#include <unotools/collatorwrapper.hxx>
#include <vcl/idle.hxx>

#include "sortkeydlg.hxx"

#include <address.hxx>
#include <sheetlimits.hxx>
#include <sortparam.hxx>

// +1 because one field is reserved for the "- undefined -" entry
inline SCCOL SC_MAXFIELDS(const ScSheetLimits& rLimits) { return rLimits.GetMaxColCount() + 1; }

class ScViewData;

// Sort Criteria

class ScTabPageSortFields : public SfxTabPage
{
public:
    ScTabPageSortFields(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rArgSet);
    virtual ~ScTabPageSortFields() override;

    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;

protected:
    virtual void        ActivatePage    ( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage  ( SfxItemSet* pSet ) override;

private:
    Idle m_aIdle;

    OUString            aStrUndefined;
    OUString            aStrColumn;
    OUString            aStrRow;

    const sal_uInt16    nWhichSort;
    ScViewData*         pViewData;
    ScSortParam         aSortData;
    std::vector<SCCOLROW>  nFieldArr;
    sal_uInt16          nFieldCount;
    sal_uInt16          nSortKeyCount;

    bool                bHasHeader;
    bool                bSortByRows;

    std::unique_ptr<weld::ScrolledWindow> m_xScrolledWindow;
    std::unique_ptr<weld::Container> m_xBox;
    ScSortKeyWindow m_aSortWin;

    void AddSortKey( sal_uInt16 nItem );

private:
    void    Init            ();
    void    FillFieldLists  ( sal_uInt16 nStartField );
    sal_uInt16  GetFieldSelPos  ( SCCOLROW nField );
    void    SetLastSortKey( sal_uInt16 nItem );

    // Handler ------------------------
    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(ScrollToEndHdl, Timer*, void);
};

// Sort Options

class ScDocument;

class ScTabPageSortOptions : public SfxTabPage
{
public:
    ScTabPageSortOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* pArgSet);

    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;

protected:
    virtual void        ActivatePage    ( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage  ( SfxItemSet* pSet ) override;

private:
    OUString            aStrRowLabel;
    OUString            aStrColLabel;
    OUString            aStrUndefined;

    const sal_uInt16    nWhichSort;
    ScSortParam         aSortData;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    ScAddress           theOutPos;

    std::unique_ptr<CollatorResource>  m_xColRes;
    std::unique_ptr<CollatorWrapper>   m_xColWrap;

    std::unique_ptr<weld::CheckButton> m_xBtnCase;
    std::unique_ptr<weld::CheckButton> m_xBtnHeader;
    std::unique_ptr<weld::CheckButton> m_xBtnFormats;
    std::unique_ptr<weld::CheckButton> m_xBtnNaturalSort;
    std::unique_ptr<weld::CheckButton> m_xBtnCopyResult;
    std::unique_ptr<weld::ComboBox> m_xLbOutPos;
    std::unique_ptr<weld::Entry> m_xEdOutPos;
    std::unique_ptr<weld::CheckButton> m_xBtnSortUser;
    std::unique_ptr<weld::ComboBox> m_xLbSortUser;
    std::unique_ptr<SvxLanguageBox> m_xLbLanguage;
    std::unique_ptr<weld::Label> m_xFtAlgorithm;
    std::unique_ptr<weld::ComboBox> m_xLbAlgorithm;
    std::unique_ptr<weld::RadioButton> m_xBtnTopDown;
    std::unique_ptr<weld::RadioButton> m_xBtnLeftRight;
    std::unique_ptr<weld::CheckButton> m_xBtnIncComments;
    std::unique_ptr<weld::CheckButton> m_xBtnIncImages;

private:
    void Init                   ();
    void FillUserSortListBox    ();

    // Handler ------------------------
    DECL_LINK( EnableHdl, weld::ToggleButton&, void );
    DECL_LINK( SelOutPosHdl, weld::ComboBox&, void );
    void EdOutPosModHdl();
    DECL_LINK( SortDirHdl, weld::ToggleButton&, void );
    void FillAlgor();
    DECL_LINK( FillAlgorHdl, weld::ComboBox&, void );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
