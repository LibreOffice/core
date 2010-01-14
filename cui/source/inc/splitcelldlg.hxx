/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: splitcelldlg.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _SVX_SPLITCELLDLG_HXX
#define _SVX_SPLITCELLDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/imagebtn.hxx>
#include <vcl/button.hxx>

#include <svx/stddlg.hxx>
#include <svx/svxdlg.hxx>

class SvxSplitTableDlg : public SvxAbstractSplittTableDialog, public SvxStandardDialog
{
    FixedText           maCountLbl;
    NumericField        maCountEdit;
    FixedLine           maCountFL;
    ImageRadioButton    maHorzBox;
    ImageRadioButton    maVertBox;
    CheckBox            maPropCB;
    FixedLine           maDirFL;
    OKButton            maOKBtn;
    CancelButton        maCancelBtn;
    HelpButton          maHelpBtn;

    long                mnMaxVertical;
    long                mnMaxHorizontal;
protected:
    virtual void Apply();

public:
    SvxSplitTableDlg(Window *pParent, bool bIsTableVertical, long nMaxVertical, long nMaxHorizontal );
    virtual ~SvxSplitTableDlg();

    DECL_LINK( ClickHdl, Button * );

    virtual bool IsHorizontal() const;
    virtual bool IsProportional() const;
    virtual long GetCount() const;

    virtual short Execute();
};

#endif
