/*************************************************************************
 *
 *  $RCSfile: chartins.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:39 $
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
/*
/*
/*  (C) 1995 - 2000 StarDivision GmbH, Hamburg, Germany
/*  $Author: hr $ $Date: 2000-09-18 17:14:39 $ $Revision: 1.1.1.1 $
/*  $Logfile:   T:/sw/source/ui/inc/chartins.hxv  $ $Workfile:   CHARTINS.HXX  $
/*
/*  Dialog Einfuegen StarChart
/*
/*------------------------------------------------------------------ */

#ifndef _CHARTINS_HXX
#define _CHARTINS_HXX

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifdef _CHARTINS_CXX

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#include "autoedit.hxx"

class SfxItemSet;
class SwWrtShell;
class SchMemChart;

//========================================================================

class SwInsertChartDlg : public SfxModelessDialog
{
    FixedText       aTextFt;
    FixedText       aRangeFt;
    AutoEdit        aRangeEd;
    HelpButton      aHelp;
    CancelButton    aCancel;
    FixedText       aHLine;
    PushButton      aPrev;
    PushButton      aNext;
    PushButton      aFinish;
    CheckBox        aFirstRow;
    CheckBox        aFirstCol;
    GroupBox        aGroup1;
    SchMemChart *   pChartData;
    ModalDialog*    pChartDlg;
    SfxItemSet*     pInItemSet;
    SfxItemSet*     pOutItemSet;
    SwWrtShell*     pWrtShell;
    String          aAktTableName;
    BOOL            bUpdateChartData;
    BOOL            bChartInserted;

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

