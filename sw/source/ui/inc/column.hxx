/*************************************************************************
 *
 *  $RCSfile: column.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ma $ $Date: 2001-03-23 16:17:47 $
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

#ifndef _COLUMN_HXX
#define _COLUMN_HXX


#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _IMAGE_HXX //autogen
#include <vcl/image.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif
#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _CTRLBOX_HXX
#include <svtools/ctrlbox.hxx>
#endif
#ifndef _VALUESET_HXX //autogen
#include <svtools/valueset.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _COLEX_HXX
#include <colex.hxx>
#endif
#ifndef _PRCNTFLD_HXX
#include <prcntfld.hxx>
#endif

const nMaxCols = 99;
class SwColMgr;
class SwWrtShell;
class SwColumnPage;
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class SwColumnDlg : public SfxModalDialog
{
    OKButton            aOK;
    CancelButton        aCancel;
    HelpButton          aHelp;

    FixedText           aApplyToFT;
    ListBox             aApplyToLB;

    SwWrtShell&         rWrtShell;
    SwColumnPage*       pTabPage;
    SfxItemSet*         pPageSet;
    SfxItemSet*         pSectionSet;
    SfxItemSet*         pSelectionSet;
    SfxItemSet*         pFrameSet;

    long                nOldSelection;
    long                nSelectionWidth;
    long                nPageWidth;

    BOOL                bPageChanged : 1;
    BOOL                bSectionChanged : 1;
    BOOL                bSelSectionChanged : 1;
    BOOL                bFrameChanged : 1;


    DECL_LINK(ObjectHdl, ListBox*);
    DECL_LINK(OkHdl, OKButton*);

public:
    SwColumnDlg(Window* pParent, SwWrtShell& rSh);
    virtual ~SwColumnDlg();

    SwWrtShell&     GetWrtShell()   { return rWrtShell; }
};

/*-----------------07.03.97 08.26-------------------

--------------------------------------------------*/
class ColumnValueSet : public ValueSet
{
    public:
        ColumnValueSet(Window* pParent, const ResId& rResId) :
            ValueSet(pParent, rResId){}
        ~ColumnValueSet();

    virtual void    UserDraw( const UserDrawEvent& rUDEvt );
};
/*--------------------------------------------------------------------
    Beschreibung:   Spaltendialog jetzt als TabPage
 --------------------------------------------------------------------*/
class SwColumnPage : public SfxTabPage
{
    FixedText       aClNrLbl;
    NumericField    aCLNrEdt;
    ColumnValueSet  aDefaultVS;
    ImageList       aPreColsIL;
    CheckBox        aBalanceColsCB;
    FixedLine       aFLGroup;

    ImageButton     aBtnUp;
    FixedText       aWidthFT;
    FixedText       aDistFT;
    FixedText       aLbl1;
    PercentField    aEd1;
    PercentField    aDistEd1;
    FixedText       aLbl2;
    PercentField    aEd2;
    PercentField    aDistEd2;
    FixedText       aLbl3;
    PercentField    aEd3;
    ImageButton     aBtnDown;
    CheckBox        aAutoWidthBox;

    FixedLine       aFLLayout;

    FixedText       aLineTypeLbl;
    LineListBox     aLineTypeDLB;
    FixedText       aLineHeightLbl;
    MetricField     aLineHeightEdit;
    FixedText       aLinePosLbl;
    ListBox         aLinePosDLB;
    FixedLine       aFLLineType;

    // Example
    SwColExample        aPgeExampleWN;
    SwColumnOnlyExample aFrmExampleWN;
    FixedLine        aFLExample;

    SwColMgr*       pColMgr;

    USHORT          nFirstVis;
    USHORT          nCols;
    long            nColWidth[nMaxCols];
    long            nColDist[nMaxCols];
    USHORT          nMinWidth;
    PercentField    *pModifiedField;
    BOOL            bFormat;
    BOOL            bFrm;
    BOOL            bHtmlMode;
    BOOL            bLockUpdate;

    // Handler
    DECL_LINK( ColModify, NumericField * );
    DECL_LINK( GapModify, PercentField * );
    DECL_LINK( EdModify, PercentField * );
    DECL_LINK( AutoWidthHdl, CheckBox * );
    DECL_LINK( Timeout, Timer * );
    DECL_LINK( SetDefaultsHdl, ValueSet * );

    DECL_LINK( Up, Button * );
    DECL_LINK( Down, Button * );
    void            Apply(Button *);
    DECL_LINK( UpdateColMgr, void* );

    void            Update();
    void            UpdateCols();
    void            Init();
    void            ResetColWidth();
    void            SetLabels( USHORT nVis );

    virtual void    ActivatePage(const SfxItemSet& rSet);
    virtual int     DeactivatePage(SfxItemSet *pSet);

    SwColumnPage(Window *pParent, const SfxItemSet &rSet);

public:
    ~SwColumnPage();

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    static USHORT* GetRanges();

    virtual BOOL    FillItemSet(SfxItemSet &rSet);
    virtual void    Reset(const SfxItemSet &rSet);

    void SetFrmMode(BOOL bMod);
    void SetPageWidth(long nPageWidth);

    void SetFormatUsed(BOOL bFmt) { bFormat = bFmt; }

    void ShowBalance(BOOL bShow) {aBalanceColsCB.Show(bShow);}

};

#endif

