/*************************************************************************
 *
 *  $RCSfile: tpsubt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-04 09:16:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

//------------------------------------------------------------------------

#define SC_MAXFIELDS    200

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
    USHORT                  nFieldArr[SC_MAXFIELDS];
    const USHORT            nFieldCount;

private:
    void            Init            ();
    void            FillListBoxes   ();
    ScSubTotalFunc  LbPosToFunc     ( USHORT nPos );
    USHORT          FuncToLbPos     ( ScSubTotalFunc eFunc );
    USHORT          GetFieldSelPos  ( USHORT nField );

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
    CheckBox    aBtnPagebreak;
    CheckBox    aBtnCase;
    CheckBox    aBtnSort;
    CheckBox    aBtnFormats;
    CheckBox    aBtnUserDef;
    ListBox     aLbUserDef;
    RadioButton aBtnAscending;
    RadioButton aBtnDescending;
    GroupBox    aGbOptions;

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

