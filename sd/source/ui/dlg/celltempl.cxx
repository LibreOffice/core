/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: celltempl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:35:47 $
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
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include <svtools/style.hxx>
#include <svtools/intitem.hxx>

#include <svx/dialogs.hrc>
#include <svx/drawitem.hxx>
#include <svx/bulitem.hxx>
#include <svx/eeitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/numitem.hxx>
#include <svx/svdmodel.hxx>

#include "DrawDocShell.hxx"
#include "sdresid.hxx"
#include "celltempl.hxx"
#include "celltempl.hrc"
#include "bulmaper.hxx"

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SdPresCellTemplateDlg::SdPresCellTemplateDlg( SdrModel* pModel, Window* pParent, SfxStyleSheetBase& rStyleBase )
: SfxStyleDialog( pParent, SdResId(TAB_CELL_TEMPLATE), rStyleBase, FALSE )
, mpColorTab( pModel->GetColorTable() )
, mpGradientList( pModel->GetGradientList() )
, mpHatchingList( pModel->GetHatchList() )
, mpBitmapList( pModel->GetBitmapList() )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_CHAR_NAME );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS );
    AddTabPage( RID_SVXPAGE_BORDER );
    AddTabPage( RID_SVXPAGE_AREA );
}

// -----------------------------------------------------------------------

SdPresCellTemplateDlg::~SdPresCellTemplateDlg()
{
}

// -----------------------------------------------------------------------

void SdPresCellTemplateDlg::PageCreated( USHORT nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_AREA:
        {
            SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
            aSet.Put (SvxColorTableItem(mpColorTab,SID_COLOR_TABLE));
            aSet.Put (SvxGradientListItem(mpGradientList,SID_GRADIENT_LIST));
            aSet.Put (SvxHatchListItem(mpHatchingList,SID_HATCH_LIST));
            aSet.Put (SvxBitmapListItem(mpBitmapList,SID_BITMAP_LIST));
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
            aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,0));
            rPage.PageCreated(aSet);
        }
        break;

        default:
            SfxTabDialog::PageCreated( nId, rPage );
            break;
    }
}

// -----------------------------------------------------------------------

const SfxItemSet* SdPresCellTemplateDlg::GetRefreshedSet()
{
    SfxItemSet* pRet = GetInputSetImpl();

    if( pRet )
    {
        pRet->ClearItem();
        pRet->SetParent( GetStyleSheet().GetItemSet().GetParent() );
    }
    else
        pRet = new SfxItemSet( GetStyleSheet().GetItemSet() );

    return pRet;
}
