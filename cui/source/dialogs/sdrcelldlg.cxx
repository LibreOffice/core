/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
, mpColorTab           ( pModel->GetColorList() )
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

void SvxFormatCellsDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_AREA:
            ( (SvxAreaTabPage&) rPage ).SetColorList( mpColorTab );
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
