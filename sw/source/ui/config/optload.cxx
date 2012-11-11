/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
#include <svx/htmlmode.hxx>
#include <fldmgr.hxx>
#include <poolfmt.hxx>
#include <expfld.hxx>
#include <caption.hxx>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>

#include <svtools/insdlg.hxx>
#include <sot/clsids.hxx>
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

SwLoadOptPage::SwLoadOptPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, SW_RES( TP_OPTLOAD_PAGE ), rSet ),

    aUpdateFL           ( this, SW_RES( FL_UPDATE ) ),
    aLinkFT             ( this, SW_RES( FT_LINK ) ),
    aAlwaysRB           ( this, SW_RES( RB_ALWAYS ) ),
    aRequestRB          ( this, SW_RES( RB_REQUEST ) ),
    aNeverRB            ( this, SW_RES( RB_NEVER  ) ),

    aFieldFT            ( this, SW_RES( FT_FIELD ) ),
    aAutoUpdateFields   ( this, SW_RES( CB_AUTO_UPDATE_FIELDS ) ),
    aAutoUpdateCharts   ( this, SW_RES( CB_AUTO_UPDATE_CHARTS ) ),

    aSettingsFL         ( this, SW_RES( FL_SETTINGS ) ),
    aMetricFT           ( this, SW_RES( FT_METRIC ) ),
    aMetricLB           ( this, SW_RES( LB_METRIC ) ),
    aTabFT              ( this, SW_RES( FT_TAB ) ),
    aTabMF              ( this, SW_RES( MF_TAB ) ),
    aUseSquaredPageMode ( this, SW_RES( CB_USE_SQUARE_PAGE_MODE ) ),
    aUseCharUnit        ( this , SW_RES( CB_USE_CHAR_UNIT ) ),
    aWordCountFL        ( this , SW_RES( FL_WORDCOUNT ) ),
    aWordCountFT        ( this , SW_RES( FT_WORDCOUNT ) ),
    aWordCountED        ( this , SW_RES( ED_WORDCOUNT ) ),

    pWrtShell   ( NULL ),
    bHTMLMode   ( sal_False ),
    nLastTab    ( 0 ),
    nOldLinkMode( MANUAL )

{
    FreeResource();

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
                sal_uInt16 nPos = aMetricLB.InsertEntry( sMetric );
                aMetricLB.SetEntryData( nPos, (void*)(long)eFUnit );
            }
            default:; //prevent warning
        }
    }
    aMetricLB.SetSelectHdl(LINK(this, SwLoadOptPage, MetricHdl));

    const SfxPoolItem* pItem;
    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem )
        && ((SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON)
    {
        aTabFT.Hide();
        aTabMF.Hide();
    }

    SvtCJKOptions aCJKOptions;
    if(!aCJKOptions.IsAsianTypographyEnabled())
    {
        aUseSquaredPageMode.Hide();
        aUseCharUnit.Hide();
    }
}

SwLoadOptPage::~SwLoadOptPage()
{
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
    if (aNeverRB.IsChecked())
        nNewLinkMode = NEVER;
    else if (aRequestRB.IsChecked())
        nNewLinkMode = MANUAL;

    SwFldUpdateFlags eFldFlags = aAutoUpdateFields.IsChecked() ?
        aAutoUpdateCharts.IsChecked() ? AUTOUPD_FIELD_AND_CHARTS : AUTOUPD_FIELD_ONLY : AUTOUPD_OFF;

    if(aAutoUpdateFields.IsChecked() != aAutoUpdateFields.GetSavedValue() ||
            aAutoUpdateCharts.IsChecked() != aAutoUpdateCharts.GetSavedValue())
    {
        pMod->ApplyFldUpdateFlags(eFldFlags);
        if(pWrtShell)
        {
            pWrtShell->SetFldUpdateFlags(eFldFlags);
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

        bRet = sal_True;
    }

    const sal_uInt16 nMPos = aMetricLB.GetSelectEntryPos();
    if ( nMPos != aMetricLB.GetSavedValue() )
    {
        // Double-Cast for VA3.0
        sal_uInt16 nFieldUnit = (sal_uInt16)(long)aMetricLB.GetEntryData( nMPos );
        rSet.Put( SfxUInt16Item( SID_ATTR_METRIC, (sal_uInt16)nFieldUnit ) );
        bRet = sal_True;
    }

    if(aTabMF.IsVisible() && aTabMF.GetText() != aTabMF.GetSavedValue())
    {
        rSet.Put(SfxUInt16Item(SID_ATTR_DEFTABSTOP,
                    (sal_uInt16)aTabMF.Denormalize(aTabMF.GetValue(FUNIT_TWIP))));
        bRet = sal_True;
    }

    sal_Bool bIsUseCharUnitFlag = aUseCharUnit.IsChecked();
    SvtCJKOptions aCJKOptions;
        bIsUseCharUnitFlag = bIsUseCharUnitFlag && aCJKOptions.IsAsianTypographyEnabled();
    if( bIsUseCharUnitFlag != aUseCharUnit.GetSavedValue())
    {
        rSet.Put(SfxBoolItem(SID_ATTR_APPLYCHARUNIT, bIsUseCharUnitFlag ));
        bRet = sal_True;
    }

    if (aWordCountED.GetText() != aWordCountED.GetSavedValue())
    {
        boost::shared_ptr< comphelper::ConfigurationChanges > batch(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Writer::WordCount::AdditionalSeperators::set(aWordCountED.GetText(), batch);
        batch->commit();
        bRet = sal_True;
    }

    sal_Bool bIsSquaredPageModeFlag = aUseSquaredPageMode.IsChecked();
    if ( bIsSquaredPageModeFlag != aUseSquaredPageMode.GetSavedValue() )
    {
        pMod->ApplyDefaultPageMode( bIsSquaredPageModeFlag );
        if ( pWrtShell )
        {
            SwDoc* pDoc = pWrtShell->GetDoc();
            pDoc->SetDefaultPageMode( bIsSquaredPageModeFlag );
            pWrtShell->SetModified();
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
        pWrtShell = (SwWrtShell*)((const SwPtrItem*)pItem)->GetValue();

    SwFldUpdateFlags eFldFlags = AUTOUPD_GLOBALSETTING;
    nOldLinkMode = GLOBALSETTING;
    if (pWrtShell)
    {
        eFldFlags = pWrtShell->GetFldUpdateFlags(sal_True);
        nOldLinkMode = pWrtShell->GetLinkUpdMode(sal_True);
    }
    if(GLOBALSETTING == nOldLinkMode)
        nOldLinkMode = pUsrPref->GetUpdateLinkMode();
    if(AUTOUPD_GLOBALSETTING == eFldFlags)
        eFldFlags = pUsrPref->GetFldUpdateFlags();

    aAutoUpdateFields.Check(eFldFlags != AUTOUPD_OFF);
    aAutoUpdateCharts.Check(eFldFlags == AUTOUPD_FIELD_AND_CHARTS);

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

        for ( sal_uInt16 i = 0; i < aMetricLB.GetEntryCount(); ++i )
        {
            if ( (int)(sal_IntPtr)aMetricLB.GetEntryData( i ) == (int)eFieldUnit )
            {
                aMetricLB.SelectEntryPos( i );
                break;
            }
        }
        ::SetFieldUnit(aTabMF, eFieldUnit);
    }
    aMetricLB.SaveValue();
    if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_DEFTABSTOP, sal_False, &pItem))
    {
        nLastTab = ((SfxUInt16Item*)pItem)->GetValue();
        aTabMF.SetValue(aTabMF.Normalize(nLastTab), FUNIT_TWIP);
    }
    aTabMF.SaveValue();

    if(SFX_ITEM_SET == rSet.GetItemState(SID_HTML_MODE, sal_False, &pItem))
    {
        bHTMLMode = 0 != (((const SfxUInt16Item*)pItem)->GetValue() & HTMLMODE_ON);
    }

    //default page mode loading
    if(pWrtShell)
    {
        sal_Bool bSquaredPageMode = pWrtShell->GetDoc()->IsSquaredPageMode();
        aUseSquaredPageMode.Check( bSquaredPageMode );
            aUseSquaredPageMode.SaveValue();
    }

    if(SFX_ITEM_SET == rSet.GetItemState(SID_ATTR_APPLYCHARUNIT, sal_False, &pItem))
    {
        sal_Bool bUseCharUnit = ((const SfxBoolItem*)pItem)->GetValue();
        aUseCharUnit.Check(bUseCharUnit);
    }
    else
    {
        aUseCharUnit.Check(pUsrPref->IsApplyCharUnit());
    }
    aUseCharUnit.SaveValue();

    aWordCountED.SetText(officecfg::Office::Writer::WordCount::AdditionalSeperators::get());
    aWordCountED.SaveValue();
}

IMPL_LINK_NOARG(SwLoadOptPage, MetricHdl)
{
    const sal_uInt16 nMPos = aMetricLB.GetSelectEntryPos();
    if(nMPos != USHRT_MAX)
    {
        // Double-Cast for VA3.0
        FieldUnit eFieldUnit = (FieldUnit)(long)aMetricLB.GetEntryData( nMPos );
        sal_Bool bModified = aTabMF.IsModified();
        long nVal = bModified ?
            sal::static_int_cast<sal_Int32, sal_Int64 >( aTabMF.Denormalize( aTabMF.GetValue( FUNIT_TWIP ) )) :
                nLastTab;
        ::SetFieldUnit( aTabMF, eFieldUnit );
        aTabMF.SetValue( aTabMF.Normalize( nVal ), FUNIT_TWIP );
        if(!bModified)
            aTabMF.ClearModifyFlag();
    }

    return 0;
}

SwCaptionOptDlg::SwCaptionOptDlg(Window* pParent, const SfxItemSet& rSet) :
    SfxSingleTabDialog(pParent, rSet, 0)
{
    // create TabPage
    SetTabPage((SwCaptionOptPage*) SwCaptionOptPage::Create(this, rSet));
}

SwCaptionOptDlg::~SwCaptionOptDlg()
{
}

SwCaptionPreview::SwCaptionPreview( Window* pParent, const ResId& rResId )
    : Window( pParent, rResId )
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

    sSWTable        (SW_RES(STR_TABLE           )),
    sSWFrame        (SW_RES(STR_FRAME           )),
    sSWGraphic      (SW_RES(STR_GRAPHIC         )),
    sOLE            (SW_RES(STR_OLE             )),

    sBegin          (SW_RESSTR(STR_BEGINNING            )),
    sEnd            (SW_RESSTR(STR_END                  )),
    sAbove          (SW_RESSTR(STR_ABOVE                )),
    sBelow          (SW_RESSTR(STR_CP_BELOW             )),
    sNone           (SW_RESSTR( STR_CATEGORY_NONE )),

    pMgr            (new SwFldMgr()),
    bHTMLMode(sal_False)
{
    Wallpaper   aBack( GetSettings().GetStyleSettings().GetWindowColor() );
    aPreview.SetBackground( aBack );

    SwStyleNameMapper::FillUIName( RES_POOLCOLL_LABEL_ABB, sIllustration );
    SwStyleNameMapper::FillUIName( RES_POOLCOLL_LABEL_TABLE, sTable );
    SwStyleNameMapper::FillUIName( RES_POOLCOLL_LABEL_FRAME, sText );
    SwStyleNameMapper::FillUIName( RES_POOLCOLL_LABEL_DRAWING, sDrawing );

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

        ::FillCharStyleListBox( aCharStyleLB, pSh->GetView().GetDocShell(), sal_True, sal_True );
    }


    nCount = pMgr->GetFormatCount(TYP_SEQFLD, sal_False);
    for ( i = 0; i < nCount; ++i )
    {
        aFormatBox.InsertEntry( pMgr->GetFormatStr(TYP_SEQFLD, i) );
        sal_uInt16 nFmtId = pMgr->GetFormatId(TYP_SEQFLD, i);
        aFormatBox.SetEntryData( i, reinterpret_cast<void*>(nFmtId) );
        if( nFmtId == nSelFmt )
            aFormatBox.SelectEntryPos( i );
    }

    for (i = 0; i < MAXLEVEL; i++)
        aLbLevel.InsertEntry(String::CreateFromInt32(i + 1));

    sal_Unicode nLvl = MAXLEVEL;
    rtl::OUString sDelim(": ");

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
    aCheckLB.InsertEntry(sSWTable);
    SetOptions(nPos++, TABLE_CAP);
    aCheckLB.InsertEntry(sSWFrame);
    SetOptions(nPos++, FRAME_CAP);
    aCheckLB.InsertEntry(sSWGraphic);
    SetOptions(nPos++, GRAPHIC_CAP);

    // get Productname and -version
    rtl::OUString sWithoutVersion( utl::ConfigManager::getProductName() );
    rtl::OUString sComplete(
        sWithoutVersion + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ")) +
        utl::ConfigManager::getProductVersion() );

    SvObjectServerList aObjS;
    aObjS.FillInsertObjects();
    aObjS.Remove( SvGlobalName( SO3_SW_CLASSID ) ); // remove Writer-ID

    for ( sal_uLong i = 0; i < aObjS.Count(); ++i )
    {
        const SvGlobalName &rOleId = aObjS[i].GetClassName();
        const String* pClassName = &aObjS[i].GetHumanName();
        if ( rOleId == SvGlobalName( SO3_OUT_CLASSID ) )
            pClassName = &sOLE;
        String sClass( *pClassName );
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
        aCategoryBox.InsertEntry( sNone );
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
            aCategoryBox.InsertEntry(SwBoxEntry(sIllustration));
            aCategoryBox.InsertEntry(SwBoxEntry(sTable));
            aCategoryBox.InsertEntry(SwBoxEntry(sText));
            aCategoryBox.InsertEntry(SwBoxEntry(sDrawing));
        }

        if(pOpt->GetCategory().Len())
            aCategoryBox.SetText(pOpt->GetCategory());
        else
            aCategoryBox.SetText( sNone );
        if (pOpt->GetCategory().Len() &&
            aCategoryBox.GetEntryPos(pOpt->GetCategory()) == COMBOBOX_ENTRY_NOTFOUND)
            aCategoryBox.InsertEntry(pOpt->GetCategory());
        if (!aCategoryBox.GetText().Len())
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
                aPosBox.InsertEntry(sAbove);
                aPosBox.InsertEntry(sBelow);
                break;
            case FRAME_CAP:
                aPosBox.InsertEntry(sBegin);
                aPosBox.InsertEntry(sEnd);
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
        if(aName == sNone)
            pOpt->SetCategory(aEmptyStr);
        else
            pOpt->SetCategory(comphelper::string::strip(aName, ' '));
        pOpt->SetNumType((sal_uInt16)(sal_uLong)aFormatBox.GetEntryData(aFormatBox.GetSelectEntryPos()));
        pOpt->SetCaption(aTextEdit.IsEnabled() ? aTextEdit.GetText() : aEmptyStr );
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

    SfxSingleTabDialog *pDlg = dynamic_cast<SfxSingleTabDialog*>(GetParent());
    PushButton *pBtn = pDlg ? pDlg->GetOKButton() : NULL;
    if (pBtn)
        pBtn->Enable(sFldTypeName.Len() != 0);
    sal_Bool bEnable = aCategoryBox.IsEnabled() && sFldTypeName != sNone;

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
    sal_Int32 nPos = pBox->GetSelectEntryPos();
    aNumberingSeparatorFT.Enable( nPos == 1 );
    aNumberingSeparatorED.Enable( nPos == 1 );
    return 0;
}

void SwCaptionOptPage::DrawSample()
{
    String aStr;

    if( aCategoryBox.GetText() != sNone)
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
