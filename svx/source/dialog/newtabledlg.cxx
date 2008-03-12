/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: newtabledlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:38:25 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

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
