/*************************************************************************
 *
 *  $RCSfile: numpara.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 15:16:09 $
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
    aNumberStyleFT          ( this, ResId( FT_NUMBER_STYLE ) ),
    aNumberStyleLB          ( this, ResId( LB_NUMBER_STYLE ) ),
    aNewStartFL             ( this, ResId( FL_NEW_START ) ),
    aNewStartCB             ( this, ResId( CB_NEW_START ) ),
    aNewStartNumberCB       ( this, ResId( CB_NUMBER_NEW_START ) ),
    aNewStartNF             ( this, ResId( NF_NEW_START ) ),
    aCountParaFL            ( this, ResId( FL_COUNT_PARA        ) ),
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
            aCountParaFL        .Hide();
            aCountParaCB        .Hide();
            aRestartParaCountCB .Hide();
            aRestartFT          .Hide();
            aRestartNF          .Hide();
        }
    }
    aNewStartCB.SetClickHdl(LINK(this, SwParagraphNumTabPage, NewStartHdl_Impl));
    aNewStartNumberCB.SetClickHdl(LINK(this, SwParagraphNumTabPage, NewStartHdl_Impl));
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
        aNewStartNumberCB.GetState() != aNewStartNumberCB.GetSavedValue()||
        aNewStartNF.GetText() != aNewStartNF.GetSavedValue())
    {
        bModified = TRUE;
        BOOL bNewStartChecked = STATE_CHECK == aNewStartCB.GetState();
        BOOL bNumberNewStartChecked = STATE_CHECK == aNewStartNumberCB.GetState();
        rSet.Put(SfxBoolItem(FN_NUMBER_NEWSTART, bNewStartChecked));
        rSet.Put(SfxUInt16Item(FN_NUMBER_NEWSTART_AT,
                  bNumberNewStartChecked && bNewStartChecked ? (USHORT)aNewStartNF.GetValue() : USHRT_MAX));
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
    BOOL bHasNumberStyle = FALSE;
    if( eItemState >= SFX_ITEM_AVAILABLE )
    {
        aStyle = ((const SfxStringItem &)rSet.Get( GetWhich(SID_ATTR_PARA_NUMRULE) )).GetValue();
        if(!aStyle.Len())
            aStyle = aNumberStyleLB.GetEntry(0);
        aNumberStyleLB.SelectEntry( aStyle );
        bHasNumberStyle = TRUE;
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
        const SfxBoolItem& rStart = (const SfxBoolItem&)rSet.Get(FN_NUMBER_NEWSTART);
        aNewStartCB.SetState(
            rStart.GetValue() ?
                        STATE_CHECK : STATE_NOCHECK );
        aNewStartCB.EnableTriState(FALSE);
    }
    else
        aNewStartCB.SetState(bHasNumberStyle ? STATE_NOCHECK : STATE_DONTKNOW);
    aNewStartCB.SaveValue();

    eItemState = rSet.GetItemState( FN_NUMBER_NEWSTART_AT);
    if( eItemState > SFX_ITEM_AVAILABLE )
    {
        USHORT nNewStart = ((const SfxUInt16Item&)rSet.Get(FN_NUMBER_NEWSTART_AT)).GetValue();
        aNewStartNumberCB.Check(USHRT_MAX != nNewStart);
        if(USHRT_MAX == nNewStart)
            nNewStart = 1;
        else
            aNewStartCB.SetState( STATE_CHECK );

        aNewStartNF.SetValue(nNewStart);
        aNewStartNumberCB.EnableTriState(FALSE);
    }
    else
        aNewStartCB.SetState(STATE_DONTKNOW);
    NewStartHdl_Impl(&aNewStartCB);
    aNewStartNF.SaveValue();
    aNewStartNumberCB.SaveValue();
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
    aNewStartFL.Show();
    aNewStartCB.Show();
    aNewStartNumberCB.Show();
    aNewStartNF.Show();
}

/*-----------------31.01.98 08:56-------------------

--------------------------------------------------*/
IMPL_LINK( SwParagraphNumTabPage, NewStartHdl_Impl, CheckBox*, pBox )
{
    BOOL bEnable = aNewStartCB.IsChecked();
    aNewStartNumberCB.Enable(bEnable);
    aNewStartNF.Enable(bEnable && aNewStartNumberCB.IsChecked());
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


