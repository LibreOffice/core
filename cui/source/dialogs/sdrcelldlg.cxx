/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

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
, maColorTab           ( pModel->GetColorTableFromSdrModel() )
, maGradientList       ( pModel->GetGradientListFromSdrModel() )
, maHatchingList       ( pModel->GetHatchListFromSdrModel() )
, maBitmapList         ( pModel->GetBitmapListFromSdrModel() )

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

void SvxFormatCellsDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_AREA:
            ( (SvxAreaTabPage&) rPage ).SetColorTable( maColorTab );
            ( (SvxAreaTabPage&) rPage ).SetGradientList( maGradientList );
            ( (SvxAreaTabPage&) rPage ).SetHatchingList( maHatchingList );
            ( (SvxAreaTabPage&) rPage ).SetBitmapList( maBitmapList );
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
