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

#include <sfx2/tabdlg.hxx>
#include <global.hxx>
#include <sheetlimits.hxx>

class ScViewData;
class ScDocument;
struct ScSubTotalParam;

class ScTpSubTotalGroup : public SfxTabPage
{
protected:
    ScTpSubTotalGroup(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet);

public:
    virtual ~ScTpSubTotalGroup() override;

    bool            DoReset         ( sal_uInt16            nGroupNo,
                                      const SfxItemSet& rArgSet  );
    bool            DoFillItemSet   ( sal_uInt16        nGroupNo,
                                      SfxItemSet&   rArgSet  );
protected:
    const OUString    aStrNone;
    const OUString    aStrColumn;

    ScViewData*             pViewData;
    ScDocument*             pDoc;

    const sal_uInt16            nWhichSubTotals;
    const ScSubTotalParam&  rSubTotalData;
    std::vector<SCCOL>      mnFieldArr;
    sal_uInt16              nFieldCount;

    std::unique_ptr<weld::ComboBox> mxLbGroup;
    std::unique_ptr<weld::TreeView> mxLbColumns;
    std::unique_ptr<weld::TreeView> mxLbFunctions;
    std::unique_ptr<weld::CheckButton> mxLbSelectAllColumns;

private:
    void            Init            ();
    void            FillListBoxes   ();
    static ScSubTotalFunc  LbPosToFunc     ( sal_uInt16 nPos );
    static sal_uInt16      FuncToLbPos     ( ScSubTotalFunc eFunc );
    sal_uInt16          GetFieldSelPos  ( SCCOL nField );

    // Handler ------------------------
    DECL_LINK( SelectListBoxHdl, weld::ComboBox&, void );
    DECL_LINK( SelectTreeListBoxHdl, weld::TreeView&, void );
    DECL_LINK(CheckHdl, const weld::TreeView::iter_col&, void);
    DECL_LINK(CheckBoxHdl, weld::Button&, void);
    void SelectHdl(const weld::Widget*);
};

class ScTpSubTotalGroup1 final : public ScTpSubTotalGroup
{
public:
    ScTpSubTotalGroup1( weld::Container* pPage, weld::DialogController* pController,
                        const SfxItemSet&    rArgSet );
    static std::unique_ptr<SfxTabPage> Create      ( weld::Container* pPage, weld::DialogController* pController,
            const SfxItemSet*     rArgSet );
    virtual ~ScTpSubTotalGroup1() override;

    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;
};

class ScTpSubTotalGroup2 final : public ScTpSubTotalGroup
{
public:
    ScTpSubTotalGroup2( weld::Container* pPage, weld::DialogController* pController,
                        const SfxItemSet&    rArgSet );
    static std::unique_ptr<SfxTabPage> Create      ( weld::Container* pPage, weld::DialogController* pController,
            const SfxItemSet*     rArgSet );
    virtual ~ScTpSubTotalGroup2() override;

    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;
};

class ScTpSubTotalGroup3 final : public ScTpSubTotalGroup
{
public:
    ScTpSubTotalGroup3( weld::Container* pPage, weld::DialogController* pController,
                        const SfxItemSet&    rArgSet );
    static  std::unique_ptr<SfxTabPage> Create      ( weld::Container* pPage, weld::DialogController* pController,
            const SfxItemSet*     rArgSet );
    virtual ~ScTpSubTotalGroup3() override;

    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;
};

class ScTpSubTotalOptions final : public SfxTabPage
{
public:
    ScTpSubTotalOptions(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rArgSet);
    static std::unique_ptr<SfxTabPage>  Create      ( weld::Container* pPage, weld::DialogController* pController,
            const SfxItemSet*     rArgSet );
    virtual ~ScTpSubTotalOptions() override;

    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;

private:
    void Init                   ();
    void FillUserSortListBox    ();

    // Handler ------------------------
    DECL_LINK(CheckHdl, weld::Button&, void);

    ScViewData*             pViewData;
    ScDocument*             pDoc;
    const sal_uInt16        nWhichSubTotals;
    const ScSubTotalParam&  rSubTotalData;

    std::unique_ptr<weld::CheckButton> m_xBtnPagebreak;
    std::unique_ptr<weld::CheckButton> m_xBtnCase;
    std::unique_ptr<weld::CheckButton> m_xBtnSort;
    std::unique_ptr<weld::Label> m_xFlSort;
    std::unique_ptr<weld::RadioButton> m_xBtnAscending;
    std::unique_ptr<weld::RadioButton> m_xBtnDescending;
    std::unique_ptr<weld::CheckButton> m_xBtnFormats;
    std::unique_ptr<weld::CheckButton> m_xBtnUserDef;
    std::unique_ptr<weld::ComboBox> m_xLbUserDef;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
