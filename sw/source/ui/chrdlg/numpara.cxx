/*************************************************************************
 *
 *  $RCSfile: numpara.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2001-02-23 12:45:28 $
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
#include "swtypes.hxx"
#include "hintids.hxx"
#include "globals.hrc"
#include "helpid.h"

#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX //autogen
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef SW_FMTLINE_HXX
#include <fmtline.hxx>
#endif
#include <numpara.hxx>
#include <numpara.hrc>

// Globals ******************************************************************

static USHORT __FAR_DATA aPageRg[] = {
    FN_NUMBER_NEWSTART, FN_NUMBER_NEWSTART_AT,
    0
};
/*-----------------31.01.98 08:34-------------------

--------------------------------------------------*/
SwParagraphNumTabPage::SwParagraphNumTabPage(Window* pParent,
                                                const SfxItemSet& rAttr ) :
    SfxTabPage(pParent, SW_RES(TP_NUMPARA), rAttr),
    aNumberStyleGB          ( this, ResId( GB_NUMBER_STYLE ) ),
    aNumberStyleLB          ( this, ResId( LB_NUMBER_STYLE ) ),
    aNewStartGB             ( this, ResId( GB_NEW_START ) ),
    aNewStartCB             ( this, ResId( CB_NEW_START ) ),
    aNewStartFT             ( this, ResId( FT_NEW_START ) ),
    aNewStartNF             ( this, ResId( NF_NEW_START ) ),
    aCountParaGB            ( this, ResId( GB_COUNT_PARA        ) ),
    aCountParaCB            ( this, ResId( CB_COUNT_PARA        ) ),
    aRestartParaCountCB     ( this, ResId( CB_RESTART_PARACOUNT ) ),
    aRestartFT              ( this, ResId( FT_RESTART_NO        ) ),
    aRestartNF              ( this, ResId( NF_RESTART_PARA      ) ),
    bModified(FALSE),
    bCurNumrule(FALSE)
{
    FreeResource();

    const SfxPoolItem* pItem;
    SfxObjectShell* pObjSh;
    if(SFX_ITEM_SET == rAttr.GetItemState(SID_HTML_MODE, FALSE, &pItem) ||
        ( 0 != ( pObjSh = SfxObjectShell::Current()) &&
                    0 != (pItem = pObjSh->GetItem(SID_HTML_MODE))))
    {
        USHORT nHtmlMode = ((const SfxUInt16Item*)pItem)->GetValue();
        if(HTMLMODE_ON & nHtmlMode)
        {
            aCountParaGB        .Hide();
            aCountParaCB        .Hide();
            aRestartParaCountCB .Hide();
            aRestartFT          .Hide();
            aRestartNF          .Hide();
        }
    }
    aNewStartCB.SetClickHdl(LINK(this, SwParagraphNumTabPage, NewStartHdl_Impl));
    aNumberStyleLB.SetSelectHdl(LINK(this, SwParagraphNumTabPage, StyleHdl_Impl));
    aCountParaCB.SetClickHdl(LINK(this,
                    SwParagraphNumTabPage, LineCountHdl_Impl));
    aRestartParaCountCB.SetClickHdl(
                    LINK(this, SwParagraphNumTabPage, LineCountHdl_Impl));
}
/*-----------------31.01.98 08:34-------------------

--------------------------------------------------*/
SwParagraphNumTabPage::~SwParagraphNumTabPage()
{
}

/*-----------------31.01.98 08:38-------------------

--------------------------------------------------*/
SfxTabPage* SwParagraphNumTabPage::Create(  Window* pParent,
                                const SfxItemSet& rSet )
{
    return new SwParagraphNumTabPage(pParent, rSet);
}

/*-----------------31.01.98 08:38-------------------

--------------------------------------------------*/
USHORT* SwParagraphNumTabPage::GetRanges()
{
    return aPageRg;
}

/*-----------------31.01.98 08:38-------------------

--------------------------------------------------*/
BOOL    SwParagraphNumTabPage::FillItemSet( SfxItemSet& rSet )
{
    if( aNumberStyleLB.GetSelectEntryPos() != aNumberStyleLB.GetSavedValue())
    {
        String aStyle;
        if(aNumberStyleLB.GetSelectEntryPos())
            aStyle = aNumberStyleLB.GetSelectEntry();
        const SfxStringItem* pOldRule = (const SfxStringItem*)GetOldItem( rSet, SID_ATTR_PARA_NUMRULE);
        SfxStringItem* pRule = (SfxStringItem*)pOldRule->Clone();
        pRule->SetValue(aStyle);
        rSet.Put(*pRule);
        delete pRule;
        bModified = TRUE;
    }
    if(aNewStartCB.GetState() != aNewStartCB.GetSavedValue() ||
        aNewStartNF.GetText() != aNewStartNF.GetSavedValue())
    {
        bModified = TRUE;
        rSet.Put(SfxBoolItem(FN_NUMBER_NEWSTART, aNewStartCB.GetState() == STATE_CHECK));
        rSet.Put(SfxUInt16Item(FN_NUMBER_NEWSTART_AT, (USHORT)aNewStartNF.GetValue()));
    }

    if(aCountParaCB.GetSavedValue() != aCountParaCB.GetState() ||
        aRestartParaCountCB.GetSavedValue() != aRestartParaCountCB.GetState() ||
            aRestartNF.GetSavedValue() != aRestartNF.GetText() )
    {
        SwFmtLineNumber aFmt;
        aFmt.SetStartValue( aRestartParaCountCB.GetState() == STATE_CHECK ?
                                aRestartNF.GetValue() : 0 );
        aFmt.SetCountLines( aCountParaCB.IsChecked() );
        rSet.Put(aFmt);
        bModified = TRUE;
    }
    return bModified;
}

/*-----------------31.01.98 08:38-------------------

--------------------------------------------------*/
void    SwParagraphNumTabPage::Reset( const SfxItemSet& rSet )
{
    SfxItemState eItemState = rSet.GetItemState( GetWhich(SID_ATTR_PARA_NUMRULE) );

    String aStyle;
    if( eItemState >= SFX_ITEM_AVAILABLE )
    {
        aStyle = ((const SfxStringItem &)rSet.Get( GetWhich(SID_ATTR_PARA_NUMRULE) )).GetValue();
        if(!aStyle.Len())
            aStyle = aNumberStyleLB.GetEntry(0);
        aNumberStyleLB.SelectEntry( aStyle );
    }
    else
    {
        aNumberStyleLB.SetNoSelection();
    }

    aNumberStyleLB.SaveValue();


    eItemState = rSet.GetItemState( FN_NUMBER_NEWSTART );
    if(eItemState > SFX_ITEM_AVAILABLE )
    {
        bCurNumrule = TRUE;
        aNewStartCB.SetState(
            ((const SfxBoolItem&)rSet.Get(FN_NUMBER_NEWSTART)).GetValue() ?
                        STATE_CHECK : STATE_NOCHECK );
        aNewStartCB.EnableTriState(FALSE);
    }
    else
        aNewStartCB.SetState(STATE_DONTKNOW);
    aNewStartCB.SaveValue();

    eItemState = rSet.GetItemState( FN_NUMBER_NEWSTART_AT);
    if( eItemState > SFX_ITEM_AVAILABLE )
    {
        USHORT nNewStart = ((const SfxUInt16Item&)rSet.Get(FN_NUMBER_NEWSTART_AT)).GetValue();
        if(USHRT_MAX == nNewStart)
            nNewStart = 1;
        aNewStartNF.SetValue(nNewStart);
        NewStartHdl_Impl(&aNewStartCB);
    }
    aNewStartNF.SaveValue();
    StyleHdl_Impl(&aNumberStyleLB);
    if( SFX_ITEM_AVAILABLE <= rSet.GetItemState(RES_LINENUMBER))
    {
        SwFmtLineNumber& rNum = (SwFmtLineNumber&)rSet.Get(RES_LINENUMBER);
        ULONG nStartValue = rNum.GetStartValue();
        BOOL bCount = rNum.IsCount();
        aCountParaCB.SetState( bCount ? STATE_CHECK : STATE_NOCHECK );
        aRestartParaCountCB.SetState( 0 != nStartValue ? STATE_CHECK : STATE_NOCHECK );
        aRestartNF.SetValue(nStartValue == 0 ? 1 : nStartValue);
        LineCountHdl_Impl(&aCountParaCB);
        aCountParaCB.EnableTriState(FALSE);
        aRestartParaCountCB.EnableTriState(FALSE);
    }
    else
    {
        aCountParaCB.SetState(STATE_DONTKNOW);
        aRestartParaCountCB.SetState(STATE_DONTKNOW);
    }
    aCountParaCB.SaveValue();
    aRestartParaCountCB.SaveValue();
    aRestartNF.SaveValue();

    bModified = FALSE;
}

/*-----------------31.01.98 08:38-------------------

--------------------------------------------------*/
void SwParagraphNumTabPage::EnableNewStart()
{
    aNewStartGB.Show();
    aNewStartCB.Show();
    aNewStartFT.Show();
    aNewStartNF.Show();
}

/*-----------------31.01.98 08:56-------------------

--------------------------------------------------*/
IMPL_LINK( SwParagraphNumTabPage, NewStartHdl_Impl, CheckBox*, pBox )
{
    BOOL bEnable = pBox->IsChecked();
    aNewStartFT.Enable(bEnable);
    aNewStartNF.Enable(bEnable);
    return 0;
}

/*-----------------05.02.98 10:01-------------------

--------------------------------------------------*/
IMPL_LINK( SwParagraphNumTabPage, LineCountHdl_Impl, CheckBox* , EMPTYARG)
{
    aRestartParaCountCB.Enable(aCountParaCB.IsChecked());

    BOOL bEnableRestartValue = aRestartParaCountCB.IsEnabled() &&
                                                aRestartParaCountCB.IsChecked();
    aRestartFT.Enable(bEnableRestartValue);
    aRestartNF.Enable(bEnableRestartValue);

    return 0;
}

/*-----------------05.02.98 13:39-------------------

--------------------------------------------------*/
IMPL_LINK( SwParagraphNumTabPage, StyleHdl_Impl, ListBox*, pBox )
{
    BOOL bEnable = bCurNumrule || pBox->GetSelectEntryPos() > 0;
    aNewStartCB.Enable(bEnable);
    NewStartHdl_Impl(&aNewStartCB);
    return 0;
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:32  hr
      initial import

      Revision 1.9  2000/09/18 16:05:14  willem.vandorp
      OpenOffice header added.

      Revision 1.8  2000/07/03 08:04:44  os
      #72742# resource warnings corrected

      Revision 1.7  1998/04/15 12:31:48  OS
      TriState-Boxen #49486#


      Rev 1.6   15 Apr 1998 14:31:48   OS
   TriState-Boxen #49486#

      Rev 1.5   17 Mar 1998 17:55:24   OS
   Vorlagenlistbox bei unbekannter Vorlage unselektiert #47570#

      Rev 1.4   03 Mar 1998 10:14:48   OS
   Zeilennumerierung nicht in HTML

      Rev 1.3   20 Feb 1998 11:23:42   MA
   Restarbeiten Zeilennummerierung

      Rev 1.2   05 Feb 1998 14:56:26   OS
   Paste-Fehler

      Rev 1.1   05 Feb 1998 14:43:30   OS
   Zeilennumerierung

      Rev 1.0   02 Feb 1998 09:15:52   OS
   Initial revision.

*************************************************************************/

