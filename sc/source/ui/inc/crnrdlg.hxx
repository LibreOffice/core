/*************************************************************************
 *
 *  $RCSfile: crnrdlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:58 $
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

#ifndef SC_CRNRDLG_HXX
#define SC_CRNRDLG_HXX

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
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


class ScViewData;
class ScDocument;


//============================================================================

class ScColRowNameRangesDlg : public ScAnyRefDlg
{
public:
                    ScColRowNameRangesDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                 ScViewData*    ptrViewData );
                    ~ScColRowNameRangesDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual BOOL    IsRefInputMode() const;
    virtual void    SetActive();
    virtual BOOL    Close();

private:
    ListBox         aLbRange;

    GroupBox        aGbAssign;
    ScRefEdit       aEdAssign;
    ScRefButton     aRbAssign;
    RadioButton     aBtnColHead;
    RadioButton     aBtnRowHead;
    FixedText       aFtAssign2;
    ScRefEdit       aEdAssign2;
    ScRefButton     aRbAssign2;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;

    ScRange         theCurArea;
    ScRange         theCurData;

    ScRangePairListRef  xColNameRanges;
    ScRangePairListRef  xRowNameRanges;

    ScViewData*     pViewData;
    ScDocument*     pDoc;
    ScRefEdit*      pEdActive;
    BOOL            bDlgLostFocus;

#ifdef _CRNRDLG_CXX
private:
    void Init               ();
    void UpdateNames        ();
    void UpdateRangeData    ( const String& rStrName, BOOL bColName );
    void SetColRowData( const ScRange& rLabelRange,BOOL bRef=FALSE);
    void AdjustColRowData( const ScRange& rDataRange,BOOL bRef=FALSE);
    DECL_LINK( CancelBtnHdl, void * );
    DECL_LINK( OkBtnHdl, void * );
    DECL_LINK( AddBtnHdl, void * );
    DECL_LINK( RemoveBtnHdl, void * );
    DECL_LINK( Range1SelectHdl, void * );
    DECL_LINK( Range1DataModifyHdl, void * );
    DECL_LINK( ColClickHdl, void * );
    DECL_LINK( RowClickHdl, void * );
    DECL_LINK( Range2DataModifyHdl, void * );
    DECL_LINK( EdGetFocusHdl, ScRefEdit* );
    DECL_LINK( EdLoseFocusHdl, ScRefEdit* );
#endif
};



#endif // SC_CRNRDLG_HXX

