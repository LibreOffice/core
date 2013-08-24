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

#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <sfx2/module.hxx>
#include <sfx2/app.hxx>
#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <svx/strarray.hxx>
#include <svx/dlgutil.hxx>
#include <vcl/msgbox.hxx>

#include "sdattr.hxx"
#include "sdresid.hxx"
#include "optsitem.hxx"
#include "tpoption.hrc"
#include "tpoption.hxx"
#include "strings.hrc"
#include "app.hrc"
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SdTpOptionsSnap::SdTpOptionsSnap( Window* pParent, const SfxItemSet& rInAttrs  ) :
        SvxGridTabPage(pParent, rInAttrs)
{
    pSnapFrames->Show();
}

// -----------------------------------------------------------------------

SdTpOptionsSnap::~SdTpOptionsSnap()
{
}

// -----------------------------------------------------------------------

sal_Bool SdTpOptionsSnap::FillItemSet( SfxItemSet& rAttrs )
{
    SvxGridTabPage::FillItemSet(rAttrs);
    SdOptionsSnapItem aOptsItem( ATTR_OPTIONS_SNAP );

    aOptsItem.GetOptionsSnap().SetSnapHelplines( pCbxSnapHelplines->IsChecked() );
    aOptsItem.GetOptionsSnap().SetSnapBorder( pCbxSnapBorder->IsChecked() );
    aOptsItem.GetOptionsSnap().SetSnapFrame( pCbxSnapFrame->IsChecked() );
    aOptsItem.GetOptionsSnap().SetSnapPoints( pCbxSnapPoints->IsChecked() );
    aOptsItem.GetOptionsSnap().SetOrtho( pCbxOrtho->IsChecked() );
    aOptsItem.GetOptionsSnap().SetBigOrtho( pCbxBigOrtho->IsChecked() );
    aOptsItem.GetOptionsSnap().SetRotate( pCbxRotate->IsChecked() );
    aOptsItem.GetOptionsSnap().SetSnapArea( (sal_Int16) pMtrFldSnapArea->GetValue() );
    aOptsItem.GetOptionsSnap().SetAngle( (sal_Int16) pMtrFldAngle->GetValue() );
    aOptsItem.GetOptionsSnap().SetEliminatePolyPointLimitAngle( (sal_Int16) pMtrFldBezAngle->GetValue() );

    rAttrs.Put( aOptsItem );

    // we get a possible existing GridItem, this ensures that we do net set
    // some default values by accident
    return( sal_True );
}

// -----------------------------------------------------------------------

void SdTpOptionsSnap::Reset( const SfxItemSet& rAttrs )
{
    SvxGridTabPage::Reset(rAttrs);

    SdOptionsSnapItem aOptsItem( (const SdOptionsSnapItem&) rAttrs.
                        Get( ATTR_OPTIONS_SNAP ) );

    pCbxSnapHelplines->Check( aOptsItem.GetOptionsSnap().IsSnapHelplines() );
    pCbxSnapBorder->Check( aOptsItem.GetOptionsSnap().IsSnapBorder() );
    pCbxSnapFrame->Check( aOptsItem.GetOptionsSnap().IsSnapFrame() );
    pCbxSnapPoints->Check( aOptsItem.GetOptionsSnap().IsSnapPoints() );
    pCbxOrtho->Check( aOptsItem.GetOptionsSnap().IsOrtho() );
    pCbxBigOrtho->Check( aOptsItem.GetOptionsSnap().IsBigOrtho() );
    pCbxRotate->Check( aOptsItem.GetOptionsSnap().IsRotate() );
    pMtrFldSnapArea->SetValue( aOptsItem.GetOptionsSnap().GetSnapArea() );
    pMtrFldAngle->SetValue( aOptsItem.GetOptionsSnap().GetAngle() );
    pMtrFldBezAngle->SetValue( aOptsItem.GetOptionsSnap().GetEliminatePolyPointLimitAngle() );

    pCbxRotate->GetClickHdl().Call(0);
}

// -----------------------------------------------------------------------

SfxTabPage* SdTpOptionsSnap::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsSnap( pWindow, rAttrs ) );
}

/*************************************************************************
|*
|*  TabPage to adjust the content options
|*
\************************************************************************/

SdTpOptionsContents::SdTpOptionsContents( Window* pParent, const SfxItemSet& rInAttrs  ) :
    SfxTabPage ( pParent, "SdViewPage", "modules/simpress/ui/sdviewpage.ui", rInAttrs )
{
    get( m_pCbxRuler, "ruler");
    get( m_pCbxDragStripes, "dragstripes");
    get( m_pCbxHandlesBezier, "handlesbezier");
    get( m_pCbxMoveOutline, "moveoutline");
}

// -----------------------------------------------------------------------

SdTpOptionsContents::~SdTpOptionsContents()
{
}

// -----------------------------------------------------------------------

sal_Bool SdTpOptionsContents::FillItemSet( SfxItemSet& rAttrs )
{
    sal_Bool bModified = sal_False;

    if( m_pCbxRuler->GetSavedValue()           != m_pCbxRuler->IsChecked() ||
        m_pCbxMoveOutline->GetSavedValue()     != m_pCbxMoveOutline->IsChecked() ||
        m_pCbxDragStripes->GetSavedValue()     != m_pCbxDragStripes->IsChecked() ||
        m_pCbxHandlesBezier->GetSavedValue()   != m_pCbxHandlesBezier->IsChecked() )
    {
        SdOptionsLayoutItem aOptsItem( ATTR_OPTIONS_LAYOUT );

        aOptsItem.GetOptionsLayout().SetRulerVisible( m_pCbxRuler->IsChecked() );
        aOptsItem.GetOptionsLayout().SetMoveOutline( m_pCbxMoveOutline->IsChecked() );
        aOptsItem.GetOptionsLayout().SetDragStripes( m_pCbxDragStripes->IsChecked() );
        aOptsItem.GetOptionsLayout().SetHandlesBezier( m_pCbxHandlesBezier->IsChecked() );

        rAttrs.Put( aOptsItem );
        bModified = sal_True;
    }
    return( bModified );
}

// -----------------------------------------------------------------------

void SdTpOptionsContents::Reset( const SfxItemSet& rAttrs )
{
    SdOptionsContentsItem aOptsItem( (const SdOptionsContentsItem&) rAttrs.
                        Get( ATTR_OPTIONS_CONTENTS ) );

    SdOptionsLayoutItem aLayoutItem( (const SdOptionsLayoutItem&) rAttrs.
                        Get( ATTR_OPTIONS_LAYOUT ) );

    m_pCbxRuler->Check( aLayoutItem.GetOptionsLayout().IsRulerVisible() );
    m_pCbxMoveOutline->Check( aLayoutItem.GetOptionsLayout().IsMoveOutline() );
    m_pCbxDragStripes->Check( aLayoutItem.GetOptionsLayout().IsDragStripes() );
    m_pCbxHandlesBezier->Check( aLayoutItem.GetOptionsLayout().IsHandlesBezier() );

    m_pCbxRuler->SaveValue();
    m_pCbxMoveOutline->SaveValue();
    m_pCbxDragStripes->SaveValue();
    m_pCbxHandlesBezier->SaveValue();
}

// -----------------------------------------------------------------------

SfxTabPage* SdTpOptionsContents::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsContents( pWindow, rAttrs ) );
}

/*************************************************************************
|*
|*  TabPage to adjust the misc options
|*
\************************************************************************/
#define TABLE_COUNT 12
#define TOKEN (sal_Unicode(':'))

SdTpOptionsMisc::SdTpOptionsMisc( Window* pParent, const SfxItemSet& rInAttrs  ) :
  SfxTabPage          ( pParent, "OptSavePage","modules/simpress/ui/optimpressgeneralpage.ui", rInAttrs )
{
    get(m_pCbxQuickEdit , "qickedit");
    get(m_pCbxPickThrough , "textselected");
    get(m_pNewDocumentFrame, "newdocumentframe");
    get(m_pCbxStartWithTemplate,"startwithwizard");
    get(m_pCbxMasterPageCache , "backgroundback");
    get(m_pCbxCopy , "copywhenmove");
    get(m_pCbxMarkedHitMovesAlways , "objalwymov");
    get(m_pLbMetric , "units");
    get(m_pCbxStartWithActualPage , "strtwithPag");
    get(m_pCbxEnableSdremote , "enremotcont");
    get(m_pCbxEnablePresenterScreen , "enprsntcons");
    get(m_pCbxUsePrinterMetrics , "printermetrics");
    get(m_pPresentationFrame , "presentationframe");
    get(m_pScaleFrame , "scaleframe");
    get(m_pCbScale , "scaleBox");
    get(m_pMtrFldTabstop , "metricFields");
    get(m_pMtrFldOriginalWidth , "metricWidthFields");
    get(m_pMtrFldOriginalHeight , "metricHeightFields");
    get(m_pMtrFldInfo1 , "metricInfo1Fields");
    get(m_pMtrFldInfo2 , "metricInfo2Fields");
    get(m_pCbxCompatibility ,"cbCompatibility" );
    get(m_pFiInfo1 , "info1");
    get(m_pFiInfo2 , "info2");
    get(m_pNewDocLb , "newdoclbl");
    get(m_pWidthLb , "widthlbl");
    get(m_pHeightLb , "heightlbl");
    get(m_pCbxDistrot , "distrotcb");

    SetExchangeSupport();

    // set metric
    FieldUnit eFUnit;

    sal_uInt16 nWhich = GetWhich( SID_ATTR_METRIC );
    if ( rInAttrs.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rInAttrs.Get( nWhich );
        eFUnit = (FieldUnit)rItem.GetValue();
    }
    else
        eFUnit = SfxModule::GetCurrentFieldUnit();

    SetFieldUnit( *m_pMtrFldTabstop , eFUnit );

    // Impress is default mode, let' hide the entire scale frame etc.
    m_pCbxDistrot->Hide();
    m_pScaleFrame->Hide();

    // fill ListBox with metrics
    SvxStringArray aMetricArr( RID_SVXSTR_FIELDUNIT_TABLE );
    sal_uInt16 i;

    for ( i = 0; i < aMetricArr.Count(); ++i )
    {
        String sMetric = aMetricArr.GetStringByPos( i );
        sal_IntPtr nFieldUnit = aMetricArr.GetValue( i );
        sal_uInt16 nPos = m_pLbMetric->InsertEntry( sMetric );
        m_pLbMetric->SetEntryData( nPos, (void*)nFieldUnit );
    }
    m_pLbMetric->SetSelectHdl( LINK( this, SdTpOptionsMisc, SelectMetricHdl_Impl ) );

    SetFieldUnit( *m_pMtrFldOriginalWidth, eFUnit );
    SetFieldUnit( *m_pMtrFldOriginalHeight, eFUnit );
    m_pMtrFldOriginalWidth->SetLast( 999999999 );
    m_pMtrFldOriginalWidth->SetMax( 999999999 );
    m_pMtrFldOriginalHeight->SetLast( 999999999 );
    m_pMtrFldOriginalHeight->SetMax( 999999999 );

    // temporary fields for info texts (for formatting/calculation)
    m_pMtrFldInfo1->SetUnit( eFUnit );
    m_pMtrFldInfo1->SetMax( 999999999 );
    m_pMtrFldInfo1->SetDecimalDigits( 2 );
    m_pMtrFldInfo2->SetUnit( eFUnit );
    m_pMtrFldInfo2->SetMax( 999999999 );
    m_pMtrFldInfo2->SetDecimalDigits( 2 );

    // determine PoolUnit
    SfxItemPool* pPool = rInAttrs.GetPool();
    DBG_ASSERT( pPool, "Where is the Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // Fuellen der CB
    sal_uInt16 aTable[ TABLE_COUNT ] =
        { 1, 2, 4, 5, 8, 10, 16, 20, 30, 40, 50, 100 };

    for( i = 0; i < TABLE_COUNT; i++ )
        m_pCbScale->InsertEntry( GetScale( 1, aTable[i] ) );
    for( i = 1; i < TABLE_COUNT; i++ )
        m_pCbScale->InsertEntry( GetScale(  aTable[i], 1 ) );
}

// -----------------------------------------------------------------------

SdTpOptionsMisc::~SdTpOptionsMisc()
{
}
// -----------------------------------------------------------------------
void SdTpOptionsMisc::ActivatePage( const SfxItemSet& rSet )
{
    // We have to call SaveValue again since it can happen that the value
    // has no effect on other TabPages
    m_pLbMetric->SaveValue();
    // change metric if necessary (since TabPage is in the Dialog where
    // the metric is set)
    const SfxPoolItem* pAttr = NULL;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_METRIC , sal_False,
                                    (const SfxPoolItem**)&pAttr ))
    {
        const SfxUInt16Item* pItem = (SfxUInt16Item*) pAttr;

        FieldUnit eFUnit = (FieldUnit)(long)pItem->GetValue();

        if( eFUnit != m_pMtrFldOriginalWidth->GetUnit() )
        {
            // set metrics
            sal_Int64 nVal = m_pMtrFldOriginalWidth->Denormalize( m_pMtrFldOriginalWidth->GetValue( FUNIT_TWIP ) );
            SetFieldUnit( *m_pMtrFldOriginalWidth, eFUnit, sal_True );
            m_pMtrFldOriginalWidth->SetValue( m_pMtrFldOriginalWidth->Normalize( nVal ), FUNIT_TWIP );

            nVal = m_pMtrFldOriginalHeight->Denormalize( m_pMtrFldOriginalHeight->GetValue( FUNIT_TWIP ) );
            SetFieldUnit( *m_pMtrFldOriginalHeight, eFUnit, sal_True );
            m_pMtrFldOriginalHeight->SetValue( m_pMtrFldOriginalHeight->Normalize( nVal ), FUNIT_TWIP );


            if( nWidth != 0 && nHeight != 0 )
            {
                m_pMtrFldInfo1->SetUnit( eFUnit );
                m_pMtrFldInfo2->SetUnit( eFUnit );

                SetMetricValue( *m_pMtrFldInfo1, nWidth, ePoolUnit );
                aInfo1 = m_pMtrFldInfo1->GetText();
                m_pFiInfo1->SetText( aInfo1 );

                SetMetricValue( *m_pMtrFldInfo2, nHeight, ePoolUnit );
                aInfo2 = m_pMtrFldInfo2->GetText();
                m_pFiInfo2->SetText( aInfo2 );
            }
        }
    }
}

// -----------------------------------------------------------------------

int SdTpOptionsMisc::DeactivatePage( SfxItemSet* pActiveSet )
{
    // check parser
    sal_Int32 nX, nY;
    if( SetScale( m_pCbScale->GetText(), nX, nY ) )
    {
        if( pActiveSet )
            FillItemSet( *pActiveSet );
        return( LEAVE_PAGE );
    }
    WarningBox aWarnBox( GetParent(), WB_YES_NO, SD_RESSTR( STR_WARN_SCALE_FAIL ) );
    short nReturn = aWarnBox.Execute();

    if( nReturn == RET_YES )
        return( KEEP_PAGE );

    if( pActiveSet )
        FillItemSet( *pActiveSet );

    return( LEAVE_PAGE );
}

// -----------------------------------------------------------------------

sal_Bool SdTpOptionsMisc::FillItemSet( SfxItemSet& rAttrs )
{
    sal_Bool bModified = sal_False;

    if( m_pCbxStartWithTemplate->GetSavedValue()   != m_pCbxStartWithTemplate->IsChecked() ||
        m_pCbxMarkedHitMovesAlways->GetSavedValue()!= m_pCbxMarkedHitMovesAlways->IsChecked() ||
        m_pCbxQuickEdit->GetSavedValue()           != m_pCbxQuickEdit->IsChecked() ||
        m_pCbxPickThrough->GetSavedValue()         != m_pCbxPickThrough->IsChecked() ||
        m_pCbxMasterPageCache->GetSavedValue()     != m_pCbxMasterPageCache->IsChecked() ||
        m_pCbxCopy->GetSavedValue()                != m_pCbxCopy->IsChecked() ||
        m_pCbxStartWithActualPage->GetSavedValue() != m_pCbxStartWithActualPage->IsChecked() ||
        m_pCbxEnableSdremote->GetSavedValue()      != m_pCbxEnableSdremote->IsChecked() ||
        m_pCbxEnablePresenterScreen->GetSavedValue()!= m_pCbxEnablePresenterScreen->IsChecked() ||
        m_pCbxCompatibility->GetSavedValue()       != m_pCbxCompatibility->IsChecked() ||
        m_pCbxUsePrinterMetrics->GetSavedValue()   != m_pCbxUsePrinterMetrics->IsChecked() )
    {
        SdOptionsMiscItem aOptsItem( ATTR_OPTIONS_MISC );

        aOptsItem.GetOptionsMisc().SetStartWithTemplate( m_pCbxStartWithTemplate->IsChecked() );
        aOptsItem.GetOptionsMisc().SetMarkedHitMovesAlways( m_pCbxMarkedHitMovesAlways->IsChecked() );
        aOptsItem.GetOptionsMisc().SetQuickEdit( m_pCbxQuickEdit->IsChecked() );
        aOptsItem.GetOptionsMisc().SetPickThrough( m_pCbxPickThrough->IsChecked() );
        aOptsItem.GetOptionsMisc().SetMasterPagePaintCaching( m_pCbxMasterPageCache->IsChecked() );
        aOptsItem.GetOptionsMisc().SetDragWithCopy( m_pCbxCopy->IsChecked() );
        aOptsItem.GetOptionsMisc().SetStartWithActualPage( m_pCbxStartWithActualPage->IsChecked() );
        aOptsItem.GetOptionsMisc().SetEnableSdremote( m_pCbxEnableSdremote->IsChecked() );
        aOptsItem.GetOptionsMisc().SetEnablePresenterScreen( m_pCbxEnablePresenterScreen->IsChecked() );
        aOptsItem.GetOptionsMisc().SetSummationOfParagraphs( m_pCbxCompatibility->IsChecked() );
        aOptsItem.GetOptionsMisc().SetPrinterIndependentLayout (
            m_pCbxUsePrinterMetrics->IsChecked()
            ? ::com::sun::star::document::PrinterIndependentLayout::DISABLED
            : ::com::sun::star::document::PrinterIndependentLayout::ENABLED);
        rAttrs.Put( aOptsItem );

        bModified = sal_True;
    }

    // metric
    const sal_uInt16 nMPos = m_pLbMetric->GetSelectEntryPos();
    if ( nMPos != m_pLbMetric->GetSavedValue() )
    {
        sal_uInt16 nFieldUnit = (sal_uInt16)(sal_IntPtr)m_pLbMetric->GetEntryData( nMPos );
        rAttrs.Put( SfxUInt16Item( GetWhich( SID_ATTR_METRIC ),
                                     (sal_uInt16)nFieldUnit ) );
        bModified |= sal_True;
    }

    // tabulator space
    if( m_pMtrFldTabstop->GetText() != m_pMtrFldTabstop->GetSavedValue() )
    {
        sal_uInt16 nWh = GetWhich( SID_ATTR_DEFTABSTOP );
        SfxMapUnit eUnit = rAttrs.GetPool()->GetMetric( nWh );
        SfxUInt16Item aDef( nWh,(sal_uInt16)GetCoreValue( *m_pMtrFldTabstop, eUnit ) );
        rAttrs.Put( aDef );
        bModified |= sal_True;
    }

    sal_Int32 nX, nY;
    if( SetScale( m_pCbScale->GetText(), nX, nY ) )
    {
        rAttrs.Put( SfxInt32Item( ATTR_OPTIONS_SCALE_X, nX ) );
        rAttrs.Put( SfxInt32Item( ATTR_OPTIONS_SCALE_Y, nY ) );

        bModified = sal_True;
    }

    return( bModified );
}

// -----------------------------------------------------------------------

void SdTpOptionsMisc::Reset( const SfxItemSet& rAttrs )
{
    SdOptionsMiscItem aOptsItem( (const SdOptionsMiscItem&) rAttrs.
                        Get( ATTR_OPTIONS_MISC ) );

    m_pCbxStartWithTemplate->Check( aOptsItem.GetOptionsMisc().IsStartWithTemplate() );
    m_pCbxMarkedHitMovesAlways->Check( aOptsItem.GetOptionsMisc().IsMarkedHitMovesAlways() );
    m_pCbxQuickEdit->Check( aOptsItem.GetOptionsMisc().IsQuickEdit() );
    m_pCbxPickThrough->Check( aOptsItem.GetOptionsMisc().IsPickThrough() );
    m_pCbxMasterPageCache->Check( aOptsItem.GetOptionsMisc().IsMasterPagePaintCaching() );
    m_pCbxCopy->Check( aOptsItem.GetOptionsMisc().IsDragWithCopy() );
    m_pCbxStartWithActualPage->Check( aOptsItem.GetOptionsMisc().IsStartWithActualPage() );
    m_pCbxEnableSdremote->Check( aOptsItem.GetOptionsMisc().IsEnableSdremote() );
    m_pCbxEnablePresenterScreen->Check( aOptsItem.GetOptionsMisc().IsEnablePresenterScreen() );
    m_pCbxCompatibility->Check( aOptsItem.GetOptionsMisc().IsSummationOfParagraphs() );
    m_pCbxUsePrinterMetrics->Check( aOptsItem.GetOptionsMisc().GetPrinterIndependentLayout()==1 );
    m_pCbxStartWithTemplate->SaveValue();
    m_pCbxMarkedHitMovesAlways->SaveValue();
    m_pCbxQuickEdit->SaveValue();
    m_pCbxPickThrough->SaveValue();

    m_pCbxMasterPageCache->SaveValue();
    m_pCbxCopy->SaveValue();
    m_pCbxEnableSdremote->SaveValue();
    m_pCbxEnablePresenterScreen->SaveValue();
    m_pCbxCompatibility->SaveValue();
    m_pCbxUsePrinterMetrics->SaveValue();

    // metric
    sal_uInt16 nWhich = GetWhich( SID_ATTR_METRIC );
    m_pLbMetric->SetNoSelection();

    if ( rAttrs.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rAttrs.Get( nWhich );
        long nFieldUnit = (long)rItem.GetValue();

        for ( sal_uInt16 i = 0; i < m_pLbMetric->GetEntryCount(); ++i )
        {
            if ( (sal_IntPtr)m_pLbMetric->GetEntryData( i ) == nFieldUnit )
            {
                m_pLbMetric->SelectEntryPos( i );
                break;
            }
        }
    }

    // tabulator space
    nWhich = GetWhich( SID_ATTR_DEFTABSTOP );
    if( rAttrs.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        SfxMapUnit eUnit = rAttrs.GetPool()->GetMetric( nWhich );
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rAttrs.Get( nWhich );
        SetMetricValue( *m_pMtrFldTabstop, rItem.GetValue(), eUnit );
    }
    m_pLbMetric->SaveValue();
    m_pMtrFldTabstop->SaveValue();
    //Scale
    sal_Int32 nX = ( (const SfxInt32Item&) rAttrs.
                 Get( ATTR_OPTIONS_SCALE_X ) ).GetValue();
    sal_Int32 nY = ( (const SfxInt32Item&) rAttrs.
                 Get( ATTR_OPTIONS_SCALE_Y ) ).GetValue();
    nWidth = ( (const SfxUInt32Item&) rAttrs.
                    Get( ATTR_OPTIONS_SCALE_WIDTH ) ).GetValue();
    nHeight = ( (const SfxUInt32Item&) rAttrs.
                    Get( ATTR_OPTIONS_SCALE_HEIGHT ) ).GetValue();

    m_pCbScale->SetText( GetScale( nX, nY ) );

    m_pMtrFldOriginalWidth->Hide();
    m_pMtrFldOriginalWidth->SetText( aInfo1 ); // empty
    m_pMtrFldOriginalHeight->Hide();
    m_pMtrFldOriginalHeight->SetText( aInfo2 ); //empty
    m_pFiInfo1->Hide();
    m_pFiInfo2->Hide();

    UpdateCompatibilityControls ();
}

// -----------------------------------------------------------------------

SfxTabPage* SdTpOptionsMisc::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SdTpOptionsMisc( pWindow, rAttrs ) );
}
//------------------------------------------------------------------------

IMPL_LINK_NOARG(SdTpOptionsMisc, SelectMetricHdl_Impl)
{
    sal_uInt16 nPos = m_pLbMetric->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        FieldUnit eUnit = (FieldUnit)(sal_IntPtr)m_pLbMetric->GetEntryData( nPos );
        sal_Int64 nVal =
            m_pMtrFldTabstop->Denormalize( m_pMtrFldTabstop->GetValue( FUNIT_TWIP ) );
        SetFieldUnit( *m_pMtrFldTabstop, eUnit );
        m_pMtrFldTabstop->SetValue( m_pMtrFldTabstop->Normalize( nVal ), FUNIT_TWIP );
    }
    return 0;
}


void SdTpOptionsMisc::SetImpressMode (void)
{
#ifndef ENABLE_SDREMOTE_BLUETOOTH
    m_pCbxEnableSdremote->Hide();
#endif
}

void    SdTpOptionsMisc::SetDrawMode()
{
    m_pScaleFrame->Show();
    m_pNewDocumentFrame->Hide();
    m_pCbxStartWithActualPage->Hide();
    m_pCbxEnableSdremote->Hide();
    m_pCbxEnablePresenterScreen->Hide();
    m_pCbxCompatibility->Hide();
    m_pNewDocLb->Hide();
    m_pCbScale->Show();
    m_pPresentationFrame->Hide();
    m_pMtrFldInfo1->Hide();
    m_pMtrFldInfo2->Hide();
    m_pWidthLb->Hide();
    m_pHeightLb->Hide();
    m_pFiInfo1->Show();
    m_pMtrFldOriginalWidth->Show();
    m_pFiInfo2->Show();
    m_pMtrFldOriginalHeight->Show();
    m_pCbxDistrot->Show();
    m_pCbxCompatibility->Hide();
    // Move the printer-independent-metrics check box in the place that the
    // spacing-between-paragraphs check box normally is in.
    m_pCbxUsePrinterMetrics->SetPosPixel (m_pCbxCompatibility->GetPosPixel());
}

// -----------------------------------------------------------------------

String SdTpOptionsMisc::GetScale( sal_Int32 nX, sal_Int32 nY )
{
    String aScale( OUString::number( nX ) );
    aScale.Append( TOKEN );
    aScale.Append( OUString::number( nY ) );

    return( aScale );
}

// -----------------------------------------------------------------------

sal_Bool SdTpOptionsMisc::SetScale( const String& aScale, sal_Int32& rX, sal_Int32& rY )
{
    if( comphelper::string::getTokenCount(aScale, TOKEN) != 2 )
        return( sal_False );

    OUString aTmp(aScale.GetToken( 0, TOKEN ));
    if (!comphelper::string::isdigitAsciiString(aTmp))
        return sal_False;

    rX = (long) aTmp.toInt32();
    if( rX == 0 )
        return( sal_False );

    aTmp = aScale.GetToken( 1, TOKEN );
    if (!comphelper::string::isdigitAsciiString(aTmp))
        return sal_False;

    rY = (long) aTmp.toInt32();
    if( rY == 0 )
        return sal_False;

    return sal_True;
}

void SdTpOptionsMisc::UpdateCompatibilityControls (void)
{
    // Disable the compatibility controls by default.  Enable them only when
    // there is at least one open document.
    sal_Bool bIsEnabled = sal_False;

    try
    {
        // Get a component enumeration from the desktop and search it for documents.
        Reference<uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext());
        do
        {
            Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(xContext);

            Reference<container::XEnumerationAccess> xComponents (
                xDesktop->getComponents(), UNO_QUERY);
            if ( ! xComponents.is())
                break;

            Reference<container::XEnumeration> xEnumeration (
                xComponents->createEnumeration());
            if ( ! xEnumeration.is())
                break;

            while (xEnumeration->hasMoreElements())
            {
                Reference<frame::XModel> xModel (xEnumeration->nextElement(), UNO_QUERY);
                if (xModel.is())
                {
                    // There is at leas one model/document: Enable the compatibility controls.
                    bIsEnabled = sal_True;
                    break;
                }
            }

        }
        while (false); // One 'loop'.
    }
    catch (const uno::Exception&)
    {
        // When there is an exception then simply use the default value of
        // bIsEnabled and disable the controls.
    }

    m_pCbxCompatibility->Enable(bIsEnabled);
    m_pCbxUsePrinterMetrics->Enable (bIsEnabled);
}

void SdTpOptionsMisc::PageCreated (SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pFlagItem,SfxUInt32Item,SID_SDMODE_FLAG,sal_False);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SD_DRAW_MODE ) == SD_DRAW_MODE )
            SetDrawMode();
        if ( ( nFlags & SD_IMPRESS_MODE ) == SD_IMPRESS_MODE )
            SetImpressMode();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
