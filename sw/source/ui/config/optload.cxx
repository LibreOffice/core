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

#include <officecfg/Office/Writer.hxx>
#include <comphelper/string.hxx>
#include <tools/shl.hxx>
#include <swtypes.hxx>
#include <helpid.h>
#include <uiitems.hxx>
#include <modcfg.hxx>
#include "swmodule.hxx"
#include "usrpref.hxx"
#include "wrtsh.hxx"
#include "linkenum.hxx"
#include <uitool.hxx>
#include <view.hxx>

#include "globals.hrc"
#include "cmdid.h"

#include "optload.hrc"
#include "optload.hxx"
#include <svx/dlgutil.hxx>
#include <sfx2/htmlmode.hxx>
#include <fldmgr.hxx>
#include <poolfmt.hxx>
#include <expfld.hxx>
#include <caption.hxx>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>

#include <svtools/insdlg.hxx>
#include "svtools/treelistentry.hxx"
#include <comphelper/classids.hxx>
#include <unotools/configmgr.hxx>
#include <docsh.hxx>
#include <config.hrc>
#include <SwStyleNameMapper.hxx>
#include <numrule.hxx>
#include <SwNodeNum.hxx>

#include <doc.hxx>
#include <svl/cjkoptions.hxx>

using namespace ::com::sun::star;

#include <svl/eitem.hxx>

SwLoadOptPage::SwLoadOptPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "OptGeneralPage",
        "modules/swriter/ui/optgeneralpage.ui", rSet)
    , m_pWrtShell(NULL)
    , m_nLastTab(0)
    , m_nOldLinkMode(MANUAL)
{
    get(m_pAlwaysRB, "always");
    get(m_pRequestRB, "onrequest");
    get(m_pNeverRB, "never");
    get(m_pAutoUpdateFields, "updatefields");
    get(m_pAutoUpdateCharts, "updatecharts");
    get(m_pMetricLB, "metric");
    get(m_pTabFT, "tablabel");
    get(m_pTabMF, "tab");
    get(m_pUseSquaredPageMode, "squaremode");
    get(m_pUseCharUnit, "usecharunit");
    get(m_pWordCountED, "wordcount");

    SvxStringArray aMetricArr( SW_RES( STR_ARR_METRIC ) );
    for ( sal_uInt16 i = 0; i < aMetricArr.Count(); ++i )
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
                // use only these metrics
                sal_uInt16 nPos = m_pMetricLB->InsertEntry( sMetric );
                m_pMetricLB->SetEntryData( nPos, (void*)(sal_IntPtr)eFUnit );
            }
            default:; //prevent warning
        }
    }
    m_pMetricLB->SetSelectHdl(LINK(this, SwLoadOptPage, MetricHdl));

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem )
        && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        m_pTabFT->Hide();
        m_pTabMF->Hide();
    }

    SvtCJKOptions aCJKOptions;
    if(!aCJKOptions.IsAsianTypographyEnabled())
    {
        m_pUseSquaredPageMode->Hide();
        m_pUseCharUnit->Hide();
    }
}

SfxTabPage* SwLoadOptPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new SwLoadOptPage(pParent, rAttrSet );
}

sal_Bool SwLoadOptPage::FillItemSet( SfxItemSet& rSet )
{
    sal_Bool bRet = sal_False;
    SwModule* pMod = SW_MOD();

    sal_uInt16 nNewLinkMode = AUTOMATIC;
    if (m_pNeverRB->IsChecked())
        nNewLinkMode = NEVER;
    else if (m_pRequestRB->IsChecked())
        nNewLinkMode = MANUAL;

    SwFldUpdateFlags eFldFlags = m_pAutoUpdateFields->IsChecked() ?
        m_pAutoUpdateCharts->IsChecked() ? AUTOUPD_FIELD_AND_CHARTS : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF;

    if(m_pAutoUpdateFields->IsChecked() != m_pAutoUpdateFields->GetSavedValue() ||
            m_pAutoUpdateCharts->IsChecked() != m_pAutoUpdateCharts->GetSavedValue())
    {
        pMod->ApplyFldUpdateFlags(eFldFlags);
        if(m_pWrtShell)
        {
            m_pWrtShell->SetFldUpdateFlags(eFldFlags);
            m_pWrtShell->SetModified();
        }
    }

    if (nNewLinkMode != m_nOldLinkMode)
    {
        pMod->ApplyLinkMode(nNewLinkMode);
        if (m_pWrtShell)
        {
            m_pWrtShell->SetLinkUpdMode( nNewLinkMode );
            m_pWrtShell->SetModified();
        }

        bRet = sal_True;
    }

    const sal_uInt16 nMPos = m_pMetricLB->GetSelectEntryPos();
    if ( nMPos != m_pMetricLB->GetSavedValue() )
    {
        // Double-Cast for VA3.0
        sal_uInt16 nFieldUnit = (sal_uInt16)(sal_IntPtr)m_pMetricLB->GetEntryData( nMPos );
        rSet.Put( SfxUInt16Item( SID_ATTR_METRIC, (sal_uInt16)nFieldUnit ) );
        bRet = sal_True;
    }

    if(m_pTabMF->IsVisible() && m_pTabMF->GetText() != m_pTabMF->GetSavedValue())
    {
        rSet.Put(SfxUInt16Item(SID_ATTR_DEFTABSTOP,
                    (sal_uInt16)m_pTabMF->Denormalize(m_pTabMF->GetValue(FUNIT_TWIP))));
        bRet = sal_True;
    }

    sal_Bool bIsUseCharUnitFlag = m_pUseCharUnit->IsChecked();
    SvtCJKOptions aCJKOptions;
        bIsUseCharUnitFlag = bIsUseCharUnitFlag && aCJKOptions.IsAsianTypographyEnabled();
    if( bIsUseCharUnitFlag != m_pUseCharUnit->GetSavedValue())
    {
        rSet.Put(SfxBoolItem(SID_ATTR_APPLYCHARUNIT, bIsUseCharUnitFlag ));
        bRet = sal_True;
    }

    if (m_pWordCountED->GetText() != m_pWordCountED->GetSavedValue())
    {
        boost::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Writer::WordCount::AdditionalSeparators::set(m_pWordCountED->GetText(), batch);
        batch->commit();
        bRet = sal_True;
    }

    sal_Bool bIsSquaredPageModeFlag = m_pUseSquaredPageMode->IsChecked();
    if ( bIsSquaredPageModeFlag != m_pUseSquaredPageMode->GetSavedValue() )
    {
        pMod->ApplyDefaultPageMode( bIsSquaredPageModeFlag );
        if ( m_pWrtShell )
        {
            SwDoc* pDoc = m_pWrtShell->GetDoc();
            pDoc->SetDefaultPageMode( bIsSquaredPageModeFlag );
            m_pWrtShell->SetModified();
        }
        bRet = sal_True;
    }

    return bRet;
}

void SwLoadOptPage::Reset( const SfxItemSet& rSet)
{
    const SwMasterUsrPref* pUsrPref = SW_MOD()->GetUsrPref(sal_False);
    const SfxPoolItem* pItem;

    if(SFX_ITEM_SET == rSet.GetItemState(FN_PARAM_WRTSHELL, sal_False, &pItem))
        m_pWrtShell = (SwWrtShell*)((const SwPtrItem*)pItem)->GetValue();

    SwFldUpdateFlags eFldFlags = AUTOUPD_GLOBALSETTING;
    m_nOldLinkMode = GLOBALSETTING;
    if (m_pWrtShell)
    {
        eFldFlags = m_pWrtShell->GetFldUpdateFlags(sal_True);
        m_nOldLinkMode = m_pWrtShell->GetLinkUpdMode(sal_True);
    }
    if(GLOBALSETTING == m_nOldLinkMode)
        m_nOldLinkMode = pUsrPref->GetUpdateLinkMode();
    if(AUTOUPD_GLOBALSETTING == eFldFlags)
        eFldFlags = pUsrPref->GetFldUpdateFlags();

    m_pAutoUpdateFields->Check(eFldFlags != AUTOUPD_OFF);
    m_pAutoUpdateCharts->Check(eFldFlags == AUTOUPD_FIELD_AND_CHARTS);

    switch (m_nOldLinkMode)
    {
        case NEVER:     m_pNeverRB->Check();   break;
        case MANUAL:    m_pRequestRB->Check(); break;
        case AUTOMATIC: m_pAlwaysRB->Check();  break;
    }

    m_pAutoUpdateFields->SaveValue();
    m_pAutoUpdateCharts->SaveValue();
    m_pMetricLB->SetNoSelection();
    if ( rSet.GetItemState( SID_ATTR_METRIC ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rSet.Get( SID_ATTR_METRIC );
        FieldUnit eFieldUnit = (FieldUnit)rItem.GetValue();

        for ( sal_uInt16 i = 0; i < m_pMetricLB->GetEntryCount(); ++i )
        {
            if ( (int)(sal_IntPtr)m_pMetricLB->GetEntryData( i ) == (int)eFieldUnit )
            {
                m_pMetricLB->SelectEntryPos( i );
                break;
            }
        }
        ::SetFieldUnit(*m_pTabMF, eFieldUnit);
    }
    m_pMetricLB->SaveValue();
    if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_DEFTABSTOP, sal_False, &pItem))
    {
        m_nLastTab = ((SfxUInt16Item*)pItem)->GetValue();
        m_pTabMF->SetValue(m_pTabMF->Normalize(m_nLastTab), FUNIT_TWIP);
    }
    m_pTabMF->SaveValue();

    //default page mode loading
    if(m_pWrtShell)
    {
        bool bSquaredPageMode = m_pWrtShell->GetDoc()->IsSquaredPageMode();
        m_pUseSquaredPageMode->Check( bSquaredPageMode );
        m_pUseSquaredPageMode->SaveValue();
    }

    if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_APPLYCHARUNIT, sal_False, &pItem))
    {
        bool bUseCharUnit = ((const SfxBoolItem*)pItem)->GetValue();
        m_pUseCharUnit->Check(bUseCharUnit);
    }
    else
    {
        m_pUseCharUnit->Check(pUsrPref->IsApplyCharUnit());
    }
    m_pUseCharUnit->SaveValue();

    m_pWordCountED->SetText(officecfg::Office::Writer::WordCount::AdditionalSeparators::get());
    m_pWordCountED->SaveValue();
}

IMPL_LINK_NOARG(SwLoadOptPage, MetricHdl)
{
    const sal_uInt16 nMPos = m_pMetricLB->GetSelectEntryPos();
    if(nMPos != USHRT_MAX)
    {
        // Double-Cast for VA3.0
        FieldUnit eFieldUnit = (FieldUnit)(sal_IntPtr)m_pMetricLB->GetEntryData( nMPos );
        sal_Bool bModified = m_pTabMF->IsModified();
        long nVal = bModified ?
            sal::static_int_cast<sal_Int32, sal_Int64 >( m_pTabMF->Denormalize( m_pTabMF->GetValue( FUNIT_TWIP ) )) :
                m_nLastTab;
        ::SetFieldUnit( *m_pTabMF, eFieldUnit );
        m_pTabMF->SetValue( m_pTabMF->Normalize( nVal ), FUNIT_TWIP );
        if(!bModified)
            m_pTabMF->ClearModifyFlag();
    }

    return 0;
}

SwCaptionOptDlg::SwCaptionOptDlg(Window* pParent, const SfxItemSet& rSet) :
    SfxNoLayoutSingleTabDialog(pParent, rSet, 0)
{
    // create TabPage
    SetTabPage((SwCaptionOptPage*) SwCaptionOptPage::Create(this, rSet));
}

SwCaptionOptDlg::~SwCaptionOptDlg()
{
}

SwCaptionPreview::SwCaptionPreview( Window* pParent )
    : Window( pParent )
{
    Init();
}

SwCaptionPreview::SwCaptionPreview( Window* pParent, const ResId& rResId )
    : Window( pParent, rResId )
{
    Init();
}

void SwCaptionPreview::Init()
{
    maDrawPos = Point( 4, 6 );

    Wallpaper   aBack( GetSettings().GetStyleSettings().GetWindowColor() );
    SetBackground( aBack );
    SetFillColor( aBack.GetColor() );
    SetLineColor( aBack.GetColor() );
    SetBorderStyle( WINDOW_BORDER_MONO );
    Font aFont(GetFont());
    aFont.SetHeight(aFont.GetHeight() * 120 / 100 );
    SetFont(aFont);
}

void SwCaptionPreview::SetPreviewText( const String& rText )
{
    if( rText != maText )
    {
        maText = rText;
        Invalidate();
    }
}

void SwCaptionPreview::Paint( const Rectangle& rRect )
{
    Window::Paint( rRect );

    DrawRect( Rectangle( Point( 0, 0 ), GetSizePixel() ) );
    DrawText( Point( 4, 6 ), maText );
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSwCaptionPreview(Window* pParent, VclBuilder::stringmap &)
{
    return new SwCaptionPreview(pParent);
}

SwCaptionOptPage::SwCaptionOptPage( Window* pParent, const SfxItemSet& rSet )
    : SfxTabPage(pParent, SW_RES(TP_OPTCAPTION_PAGE), rSet),
    aCheckFT        (this, SW_RES(FT_OBJECTS    )),
    aCheckLB        (this, SW_RES(CLB_OBJECTS   )),
    aFtCaptionOrder(this, SW_RES( FT_ORDER )),
    aLbCaptionOrder(this, SW_RES( LB_ORDER )),
    aPreview        (this, SW_RES(WIN_PREVIEW   )),
    aSettingsGroupFL(this, SW_RES(FL_SETTINGS_2 )),
    aCategoryText   (this, SW_RES(TXT_CATEGORY  )),
    aCategoryBox    (this, SW_RES(BOX_CATEGORY  )),
    aFormatText     (this, SW_RES(TXT_FORMAT    )),
    aFormatBox      (this, SW_RES(BOX_FORMAT    )),
    aNumberingSeparatorFT(this, SW_RES(FT_NUM_SEP  )),
    aNumberingSeparatorED(this, SW_RES(ED_NUM_SEP  )),
    aTextText       (this, SW_RES(TXT_TEXT      )),
    aTextEdit       (this, SW_RES(EDT_TEXT      )),
    aPosText        (this, SW_RES(TXT_POS       )),
    aPosBox         (this, SW_RES(BOX_POS       )),
    aNumCaptFL      (this, SW_RES(FL_NUMCAPT    )),
    aFtLevel        (this, SW_RES(FT_LEVEL      )),
    aLbLevel        (this, SW_RES(LB_LEVEL      )),
    aFtDelim        (this, SW_RES(FT_SEPARATOR  )),
    aEdDelim        (this, SW_RES(ED_SEPARATOR  )),
    aCategoryFL     (this, SW_RES(FL_CATEGORY   )),
    aCharStyleFT    (this, SW_RES(FT_CHARSTYLE  )),
    aCharStyleLB    (this, SW_RES(LB_CHARSTYLE  )),
    aApplyBorderCB  (this, SW_RES(CB_APPLYBORDER)),

    m_sSWTable      (SW_RES(STR_TABLE           )),
    m_sSWFrame      (SW_RES(STR_FRAME           )),
    m_sSWGraphic    (SW_RES(STR_GRAPHIC         )),
    m_sOLE          (SW_RES(STR_OLE             )),

    m_sBegin        (SW_RESSTR(STR_BEGINNING            )),
    m_sEnd          (SW_RESSTR(STR_END                  )),
    m_sAbove        (SW_RESSTR(STR_ABOVE                )),
    m_sBelow        (SW_RESSTR(STR_CP_BELOW             )),
    m_sNone         (SW_RESSTR( STR_CATEGORY_NONE )),

    pMgr            (new SwFldMgr()),
    bHTMLMode(sal_False)
{
    Wallpaper   aBack( GetSettings().GetStyleSettings().GetWindowColor() );
    aPreview.SetBackground( aBack );

    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_ABB, m_sIllustration);
    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_TABLE, m_sTable);
    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_FRAME, m_sText);
    SwStyleNameMapper::FillUIName(RES_POOLCOLL_LABEL_DRAWING, m_sDrawing);

    sal_uInt16 i, nCount;
    SwWrtShell *pSh = ::GetActiveWrtShell();

    // aFormatBox
    sal_uInt16 nSelFmt = SVX_NUM_ARABIC;
    if (pSh)
    {
        nCount = pMgr->GetFldTypeCount();
        SwFieldType* pFldType;
        for ( i = nCount; i; )
        {
            pFldType = pMgr->GetFldType(USHRT_MAX, --i);
            if (pFldType->GetName().equals(aCategoryBox.GetText()))
            {
                nSelFmt = (sal_uInt16)((SwSetExpFieldType*)pFldType)->GetSeqFormat();
                break;
            }
        }

        ::FillCharStyleListBox( aCharStyleLB, pSh->GetView().GetDocShell(), true, true );
    }


    nCount = pMgr->GetFormatCount(TYP_SEQFLD, false);
    for ( i = 0; i < nCount; ++i )
    {
        aFormatBox.InsertEntry( pMgr->GetFormatStr(TYP_SEQFLD, i) );
        sal_uInt16 nFmtId = pMgr->GetFormatId(TYP_SEQFLD, i);
        aFormatBox.SetEntryData( i, reinterpret_cast<void*>(nFmtId) );
        if( nFmtId == nSelFmt )
            aFormatBox.SelectEntryPos( i );
    }

    for (i = 0; i < MAXLEVEL; i++)
        aLbLevel.InsertEntry(OUString::number(i + 1));

    sal_Unicode nLvl = MAXLEVEL;
    OUString sDelim(": ");

    if (pSh)
    {
        SwSetExpFieldType* pFldType = (SwSetExpFieldType*)pMgr->GetFldType(
                                            RES_SETEXPFLD, aCategoryBox.GetText() );
        if( pFldType )
        {
            sDelim = pFldType->GetDelimiter();
            nLvl = pFldType->GetOutlineLvl();
        }
    }

    aLbLevel.SelectEntryPos( nLvl < MAXLEVEL ? nLvl + 1 : 0 );
    aEdDelim.SetText( sDelim );

    aCheckLB.SetHelpId(HID_OPTCAPTION_CLB);

    FreeResource();

    Link aLk = LINK( this, SwCaptionOptPage, ModifyHdl );
    aCategoryBox.SetModifyHdl( aLk );
    aNumberingSeparatorED.SetModifyHdl( aLk );
    aTextEdit   .SetModifyHdl( aLk );

    aLk = LINK(this, SwCaptionOptPage, SelectHdl);
    aCategoryBox.SetSelectHdl( aLk );
    aFormatBox  .SetSelectHdl( aLk );

    aLbCaptionOrder.SetSelectHdl( LINK(this, SwCaptionOptPage, OrderHdl));

    aCheckLB.SetSelectHdl( LINK(this, SwCaptionOptPage, ShowEntryHdl) );
    aCheckLB.SetCheckButtonHdl( LINK(this, SwCaptionOptPage, ShowEntryHdl) );
    aCheckLB.SetDeselectHdl( LINK(this, SwCaptionOptPage, SaveEntryHdl) );
}

SwCaptionOptPage::~SwCaptionOptPage()
{
    DelUserData();
    delete pMgr;
}

SfxTabPage* SwCaptionOptPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet )
{
    return new SwCaptionOptPage(pParent, rAttrSet );
}

sal_Bool SwCaptionOptPage::FillItemSet( SfxItemSet&  )
{
    sal_Bool bRet = sal_False;
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();

    SaveEntry(aCheckLB.FirstSelected());    // apply current entry

    SvTreeListEntry* pEntry = aCheckLB.First();

    while (pEntry)
    {
        InsCaptionOpt* pData = (InsCaptionOpt*)pEntry->GetUserData();
        bRet |= pModOpt->SetCapOption(bHTMLMode, pData);
        pEntry = aCheckLB.Next(pEntry);
    }

    sal_uInt16 nCheckCount = aCheckLB.GetCheckedEntryCount();
    pModOpt->SetInsWithCaption( bHTMLMode, nCheckCount > 0 );

    sal_Int32 nPos = aLbCaptionOrder.GetSelectEntryPos();
    pModOpt->SetCaptionOrderNumberingFirst(nPos == 1 ? sal_True : sal_False );

    return bRet;
}

void SwCaptionOptPage::Reset( const SfxItemSet& rSet)
{
    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem))
    {
        bHTMLMode = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);
    }

    DelUserData();
    aCheckLB.GetModel()->Clear();   // remove all entries

    // Writer objects
    sal_uInt16 nPos = 0;
    aCheckLB.InsertEntry(m_sSWTable);
    SetOptions(nPos++, TABLE_CAP);
    aCheckLB.InsertEntry(m_sSWFrame);
    SetOptions(nPos++, FRAME_CAP);
    aCheckLB.InsertEntry(m_sSWGraphic);
    SetOptions(nPos++, GRAPHIC_CAP);

    // get Productname and -version
    OUString sWithoutVersion( utl::ConfigManager::getProductName() );
    OUString sComplete(
        sWithoutVersion + OUString(" ") +
        utl::ConfigManager::getProductVersion() );

    SvObjectServerList aObjS;
    aObjS.FillInsertObjects();
    aObjS.Remove( SvGlobalName( SO3_SW_CLASSID ) ); // remove Writer-ID

    for ( sal_uLong i = 0; i < aObjS.Count(); ++i )
    {
        const SvGlobalName &rOleId = aObjS[i].GetClassName();
        String sClass;
        if (rOleId == SvGlobalName(SO3_OUT_CLASSID))
            sClass = aObjS[i].GetHumanName();
        else
            sClass = m_sOLE;
        // don't show product version
        sClass.SearchAndReplace( sComplete, sWithoutVersion );
        aCheckLB.InsertEntry( sClass );
        SetOptions( nPos++, OLE_CAP, &rOleId );
    }
    aLbCaptionOrder.SelectEntryPos(
        SW_MOD()->GetModuleConfig()->IsCaptionOrderNumberingFirst() ? 1 : 0);
    ModifyHdl();
}

void SwCaptionOptPage::SetOptions(const sal_uInt16 nPos,
        const SwCapObjType eObjType, const SvGlobalName *pOleId)
{
    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
    const InsCaptionOpt* pOpt = pModOpt->GetCapOption(bHTMLMode, eObjType, pOleId);

    if (pOpt)
    {
        aCheckLB.SetEntryData(nPos, new InsCaptionOpt(*pOpt));
        aCheckLB.CheckEntryPos(nPos, pOpt->UseCaption());
    }
    else
        aCheckLB.SetEntryData(nPos, new InsCaptionOpt(eObjType, pOleId));
}

void SwCaptionOptPage::DelUserData()
{
    SvTreeListEntry* pEntry = aCheckLB.First();

    while (pEntry)
    {
        delete (InsCaptionOpt*)pEntry->GetUserData();
        pEntry->SetUserData(0);
        pEntry = aCheckLB.Next(pEntry);
    }
}

IMPL_LINK_NOARG(SwCaptionOptPage, ShowEntryHdl)
{
    SvTreeListEntry* pSelEntry = aCheckLB.FirstSelected();

    if (pSelEntry)
    {
        sal_Bool bChecked = aCheckLB.IsChecked((sal_uInt16)aCheckLB.GetModel()->GetAbsPos(pSelEntry));

        aSettingsGroupFL.Enable( bChecked );
        aCategoryText.Enable( bChecked );
        aCategoryBox.Enable( bChecked );
        aFormatText.Enable( bChecked );
        aFormatBox.Enable( bChecked );
        sal_Bool bNumSep = bChecked && aLbCaptionOrder.GetSelectEntryPos() == 1;
        aNumberingSeparatorED.Enable( bNumSep );
        aNumberingSeparatorFT.Enable( bNumSep );
        aTextText.Enable( bChecked );
        aTextEdit.Enable( bChecked );
        aPosText.Enable( bChecked );
        aPosBox.Enable( bChecked );
        aNumCaptFL.Enable( bChecked );
        aFtLevel.Enable( bChecked );
        aLbLevel.Enable( bChecked );
        aFtDelim.Enable( bChecked );
        aEdDelim.Enable( bChecked );
        aCategoryFL.Enable( bChecked );
        aCharStyleFT.Enable( bChecked );
        aCharStyleLB.Enable( bChecked );
        aApplyBorderCB.Enable( bChecked );
        aPreview.Enable( bChecked );

        SwWrtShell *pSh = ::GetActiveWrtShell();

        InsCaptionOpt* pOpt = (InsCaptionOpt*)pSelEntry->GetUserData();

        aCategoryBox.Clear();
        aCategoryBox.InsertEntry(String(m_sNone)); //FIXME
        if (pSh)
        {
            sal_uInt16 nCount = pMgr->GetFldTypeCount();

            for (sal_uInt16 i = 0; i < nCount; i++)
            {
                SwFieldType *pType = pMgr->GetFldType( USHRT_MAX, i );
                if( pType->Which() == RES_SETEXPFLD &&
                    ((SwSetExpFieldType *) pType)->GetType() & nsSwGetSetExpType::GSE_SEQ )
                    aCategoryBox.InsertEntry(SwBoxEntry(pType->GetName()));
            }
        }
        else
        {
            aCategoryBox.InsertEntry(SwBoxEntry(m_sIllustration));
            aCategoryBox.InsertEntry(SwBoxEntry(m_sTable));
            aCategoryBox.InsertEntry(SwBoxEntry(m_sText));
            aCategoryBox.InsertEntry(SwBoxEntry(m_sDrawing));
        }

        if(pOpt->GetCategory().Len())
            aCategoryBox.SetText(pOpt->GetCategory());
        else
            aCategoryBox.SetText(m_sNone);
        if (pOpt->GetCategory().Len() &&
            aCategoryBox.GetEntryPos(OUString(pOpt->GetCategory())) == COMBOBOX_ENTRY_NOTFOUND)
            aCategoryBox.InsertEntry(pOpt->GetCategory());
        if (aCategoryBox.GetText().isEmpty())
        {
            sal_uInt16 nPos = 0;
            switch(pOpt->GetObjType())
            {
                case OLE_CAP:
                case GRAPHIC_CAP:       nPos = 1;   break;
                case TABLE_CAP:         nPos = 2;   break;
                case FRAME_CAP:         nPos = 3;   break;
            }
            aCategoryBox.SetText(aCategoryBox.GetEntry(nPos).GetName());
        }

        for (sal_uInt16 i = 0; i < aFormatBox.GetEntryCount(); i++)
        {
            if (pOpt->GetNumType() == (sal_uInt16)(sal_uLong)aFormatBox.GetEntryData(i))
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
                aPosBox.InsertEntry(m_sAbove);
                aPosBox.InsertEntry(m_sBelow);
                break;
            case FRAME_CAP:
                aPosBox.InsertEntry(m_sBegin);
                aPosBox.InsertEntry(m_sEnd);
                break;
        }
        aPosBox.SelectEntryPos(pOpt->GetPos());
        aPosBox.Enable( pOpt->GetObjType() != GRAPHIC_CAP &&
                pOpt->GetObjType() != OLE_CAP &&
                aPosText.IsEnabled() );
        aPosBox.SelectEntryPos(pOpt->GetPos());

        sal_uInt16 nLevelPos = ( pOpt->GetLevel() < MAXLEVEL ) ? pOpt->GetLevel() + 1 : 0;
        aLbLevel.SelectEntryPos( nLevelPos );
        aEdDelim.SetText(pOpt->GetSeparator());
        aNumberingSeparatorED.SetText( pOpt->GetNumSeparator() );
        if(pOpt->GetCharacterStyle().Len())
            aCharStyleLB.SelectEntry( pOpt->GetCharacterStyle() );
        else
            aCharStyleLB.SelectEntryPos( 0 );
        aApplyBorderCB.Enable( aCategoryBox.IsEnabled() &&
                pOpt->GetObjType() != TABLE_CAP && pOpt->GetObjType() != FRAME_CAP );
        aApplyBorderCB.Check( pOpt->CopyAttributes() );
    }

    ModifyHdl();

    return 0;
}

IMPL_LINK_NOARG(SwCaptionOptPage, SaveEntryHdl)
{
    SvTreeListEntry* pEntry = aCheckLB.GetHdlEntry();

    if (pEntry) // save all
        SaveEntry(pEntry);

    return 0;
}

void SwCaptionOptPage::SaveEntry(SvTreeListEntry* pEntry)
{
    if (pEntry)
    {
        InsCaptionOpt* pOpt = (InsCaptionOpt*)pEntry->GetUserData();

        pOpt->UseCaption() = aCheckLB.IsChecked((sal_uInt16)aCheckLB.GetModel()->GetAbsPos(pEntry));
        String aName( aCategoryBox.GetText() );
        if (aName == m_sNone)
            pOpt->SetCategory(aEmptyStr);
        else
            pOpt->SetCategory(comphelper::string::strip(aName, ' '));
        pOpt->SetNumType((sal_uInt16)(sal_uLong)aFormatBox.GetEntryData(aFormatBox.GetSelectEntryPos()));
        pOpt->SetCaption(aTextEdit.IsEnabled() ? aTextEdit.GetText() : OUString(aEmptyStr) );
        pOpt->SetPos(aPosBox.GetSelectEntryPos());
        sal_uInt16 nPos = aLbLevel.GetSelectEntryPos();
        sal_uInt16 nLevel = ( nPos > 0 && nPos != LISTBOX_ENTRY_NOTFOUND ) ? nPos - 1 : MAXLEVEL;
        pOpt->SetLevel(nLevel);
        pOpt->SetSeparator(aEdDelim.GetText());
        pOpt->SetNumSeparator( aNumberingSeparatorED.GetText());
        if(!aCharStyleLB.GetSelectEntryPos())
            pOpt->SetCharacterStyle(aEmptyStr);
        else
            pOpt->SetCharacterStyle(aCharStyleLB.GetSelectEntry());
        pOpt->CopyAttributes() = aApplyBorderCB.IsChecked();
    }
}

IMPL_LINK_NOARG(SwCaptionOptPage, ModifyHdl)
{
    String sFldTypeName = aCategoryBox.GetText();

    SfxNoLayoutSingleTabDialog *pDlg = dynamic_cast<SfxNoLayoutSingleTabDialog*>(GetParentDialog());
    PushButton *pBtn = pDlg ? pDlg->GetOKButton() : NULL;
    if (pBtn)
        pBtn->Enable(sFldTypeName.Len() != 0);
    sal_Bool bEnable = aCategoryBox.IsEnabled() && sFldTypeName != m_sNone;

    aFormatText.Enable(bEnable);
    aFormatBox.Enable(bEnable);
    aTextText.Enable(bEnable);
    aTextEdit.Enable(bEnable);

    DrawSample();
    return 0;
}

IMPL_LINK_NOARG_INLINE_START(SwCaptionOptPage, SelectHdl)
{
    DrawSample();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwCaptionOptPage, SelectHdl)

IMPL_LINK( SwCaptionOptPage, OrderHdl, ListBox*, pBox )
{
    DrawSample();

    SvTreeListEntry* pSelEntry = aCheckLB.FirstSelected();
    sal_Bool bChecked = sal_False;
    if (pSelEntry)
    {
        bChecked = aCheckLB.IsChecked((sal_uInt16)aCheckLB.GetModel()->GetAbsPos(pSelEntry));
    }

    sal_Int32 nPos = pBox->GetSelectEntryPos();
    aNumberingSeparatorFT.Enable( bChecked && nPos == 1 );
    aNumberingSeparatorED.Enable( bChecked && nPos == 1 );
    return 0;
}

void SwCaptionOptPage::DrawSample()
{
    String aStr;

    if( aCategoryBox.GetText() != m_sNone)
    {
        //#i61007# order of captions
        bool bOrderNumberingFirst = aLbCaptionOrder.GetSelectEntryPos() == 1;
        // number
        sal_uInt16 nNumFmt = (sal_uInt16)(sal_uLong)aFormatBox.GetEntryData(
                                        aFormatBox.GetSelectEntryPos() );
        if( SVX_NUM_NUMBER_NONE != nNumFmt )
        {
            //#i61007# order of captions
            if( !bOrderNumberingFirst )
            {
                // category
                aStr += aCategoryBox.GetText();
                aStr += ' ';
            }

            SwWrtShell *pSh = ::GetActiveWrtShell();
            String sFldTypeName( aCategoryBox.GetText() );
            if (pSh)
            {
                SwSetExpFieldType* pFldType = (SwSetExpFieldType*)pMgr->GetFldType(
                                                RES_SETEXPFLD, sFldTypeName );
                if( pFldType && pFldType->GetOutlineLvl() < MAXLEVEL )
                {
                    sal_uInt8 nLvl = pFldType->GetOutlineLvl();
                    SwNumberTree::tNumberVector aNumVector;
                    for( sal_uInt8 i = 0; i <= nLvl; ++i )
                        aNumVector.push_back(1);

                    String sNumber( pSh->GetOutlineNumRule()->MakeNumString(
                                                            aNumVector, sal_False ));
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
        //#i61007# order of captions
        if( bOrderNumberingFirst )
        {
            aStr += aNumberingSeparatorED.GetText();
            aStr += aCategoryBox.GetText();
        }
        aStr += aTextEdit.GetText();
    }
    aPreview.SetPreviewText( aStr );
}

// Description: ComboBox without Spaces
void CaptionComboBox::KeyInput(const KeyEvent& rEvt)
{
    if( rEvt.GetKeyCode().GetCode() != KEY_SPACE )
        SwComboBox::KeyInput(rEvt);
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
