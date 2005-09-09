/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: srtdlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 10:02:36 $
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
#ifndef _SRTDLG_HXX
#define _SRTDLG_HXX

#ifndef _FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SVX_STDDLG_HXX
#include <svx/stddlg.hxx>
#endif
#ifndef _SVX_LANGBOX_HXX
#include <svx/langbox.hxx>
#endif

class SwWrtShell;
class CollatorRessource;

class SwSortDlg : public SvxStandardDialog
{
    FixedText           aColLbl;
    FixedText           aTypLbl;
    FixedText           aDirLbl;

    FixedLine           aDirFL;

    CheckBox            aKeyCB1;
    NumericField        aColEdt1;
    ListBox             aTypDLB1;
    RadioButton         aSortUpRB;
    RadioButton         aSortDnRB;

    CheckBox            aKeyCB2;
    NumericField        aColEdt2;
    ListBox             aTypDLB2;
    RadioButton         aSortUp2RB;
    RadioButton         aSortDn2RB;

    CheckBox            aKeyCB3;
    NumericField        aColEdt3;
    ListBox             aTypDLB3;
    RadioButton         aSortUp3RB;
    RadioButton         aSortDn3RB;

    FixedLine           aSortFL;
    RadioButton         aColumnRB;
    RadioButton         aRowRB;

    FixedLine           aDelimFL;
    RadioButton         aDelimTabRB;
    RadioButton         aDelimFreeRB;
    Edit                aDelimEdt;
    PushButton          aDelimPB;

    FixedLine           aLangFL;
    SvxLanguageBox      aLangLB;

    FixedLine           aSortOptFL;
    CheckBox            aCaseCB;

    OKButton            aOkBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;

    String aColTxt;
    String aRowTxt;
    String aNumericTxt;

    SwWrtShell          &rSh;
    CollatorRessource* pColRes;

    USHORT nX;
    USHORT nY;

    virtual void        Apply();
    sal_Unicode         GetDelimChar() const;

    DECL_LINK( CheckHdl, CheckBox * );
    DECL_LINK( DelimHdl, RadioButton* );
    DECL_LINK( LanguageHdl, ListBox* );
    DECL_LINK( DelimCharHdl, PushButton* );

public:
    SwSortDlg(Window * pParent, SwWrtShell &rSh);
    ~SwSortDlg();
};

#endif

