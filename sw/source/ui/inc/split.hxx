/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: split.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:09:35 $
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
#ifndef _TAB_SPLIT_HXX
#define _TAB_SPLIT_HXX

#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

class SwWrtShell;

class SwSplitTableDlg : public SvxStandardDialog
{
    FixedText           aCountLbl;
    NumericField        aCountEdit;
    FixedLine            aCountFL;
    ImageRadioButton    aHorzBox;
    ImageRadioButton    aVertBox;
    CheckBox            aPropCB;
    FixedLine            aDirFL;
    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;
    SwWrtShell&         rSh;

protected:
    virtual void Apply();

public:
    SwSplitTableDlg(Window *pParent, SwWrtShell& rShell );
    DECL_LINK( ClickHdl, Button * );

    BOOL                IsHorizontal() const { return aHorzBox.IsChecked(); }
    BOOL                IsProportional() const { return aPropCB.IsChecked() && aHorzBox.IsChecked(); }
    long                GetCount() const { return sal::static_int_cast< long >(aCountEdit.GetValue()); }
};

#endif
