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

    bool            DoReset         ( sal_uInt16            nGroupNo,
                                      const SfxItemSet& rArgSet  );
    bool            DoFillItemSet   ( sal_uInt16        nGroupNo,
                                      SfxItemSet&   rArgSet  );
protected:
    ListBox*        mpLbGroup;
    SvxCheckListBox* mpLbColumns;
    ListBox*         mpLbFunctions;
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
    ScSubTotalFunc  LbPosToFunc     ( sal_uInt16 nPos );
    sal_uInt16          FuncToLbPos     ( ScSubTotalFunc eFunc );
    sal_uInt16          GetFieldSelPos  ( SCCOL nField );

    // Handler ------------------------
    DECL_LINK( SelectHdl, ListBox * );
    DECL_LINK( CheckHdl, ListBox * );
};

class ScTpSubTotalGroup1 : public ScTpSubTotalGroup
{
protected:
            ScTpSubTotalGroup1( vcl::Window*              pParent,
                                const SfxItemSet&    rArgSet );

public:
    virtual ~ScTpSubTotalGroup1();

    static  SfxTabPage* Create      ( vcl::Window*               pParent,
                                      const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) SAL_OVERRIDE;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) SAL_OVERRIDE;
};

class ScTpSubTotalGroup2 : public ScTpSubTotalGroup
{
protected:
            ScTpSubTotalGroup2( vcl::Window*              pParent,
                                const SfxItemSet&    rArgSet );

public:
    virtual ~ScTpSubTotalGroup2();

    static  SfxTabPage* Create      ( vcl::Window*               pParent,
                                      const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) SAL_OVERRIDE;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) SAL_OVERRIDE;
};

class ScTpSubTotalGroup3 : public ScTpSubTotalGroup
{
protected:
            ScTpSubTotalGroup3( vcl::Window*              pParent,
                                const SfxItemSet&    rArgSet );

public:
    virtual ~ScTpSubTotalGroup3();

    static  SfxTabPage* Create      ( vcl::Window*               pParent,
                                      const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) SAL_OVERRIDE;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) SAL_OVERRIDE;
};

class ScTpSubTotalOptions : public SfxTabPage
{
protected:
            ScTpSubTotalOptions( vcl::Window*             pParent,
                                  const SfxItemSet&  rArgSet );

public:
    virtual ~ScTpSubTotalOptions();

    static SfxTabPage*  Create      ( vcl::Window*               pParent,
                                      const SfxItemSet*     rArgSet );
    virtual bool        FillItemSet ( SfxItemSet* rArgSet ) SAL_OVERRIDE;
    virtual void        Reset       ( const SfxItemSet* rArgSet ) SAL_OVERRIDE;

private:
    CheckBox*    pBtnPagebreak;
    CheckBox*    pBtnCase;
    CheckBox*    pBtnSort;
    FixedText*   pFlSort;
    RadioButton* pBtnAscending;
    RadioButton* pBtnDescending;
    CheckBox*    pBtnFormats;
    CheckBox*    pBtnUserDef;
    ListBox*     pLbUserDef;

    ScViewData*             pViewData;
    ScDocument*             pDoc;
    const sal_uInt16            nWhichSubTotals;
    const ScSubTotalParam&  rSubTotalData;

private:
    void Init                   ();
    void FillUserSortListBox    ();

    // Handler ------------------------
    DECL_LINK( CheckHdl, CheckBox * );
};

#endif // INCLUDED_SC_SOURCE_UI_INC_TPSUBT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
