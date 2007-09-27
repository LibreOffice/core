/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: instable.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:03:15 $
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
#ifndef _INSTABLE_HXX
#define _INSTABLE_HXX

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _ACTCTRL_HXX
#include <actctrl.hxx>
#endif

#ifndef _TEXTCONTROLCOMBO_HXX
#include <textcontrolcombo.hxx>
#endif

class SwWrtShell;
class SwTableAutoFmt;
class SwView;
struct SwInsertTableOptions;


class SwInsTableDlg : public SfxModalDialog
{
    FixedText       aNameFT;
    TableNameEdit   aNameEdit;

    FixedLine       aFL;
    FixedText       aColLbl;
    NumericField    aColEdit;
    FixedText       aRowLbl;
    NumericField    aRowEdit;

    FixedLine       aOptionsFL;
    CheckBox        aHeaderCB;
    CheckBox        aRepeatHeaderCB;
    FixedText       aRepeatHeaderFT;    // "dummy" to build before and after FT
    FixedText       aRepeatHeaderBeforeFT;
    NumericField    aRepeatHeaderNF;
    FixedText       aRepeatHeaderAfterFT;
    TextControlCombo    aRepeatHeaderCombo;

    CheckBox        aDontSplitCB;
    CheckBox        aBorderCB;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;
    PushButton      aAutoFmtBtn;

    SwWrtShell*     pShell;
    SwTableAutoFmt* pTAutoFmt;
    sal_Int64       nEnteredValRepeatHeaderNF;

    DECL_LINK( ModifyName, Edit * );
    DECL_LINK( ModifyRowCol, NumericField * );
    DECL_LINK( AutoFmtHdl, PushButton* );
    DECL_LINK( CheckBoxHdl, CheckBox *pCB = 0 );
    DECL_LINK( ReapeatHeaderCheckBoxHdl, void* p = 0 );
    DECL_LINK( ModifyRepeatHeaderNF_Hdl, void* p = 0 );

public:
    SwInsTableDlg( SwView& rView );
    ~SwInsTableDlg();

    void GetValues( String& rName, USHORT& rRow, USHORT& rCol,
                    SwInsertTableOptions& rInsTblOpts, String& rTableAutoFmtName,
                    SwTableAutoFmt *& prTAFmt );
};

#endif
