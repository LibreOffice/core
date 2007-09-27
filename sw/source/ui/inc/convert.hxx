/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: convert.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:56:31 $
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
#ifndef _SW_CONVERT_HXX
#define _SW_CONVERT_HXX

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _TEXTCONTROLCOMBO_HXX
#include <textcontrolcombo.hxx>
#endif

class SwTableAutoFmt;
class SwView;
class SwWrtShell;
class NumericField;
struct SwInsertTableOptions;

class SwConvertTableDlg: public SfxModalDialog
{
    RadioButton     aTabBtn;
    RadioButton     aSemiBtn;
    RadioButton     aParaBtn;
    RadioButton     aOtherBtn;
    Edit            aOtherEd;
    CheckBox        aKeepColumn;
    FixedLine       aDelimFL;

    CheckBox        aHeaderCB;
    CheckBox        aRepeatHeaderCB;

    FixedText       aRepeatHeaderFT;    // "dummy" to build before and after FT
    FixedText           aRepeatHeaderBeforeFT;
    NumericField        aRepeatHeaderNF;
    FixedText           aRepeatHeaderAfterFT;
    TextControlCombo    aRepeatHeaderCombo;

    CheckBox        aDontSplitCB;
    CheckBox        aBorderCB;
    FixedLine       aOptionsFL;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;
    PushButton      aAutoFmtBtn;

    String          sConvertTextTable;
    SwTableAutoFmt* pTAutoFmt;
    SwWrtShell*     pShell;

    DECL_LINK( AutoFmtHdl, PushButton* );
    DECL_LINK( BtnHdl, Button* );
    DECL_LINK( CheckBoxHdl, CheckBox *pCB = 0 );
    DECL_LINK( ReapeatHeaderCheckBoxHdl, void* p = 0 );

public:
    SwConvertTableDlg( SwView& rView, bool bToTable );
    ~SwConvertTableDlg();

    void GetValues( sal_Unicode& rDelim,
                    SwInsertTableOptions& rInsTblOpts,
                    SwTableAutoFmt *& prTAFmt );
};


#endif
