/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: splitcelldlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:41:38 $
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
