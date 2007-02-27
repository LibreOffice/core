/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: condfrmt.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 13:20:34 $
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


#ifndef SC_CONDFRMT_HXX_
#define SC_CONDFRMT_HXX_

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif


class ScDocument;
class ScConditionalFormat;


//============================================================================
//  class ScConditionalFormat
//
// Dialog zum Festlegen von bedingten Formaten

class ScConditionalFormatDlg : public ScAnyRefDlg
{
public:
                    ScConditionalFormatDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                                            ScDocument* pCurDoc,
                                            const ScConditionalFormat* pCurrentFormat );
                    ~ScConditionalFormatDlg();

    virtual void    SetReference( const ScRange& rRef, ScDocument* pDoc );
    virtual void    AddRefEntry();
    virtual BOOL    IsRefInputMode() const;
    virtual void    SetActive();
    virtual BOOL    Close();

private:
    CheckBox            aCbxCond1;
    ListBox             aLbCond11;
    ListBox             aLbCond12;
    ScRefEdit           aEdtCond11;
    ScRefButton         aRbCond11;
    FixedText           aFtCond1And;
    ScRefEdit           aEdtCond12;
    ScRefButton         aRbCond12;
    FixedText           aFtCond1Template;
    ListBox             aLbCond1Template;
    FixedLine           aFlSep1;

    CheckBox            aCbxCond2;
    ListBox             aLbCond21;
    ListBox             aLbCond22;
    ScRefEdit           aEdtCond21;
    ScRefButton         aRbCond21;
    FixedText           aFtCond2And;
    ScRefEdit           aEdtCond22;
    ScRefButton         aRbCond22;
    FixedText           aFtCond2Template;
    ListBox             aLbCond2Template;
    FixedLine           aFlSep2;

    CheckBox            aCbxCond3;
    ListBox             aLbCond31;
    ListBox             aLbCond32;
    ScRefEdit           aEdtCond31;
    ScRefButton         aRbCond31;
    FixedText           aFtCond3And;
    ScRefEdit           aEdtCond32;
    ScRefButton         aRbCond32;
    FixedText           aFtCond3Template;
    ListBox             aLbCond3Template;

    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    Point               aCond1Pos1;
    Point               aCond1Pos2;
    Point               aRBtn1Pos1;
    Point               aRBtn1Pos2;
    Size                aCond1Size1;
    Size                aCond1Size2;
    Size                aCond1Size3;

    Point               aCond2Pos1;
    Point               aCond2Pos2;
    Point               aRBtn2Pos1;
    Point               aRBtn2Pos2;
    Size                aCond2Size1;
    Size                aCond2Size2;
    Size                aCond2Size3;

    Point               aCond3Pos1;
    Point               aCond3Pos2;
    Point               aRBtn3Pos1;
    Point               aRBtn3Pos2;
    Size                aCond3Size1;
    Size                aCond3Size2;
    Size                aCond3Size3;

    ScRefEdit*          pEdActive;
    BOOL                bDlgLostFocus;
    ScDocument*         pDoc;

#ifdef _CONDFRMT_CXX
    void    GetConditionalFormat( ScConditionalFormat& rCndFmt );

    DECL_LINK( ClickCond1Hdl, void * );
    DECL_LINK( ChangeCond11Hdl, void * );
    DECL_LINK( ChangeCond12Hdl, void * );

    DECL_LINK( ClickCond2Hdl, void * );
    DECL_LINK( ChangeCond21Hdl, void * );
    DECL_LINK( ChangeCond22Hdl, void * );

    DECL_LINK( ClickCond3Hdl, void * );
    DECL_LINK( ChangeCond31Hdl, void * );
    DECL_LINK( ChangeCond32Hdl, void * );

    DECL_LINK( GetFocusHdl, Control* );
    DECL_LINK( LoseFocusHdl, Control* );
    DECL_LINK( BtnHdl, PushButton* );
#endif // _CONDFRMT_CXX
};

#endif // SC_CONDFRMT_HXX_


