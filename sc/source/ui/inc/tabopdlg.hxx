/*************************************************************************
 *
 *  $RCSfile: tabopdlg.hxx,v $
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

#ifndef SC_TABOPDLG_HXX
#define SC_TABOPDLG_HXX

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif


//------------------------------------------------------------------------

enum ScTabOpErr
{
    TABOPERR_NOFORMULA = 1,
    TABOPERR_NOCOLROW,
    TABOPERR_WRONGFORMULA,
    TABOPERR_WRONGROW,
    TABOPERR_NOCOLFORMULA,
    TABOPERR_WRONGCOL,
    TABOPERR_NOROWFORMULA
};

//========================================================================

class ScTabOpDlg : public ScAnyRefDlg
{
public:
                    ScTabOpDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                ScDocument*     pDocument,
                                const ScRefTripel&  rCursorPos );
                    ~ScTabOpDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual BOOL    IsRefInputMode() const { return TRUE; }
    virtual void    SetActive();

    virtual BOOL    Close();

private:
    FixedText       aFtFormulaRange;
    ScRefEdit       aEdFormulaRange;
    ScRefButton     aRBFormulaRange;

    FixedText       aFtRowCell;
    ScRefEdit       aEdRowCell;
    ScRefButton     aRBRowCell;

    FixedText       aFtColCell;
    ScRefEdit       aEdColCell;
    ScRefButton     aRBColCell;

    GroupBox        aGbVariables;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;

    ScRefTripel     theFormulaCell;
    ScRefTripel     theFormulaEnd;
    ScRefTripel     theRowCell;
    ScRefTripel     theColCell;

    ScDocument*     pDoc;
    const USHORT    nCurTab;
    ScRefEdit*      pEdActive;
    BOOL            bDlgLostFocus;
    const String    errMsgNoFormula;
    const String    errMsgNoColRow;
    const String    errMsgWrongFormula;
    const String    errMsgWrongRowCol;
    const String    errMsgNoColFormula;
    const String    errMsgNoRowFormula;

#ifdef _TABOPDLG_CXX
    void    Init();
    void    RaiseError( ScTabOpErr eError );

    DECL_LINK( BtnHdl, PushButton* );
    DECL_LINK( EdGetFocusHdl, ScRefEdit* );
    DECL_LINK( EdLoseFocusHdl, ScRefEdit* );
#endif  // _TABOPDLG_CXX
};

#endif // SC_TABOPDLG_HXX



