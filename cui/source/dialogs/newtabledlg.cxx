/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: newtabledlg.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------

#include "svx/dialogs.hrc"
#include "svx/dialmgr.hxx"
#include "newtabledlg.hxx"
#include "newtabledlg.hrc"

SvxNewTableDialog::SvxNewTableDialog( Window* pParent )
: ModalDialog( pParent, SVX_RES( RID_SVX_NEWTABLE_DLG ) )
, maFtColumns( this, SVX_RES( FT_COLUMNS ) )
, maNumColumns( this, SVX_RES( NF_COLUMNS ) )
, maFtRows( this, SVX_RES( FT_ROWS ) )
, maNumRows( this, SVX_RES( NF_ROWS ) )
, maFlSep( this, SVX_RES( FL_SEP ) )
, maHelpButton( this, SVX_RES( BTN_HELP ) )
, maOkButton( this, SVX_RES( BTN_OK ) )
, maCancelButton( this, SVX_RES( BTN_CANCEL ) )
{
    maNumRows.SetValue(2);
    maNumColumns.SetValue(5);
    FreeResource();
}

short SvxNewTableDialog::Execute(void)
{
    return ModalDialog::Execute();
}

void SvxNewTableDialog::Apply(void)
{
}

sal_Int32 SvxNewTableDialog::getRows() const
{
    return sal::static_int_cast< sal_Int32 >( maNumRows.GetValue() );
}

sal_Int32 SvxNewTableDialog::getColumns() const
{
    return sal::static_int_cast< sal_Int32 >( maNumColumns.GetValue() );
}
