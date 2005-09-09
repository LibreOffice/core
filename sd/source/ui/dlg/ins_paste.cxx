/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ins_paste.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:03:39 $
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

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include "ins_paste.hrc"
#include "ins_paste.hxx"

// --------------------
// - SdInsertPasteDlg -
// --------------------

SdInsertPasteDlg::SdInsertPasteDlg( Window* pWindow ) :
    ModalDialog( pWindow, SdResId( DLG_INSERT_PASTE ) ),
    aFlPosition( this, SdResId( FL_POSITION ) ),
    aRbBefore( this, SdResId( RB_BEFORE ) ),
    aRbAfter( this, SdResId( RB_AFTER ) ),
    aBtnOK( this, SdResId( BTN_OK ) ),
    aBtnCancel( this, SdResId( BTN_CANCEL ) ),
    aBtnHelp( this, SdResId( BTN_HELP ) )
{
    FreeResource();
    aRbAfter.Check( TRUE );
}

// -----------------------------------------------------------------------------

SdInsertPasteDlg::~SdInsertPasteDlg()
{
}

// -----------------------------------------------------------------------------

BOOL SdInsertPasteDlg::IsInsertBefore() const
{
    return( aRbBefore.IsChecked() );
}
