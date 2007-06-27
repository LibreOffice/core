/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bbdlg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 16:48:42 $
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

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

#include <svx/dialogs.hrc>
#include "page.hrc"

#include "bbdlg.hxx"
#include "border.hxx"
#include "backgrnd.hxx"
#include <svx/dialmgr.hxx>

// class SvxBorderBackgroundDlg ------------------------------------------

SvxBorderBackgroundDlg::SvxBorderBackgroundDlg( Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                BOOL bEnableSelector ) :

    SfxTabDialog( pParent, SVX_RES( RID_SVXDLG_BBDLG ), &rCoreSet ),
    bEnableBackgroundSelector( bEnableSelector )
{
    FreeResource();
    AddTabPage( RID_SVXPAGE_BORDER, SvxBorderTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_BACKGROUND, SvxBackgroundTabPage::Create, 0 );
}

// -----------------------------------------------------------------------

SvxBorderBackgroundDlg::~SvxBorderBackgroundDlg()
{
}

// -----------------------------------------------------------------------

void SvxBorderBackgroundDlg::PageCreated( USHORT nPageId, SfxTabPage& rTabPage )
{
    // Umschalten zwischen Farbe/Grafik ermoeglichen:

    if ( bEnableBackgroundSelector && (RID_SVXPAGE_BACKGROUND == nPageId) )
        ((SvxBackgroundTabPage&)rTabPage).ShowSelector( );
}


