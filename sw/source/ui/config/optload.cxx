/*************************************************************************
 *
 *  $RCSfile: optload.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2001-03-22 09:28:06 $
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

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#include "swmodule.hxx"
#include "usrpref.hxx"
#include "wrtsh.hxx"
#include "linkenum.hxx"

#include "globals.hrc"
#include "cmdid.h"

#include "optload.hrc"
#include "optload.hxx"

#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include <svx/dlgutil.hxx>
#endif
#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _EXPFLD_HXX //autogen
#include <expfld.hxx>
#endif
#ifndef _INSDLG_HXX //autogen
#include <so3/insdlg.hxx>
#endif
#ifndef _OUTPLACE_HXX //autogen
#include <so3/outplace.hxx>
#endif
#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif

/* -----------------22.10.98 15:12-------------------
 *
 * --------------------------------------------------*/
SwLoadOptPage::SwLoadOptPage( Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, SW_RES(TP_OPTLOAD_PAGE), rSet),
    aUpdateGB   (this, ResId(GB_UPDATE    )),
    aLinkFT     (this, ResId(FT_LINK    )),
    aAlwaysRB   (this, ResId(RB_ALWAYS  )),
    aRequestRB  (this, ResId(RB_REQUEST )),
    aNeverRB    (this, ResId(RB_NEVER   )),
    aFieldFT    (this, ResId(FT_FIELD   )),
    aAutoUpdateFields(this, ResId(CB_AUTO_UPDATE_FIELDS )),
    aAutoUpdateCharts(this, ResId(CB_AUTO_UPDATE_CHARTS )),
    aCaptionGB      (this, ResId(GB_CAPTION         )),
    aCaptionCB      (this, ResId(CB_CAPTION         )),
    aCaptionFT      (this, ResId(TXT_OPTIONS        )),
    aCaptionPB      (this, ResId(PB_OPTIONS         )),
    aSettingsGB   ( this,   SW_RES( GB_SETTINGS   ) ),
    aMetricLB     ( this,   SW_RES( LB_METRIC   ) ),
    aMetricFT     ( this,   SW_RES( FT_METRIC   ) ),
    aTabFT        ( this,   SW_RES( FT_TAB      ) ),
    aTabMF        ( this,   SW_RES( MF_TAB      ) ),
    aMergeDistCB(this, ResId(CB_MERGE_PARA_DIST )),
    aMergeDistPageStartCB(this, ResId(CB_MERGE_PARA_DIST_PAGESTART  )),
    aCompatGB   (this, ResId(GB_COMPAT  )),
    pWrtShell   (0),
    aMetricArr    ( SW_RES( ST_METRIC ) ),
    nLastTab(0),
    nOldLinkMode(MANUAL),
    bHTMLMode(FALSE)
{
    FreeResource();

    for ( USHORT i = 0; i < aMetricArr.Count(); ++i )
    {
        String sMetric = aMetricArr.GetStringByPos( i );
        FieldUnit eFUnit = (FieldUnit)aMetricArr.GetValue( i );

        switch ( eFUnit )
        {
            case FUNIT_MM:
            case FUNIT_CM:
            case FUNIT_POINT:
            case FUNIT_PICA:
            case FUNIT_INCH:
            {
                // nur diese Metriken benutzen
                USHORT nPos = aMetricLB.InsertEntry( sMetric );
                aMetricLB.SetEntryData( nPos, (void*)(long)eFUnit );
            }
        }
    }
    aMetricLB.SetSelectHdl(LINK(this, SwLoadOptPage, MetricHdl));

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, FALSE, &pItem )
        && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        aTabFT.Hide();
        aTabMF.Hide();
    }
    aAutoUpdateFields.SetClickHdl(LINK(this, SwLoadOptPage, UpdateHdl));
    aCaptionPB.SetClickHdl(LINK(this, SwLoadOptPage, CaptionHdl));
}

/*-----------------18.01.97 12.43-------------------

--------------------------------------------------*/

SwLoadOptPage::~SwLoadOptPage()
{
}
/* -----------------09.02.99 12:10-------------------
 *
 * --------------------------------------------------*/
IMPL_LINK(  SwLoadOptPage, UpdateHdl, CheckBox*, pBox )
{
    aAutoUpdateCharts.Enable(pBox->IsChecked());
    return 0;
}

/*-----------------18.01.97 12.43-------------------

--------------------------------------------------*/

SfxTabPage* __EXPORT SwLoadOptPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new SwLoadOptPage(pParent, rAttrSet );
}

/*-----------------18.01.97 12.42-------------------

--------------------------------------------------*/

BOOL __EXPORT SwLoadOptPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bRet = FALSE;
    SwModule* pMod = SW_MOD();

    USHORT nNewLinkMode;
    if (aNeverRB.IsChecked())
        nNewLinkMode = NEVER;
    else if (aRequestRB.IsChecked())
        nNewLinkMode = MANUAL;
    else if (aAlwaysRB.IsChecked())
        nNewLinkMode = AUTOMATIC;

    USHORT nFldFlags = aAutoUpdateFields.IsChecked() ?
        aAutoUpdateCharts.IsChecked() ? AUTOUPD_FIELD_AND_CHARTS : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF;

    if(aAutoUpdateFields.IsChecked() != aAutoUpdateFields.GetSavedValue() ||
            aAutoUpdateCharts.IsChecked() != aAutoUpdateCharts.GetSavedValue())
    {
        pMod->ApplyFldUpdateFlags(nFldFlags);
        if(pWrtShell)
        {
            pWrtShell->SetFldUpdateFlags(nFldFlags);
            pWrtShell->SetModified();
        }
    }

    if (nNewLinkMode != nOldLinkMode)
    {
        pMod->ApplyLinkMode(nNewLinkMode);
        if (pWrtShell)
        {
            pWrtShell->SetLinkUpdMode( nNewLinkMode );
            pWrtShell->SetModified();
        }

        bRet = TRUE;
    }
    if (pWrtShell)
    {
        if(aMergeDistCB.IsChecked() != aMergeDistCB.GetSavedValue() ||
            aMergeDistPageStartCB.IsChecked() != aMergeDistPageStartCB.GetSavedValue())
        {
            pWrtShell->SetParaSpaceMax(aMergeDistCB.IsChecked(),
                                aMergeDistPageStartCB.IsChecked());
        }

    }
    const USHORT nMPos = aMetricLB.GetSelectEntryPos();
    if ( nMPos != aMetricLB.GetSavedValue() )
    {
        // Doppel-Cast fuer VA3.0
        USHORT nFieldUnit = (USHORT)(long)aMetricLB.GetEntryData( nMPos );
        rSet.Put( SfxUInt16Item( SID_ATTR_METRIC, (UINT16)nFieldUnit ) );
        bRet = TRUE;
    }

    if(aTabMF.IsVisible() && aTabMF.GetText() != aTabMF.GetSavedValue())
    {
        rSet.Put(SfxUInt16Item(SID_ATTR_DEFTABSTOP,
                    (USHORT)aTabMF.Denormalize(aTabMF.GetValue(FUNIT_TWIP))));
        bRet = TRUE;
    }

    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    if (aCaptionCB.GetSavedValue() != aCaptionCB.GetState())
    {
        pModOpt->SetInsWithCaption(bHTMLMode, aCaptionCB.IsChecked());
        bRet = TRUE;
    }

    return bRet;
}
/*-----------------18.01.97 12.42-------------------

--------------------------------------------------*/
inline lcl_MoveWin(Window& rWin, long nDiff)
{
    Point aPos(rWin.GetPosPixel());
    aPos.Y() -= nDiff;
    rWin.SetPosPixel(aPos);
}
void __EXPORT SwLoadOptPage::Reset( const SfxItemSet& rSet)
{
    const SwMasterUsrPref* pUsrPref = SW_MOD()->GetUsrPref(FALSE);
    const SfxPoolItem* pItem;

    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_WRTSHELL, FALSE, &pItem))
        pWrtShell = (SwWrtShell*)((const SwPtrItem*)pItem)->GetValue();

    sal_Int32 nFldFlags = AUTOUPD_GLOBALSETTING;
    nOldLinkMode = GLOBALSETTING;
    if (pWrtShell)
    {
        nFldFlags = pWrtShell->GetFldUpdateFlags(TRUE);
        nOldLinkMode = pWrtShell->GetLinkUpdMode(TRUE);
        aMergeDistCB.Check(pWrtShell->IsParaSpaceMax());
        aMergeDistPageStartCB.Check(pWrtShell->IsParaSpaceMaxAtPages());

        aMergeDistCB.SaveValue();
        aMergeDistPageStartCB.SaveValue();
    }
    else
    {
        nOldLinkMode = pUsrPref->GetUpdateLinkMode();
        nFldFlags = pUsrPref->GetFldUpdateFlags();
    }

    aAutoUpdateFields.Check(nFldFlags != AUTOUPD_OFF);
    aAutoUpdateCharts.Check(nFldFlags == AUTOUPD_FIELD_AND_CHARTS);
    aAutoUpdateCharts.Enable(nFldFlags != AUTOUPD_OFF);

    aMergeDistCB.Enable(pWrtShell != 0);
    aMergeDistPageStartCB.Enable(pWrtShell != 0);
    aCompatGB.Enable(pWrtShell != 0);

    switch (nOldLinkMode)
    {
        case NEVER:     aNeverRB.Check();   break;
        case MANUAL:    aRequestRB.Check(); break;
        case AUTOMATIC: aAlwaysRB.Check();  break;
    }

    aAutoUpdateFields.SaveValue();
    aAutoUpdateCharts.SaveValue();
    aMetricLB.SetNoSelection();
    if ( rSet.GetItemState( SID_ATTR_METRIC ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rSet.Get( SID_ATTR_METRIC );
        FieldUnit eFieldUnit = (FieldUnit)rItem.GetValue();

        for ( USHORT i = 0; i < aMetricLB.GetEntryCount(); ++i )
        {
            if ( (int)aMetricLB.GetEntryData( i ) == (int)eFieldUnit )
            {
                aMetricLB.SelectEntryPos( i );
                break;
            }
        }
        ::SetFieldUnit(aTabMF, eFieldUnit);
    }
    aMetricLB.SaveValue();
    if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_DEFTABSTOP, FALSE, &pItem))
    {
        nLastTab = ((SfxUInt16Item*)pItem)->GetValue();
        aTabMF.SetValue(aTabMF.Normalize(nLastTab), FUNIT_TWIP);
    }
    aTabMF.SaveValue();

    const SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    aCaptionCB.Check(pModOpt->IsInsWithCaption(bHTMLMode));
    aCaptionCB.SaveValue();

    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, FALSE, &pItem))
    {
        bHTMLMode = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);
    }

    //hide some controls in HTML
    if(bHTMLMode)
    {
        aCaptionGB.Hide();
        aCaptionCB.Hide();
        aCaptionFT.Hide();
        aCaptionPB.Hide();

        long nDiff = aSettingsGB.GetPosPixel().Y() - aCaptionGB.GetPosPixel().Y();
        lcl_MoveWin(aSettingsGB, nDiff);
        lcl_MoveWin(aMetricFT, nDiff);
        lcl_MoveWin(aMetricLB, nDiff);
        lcl_MoveWin(aTabFT, nDiff);
        lcl_MoveWin(aTabMF, nDiff);
        lcl_MoveWin(aCompatGB, nDiff);
        lcl_MoveWin(aMergeDistCB, nDiff);
        lcl_MoveWin(aMergeDistPageStartCB, nDiff);
    }
}
/*-----------------13.01.97 14.44-------------------
    Metric des Deftabstops umschalten
--------------------------------------------------*/

IMPL_LINK(SwLoadOptPage, MetricHdl, ListBox*, EMPTYARG)
{
    const USHORT nMPos = aMetricLB.GetSelectEntryPos();
    if(nMPos != USHRT_MAX)
    {
        // Doppel-Cast fuer VA3.0
        FieldUnit eFieldUnit = (FieldUnit)(long)aMetricLB.GetEntryData( nMPos );
        BOOL bModified = aTabMF.IsModified();
        long nVal = bModified ?
            aTabMF.Denormalize( aTabMF.GetValue( FUNIT_TWIP ) ) :
                nLastTab;
        ::SetFieldUnit( aTabMF, eFieldUnit );
        aTabMF.SetValue( aTabMF.Normalize( nVal ), FUNIT_TWIP );
        if(!bModified)
            aTabMF.ClearModifyFlag();
    }

    return 0;
}
/*********************************************************************/
/*                                                                   */
/*********************************************************************/

IMPL_LINK(SwLoadOptPage, CaptionHdl, PushButton*, EMPTYARG)
{
    SwCaptionOptDlg aDlg(this, GetItemSet());
    aDlg.Execute();

    return 0;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwCaptionOptDlg::SwCaptionOptDlg(Window* pParent, const SfxItemSet& rSet) :
    SfxSingleTabDialog(pParent, rSet, 0)
{
    // TabPage erzeugen
    SwCaptionOptPage* pPage = (SwCaptionOptPage*) SwCaptionOptPage::Create(this, rSet);
    SetTabPage(pPage);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwCaptionOptDlg::~SwCaptionOptDlg()
{
}
/* -----------------22.10.98 15:12-------------------
 *
 * --------------------------------------------------*/

SwCaptionOptPage::SwCaptionOptPage( Window* pParent, const SfxItemSet& rSet )
    : SfxTabPage(pParent, SW_RES(TP_OPTCAPTION_PAGE), rSet),
    aCheckLB        (this, SW_RES(CLB_OBJECTS   )),
    aSampleText     (this, SW_RES(TXT_SAMPLE    )),
    aCategoryText   (this, SW_RES(TXT_CATEGORY  )),
    aCategoryBox    (this, SW_RES(BOX_CATEGORY  )),
    aFormatText     (this, SW_RES(TXT_FORMAT    )),
    aFormatBox      (this, SW_RES(BOX_FORMAT    )),
    aTextText       (this, SW_RES(TXT_TEXT      )),
    aTextEdit       (this, SW_RES(EDT_TEXT      )),
    aPosText        (this, SW_RES(TXT_POS       )),
    aPosBox         (this, SW_RES(BOX_POS       )),
    aFtLevel        (this, SW_RES(FT_LEVEL      )),
    aLbLevel        (this, SW_RES(LB_LEVEL      )),
    aFtDelim        (this, SW_RES(FT_SEPARATOR  )),
    aEdDelim        (this, SW_RES(ED_SEPARATOR  )),
    aSettingsGroup  (this, SW_RES(GRP_SETTINGS  )),

    sSWTable        (SW_RES(STR_TABLE           )),
    sSWFrame        (SW_RES(STR_FRAME           )),
    sSWGraphic      (SW_RES(STR_GRAPHIC         )),
    sOLE            (SW_RES(STR_OLE             )),

    sBegin          (SW_RESSTR(STR_BEGINNING            )),
    sEnd            (SW_RESSTR(STR_END                  )),
    sAbove          (SW_RESSTR(STR_ABOVE                )),
    sBelow          (SW_RESSTR(STR_CP_BELOW             )),

    pMgr            (new SwFldMgr()),
    bHTMLMode(FALSE)
{
    GetDocPoolNm( RES_POOLCOLL_LABEL_ABB, sIllustration );
    GetDocPoolNm( RES_POOLCOLL_LABEL_TABLE, sTable );
    GetDocPoolNm( RES_POOLCOLL_LABEL_FRAME, sText );

    USHORT i, nCount;
    SwWrtShell *pSh = ::GetActiveWrtShell();

    // aFormatBox
    USHORT nSelFmt = SVX_NUM_ARABIC;
    if (pSh)
    {
        nCount = pMgr->GetFldTypeCount();
        SwFieldType* pFldType;
        for ( i = nCount; i; )
            if( ( pFldType = pMgr->GetFldType(USHRT_MAX, --i))->GetName() ==
                aCategoryBox.GetText() )
            {
                nSelFmt = (USHORT)((SwSetExpFieldType*)pFldType)->GetSeqFormat();
                break;
            }
    }


    nCount = pMgr->GetFormatCount(TYP_SEQFLD, FALSE);
    for ( i = 0; i < nCount; ++i )
    {
        aFormatBox.InsertEntry( pMgr->GetFormatStr(TYP_SEQFLD, i) );
        USHORT nFmtId = pMgr->GetFormatId(TYP_SEQFLD, i);
        aFormatBox.SetEntryData( i, (void*)nFmtId );
        if( nFmtId == nSelFmt )
            aFormatBox.SelectEntryPos( i );
    }

    for (i = 0; i < MAXLEVEL; i++)
        aLbLevel.InsertEntry(String::CreateFromInt32(i + 1));

    sal_Unicode cDelim = '.', nLvl = MAXLEVEL;

    if (pSh)
    {
        SwSetExpFieldType* pFldType = (SwSetExpFieldType*)pMgr->GetFldType(
                                            RES_SETEXPFLD, aCategoryBox.GetText() );
        if( pFldType )
        {
            cDelim = pFldType->GetDelimiter();
            nLvl = pFldType->GetOutlineLvl();
        }
    }

    aLbLevel.SelectEntryPos( nLvl < MAXLEVEL ? nLvl + 1 : 0 );
    aEdDelim.SetText( cDelim );

    aCheckLB.SetHelpId(HID_OPTCAPTION_CLB);

    FreeResource();

    Link aLk = LINK( this, SwCaptionOptPage, ModifyHdl );
    aCategoryBox.SetModifyHdl( aLk );
    aTextEdit   .SetModifyHdl( aLk );

    aLk = LINK(this, SwCaptionOptPage, SelectHdl);
    aCategoryBox.SetSelectHdl( aLk );
    aFormatBox  .SetSelectHdl( aLk );

    aCheckLB.SetSelectHdl( LINK(this, SwCaptionOptPage, ShowEntryHdl) );
    aCheckLB.SetDeselectHdl( LINK(this, SwCaptionOptPage, SaveEntryHdl) );
}

/*-----------------18.01.97 12.43-------------------

--------------------------------------------------*/

SwCaptionOptPage::~SwCaptionOptPage()
{
    DelUserData();
    delete pMgr;
}

/*-----------------18.01.97 12.43-------------------

--------------------------------------------------*/

SfxTabPage* SwCaptionOptPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new SwCaptionOptPage(pParent, rAttrSet );
}

/*-----------------18.01.97 12.42-------------------

--------------------------------------------------*/

BOOL SwCaptionOptPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bRet = FALSE;
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    SaveEntry(aCheckLB.FirstSelected());    // Aktuellen Eintrag uebernehmen

    SvLBoxEntry* pEntry = aCheckLB.First();

    while (pEntry)
    {
        InsCaptionOpt* pData = (InsCaptionOpt*)pEntry->GetUserData();
        bRet |= pModOpt->SetCapOption(bHTMLMode, pData);
        pEntry = aCheckLB.Next(pEntry);
    }

    return bRet;
}

/*-----------------18.01.97 12.42-------------------

--------------------------------------------------*/

void SwCaptionOptPage::Reset( const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, FALSE, &pItem))
    {
        bHTMLMode = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);
    }

    USHORT nPos = 0;

    DelUserData();
    aCheckLB.GetModel()->Clear();   // Alle Eintraege entfernen

    aCheckLB.InsertEntry(sSWTable);     SetOptions(nPos++, TABLE_CAP);
    aCheckLB.InsertEntry(sSWFrame);     SetOptions(nPos++, FRAME_CAP);
    aCheckLB.InsertEntry(sSWGraphic);   SetOptions(nPos++, GRAPHIC_CAP);

    SvObjectServerList aObjS;
    aObjS.FillInsertObjects();
    aObjS.Remove(*SwDocShell::ClassFactory());  // Writer-Id wieder entfernen

    for (ULONG i = 0; i < aObjS.Count(); i++)
    {
        const SvGlobalName &rOleId = aObjS[i].GetClassName();
        const String *pClassName = &aObjS[i].GetHumanName();
        if (rOleId == *SvOutPlaceObject::ClassFactory())
            pClassName = &sOLE;
        aCheckLB.InsertEntry(*pClassName);
        SetOptions(nPos++, OLE_CAP, &rOleId);
    }

    ModifyHdl();
}

/*-----------------18.01.97 12.42-------------------

--------------------------------------------------*/

void SwCaptionOptPage::SetOptions(const USHORT nPos, const SwCapObjType eType, const SvGlobalName *pOleId)
{
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    const InsCaptionOpt* pOpt = pModOpt->GetCapOption(bHTMLMode, eType, pOleId);

    if (pOpt)
    {
        aCheckLB.SetEntryData(nPos, new InsCaptionOpt(*pOpt));
        aCheckLB.CheckEntryPos(nPos, pOpt->UseCaption());
    }
    else
        aCheckLB.SetEntryData(nPos, new InsCaptionOpt(eType, pOleId));
}

/*-----------------18.01.97 12.42-------------------

--------------------------------------------------*/

void SwCaptionOptPage::DelUserData()
{
    SvLBoxEntry* pEntry = aCheckLB.First();

    while (pEntry)
    {
        delete (InsCaptionOpt*)pEntry->GetUserData();
        pEntry->SetUserData(0);
        pEntry = aCheckLB.Next(pEntry);
    }
}

/* -----------------26.10.98 11:06-------------------
 *
 * --------------------------------------------------*/

IMPL_LINK( SwCaptionOptPage, ShowEntryHdl, SvxCheckListBox *, EMPTYARG )
{
    SvLBoxEntry* pSelEntry = aCheckLB.FirstSelected();

    if (pSelEntry)
    {
        SwWrtShell *pSh = ::GetActiveWrtShell();

        InsCaptionOpt* pOpt = (InsCaptionOpt*)pSelEntry->GetUserData();

        aCategoryBox.Clear();
        if (pSh)
        {
            USHORT nCount = pMgr->GetFldTypeCount();

            for (USHORT i = 0; i < nCount; i++)
            {
                SwFieldType *pType = pMgr->GetFldType( USHRT_MAX, i );
                if( pType->Which() == RES_SETEXPFLD &&
                    ((SwSetExpFieldType *) pType)->GetType() & GSE_SEQ )
                    aCategoryBox.InsertEntry(SwBoxEntry(pType->GetName(), i));
            }
        }
        else
        {
            aCategoryBox.InsertEntry(SwBoxEntry(sIllustration, 0));
            aCategoryBox.InsertEntry(SwBoxEntry(sTable, 1));
            aCategoryBox.InsertEntry(SwBoxEntry(sText, 2));
        }

        aCategoryBox.SetText(pOpt->GetCategory());
        if (pOpt->GetCategory().Len() &&
            aCategoryBox.GetEntryPos(pOpt->GetCategory()) == COMBOBOX_ENTRY_NOTFOUND)
            aCategoryBox.InsertEntry(pOpt->GetCategory());
        if (!aCategoryBox.GetText().Len())
        {
            USHORT nPos = 0;
            switch(pOpt->GetObjType())
            {
                case OLE_CAP:
                case GRAPHIC_CAP:       nPos = 0;   break;
                case TABLE_CAP:         nPos = 1;   break;
                case FRAME_CAP:         nPos = 2;   break;
            }
            aCategoryBox.SetText(aCategoryBox.GetEntry(nPos).aName);
        }

        for (USHORT i = 0; i < aFormatBox.GetEntryCount(); i++)
        {
            if (pOpt->GetNumType() == (USHORT)(ULONG)aFormatBox.GetEntryData(i))
            {
                aFormatBox.SelectEntryPos(i);
                break;
            }
        }
        aTextEdit.SetText(pOpt->GetCaption());

        aPosBox.Clear();
        switch (pOpt->GetObjType())
        {
            case GRAPHIC_CAP:
            case TABLE_CAP:
            case OLE_CAP:
                aPosBox.InsertEntry(sAbove);
                aPosBox.InsertEntry(sBelow);
                break;
            case FRAME_CAP:
                aPosBox.InsertEntry(sBegin);
                aPosBox.InsertEntry(sEnd);
                break;
        }
        aPosBox.SelectEntryPos(pOpt->GetPos());
        aPosBox.Enable(pOpt->GetObjType() != GRAPHIC_CAP);

        aPosBox.SelectEntryPos(pOpt->GetPos());
        aLbLevel.SelectEntryPos(pOpt->GetLevel());
        aEdDelim.SetText(pOpt->GetSeparator());
    }

    ModifyHdl();

    return 0;
}

/* -----------------26.10.98 11:06-------------------
 *
 * --------------------------------------------------*/

IMPL_LINK( SwCaptionOptPage, SaveEntryHdl, SvxCheckListBox *, EMPTYARG )
{
    SvLBoxEntry* pEntry = aCheckLB.GetHdlEntry();

    if (pEntry)     // Alles speichern
        SaveEntry(pEntry);

    return 0;
}

/* -----------------05.11.98 16:23-------------------
 *
 * --------------------------------------------------*/

void SwCaptionOptPage::SaveEntry(SvLBoxEntry* pEntry)
{
    if (pEntry)
    {
        InsCaptionOpt* pOpt = (InsCaptionOpt*)pEntry->GetUserData();

        pOpt->UseCaption() = aCheckLB.IsChecked(aCheckLB.GetModel()->GetAbsPos(pEntry));
        String aName( aCategoryBox.GetText() );
        aName.EraseLeadingChars (' ');
        aName.EraseTrailingChars(' ');
        pOpt->SetCategory(aName);
        pOpt->SetNumType((USHORT)(ULONG)aFormatBox.GetEntryData(aFormatBox.GetSelectEntryPos()));
        pOpt->SetCaption(aTextEdit.GetText());
        pOpt->SetPos(aPosBox.GetSelectEntryPos());
        pOpt->SetLevel(aLbLevel.GetSelectEntryPos());
        pOpt->SetSeparator(aEdDelim.GetText().GetChar(0));
    }
}

/* -----------------26.10.98 11:06-------------------
 *
 * --------------------------------------------------*/

IMPL_LINK( SwCaptionOptPage, ModifyHdl, Edit *, EMPTYARG )
{
    String sFldTypeName = aCategoryBox.GetText();
/*
    SwFieldType* pType = sFldTypeName.Len() ? pMgr->GetFldType(RES_SETEXPFLD, sFldTypeName) : 0;

    ((SfxSingleTabDialog*)GetParent())->GetOKButton()->Enable( (!pType || ((SwSetExpFieldType*)pType)->GetType() == GSE_SEQ)
                                    && sFldTypeName.Len() != 0 );
*/
    SfxSingleTabDialog *pDlg = (SfxSingleTabDialog *)GetParent();
    PushButton *pBtn = pDlg->GetOKButton();
    if (pBtn)
        pBtn->Enable(sFldTypeName.Len() != 0);

    DrawSample();
    return 0;
}

/* -----------------26.10.98 10:58-------------------
 *
 * --------------------------------------------------*/

IMPL_LINK_INLINE_START( SwCaptionOptPage, SelectHdl, ListBox *, pBox )
{
    DrawSample();
    return 0;
}
IMPL_LINK_INLINE_END( SwCaptionOptPage, SelectHdl, ListBox *, EMPTYARG )

/* -----------------26.10.98 10:58-------------------
 *
 * --------------------------------------------------*/

void SwCaptionOptPage::DrawSample()
{
    String aStr;

    // Nummer
    USHORT nNumFmt = (USHORT)(ULONG)aFormatBox.GetEntryData(
                                    aFormatBox.GetSelectEntryPos() );
    if( SVX_NUM_NUMBER_NONE != nNumFmt )
    {
        // Kategorie
        aStr += aCategoryBox.GetText();
        aStr += ' ';

        SwWrtShell *pSh = ::GetActiveWrtShell();
        String sFldTypeName( aCategoryBox.GetText() );
        if (pSh)
        {
            SwSetExpFieldType* pFldType = (SwSetExpFieldType*)pMgr->GetFldType(
                                            RES_SETEXPFLD, sFldTypeName );
            if( pFldType && pFldType->GetOutlineLvl() < MAXLEVEL )
            {
                BYTE nLvl = pFldType->GetOutlineLvl();
                SwNodeNum aNum( nLvl );
                for( BYTE i = 0; i <= nLvl; ++i )
                    *(aNum.GetLevelVal() + i) = 1;

                String sNumber( pSh->GetOutlineNumRule()->MakeNumString(
                                                        aNum, FALSE ));
                if( sNumber.Len() )
                    (aStr += sNumber) += pFldType->GetDelimiter();
            }
        }

        switch( nNumFmt )
        {
            case SVX_NUM_CHARS_UPPER_LETTER:    aStr += 'A'; break;
            case SVX_NUM_CHARS_UPPER_LETTER_N:  aStr += 'A'; break;
            case SVX_NUM_CHARS_LOWER_LETTER:    aStr += 'a'; break;
            case SVX_NUM_CHARS_LOWER_LETTER_N:  aStr += 'a'; break;
            case SVX_NUM_ROMAN_UPPER:           aStr += 'I'; break;
            case SVX_NUM_ROMAN_LOWER:           aStr += 'i'; break;
            //case ARABIC:
            default:                    aStr += '1'; break;
        }
    }

    aStr += aTextEdit.GetText();
    aSampleText.SetText(aStr);
}

/*------------------------------------------------------------------------
 Beschreibung:  ComboBox ohne Spaces
------------------------------------------------------------------------*/

void CaptionComboBox::KeyInput(const KeyEvent& rEvt)
{
    if( rEvt.GetKeyCode().GetCode() != KEY_SPACE )
        SwComboBox::KeyInput(rEvt);
}



