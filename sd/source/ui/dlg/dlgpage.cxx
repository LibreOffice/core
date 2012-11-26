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

#include <svl/intitem.hxx>
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#include <svx/tabarea.hxx>
#include <svx/drawitem.hxx>

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif
#include "dlgpage.hxx"

#include "DrawDocShell.hxx"
#include <svl/aeitem.hxx>
#include <svx/flagsdef.hxx>
#include <editeng/svxenum.hxx>

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

SdPageDlg::SdPageDlg( SfxObjectShell* pDocSh, Window* pParent, const SfxItemSet* pAttr, bool bAreaPage ) :
        SfxTabDialog ( pParent, SdResId( TAB_PAGE ), pAttr ),
        mrOutAttrs          ( *pAttr ),
        mpDocShell          ( pDocSh )
{
    SvxColorTableItem aColorTableItem(*( (const SvxColorTableItem*)
        ( mpDocShell->GetItem( SID_COLOR_TABLE ) ) ) );
    SvxGradientListItem aGradientListItem(*( (const SvxGradientListItem*)
        ( mpDocShell->GetItem( SID_GRADIENT_LIST ) ) ) );
    SvxBitmapListItem aBitmapListItem(*( (const SvxBitmapListItem*)
        ( mpDocShell->GetItem( SID_BITMAP_LIST ) ) ) );
    SvxHatchListItem aHatchListItem(*( (const SvxHatchListItem*)
        ( mpDocShell->GetItem( SID_HATCH_LIST ) ) ) );

    mpColorTab = aColorTableItem.GetColorTable();
    mpGradientList = aGradientListItem.GetGradientList();
    mpHatchingList = aHatchListItem.GetHatchList();
    mpBitmapList = aBitmapListItem.GetBitmapList();

    FreeResource();

    AddTabPage( RID_SVXPAGE_PAGE);
    AddTabPage( RID_SVXPAGE_AREA);

    if(!bAreaPage)  // I have to add the page before I remove it !
        RemoveTabPage( RID_SVXPAGE_AREA );
}


/*************************************************************************
|*
|* Seite wird erzeugt
|*
\************************************************************************/

void SdPageDlg::PageCreated(sal_uInt16 nId, SfxTabPage& rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch(nId)
    {
    case RID_SVXPAGE_PAGE:
        aSet.Put (SfxAllEnumItem((const sal_uInt16)SID_ENUM_PAGE_MODE, SVX_PAGE_MODE_PRESENTATION));
        aSet.Put (SfxAllEnumItem((const sal_uInt16)SID_PAPER_START, PAPER_A0));
        aSet.Put (SfxAllEnumItem((const sal_uInt16)SID_PAPER_END, PAPER_E));
        rPage.PageCreated(aSet);
        break;
    case RID_SVXPAGE_AREA:
            aSet.Put (SvxColorTableItem(mpColorTab,SID_COLOR_TABLE));
            aSet.Put (SvxGradientListItem(mpGradientList,SID_GRADIENT_LIST));
            aSet.Put (SvxHatchListItem(mpHatchingList,SID_HATCH_LIST));
            aSet.Put (SvxBitmapListItem(mpBitmapList,SID_BITMAP_LIST));
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,0));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,1));
            aSet.Put (SfxUInt16Item(SID_TABPAGE_POS,0));
            rPage.PageCreated(aSet);
        break;
    }
}



