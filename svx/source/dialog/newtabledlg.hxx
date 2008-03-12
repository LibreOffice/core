/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: newtabledlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:39:25 $
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
#ifndef _SVX_NEWTABLEDLG_HXX
#define _SVX_NEWTABLEDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/button.hxx>

#include <svx/stddlg.hxx>
#include <svx/svxdlg.hxx>

class SvxNewTableDialog : public SvxAbstractNewTableDialog, public ModalDialog
{
private:
    FixedText           maFtColumns;
    NumericField        maNumColumns;
    FixedText           maFtRows;
    NumericField        maNumRows;

    FixedLine           maFlSep;
    HelpButton          maHelpButton;
    OKButton            maOkButton;
    CancelButton        maCancelButton;

public:
    SvxNewTableDialog( Window* pWindow );

    virtual short Execute(void);
    virtual void Apply(void);

    virtual sal_Int32 getRows() const;
    virtual sal_Int32 getColumns() const;
};

#endif // _SVX_NEWTABLEDLG_HXX


