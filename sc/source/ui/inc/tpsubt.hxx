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

#ifndef INCLUDED_SC_SOURCE_UI_INC_TPSUBT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_TPSUBT_HXX

#include <sfx2/tabdlg.hxx>
#include <svx/checklbx.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include "global.hxx"

// +1 because one field is reserved for the "- none -" entry
#define SC_MAXFIELDS    MAXCOLCOUNT+1

class ScViewData;
class ScDocument;
struct ScSubTotalParam;

class ScTpSubTotalGroup : public SfxTabPage
{
protected:
    ScTpSubTotalGroup( vcl::Window* pParent,
                       const SfxItemSet& rArgSet );

public:
    virtual ~ScTpSubTotalGroup();
    virtual void dispose() override;

    bool            DoReset         ( sal_uInt16            nGroupNo,
                                      const SfxItemSet& rArgSet  );
    bool            DoFillItemSet   ( sal_uInt16        nGroupNo,
                                      SfxItemSet&   rArgSet  );
protected:
    VclPtr<ListBox>        mpLbGroup;
    VclPtr<SvxCheckListBox> mpLbColumns;
    VclPtr<ListBox>         mpLbFunctions;
    const OUString    aStrNone;
    const OUString    aStrColumn;

    ScViewData*             pViewData;
    ScDocument*             pDoc;

    const sal_uInt16            nWhichSubTotals;
    const ScSubTotalParam&  rSubTotalData;
    SCCOL                   nFieldArr[SC_MAXFIELDS];
    const sal_uInt16            nFieldCount;

private:
    void            Init            ();
    void            FillListBoxes   ();
    static ScSubTotalFunc  LbPosToFunc     ( sal_uInt16 nPos );
    static sal_uInt16      FuncToLbPos     ( ScSubTotalFunc eFunc );
    sal_uInt16          GetFieldSelPos  ( SCCOL nField );

    // Handler ------------------------
    DECL_LINK_TYPED( SelectListBoxHdl, ListBox&, void );
    DECL_LINK_TYPED( SelectTreeListBoxHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED( CheckHdl, SvTreeListBox*, void );
    void SelectHdl(void *);
};

class ScTpSubTotalGroup1 : public ScTpSubTotalGroup
{
    friend class VclPtr<ScTpSubTotalGroup1>;
protected:
    ScTpSubTotalGroup1( vcl::Window*              pParent,
                        const SfxItemSet&    rArgSet );

public:
    virtual ~ScTpSubTotalGroup1();

    static  VclPtr<SfxTabPage> Create      ( vcl::Window*               pParent,
            const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;
};

class ScTpSubTotalGroup2 : public ScTpSubTotalGroup
{
    friend class VclPtr<ScTpSubTotalGroup2>;
protected:
    ScTpSubTotalGroup2( vcl::Window*              pParent,
                        const SfxItemSet&    rArgSet );

public:
    virtual ~ScTpSubTotalGroup2();

    static  VclPtr<SfxTabPage> Create      ( vcl::Window*               pParent,
            const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;
};

class ScTpSubTotalGroup3 : public ScTpSubTotalGroup
{
    friend class VclPtr<ScTpSubTotalGroup3>;
protected:
    ScTpSubTotalGroup3( vcl::Window*              pParent,
                        const SfxItemSet&    rArgSet );

public:
    virtual ~ScTpSubTotalGroup3();

    static  VclPtr<SfxTabPage> Create      ( vcl::Window*               pParent,
            const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;
};

class ScTpSubTotalOptions : public SfxTabPage
{
    friend class VclPtr<ScTpSubTotalOptions>;
protected:
    ScTpSubTotalOptions( vcl::Window*             pParent,
                         const SfxItemSet&  rArgSet );

public:
    virtual ~ScTpSubTotalOptions();
    virtual void        dispose() override;
    static VclPtr<SfxTabPage>  Create      ( vcl::Window*               pParent,
            const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) override;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) override;

private:
    VclPtr<CheckBox>    pBtnPagebreak;
    VclPtr<CheckBox>    pBtnCase;
    VclPtr<CheckBox>    pBtnSort;
    VclPtr<FixedText>   pFlSort;
    VclPtr<RadioButton> pBtnAscending;
    VclPtr<RadioButton> pBtnDescending;
    VclPtr<CheckBox>    pBtnFormats;
    VclPtr<CheckBox>    pBtnUserDef;
    VclPtr<ListBox>     pLbUserDef;

    ScViewData*             pViewData;
    ScDocument*             pDoc;
    const sal_uInt16            nWhichSubTotals;
    const ScSubTotalParam&  rSubTotalData;

private:
    void Init                   ();
    void FillUserSortListBox    ();

    // Handler ------------------------
    DECL_LINK_TYPED( CheckHdl, Button*, void );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_TPSUBT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
