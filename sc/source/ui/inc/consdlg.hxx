/*************************************************************************
 *
 *  $RCSfile: consdlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:57 $
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

#ifndef SC_CONSDLG_HXX
#define SC_CONSDLG_HXX

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif

class ScViewData;
class ScDocument;
class ScRangeUtil;
class ScAreaData;


//============================================================================

class ScConsolidateDlg : public ScAnyRefDlg
{
public:
                    ScConsolidateDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                      const SfxItemSet& rArgSet );
                    ~ScConsolidateDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );

    virtual BOOL    IsRefInputMode() const { return TRUE; }
    virtual void    SetActive();

    virtual BOOL    Close();

protected:
    virtual void    Deactivate();

private:
    FixedText       aFtFunc;
    ListBox         aLbFunc;

    FixedText       aFtConsAreas;
    MultiListBox    aLbConsAreas;

    ListBox         aLbDataArea;
    FixedText       aFtDataArea;
    ScRefEdit       aEdDataArea;
    ScRefButton     aRbDataArea;

    ListBox         aLbDestArea;
    FixedText       aFtDestArea;
    ScRefEdit       aEdDestArea;
    ScRefButton     aRbDestArea;

    CheckBox        aBtnByRow;
    CheckBox        aBtnByCol;

    GroupBox        aGbConsBy;
    CheckBox        aBtnRefs;

    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    MoreButton      aBtnMore;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;

    String          aStrUndefined;

    ScConsolidateParam  theConsData;
    ScViewData*         pViewData;
    ScDocument*         pDoc;
    ScRangeUtil*        pRangeUtil;
    ScAreaData*         pAreaData;
    USHORT              nAreaDataCount;
    USHORT              nWhichCons;

    ScRefEdit*          pRefInputEdit;
    BOOL                bDlgLostFocus;

#ifdef _CONSDLG_CXX
private:
    void Init               ();
    void FillAreaLists      ();
    BOOL VerifyEdit         ( ScRefEdit* pEd );

    DECL_LINK( OkHdl,        void* );
    DECL_LINK( ClickHdl,     PushButton* );
    DECL_LINK( GetFocusHdl, Control* );
    DECL_LINK( ModifyHdl,    ScRefEdit* );
    DECL_LINK( SelectHdl,    ListBox* );

    ScSubTotalFunc  LbPosToFunc( USHORT nPos );
    USHORT          FuncToLbPos( ScSubTotalFunc eFunc );
#endif // _CONSDLG_CXX
};



#endif // SC_CONSDLG_HXX

