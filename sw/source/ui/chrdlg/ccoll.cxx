/*************************************************************************
 *
 *  $RCSfile: ccoll.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 16:18:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#include "cmdid.h"

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#include "swmodule.hxx"
#include "view.hxx"
#include "wrtsh.hxx"
#include "globals.hrc"
#include "helpid.h"


#ifndef _SFX_STYFITEM_HXX //autogen
#include <sfx2/styfitem.hxx>
#endif

#include "uitool.hxx"
#include "ccoll.hxx"
#include "fmtcol.hxx"
#include "hintids.hxx"
#include "docsh.hxx"
#include "docstyle.hxx"
#include "hints.hxx"

#include "chrdlg.hrc"
#include "ccoll.hrc"

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

// Globals ******************************************************************

//CHINA001 static USHORT __FAR_DATA aPageRg[] = {
//CHINA001 FN_COND_COLL, FN_COND_COLL,
//CHINA001 0
//CHINA001 };

// Achtung im Code wird dieses Array direkt (0, 1, ...) indiziert
//CHINA001 static long nTabs[] =
//CHINA001  {   2, // Number of Tabs
//CHINA001 0, 100
//CHINA001  };


CommandStruct SwCondCollItem::aCmds[] = {
         PARA_IN_TABLEHEAD, 0,
         PARA_IN_TABLEBODY, 0,
         PARA_IN_FRAME,     0,
         PARA_IN_SECTION,   0,
         PARA_IN_FOOTENOTE, 0,
         PARA_IN_ENDNOTE,   0,
         PARA_IN_HEADER,    0,
         PARA_IN_FOOTER,    0,
         PARA_IN_OUTLINE,   0,
         PARA_IN_OUTLINE,   1,
         PARA_IN_OUTLINE,   2,
         PARA_IN_OUTLINE,   3,
         PARA_IN_OUTLINE,   4,
         PARA_IN_OUTLINE,   5,
         PARA_IN_OUTLINE,   6,
         PARA_IN_OUTLINE,   7,
         PARA_IN_OUTLINE,   8,
         PARA_IN_OUTLINE,   9,
         PARA_IN_LIST,      0,
         PARA_IN_LIST,      1,
         PARA_IN_LIST,      2,
         PARA_IN_LIST,      3,
         PARA_IN_LIST,      4,
         PARA_IN_LIST,      5,
         PARA_IN_LIST,      6,
         PARA_IN_LIST,      7,
         PARA_IN_LIST,      8,
         PARA_IN_LIST,      9
    };



TYPEINIT1_AUTOFACTORY(SwCondCollItem, SfxPoolItem)


/****************************************************************************
Page: Ctor
****************************************************************************/


//CHINA001 SwCondCollPage::SwCondCollPage(Window *pParent, const SfxItemSet &rSet)
//CHINA001
//CHINA001 : SfxTabPage(pParent, SW_RES(TP_CONDCOLL), rSet),
//CHINA001 aConditionFL( this, ResId( FL_CONDITION )),
//CHINA001 aConditionCB( this, ResId( CB_CONDITION ) ),
//CHINA001 aContextFT  ( this, ResId( FT_CONTEXT    ) ),
//CHINA001 aUsedFT     ( this, ResId( FT_USED       ) ),
//CHINA001 aTbLinks(      this, ResId( TB_CONDCOLLS ) ),
//CHINA001 aStyleFT    ( this, ResId( FT_STYLE  ) ),
//CHINA001 aStyleLB    ( this, ResId( LB_STYLE  ) ),
//CHINA001 aFilterLB   ( this, ResId( LB_FILTER     ) ),
//CHINA001 aRemovePB   ( this, ResId( PB_REMOVE     ) ),
//CHINA001 aAssignPB   ( this, ResId( PB_ASSIGN     ) ),
//CHINA001 sNoTmpl      (       ResId( STR_NOTEMPL  ) ),
//CHINA001 aStrArr  (       ResId( STR_REGIONS  ) ),
//CHINA001 rSh(::GetActiveView()->GetWrtShell()),
//CHINA001 pFmt(0),
//CHINA001 pCmds( SwCondCollItem::GetCmds() ),
//CHINA001 bNewTemplate(FALSE)
//CHINA001 {
//CHINA001 FreeResource();
//CHINA001 SetExchangeSupport();
//CHINA001
//CHINA001 // Handler installieren
//CHINA001 aConditionCB.SetClickHdl(    LINK(this, SwCondCollPage, OnOffHdl));
//CHINA001 aTbLinks.SetDoubleClickHdl( LINK(this, SwCondCollPage, AssignRemoveHdl ));
//CHINA001 aStyleLB.SetDoubleClickHdl( LINK(this, SwCondCollPage, AssignRemoveHdl ));
//CHINA001 aRemovePB.SetClickHdl(       LINK(this, SwCondCollPage, AssignRemoveHdl ));
//CHINA001 aAssignPB.SetClickHdl(       LINK(this, SwCondCollPage, AssignRemoveHdl ));
//CHINA001 aTbLinks.SetSelectHdl(       LINK(this, SwCondCollPage, SelectHdl));
//CHINA001 aStyleLB.SetSelectHdl(       LINK(this, SwCondCollPage, SelectHdl));
//CHINA001 aFilterLB.SetSelectHdl(      LINK(this, SwCondCollPage, SelectHdl));
//CHINA001
//CHINA001 aTbLinks.SetWindowBits(WB_HSCROLL|WB_CLIPCHILDREN);
//CHINA001 aTbLinks.SetSelectionMode( SINGLE_SELECTION );
//CHINA001 aTbLinks.SetTabs( &nTabs[0], MAP_APPFONT );
//CHINA001 aTbLinks.Resize();   // OS: Hack fuer richtige Selektion
//CHINA001 aTbLinks.SetSpaceBetweenEntries( 0 );
//CHINA001 aTbLinks.SetHelpId(HID_COND_COLL_TABLIST);
//CHINA001
//CHINA001 SfxStyleFamilies aFamilies(SW_RES(DLG_STYLE_DESIGNER));
//CHINA001 const SfxStyleFamilyItem* pFamilyItem;
//CHINA001 USHORT nCount = aFamilies.Count();
//CHINA001 USHORT i;
//CHINA001
//CHINA001 for( i = 0; i < nCount; ++i)
//CHINA001 {
//CHINA001 if(SFX_STYLE_FAMILY_PARA == (USHORT)(pFamilyItem = aFamilies.GetObject(i))->GetFamily())
//CHINA001 break;
//CHINA001  }
//CHINA001
//CHINA001 const SfxStyleFilter& rFilterList = pFamilyItem->GetFilterList();
//CHINA001 for( i = 0; i < rFilterList.Count(); ++i)
//CHINA001 {
//CHINA001 aFilterLB.InsertEntry(rFilterList.GetObject(i)->aName);
//CHINA001 USHORT* pFilter = new USHORT(rFilterList.GetObject(i)->nFlags);
//CHINA001 aFilterLB.SetEntryData(i, pFilter);
//CHINA001  }
//CHINA001 aFilterLB.SelectEntryPos(1);
//CHINA001
//CHINA001 aTbLinks.Show();
//CHINA001
//CHINA001 }

/****************************************************************************
Page: Dtor
****************************************************************************/


//CHINA001 __EXPORT SwCondCollPage::~SwCondCollPage()
//CHINA001 {
//CHINA001 for(USHORT i = 0; i < aFilterLB.GetEntryCount(); ++i)
//CHINA001 delete (USHORT*)aFilterLB.GetEntryData(i);
//CHINA001
//CHINA001 }


//CHINA001 int __EXPORT SwCondCollPage::DeactivatePage(SfxItemSet * pSet)
//CHINA001 {
//CHINA001 if( pSet )
//CHINA001 FillItemSet(*pSet);
//CHINA001
//CHINA001 return LEAVE_PAGE;
//CHINA001 }

/****************************************************************************
Page: Factory
****************************************************************************/


//CHINA001 SfxTabPage* __EXPORT SwCondCollPage::Create(Window *pParent, const SfxItemSet &rSet)
//CHINA001 {
//CHINA001 return new SwCondCollPage(pParent, rSet);
//CHINA001 }

/****************************************************************************
Page: FillItemSet-Overload
****************************************************************************/


//CHINA001 BOOL __EXPORT SwCondCollPage::FillItemSet(SfxItemSet &rSet)
//CHINA001 {
//CHINA001 BOOL bModified = TRUE;
//CHINA001 SwCondCollItem aCondItem;
//CHINA001 for(USHORT i = 0; i < aStrArr.Count(); i++)
//CHINA001  {
//CHINA001 String sEntry = aTbLinks.GetEntryText(i, 1);
//CHINA001 aCondItem.SetStyle( &sEntry, i);
//CHINA001  }
//CHINA001 rSet.Put(aCondItem);
//CHINA001 return bModified;
//CHINA001 }

/****************************************************************************
Page: Reset-Overload
****************************************************************************/


//CHINA001 void __EXPORT SwCondCollPage::Reset(const SfxItemSet &rSet)
//CHINA001 {
//CHINA001 if(bNewTemplate)
//CHINA001 aConditionCB.Enable();
//CHINA001 if(RES_CONDTXTFMTCOLL == pFmt->Which())
//CHINA001 aConditionCB.Check();
//CHINA001 OnOffHdl(&aConditionCB);
//CHINA001
//CHINA001 aTbLinks.Clear();
//CHINA001
//CHINA001 SfxStyleSheetBasePool* pPool = rSh.GetView().GetDocShell()->GetStyleSheetPool();
//CHINA001 pPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL);
//CHINA001 aStyleLB.Clear();
//CHINA001 const SfxStyleSheetBase* pBase = pPool->First();
//CHINA001 while( pBase )
//CHINA001  {
//CHINA001 if(!pFmt || pBase->GetName() != pFmt->GetName())
//CHINA001 aStyleLB.InsertEntry(pBase->GetName());
//CHINA001 pBase = pPool->Next();
//CHINA001  }
//CHINA001 aStyleLB.SelectEntryPos(0);
//CHINA001
//CHINA001 for( USHORT n = 0; n < aStrArr.Count(); n++)
//CHINA001  {
//CHINA001 String aEntry( aStrArr.GetString(n) );
//CHINA001 aEntry += '\t';
//CHINA001
//CHINA001 const SwCollCondition* pCond;
//CHINA001 if( pFmt && RES_CONDTXTFMTCOLL == pFmt->Which() &&
//CHINA001 0 != ( pCond = ((SwConditionTxtFmtColl*)pFmt)->
//CHINA001 HasCondition( SwCollCondition( 0, pCmds[n].nCnd, pCmds[n].nSubCond ) ) )
//CHINA001 && pCond->GetTxtFmtColl() )
//CHINA001      {
//CHINA001 aEntry += pCond->GetTxtFmtColl()->GetName();
//CHINA001      }
//CHINA001
//CHINA001 SvLBoxEntry* pE = aTbLinks.InsertEntry( aEntry, n );
//CHINA001 if(0 == n)
//CHINA001 aTbLinks.Select(pE);
//CHINA001  }
//CHINA001
//CHINA001 }

/****************************************************************************

****************************************************************************/


//CHINA001 USHORT* __EXPORT SwCondCollPage::GetRanges()
//CHINA001 {
//CHINA001 return aPageRg;
//CHINA001 }


/****************************************************************************

****************************************************************************/


//CHINA001 IMPL_LINK( SwCondCollPage, OnOffHdl, CheckBox*, pBox )
//CHINA001 {
//CHINA001 const BOOL bEnable = pBox->IsChecked();
//CHINA001 aContextFT.Enable( bEnable );
//CHINA001 aUsedFT   .Enable( bEnable );
//CHINA001 aTbLinks  .Enable( bEnable );
//CHINA001 aStyleFT  .Enable( bEnable );
//CHINA001 aStyleLB  .Enable( bEnable );
//CHINA001 aFilterLB .Enable( bEnable );
//CHINA001 aRemovePB .Enable( bEnable );
//CHINA001 aAssignPB .Enable( bEnable );
//CHINA001 if( bEnable )
//CHINA001 SelectHdl(0);
//CHINA001 return 0;
//CHINA001 }
//CHINA001
/****************************************************************************

****************************************************************************/


//CHINA001 IMPL_LINK( SwCondCollPage, AssignRemoveHdl, PushButton*, pBtn)
//CHINA001 {
//CHINA001 SvLBoxEntry* pE = aTbLinks.FirstSelected();
//CHINA001 ULONG nPos;
//CHINA001 if( !pE || LISTBOX_ENTRY_NOTFOUND ==
//CHINA001 ( nPos = aTbLinks.GetModel()->GetAbsPos( pE ) ) )
//CHINA001  {
//CHINA001 ASSERT( pE, "wo kommt der leere Eintrag her?" );
//CHINA001 return 0;
//CHINA001  }
//CHINA001
//CHINA001 String sSel = aStrArr.GetString( USHORT(nPos) );
//CHINA001 sSel += '\t';
//CHINA001
//CHINA001 const BOOL bAssEnabled = pBtn != &aRemovePB && aAssignPB.IsEnabled();
//CHINA001 aAssignPB.Enable( !bAssEnabled );
//CHINA001 aRemovePB.Enable(  bAssEnabled );
//CHINA001 if ( bAssEnabled )
//CHINA001 sSel += aStyleLB.GetSelectEntry();
//CHINA001
//CHINA001 aTbLinks.SetUpdateMode(FALSE);
//CHINA001 aTbLinks.GetModel()->Remove(pE);
//CHINA001 pE = aTbLinks.InsertEntry(sSel, nPos);
//CHINA001 aTbLinks.Select(pE);
//CHINA001 aTbLinks.MakeVisible(pE);
//CHINA001 aTbLinks.SetUpdateMode(TRUE);
//CHINA001 return 0;
//CHINA001 }
//CHINA001
/****************************************************************************

****************************************************************************/


//CHINA001 IMPL_LINK( SwCondCollPage, SelectHdl, ListBox*, pBox)
//CHINA001 {
//CHINA001 if(pBox == &aFilterLB)
//CHINA001  {
//CHINA001 aStyleLB.Clear();
//CHINA001 USHORT nSearchFlags = pBox->GetSelectEntryPos();
//CHINA001 nSearchFlags = *(USHORT*)aFilterLB.GetEntryData(nSearchFlags);
//CHINA001 SfxStyleSheetBasePool* pPool = rSh.GetView().GetDocShell()->GetStyleSheetPool();
//CHINA001 pPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, nSearchFlags);
//CHINA001 const SfxStyleSheetBase* pBase = pPool->First();
//CHINA001 while( pBase )
//CHINA001      {
//CHINA001 if(!pFmt || pBase->GetName() != pFmt->GetName())
//CHINA001 aStyleLB.InsertEntry(pBase->GetName());
//CHINA001 pBase = pPool->Next();
//CHINA001      }
//CHINA001 aStyleLB.SelectEntryPos(0);
//CHINA001 SelectHdl(&aStyleLB);
//CHINA001
//CHINA001  }
//CHINA001  else
//CHINA001  {
//CHINA001 String sTbEntry;
//CHINA001 SvLBoxEntry* pE = aTbLinks.FirstSelected();
//CHINA001 if(pE)
//CHINA001 sTbEntry = aTbLinks.GetEntryText(pE);
//CHINA001 sTbEntry = sTbEntry.GetToken(1, '\t');
//CHINA001 String sStyle = aStyleLB.GetSelectEntry();
//CHINA001
//CHINA001 aAssignPB.Enable( sStyle != sTbEntry && aConditionCB.IsChecked() );
//CHINA001
//CHINA001 if(pBox != &aStyleLB)
//CHINA001 aRemovePB.Enable( aConditionCB.IsChecked() && sTbEntry.Len() );
//CHINA001  }
//CHINA001 return 0;
//CHINA001 }

/****************************************************************************

****************************************************************************/


//CHINA001 void SwCondCollPage::SetCollection( SwFmt* pFormat, BOOL bNew )
//CHINA001 {
//CHINA001 pFmt = pFormat;
//CHINA001 bNewTemplate = bNew;
//CHINA001 }


/****************************************************************************
    Item fuer den Transport der Bedingungstabelle
****************************************************************************/


SwCondCollItem::SwCondCollItem(USHORT nWhich ) :
    SfxPoolItem(nWhich)
{

}
/****************************************************************************

****************************************************************************/


SwCondCollItem::~SwCondCollItem()
{
}

/****************************************************************************

****************************************************************************/


SfxPoolItem*   SwCondCollItem::Clone( SfxItemPool *pPool ) const
{
    return new SwCondCollItem(*this);
}

/****************************************************************************

****************************************************************************/


int SwCondCollItem::operator==( const SfxPoolItem& rItem) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rItem), "unterschiedliche Typen" );
    BOOL bReturn = TRUE;
    for(USHORT i = 0; i < COND_COMMAND_COUNT; i++)
        if(sStyles[i] != ((SwCondCollItem&)rItem).sStyles[i])
        {
            bReturn = FALSE;
            break;
        }

    return bReturn;
}

/****************************************************************************

****************************************************************************/


const String&   SwCondCollItem::GetStyle(USHORT nPos) const
{
#ifndef IRIX
    return nPos < COND_COMMAND_COUNT ? sStyles[nPos] : aEmptyStr;
#else
    return nPos < COND_COMMAND_COUNT ? (String)sStyles[nPos] : aEmptyStr;
#endif
}

/****************************************************************************

****************************************************************************/


void SwCondCollItem::SetStyle(const String* pStyle, USHORT nPos)
{
    if( nPos < COND_COMMAND_COUNT )
#ifndef IRIX
        sStyles[nPos] = pStyle ? *pStyle : aEmptyStr;
#else
        sStyles[nPos] = pStyle ? (String)*pStyle : aEmptyStr;
#endif
}


/****************************************************************************

****************************************************************************/


const CommandStruct* SwCondCollItem::GetCmds()
{
    return aCmds;
}




