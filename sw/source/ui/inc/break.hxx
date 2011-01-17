/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _BREAK_HXX
#define _BREAK_HXX

#include <svx/stddlg.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

class SwWrtShell;

class SwBreakDlg: public SvxStandardDialog
{
    SwWrtShell     &rSh;
    RadioButton     aLineBtn;
    RadioButton     aColumnBtn;
    RadioButton     aPageBtn;
    FixedText       aPageCollText;
    ListBox         aPageCollBox;
    CheckBox        aPageNumBox;
    NumericField    aPageNumEdit;
    FixedLine       aBreakFL;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;

    String          aTemplate;
    sal_uInt16          nKind;
    sal_uInt16          nPgNum;

    sal_Bool            bHtmlMode;

    DECL_LINK( ClickHdl, void * );
    DECL_LINK( PageNumHdl, CheckBox * );
    DECL_LINK( PageNumModifyHdl, Edit * );
    DECL_LINK( OkHdl, Button * );

    void CheckEnable();

protected:
    virtual void Apply();

public:
    SwBreakDlg( Window *pParent, SwWrtShell &rSh );
    ~SwBreakDlg();

    String  GetTemplateName() { return aTemplate; }
    sal_uInt16  GetKind() { return nKind; }
    sal_uInt16  GetPageNumber() { return nPgNum; }
};

#endif

