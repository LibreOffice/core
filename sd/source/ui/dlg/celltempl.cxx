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
: SfxStyleDialog( pParent, SdResId(TAB_CELL_TEMPLATE), rStyleBase, false )
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
