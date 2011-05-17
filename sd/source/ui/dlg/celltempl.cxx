/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include <svl/style.hxx>
#include <svl/intitem.hxx>

#include <svx/dialogs.hrc>
#include <svx/drawitem.hxx>
#include <editeng/bulitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/numitem.hxx>
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
: SfxStyleDialog( pParent, SdResId(TAB_CELL_TEMPLATE), rStyleBase, sal_False )
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

void SdPresCellTemplateDlg::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
