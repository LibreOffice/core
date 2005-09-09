/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chartins.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:03:54 $
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
#ifndef _CHARTINS_HXX
#define _CHARTINS_HXX

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifdef _CHARTINS_CXX

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#include "autoedit.hxx"

class SfxItemSet;
class SwWrtShell;
class SchMemChart;

//========================================================================

class SwInsertChartDlg : public SfxModelessDialog
{
    FixedLine       aFL1;
    CheckBox        aFirstRow;
    CheckBox        aFirstCol;
    FixedText       aRangeFt;
    AutoEdit        aRangeEd;

    FixedInfo       aTextFt;

    HelpButton      aHelp;
    CancelButton    aCancel;

    FixedLine       aHLine;
    PushButton      aPrev;
    PushButton      aNext;
    PushButton      aFinish;

    SchMemChart *   pChartData;
    ModalDialog*    pChartDlg;
    SfxItemSet*     pInItemSet;
    SfxItemSet*     pOutItemSet;
    SwWrtShell*     pWrtShell;
    String          aAktTableName;
    BOOL            bUpdateChartData;
    BOOL            bChartInserted;
    BOOL            bChildOpen;

    void UpdateData();
    virtual BOOL        Close();

public:
    SwInsertChartDlg( SfxBindings*, SfxChildWindow*,
                      Window *pParent, SwWrtShell * );
    ~SwInsertChartDlg();

    DECL_LINK( SelTblCellsNotify, SwWrtShell * );
    DECL_LINK( ModifyHdl, Edit* );
    DECL_LINK( NextHdl, Button* );
    DECL_LINK( FinishHdl, Button* );
    DECL_LINK( CloseHdl, Button* );
    DECL_LINK( ClickHdl, CheckBox* );

    virtual void    Activate();
};
#endif

class SwInsertChartChild : public SfxChildWindow
{
public:
    SwInsertChartChild( Window* ,
                        USHORT nId,
                        SfxBindings*,
                        SfxChildWinInfo*  );
    SFX_DECL_CHILDWINDOW( SwInsertChartChild );
};


#endif

