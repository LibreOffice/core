/*************************************************************************
 *
 *  $RCSfile: ccoll.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:32 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

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

static USHORT __FAR_DATA aPageRg[] = {
    FN_COND_COLL, FN_COND_COLL,
    0
};

// Achtung im Code wird dieses Array direkt (0, 1, ...) indiziert
static long nTabs[] =
    {   2, // Number of Tabs
        0, 100
    };


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


SwCondCollPage::SwCondCollPage(Window *pParent, const SfxItemSet &rSet)

    : SfxTabPage(pParent, SW_RES(TP_CONDCOLL), rSet),
    aConditionCB( this, SW_RES( CB_CONDITION ) ),
    aContextFT  ( this, SW_RES( FT_CONTEXT  ) ),
    aUsedFT     ( this, SW_RES( FT_USED         ) ),
    aTbLinks(     this, SW_RES( TB_CONDCOLLS ) ),
    aStyleFT    ( this, SW_RES( FT_STYLE    ) ),
    aStyleLB    ( this, SW_RES( LB_STYLE    ) ),
    aFilterLB   ( this, SW_RES( LB_FILTER   ) ),
    aRemovePB   ( this, SW_RES( PB_REMOVE   ) ),
    aAssignPB   ( this, SW_RES( PB_ASSIGN   ) ),
    sNoTmpl     (       SW_RES( STR_NOTEMPL  ) ),
    aStrArr     (       SW_RES( STR_REGIONS  ) ),
    rSh(::GetActiveView()->GetWrtShell()),
    pFmt(0),
    pCmds( SwCondCollItem::GetCmds() ),
    bNewTemplate(FALSE)
{
    FreeResource();
    SetExchangeSupport();

    // Handler installieren
    aConditionCB.SetClickHdl(   LINK(this, SwCondCollPage, OnOffHdl));
    aTbLinks.SetDoubleClickHdl( LINK(this, SwCondCollPage, AssignRemoveHdl ));
    aStyleLB.SetDoubleClickHdl( LINK(this, SwCondCollPage, AssignRemoveHdl ));
    aRemovePB.SetClickHdl(      LINK(this, SwCondCollPage, AssignRemoveHdl ));
    aAssignPB.SetClickHdl(      LINK(this, SwCondCollPage, AssignRemoveHdl ));
    aTbLinks.SetSelectHdl(      LINK(this, SwCondCollPage, SelectHdl));
    aStyleLB.SetSelectHdl(      LINK(this, SwCondCollPage, SelectHdl));
    aFilterLB.SetSelectHdl(     LINK(this, SwCondCollPage, SelectHdl));

    aTbLinks.SetWindowBits(WB_HSCROLL|WB_CLIPCHILDREN);
    aTbLinks.SetSelectionMode( SINGLE_SELECTION );
    aTbLinks.SetTabs( &nTabs[0], MAP_APPFONT );
    aTbLinks.Resize();  // OS: Hack fuer richtige Selektion
    aTbLinks.SetSpaceBetweenEntries( 0 );
    aTbLinks.SetHelpId(HID_COND_COLL_TABLIST);

    SfxStyleFamilies aFamilies(SW_RES(DLG_STYLE_DESIGNER));
    const SfxStyleFamilyItem* pFamilyItem;
    USHORT nCount = aFamilies.Count();

    for(USHORT i = 0; i < nCount; ++i)
    {
        if(SFX_STYLE_FAMILY_PARA == (USHORT)(pFamilyItem = aFamilies.GetObject(i))->GetFamily())
            break;
    }

    const SfxStyleFilter& rFilterList = pFamilyItem->GetFilterList();
    for( i = 0; i < rFilterList.Count(); ++i)
    {
        aFilterLB.InsertEntry(rFilterList.GetObject(i)->aName);
        USHORT* pFilter = new USHORT(rFilterList.GetObject(i)->nFlags);
        aFilterLB.SetEntryData(i, pFilter);
    }
    aFilterLB.SelectEntryPos(1);

    aTbLinks.Show();

}

/****************************************************************************
Page: Dtor
****************************************************************************/


__EXPORT SwCondCollPage::~SwCondCollPage()
{
    for(USHORT i = 0; i < aFilterLB.GetEntryCount(); ++i)
        delete (USHORT*)aFilterLB.GetEntryData(i);

}


int __EXPORT SwCondCollPage::DeactivatePage(SfxItemSet * pSet)
{
    FillItemSet(*pSet);

    return LEAVE_PAGE;
}

/****************************************************************************
Page: Factory
****************************************************************************/


SfxTabPage* __EXPORT SwCondCollPage::Create(Window *pParent, const SfxItemSet &rSet)
{
    return new SwCondCollPage(pParent, rSet);
}

/****************************************************************************
Page: FillItemSet-Overload
****************************************************************************/


BOOL __EXPORT SwCondCollPage::FillItemSet(SfxItemSet &rSet)
{
    BOOL bModified = TRUE;
    SwCondCollItem aCondItem;
    for(USHORT i = 0; i < aStrArr.Count(); i++)
    {
        String sEntry = aTbLinks.GetEntryText(i, 1);
        aCondItem.SetStyle( &sEntry, i);
    }
    rSet.Put(aCondItem);
    return bModified;
}

/****************************************************************************
Page: Reset-Overload
****************************************************************************/


void __EXPORT SwCondCollPage::Reset(const SfxItemSet &rSet)
{
    if(bNewTemplate)
        aConditionCB.Enable();
    if(RES_CONDTXTFMTCOLL == pFmt->Which())
        aConditionCB.Check();
    OnOffHdl(&aConditionCB);

    aTbLinks.Clear();

    SfxStyleSheetBasePool* pPool = rSh.GetView().GetDocShell()->GetStyleSheetPool();
    pPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_ALL);
    aStyleLB.Clear();
    const SfxStyleSheetBase* pBase = pPool->First();
    while( pBase )
    {
        if(!pFmt || pBase->GetName() != pFmt->GetName())
            aStyleLB.InsertEntry(pBase->GetName());
        pBase = pPool->Next();
    }
    aStyleLB.SelectEntryPos(0);

    for( USHORT n = 0; n < aStrArr.Count(); n++)
    {
        String aEntry( aStrArr.GetString(n) );
        aEntry += '\t';

        const SwCollCondition* pCond;
        if( pFmt && RES_CONDTXTFMTCOLL == pFmt->Which() &&
            0 != ( pCond = ((SwConditionTxtFmtColl*)pFmt)->
            HasCondition( SwCollCondition( 0, pCmds[n].nCnd, pCmds[n].nSubCond ) ) )
            && pCond->GetTxtFmtColl() )
        {
            aEntry += pCond->GetTxtFmtColl()->GetName();
        }

        SvLBoxEntry* pE = aTbLinks.InsertEntry( aEntry, n );
        if(0 == n)
            aTbLinks.Select(pE);
    }

}

/****************************************************************************

****************************************************************************/


USHORT* __EXPORT SwCondCollPage::GetRanges()
{
    return aPageRg;
}


/****************************************************************************

****************************************************************************/


IMPL_LINK( SwCondCollPage, OnOffHdl, CheckBox*, pBox )
{
    const BOOL bEnable = pBox->IsChecked();
    aContextFT.Enable( bEnable );
    aUsedFT   .Enable( bEnable );
    aTbLinks  .Enable( bEnable );
    aStyleFT  .Enable( bEnable );
    aStyleLB  .Enable( bEnable );
    aFilterLB .Enable( bEnable );
    aRemovePB .Enable( bEnable );
    aAssignPB .Enable( bEnable );
    if( bEnable )
        SelectHdl(0);
    return 0;
}

/****************************************************************************

****************************************************************************/


IMPL_LINK( SwCondCollPage, AssignRemoveHdl, PushButton*, pBtn)
{
    SvLBoxEntry* pE = aTbLinks.FirstSelected();
    ULONG nPos;
    if( !pE || LISTBOX_ENTRY_NOTFOUND ==
        ( nPos = aTbLinks.GetModel()->GetAbsPos( pE ) ) )
    {
        ASSERT( pE, "wo kommt der leere Eintrag her?" );
        return 0;
    }

    String sSel = aStrArr.GetString( USHORT(nPos) );
    sSel += '\t';

    const BOOL bAssEnabled = pBtn != &aRemovePB && aAssignPB.IsEnabled();
    aAssignPB.Enable( !bAssEnabled );
    aRemovePB.Enable(  bAssEnabled );
    if ( bAssEnabled )
        sSel += aStyleLB.GetSelectEntry();

    aTbLinks.SetUpdateMode(FALSE);
    aTbLinks.GetModel()->Remove(pE);
    pE = aTbLinks.InsertEntry(sSel, nPos);
    aTbLinks.Select(pE);
    aTbLinks.MakeVisible(pE);
    aTbLinks.SetUpdateMode(TRUE);
    return 0;
}

/****************************************************************************

****************************************************************************/


IMPL_LINK( SwCondCollPage, SelectHdl, ListBox*, pBox)
{
    if(pBox == &aFilterLB)
    {
        aStyleLB.Clear();
        USHORT nSearchFlags = pBox->GetSelectEntryPos();
        nSearchFlags = *(USHORT*)aFilterLB.GetEntryData(nSearchFlags);
        SfxStyleSheetBasePool* pPool = rSh.GetView().GetDocShell()->GetStyleSheetPool();
        pPool->SetSearchMask(SFX_STYLE_FAMILY_PARA, nSearchFlags);
        const SfxStyleSheetBase* pBase = pPool->First();
        while( pBase )
        {
            if(!pFmt || pBase->GetName() != pFmt->GetName())
                aStyleLB.InsertEntry(pBase->GetName());
            pBase = pPool->Next();
        }
        aStyleLB.SelectEntryPos(0);
        SelectHdl(&aStyleLB);

    }
    else
    {
        String sTbEntry;
        SvLBoxEntry* pE = aTbLinks.FirstSelected();
        if(pE)
            sTbEntry = aTbLinks.GetEntryText(pE);
        sTbEntry = sTbEntry.GetToken(1, '\t');
        String sStyle = aStyleLB.GetSelectEntry();

        aAssignPB.Enable( sStyle != sTbEntry && aConditionCB.IsChecked() );

        if(pBox != &aStyleLB)
            aRemovePB.Enable( aConditionCB.IsChecked() && sTbEntry.Len() );
    }
    return 0;
}

/****************************************************************************

****************************************************************************/


void SwCondCollPage::SetCollection( SwFmt* pFormat, BOOL bNew )
{
    pFmt = pFormat;
    bNewTemplate = bNew;
}


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


/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.21  2000/09/18 16:05:13  willem.vandorp
      OpenOffice header added.

      Revision 1.20  2000/04/20 12:54:31  os
      GetName() returns String&

      Revision 1.19  2000/02/11 14:43:33  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.18  1999/05/06 07:52:58  OS
      #65847# Font an der TabListBox nicht aendern


      Rev 1.17   06 May 1999 09:52:58   OS
   #65847# Font an der TabListBox nicht aendern

      Rev 1.16   10 Jul 1998 13:46:30   MH
   internal comp.err

      Rev 1.15   21 Jan 1998 17:57:38   JP
   neu: neue Bedinung - Endnote

      Rev 1.14   17 Jan 1998 09:45:14   OS
   Numerierung und Gliederung bis 10.

      Rev 1.13   30 Sep 1997 15:53:30   TJ
   include

      Rev 1.12   01 Sep 1997 13:25:28   OS
   DLL-Umstellung

      Rev 1.11   09 Jul 1997 17:20:50   HJS
   includes

      Rev 1.10   06 Jun 1997 16:25:54   OS
   im Reset ::Clear auch an der Vorlagen-Box rufen

      Rev 1.9   05 Feb 1997 10:18:52   OM
   FillItemSet in DeactivatePage rufen

      Rev 1.8   14 Nov 1996 12:44:36   OS
   DLG_STYLE_DESIGNER

      Rev 1.7   14 Nov 1996 08:39:04   SWG
   neu: RES_TEMPLATE_FAMILIES

      Rev 1.6   11 Nov 1996 09:16:40   MA
   ResMgr

      Rev 1.5   02 Oct 1996 08:35:56   MA
   Umstellung Enable/Disable

      Rev 1.4   25 Sep 1996 11:20:32   PL
   IRIX

      Rev 1.3   23 Sep 1996 08:23:44   OS
   keine GroupBox fuer Bedingungs-TabPage

      Rev 1.2   18 Sep 1996 19:01:38   OS
   neu: HScrollBars, Disable/Enable-Bug behoben

      Rev 1.1   17 Sep 1996 16:19:18   OS
   UI-Anpassung bedingte Vorlagen

      Rev 1.0   06 Sep 1996 19:01:44   JP
   TabPage fuer bedingte Vorlagen

*************************************************************************/


