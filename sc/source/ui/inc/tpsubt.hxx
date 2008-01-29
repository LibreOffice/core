/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tpsubt.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:43:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_TPSUBT_HXX
#define SC_TPSUBT_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SVX_CHECKLBX_HXX //autogen
#include <svx/checklbx.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

//------------------------------------------------------------------------

// +1 because one field is reserved for the "- none -" entry
#define SC_MAXFIELDS    MAXCOLCOUNT+1

class ScViewData;
class ScDocument;

//========================================================================
// Gruppenseiten: Basisklasse

class ScTpSubTotalGroup : public SfxTabPage
{
protected:
            ScTpSubTotalGroup( Window* pParent, USHORT nResId,
                               const SfxItemSet& rArgSet );

public:
    virtual ~ScTpSubTotalGroup();

    static USHORT*  GetRanges       ();
    BOOL            DoReset         ( USHORT            nGroupNo,
                                      const SfxItemSet& rArgSet  );
    BOOL            DoFillItemSet   ( USHORT        nGroupNo,
                                      SfxItemSet&   rArgSet  );
protected:
    FixedText       aFtGroup;
    ListBox         aLbGroup;
    FixedText       aFtColumns;
    SvxCheckListBox aLbColumns;
    FixedText       aFtFunctions;
    ListBox         aLbFunctions;
    const String    aStrNone;
    const String    aStrColumn;

    ScViewData*             pViewData;
    ScDocument*             pDoc;

    const USHORT            nWhichSubTotals;
    const ScSubTotalParam&  rSubTotalData;
    SCCOL                   nFieldArr[SC_MAXFIELDS];
    const USHORT            nFieldCount;

private:
    void            Init            ();
    void            FillListBoxes   ();
    ScSubTotalFunc  LbPosToFunc     ( USHORT nPos );
    USHORT          FuncToLbPos     ( ScSubTotalFunc eFunc );
    USHORT          GetFieldSelPos  ( SCCOL nField );

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
    virtual BOOL        FillItemSet ( SfxItemSet& rArgSet );
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
    virtual BOOL        FillItemSet ( SfxItemSet& rArgSet );
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
    virtual BOOL        FillItemSet ( SfxItemSet& rArgSet );
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

    static USHORT*      GetRanges   ();
    static SfxTabPage*  Create      ( Window*               pParent,
                                      const SfxItemSet&     rArgSet );
    virtual BOOL        FillItemSet ( SfxItemSet& rArgSet );
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
    const USHORT            nWhichSubTotals;
    const ScSubTotalParam&  rSubTotalData;

private:
    void Init                   ();
    void FillUserSortListBox    ();

    // Handler ------------------------
    DECL_LINK( CheckHdl, CheckBox * );
};



#endif // SC_TPSORT_HXX

