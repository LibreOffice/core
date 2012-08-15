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

#ifndef SC_TPSUBT_HXX
#define SC_TPSUBT_HXX

#include <sfx2/tabdlg.hxx>
#include <svx/checklbx.hxx>
#include <vcl/fixed.hxx>
#include "global.hxx"

//------------------------------------------------------------------------

// +1 because one field is reserved for the "- none -" entry
#define SC_MAXFIELDS    MAXCOLCOUNT+1

class ScViewData;
class ScDocument;
struct ScSubTotalParam;

//========================================================================
// Gruppenseiten: Basisklasse

class ScTpSubTotalGroup : public SfxTabPage
{
protected:
            ScTpSubTotalGroup( Window* pParent, sal_uInt16 nResId,
                               const SfxItemSet& rArgSet );

public:
    virtual ~ScTpSubTotalGroup();

    bool            DoReset         ( sal_uInt16            nGroupNo,
                                      const SfxItemSet& rArgSet  );
    bool            DoFillItemSet   ( sal_uInt16        nGroupNo,
                                      SfxItemSet&   rArgSet  );
protected:
    FixedText       aFtGroup;
    ListBox         aLbGroup;
    FixedText       aFtColumns;
    SvxCheckListBox aLbColumns;
    FixedText       aFtFunctions;
    ListBox         aLbFunctions;
    const rtl::OUString    aStrNone;
    const rtl::OUString    aStrColumn;

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

//------------------------------------------------------------------------

class ScTpSubTotalGroup1 : public ScTpSubTotalGroup
{
protected:
            ScTpSubTotalGroup1( Window*              pParent,
                                const SfxItemSet&    rArgSet );

public:
    virtual ~ScTpSubTotalGroup1();

    static  SfxTabPage* Create      ( Window*               pParent,
                                      const SfxItemSet&     rArgSet );
    virtual sal_Bool    FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );
};

//------------------------------------------------------------------------

class ScTpSubTotalGroup2 : public ScTpSubTotalGroup
{
protected:
            ScTpSubTotalGroup2( Window*              pParent,
                                const SfxItemSet&    rArgSet );

public:
    virtual ~ScTpSubTotalGroup2();

    static  SfxTabPage* Create      ( Window*               pParent,
                                      const SfxItemSet&     rArgSet );
    virtual sal_Bool    FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );
};

//------------------------------------------------------------------------

class ScTpSubTotalGroup3 : public ScTpSubTotalGroup
{
protected:
            ScTpSubTotalGroup3( Window*              pParent,
                                const SfxItemSet&    rArgSet );

public:
    virtual ~ScTpSubTotalGroup3();

    static  SfxTabPage* Create      ( Window*               pParent,
                                      const SfxItemSet&     rArgSet );
    virtual sal_Bool    FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );
};

//========================================================================
// Optionen:

class ScTpSubTotalOptions : public SfxTabPage
{
protected:
            ScTpSubTotalOptions( Window*             pParent,
                                  const SfxItemSet&  rArgSet );

public:
    virtual ~ScTpSubTotalOptions();

    static SfxTabPage*  Create      ( Window*               pParent,
                                      const SfxItemSet&     rArgSet );
    virtual sal_Bool    FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );

private:
    FixedLine   aFlGroup;
    CheckBox    aBtnPagebreak;
    CheckBox    aBtnCase;
    CheckBox    aBtnSort;
    FixedLine   aFlSort;
    RadioButton aBtnAscending;
    RadioButton aBtnDescending;
    CheckBox    aBtnFormats;
    CheckBox    aBtnUserDef;
    ListBox     aLbUserDef;

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



#endif // SC_TPSORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
