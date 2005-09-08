/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: subtdlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:42:38 $
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

#undef SC_DLLIMPLEMENTATION

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "tpsubt.hxx"
#include "scresid.hxx"
#include "subtdlg.hxx"
#include "subtdlg.hrc"

#pragma hdrstop

//==================================================================

ScSubTotalDlg::ScSubTotalDlg( Window*           pParent,
                              const SfxItemSet* pArgSet ) :
        SfxTabDialog( pParent,
                      ScResId( RID_SCDLG_SUBTOTALS ),
                      pArgSet ),
        aBtnRemove  ( this, ScResId( BTN_REMOVE ) )
{
    AddTabPage( PAGE_GROUP1,  ScTpSubTotalGroup1::Create,  0 );
    AddTabPage( PAGE_GROUP2,  ScTpSubTotalGroup2::Create,  0 );
    AddTabPage( PAGE_GROUP3,  ScTpSubTotalGroup3::Create,  0 );
    AddTabPage( PAGE_OPTIONS, ScTpSubTotalOptions::Create, 0 );
    aBtnRemove.SetClickHdl( LINK( this, ScSubTotalDlg, RemoveHdl ) );
    FreeResource();
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( ScSubTotalDlg, RemoveHdl, PushButton *, pBtn )
{
    if ( pBtn == &aBtnRemove )
    {
        EndDialog( SCRET_REMOVE );
    }
    return 0;
}
IMPL_LINK_INLINE_END( ScSubTotalDlg, RemoveHdl, PushButton *, pBtn )

