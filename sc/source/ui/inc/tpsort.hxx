/*************************************************************************
 *
 *  $RCSfile: tpsort.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:01 $
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

#ifndef SC_TPSORT_HXX
#define SC_TPSORT_HXX


#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif


#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

//------------------------------------------------------------------------

#define SC_MAXFIELDS    200

class ScViewData;
class ScSortDlg;

//========================================================================
// Kriterien

class ScTabPageSortFields : public SfxTabPage
{
public:
                ScTabPageSortFields( Window*             pParent,
                                     const SfxItemSet&   rArgSet );
                ~ScTabPageSortFields();

    static  SfxTabPage* Create      ( Window*               pParent,
                                      const SfxItemSet&     rArgSet );
    static  USHORT*     GetRanges   ();
    virtual BOOL        FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );

protected:
// fuer Datenaustausch (sollte noch umgestellt werden!)
//  virtual void        ActivatePage    ( const SfxItemSet& rSet );
    virtual void        ActivatePage    ();
    virtual int         DeactivatePage  ( SfxItemSet* pSet = 0);

private:
    ListBox         aLbSort1;
    RadioButton     aBtnUp1;
    RadioButton     aBtnDown1;
    GroupBox        aGbSort1;

    ListBox         aLbSort2;
    RadioButton     aBtnUp2;
    RadioButton     aBtnDown2;
    GroupBox        aGbSort2;

    ListBox         aLbSort3;
    RadioButton     aBtnUp3;
    RadioButton     aBtnDown3;
    GroupBox        aGbSort3;

    String          aStrUndefined;
    String          aStrColumn;
    String          aStrRow;

    const USHORT        nWhichSort;
    ScSortDlg*          pDlg;
    ScViewData*         pViewData;
    const ScSortParam&  rSortData;
    USHORT              nFieldArr[SC_MAXFIELDS];
    USHORT              nFieldCount;
    USHORT              nFirstCol;
    USHORT              nFirstRow;
    BOOL                bHasHeader;
    BOOL                bSortByRows;

    ListBox*            aSortLbArr[3];
    RadioButton*        aDirBtnArr[3][2];
    GroupBox*           aGbArr[3];

#ifdef _TPSORT_CXX
private:
    void    Init            ();
    void    DisableField    ( USHORT nField );
    void    EnableField     ( USHORT nField );
    void    FillFieldLists  ();
    USHORT  GetFieldSelPos  ( USHORT nField );

    // Handler ------------------------
    DECL_LINK( SelectHdl, ListBox * );
#endif
};

//========================================================================
// Sortieroptionen:

class ScDocument;
class ScRangeData;

class ScTabPageSortOptions : public SfxTabPage
{
public:
                ScTabPageSortOptions( Window*            pParent,
                                      const SfxItemSet&  rArgSet );
                ~ScTabPageSortOptions();

    static  SfxTabPage* Create      ( Window*               pParent,
                                      const SfxItemSet&     rArgSet );
    static  USHORT*     GetRanges   ();
    virtual BOOL        FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );

protected:
// fuer Datenaustausch (sollte noch umgestellt werden!)
//  virtual void        ActivatePage    ( const SfxItemSet& rSet );
    virtual void        ActivatePage    ();
    virtual int         DeactivatePage  ( SfxItemSet* pSet = 0);

private:

    CheckBox    aBtnCase;
    CheckBox    aBtnHeader;
    CheckBox    aBtnFormats;

    CheckBox    aBtnCopyResult;
    ListBox     aLbOutPos;
    Edit        aEdOutPos;

    CheckBox    aBtnSortUser;
    ListBox     aLbSortUser;

    RadioButton aBtnTopDown;
    RadioButton aBtnLeftRight;
    GroupBox    aGbDirection;
    FixedText   aFtAreaLabel;
    FixedInfo   aFtArea;
    String      aStrRowLabel;
    String      aStrColLabel;
    String      aStrUndefined;
    String      aStrNoName;

    const USHORT        nWhichSort;
    const ScSortParam&  rSortData;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    ScSortDlg*          pDlg;
    ScAddress           theOutPos;

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
#endif
};



#endif // SC_TPSORT_HXX

