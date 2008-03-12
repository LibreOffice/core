/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrcelldlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:40:29 $
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

#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#include "flagsdef.hxx"
#include "svx/dialogs.hrc"
#include "sdrcelldlg.hxx"
#include "svx/dialmgr.hxx"
#include "cuitabarea.hxx"
#include "svx/svdmodel.hxx"
#include "border.hxx"

SvxFormatCellsDialog::SvxFormatCellsDialog( Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel )
: SfxTabDialog        ( pParent, SVX_RES( RID_SVX_FORMAT_CELLS_DLG ), pAttr )
, mrOutAttrs            ( *pAttr )
, mpColorTab           ( pModel->GetColorTable() )
, mpGradientList       ( pModel->GetGradientList() )
, mpHatchingList       ( pModel->GetHatchList() )
, mpBitmapList         ( pModel->GetBitmapList() )

{
    FreeResource();

    AddTabPage( RID_SVXPAGE_CHAR_NAME );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS );
    AddTabPage( RID_SVXPAGE_BORDER );
    AddTabPage( RID_SVXPAGE_AREA );

/*
    SvtCJKOptions aCJKOptions;
    if( aCJKOptions.IsAsianTypographyEnabled() )
        AddTabPage( RID_SVXPAGE_PARA_ASIAN);
    else
        RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );
*/
}

SvxFormatCellsDialog::~SvxFormatCellsDialog()
{
}

void SvxFormatCellsDialog::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_AREA:
            ( (SvxAreaTabPage&) rPage ).SetColorTable( mpColorTab );
            ( (SvxAreaTabPage&) rPage ).SetGradientList( mpGradientList );
            ( (SvxAreaTabPage&) rPage ).SetHatchingList( mpHatchingList );
            ( (SvxAreaTabPage&) rPage ).SetBitmapList( mpBitmapList );
            ( (SvxAreaTabPage&) rPage ).SetPageType( PT_AREA );
            ( (SvxAreaTabPage&) rPage ).SetDlgType( 1 );
            ( (SvxAreaTabPage&) rPage ).SetPos( 0 );
//          ( (SvxAreaTabPage&) rPage ).SetAreaTP( &mbAreaTP );
//          ( (SvxAreaTabPage&) rPage ).SetGrdChgd( &mnGradientListState );
//          ( (SvxAreaTabPage&) rPage ).SetHtchChgd( &mnHatchingListState );
//          ( (SvxAreaTabPage&) rPage ).SetBmpChgd( &mnBitmapListState );
//          ( (SvxAreaTabPage&) rPage ).SetColorChgd( &mnColorTableState );
            ( (SvxAreaTabPage&) rPage ).Construct();
            // ActivatePage() wird das erste mal nicht gerufen
            ( (SvxAreaTabPage&) rPage ).ActivatePage( mrOutAttrs );

        break;

        default:
            SfxTabDialog::PageCreated( nId, rPage );
            break;
    }
}

void SvxFormatCellsDialog::Apply()
{
}
