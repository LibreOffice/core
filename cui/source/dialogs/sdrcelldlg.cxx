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

#include <svl/cjkoptions.hxx>
#include <svx/flagsdef.hxx>
#include "cuires.hrc"
#include "sdrcelldlg.hxx"
#include "dialmgr.hxx"
#include "cuitabarea.hxx"
#include "svx/svdmodel.hxx"
#include "border.hxx"
#include <svx/dialogs.hrc> // RID_SVXPAGE_...

SvxFormatCellsDialog::SvxFormatCellsDialog( Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel )
: SfxTabDialog        ( pParent, CUI_RES( RID_SVX_FORMAT_CELLS_DLG ), pAttr )
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
            ( (SvxAreaTabPage&) rPage ).Construct();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
