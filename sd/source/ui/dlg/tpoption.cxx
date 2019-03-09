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
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <sfx2/module.hxx>
#include <svx/svxids.hrc>
#include <svx/strarray.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svtools/unitconv.hxx>

#include <sdattr.hrc>
#include <sdresid.hxx>
#include <optsitem.hxx>
#include <tpoption.hxx>
#include <strings.hrc>
#include <app.hrc>
#include <svl/intitem.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SdTpOptionsSnap::SdTpOptionsSnap(TabPageParent pParent, const SfxItemSet& rInAttrs)
    : SvxGridTabPage(pParent, rInAttrs)
{
    m_xSnapFrames->show();
}

SdTpOptionsSnap::~SdTpOptionsSnap()
{
}

bool SdTpOptionsSnap::FillItemSet( SfxItemSet* rAttrs )
{
    SvxGridTabPage::FillItemSet(rAttrs);
    SdOptionsSnapItem aOptsItem;

    aOptsItem.GetOptionsSnap().SetSnapHelplines( m_xCbxSnapHelplines->get_active() );
    aOptsItem.GetOptionsSnap().SetSnapBorder( m_xCbxSnapBorder->get_active() );
    aOptsItem.GetOptionsSnap().SetSnapFrame( m_xCbxSnapFrame->get_active() );
    aOptsItem.GetOptionsSnap().SetSnapPoints( m_xCbxSnapPoints->get_active() );
    aOptsItem.GetOptionsSnap().SetOrtho( m_xCbxOrtho->get_active() );
    aOptsItem.GetOptionsSnap().SetBigOrtho( m_xCbxBigOrtho->get_active() );
    aOptsItem.GetOptionsSnap().SetRotate( m_xCbxRotate->get_active() );
    aOptsItem.GetOptionsSnap().SetSnapArea(static_cast<sal_Int16>(m_xMtrFldSnapArea->get_value(FieldUnit::PIXEL)));
    aOptsItem.GetOptionsSnap().SetAngle(static_cast<sal_Int16>(m_xMtrFldAngle->get_value(FieldUnit::DEGREE)));
    aOptsItem.GetOptionsSnap().SetEliminatePolyPointLimitAngle(static_cast<sal_Int16>(m_xMtrFldBezAngle->get_value(FieldUnit::DEGREE)));

    rAttrs->Put( aOptsItem );

    // we get a possible existing GridItem, this ensures that we do not set
    // some default values by accident
    return true;
}

void SdTpOptionsSnap::Reset( const SfxItemSet* rAttrs )
{
    SvxGridTabPage::Reset(rAttrs);

    SdOptionsSnapItem aOptsItem( static_cast<const SdOptionsSnapItem&>( rAttrs->
                        Get( ATTR_OPTIONS_SNAP ) ) );

    m_xCbxSnapHelplines->set_active( aOptsItem.GetOptionsSnap().IsSnapHelplines() );
    m_xCbxSnapBorder->set_active( aOptsItem.GetOptionsSnap().IsSnapBorder() );
    m_xCbxSnapFrame->set_active( aOptsItem.GetOptionsSnap().IsSnapFrame() );
    m_xCbxSnapPoints->set_active( aOptsItem.GetOptionsSnap().IsSnapPoints() );
    m_xCbxOrtho->set_active( aOptsItem.GetOptionsSnap().IsOrtho() );
    m_xCbxBigOrtho->set_active( aOptsItem.GetOptionsSnap().IsBigOrtho() );
    m_xCbxRotate->set_active( aOptsItem.GetOptionsSnap().IsRotate() );
    m_xMtrFldSnapArea->set_value(aOptsItem.GetOptionsSnap().GetSnapArea(), FieldUnit::PIXEL);
    m_xMtrFldAngle->set_value(aOptsItem.GetOptionsSnap().GetAngle(), FieldUnit::DEGREE);
    m_xMtrFldBezAngle->set_value(aOptsItem.GetOptionsSnap().GetEliminatePolyPointLimitAngle(), FieldUnit::DEGREE);

    ClickRotateHdl_Impl(*m_xCbxRotate);
}

VclPtr<SfxTabPage> SdTpOptionsSnap::Create( TabPageParent pWindow,
                                            const SfxItemSet* rAttrs )
{
    return VclPtr<SdTpOptionsSnap>::Create(pWindow, *rAttrs);
}

/*************************************************************************
|*
|*  TabPage to adjust the content options
|*
\************************************************************************/

SdTpOptionsContents::SdTpOptionsContents( vcl::Window* pParent, const SfxItemSet& rInAttrs  ) :
    SfxTabPage ( pParent, "SdViewPage", "modules/simpress/ui/sdviewpage.ui", &rInAttrs )
{
    get( m_pCbxRuler, "ruler");
    get( m_pCbxDragStripes, "dragstripes");
    get( m_pCbxHandlesBezier, "handlesbezier");
    get( m_pCbxMoveOutline, "moveoutline");
}

SdTpOptionsContents::~SdTpOptionsContents()
{
    disposeOnce();
}

void SdTpOptionsContents::dispose()
{
    m_pCbxRuler.clear();
    m_pCbxDragStripes.clear();
    m_pCbxHandlesBezier.clear();
    m_pCbxMoveOutline.clear();
    SfxTabPage::dispose();
}

bool SdTpOptionsContents::FillItemSet( SfxItemSet* rAttrs )
{
    bool bModified = false;

    if( m_pCbxRuler->IsValueChangedFromSaved() ||
        m_pCbxMoveOutline->IsValueChangedFromSaved() ||
        m_pCbxDragStripes->IsValueChangedFromSaved() ||
        m_pCbxHandlesBezier->IsValueChangedFromSaved() )
    {
        SdOptionsLayoutItem aOptsItem;

        aOptsItem.GetOptionsLayout().SetRulerVisible( m_pCbxRuler->IsChecked() );
        aOptsItem.GetOptionsLayout().SetMoveOutline( m_pCbxMoveOutline->IsChecked() );
        aOptsItem.GetOptionsLayout().SetDragStripes( m_pCbxDragStripes->IsChecked() );
        aOptsItem.GetOptionsLayout().SetHandlesBezier( m_pCbxHandlesBezier->IsChecked() );

        rAttrs->Put( aOptsItem );
        bModified = true;
    }
    return bModified;
}

void SdTpOptionsContents::Reset( const SfxItemSet* rAttrs )
{
    SdOptionsLayoutItem aLayoutItem( static_cast<const SdOptionsLayoutItem&>( rAttrs->
                        Get( ATTR_OPTIONS_LAYOUT ) ) );

    m_pCbxRuler->Check( aLayoutItem.GetOptionsLayout().IsRulerVisible() );
    m_pCbxMoveOutline->Check( aLayoutItem.GetOptionsLayout().IsMoveOutline() );
    m_pCbxDragStripes->Check( aLayoutItem.GetOptionsLayout().IsDragStripes() );
    m_pCbxHandlesBezier->Check( aLayoutItem.GetOptionsLayout().IsHandlesBezier() );

    m_pCbxRuler->SaveValue();
    m_pCbxMoveOutline->SaveValue();
    m_pCbxDragStripes->SaveValue();
    m_pCbxHandlesBezier->SaveValue();
}

VclPtr<SfxTabPage> SdTpOptionsContents::Create( TabPageParent pWindow,
                                                const SfxItemSet* rAttrs )
{
    return VclPtr<SdTpOptionsContents>::Create( pWindow.pParent, *rAttrs );
}

/*************************************************************************
|*
|*  TabPage to adjust the misc options
|*
\************************************************************************/
#define TABLE_COUNT 12
#define TOKEN ':'

SdTpOptionsMisc::SdTpOptionsMisc(vcl::Window* pParent, const SfxItemSet& rInAttrs)
    : SfxTabPage(pParent, "OptSavePage", "modules/simpress/ui/optimpressgeneralpage.ui", &rInAttrs)
    , nWidth(0)
    , nHeight(0)
{
    get(m_pCbxQuickEdit , "qickedit");
    get(m_pCbxPickThrough , "textselected");
    get(m_pNewDocumentFrame, "newdocumentframe");
    get(m_pCbxStartWithTemplate,"startwithwizard");
    get(m_pCbxMasterPageCache , "backgroundback");
    get(m_pCbxCopy , "copywhenmove");
    get(m_pCbxMarkedHitMovesAlways , "objalwymov");
    get(m_pLbMetric , "units");
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
    if ( rInAttrs.GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SfxUInt16Item& rItem = static_cast<const SfxUInt16Item&>(rInAttrs.Get( nWhich ));
        eFUnit = static_cast<FieldUnit>(rItem.GetValue());
    }
    else
        eFUnit = SfxModule::GetCurrentFieldUnit();

    SetFieldUnit( *m_pMtrFldTabstop , eFUnit );

    // Impress is default mode, let' hide the entire scale frame etc.
    m_pCbxDistrot->Hide();
    m_pScaleFrame->Hide();

    // fill ListBox with metrics
    for (sal_uInt32 i = 0; i < SvxFieldUnitTable::Count(); ++i)
    {
        OUString sMetric = SvxFieldUnitTable::GetString(i);
        sal_IntPtr nFieldUnit = sal_uInt16(SvxFieldUnitTable::GetValue(i));
        sal_Int32 nPos = m_pLbMetric->InsertEntry( sMetric );
        m_pLbMetric->SetEntryData( nPos, reinterpret_cast<void*>(nFieldUnit) );
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

    // Fill the CB
    sal_uInt16 aTable[ TABLE_COUNT ] =
        { 1, 2, 4, 5, 8, 10, 16, 20, 30, 40, 50, 100 };

    for( sal_uInt16 i = 0; i < TABLE_COUNT; i++ )
        m_pCbScale->InsertEntry( GetScale( 1, aTable[i] ) );
    for( sal_uInt16 i = 1; i < TABLE_COUNT; i++ )
        m_pCbScale->InsertEntry( GetScale(  aTable[i], 1 ) );
}

SdTpOptionsMisc::~SdTpOptionsMisc()
{
    disposeOnce();
}

void SdTpOptionsMisc::dispose()
{
    m_pCbxQuickEdit.clear();
    m_pCbxPickThrough.clear();
    m_pNewDocumentFrame.clear();
    m_pCbxStartWithTemplate.clear();
    m_pCbxMasterPageCache.clear();
    m_pCbxCopy.clear();
    m_pCbxMarkedHitMovesAlways.clear();
    m_pPresentationFrame.clear();
    m_pLbMetric.clear();
    m_pMtrFldTabstop.clear();
    m_pCbxEnableSdremote.clear();
    m_pCbxEnablePresenterScreen.clear();
    m_pCbxUsePrinterMetrics.clear();
    m_pCbxCompatibility.clear();
    m_pScaleFrame.clear();
    m_pCbScale.clear();
    m_pNewDocLb.clear();
    m_pFiInfo1.clear();
    m_pMtrFldOriginalWidth.clear();
    m_pWidthLb.clear();
    m_pHeightLb.clear();
    m_pFiInfo2.clear();
    m_pMtrFldOriginalHeight.clear();
    m_pCbxDistrot.clear();
    m_pMtrFldInfo1.clear();
    m_pMtrFldInfo2.clear();
    SfxTabPage::dispose();
}

void SdTpOptionsMisc::ActivatePage( const SfxItemSet& rSet )
{
    // We have to call SaveValue again since it can happen that the value
    // has no effect on other TabPages
    m_pLbMetric->SaveValue();
    // change metric if necessary (since TabPage is in the Dialog where
    // the metric is set)
    const SfxPoolItem* pAttr = nullptr;
    if( SfxItemState::SET != rSet.GetItemState( SID_ATTR_METRIC , false,
                                    &pAttr ))
        return;

    const SfxUInt16Item* pItem = static_cast<const SfxUInt16Item*>(pAttr);

    FieldUnit eFUnit = static_cast<FieldUnit>(static_cast<long>(pItem->GetValue()));

    if( eFUnit == m_pMtrFldOriginalWidth->GetUnit() )
        return;

    // set metrics
    sal_Int64 nVal = m_pMtrFldOriginalWidth->Denormalize( m_pMtrFldOriginalWidth->GetValue( FieldUnit::TWIP ) );
    SetFieldUnit( *m_pMtrFldOriginalWidth, eFUnit, true );
    m_pMtrFldOriginalWidth->SetValue( m_pMtrFldOriginalWidth->Normalize( nVal ), FieldUnit::TWIP );

    nVal = m_pMtrFldOriginalHeight->Denormalize( m_pMtrFldOriginalHeight->GetValue( FieldUnit::TWIP ) );
    SetFieldUnit( *m_pMtrFldOriginalHeight, eFUnit, true );
    m_pMtrFldOriginalHeight->SetValue( m_pMtrFldOriginalHeight->Normalize( nVal ), FieldUnit::TWIP );

    if( nWidth == 0 || nHeight == 0 )
        return;

    m_pMtrFldInfo1->SetUnit( eFUnit );
    m_pMtrFldInfo2->SetUnit( eFUnit );

    SetMetricValue( *m_pMtrFldInfo1, nWidth, ePoolUnit );
    aInfo1 = m_pMtrFldInfo1->GetText();
    m_pFiInfo1->SetText( aInfo1 );

    SetMetricValue( *m_pMtrFldInfo2, nHeight, ePoolUnit );
    aInfo2 = m_pMtrFldInfo2->GetText();
    m_pFiInfo2->SetText( aInfo2 );
}

DeactivateRC SdTpOptionsMisc::DeactivatePage( SfxItemSet* pActiveSet )
{
    // check parser
    sal_Int32 nX, nY;
    if( SetScale( m_pCbScale->GetText(), nX, nY ) )
    {
        if( pActiveSet )
            FillItemSet( pActiveSet );
        return DeactivateRC::LeavePage;
    }

    vcl::Window* pWin = GetParent();
    std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                               VclMessageType::Warning, VclButtonsType::YesNo,
                                               SdResId(STR_WARN_SCALE_FAIL)));
    if (xWarn->run() == RET_YES)
        return DeactivateRC::KeepPage;

    if( pActiveSet )
        FillItemSet( pActiveSet );

    return DeactivateRC::LeavePage;
}

bool SdTpOptionsMisc::FillItemSet( SfxItemSet* rAttrs )
{
    bool bModified = false;

    if( m_pCbxStartWithTemplate->IsValueChangedFromSaved()     ||
        m_pCbxMarkedHitMovesAlways->IsValueChangedFromSaved()  ||
        m_pCbxQuickEdit->IsValueChangedFromSaved()             ||
        m_pCbxPickThrough->IsValueChangedFromSaved()           ||
        m_pCbxMasterPageCache->IsValueChangedFromSaved()       ||
        m_pCbxCopy->IsValueChangedFromSaved()                  ||
        m_pCbxEnableSdremote->IsValueChangedFromSaved()        ||
        m_pCbxEnablePresenterScreen->IsValueChangedFromSaved() ||
        m_pCbxCompatibility->IsValueChangedFromSaved()         ||
        m_pCbxUsePrinterMetrics->IsValueChangedFromSaved() )
    {
        SdOptionsMiscItem aOptsItem;

        aOptsItem.GetOptionsMisc().SetStartWithTemplate( m_pCbxStartWithTemplate->IsChecked() );
        aOptsItem.GetOptionsMisc().SetMarkedHitMovesAlways( m_pCbxMarkedHitMovesAlways->IsChecked() );
        aOptsItem.GetOptionsMisc().SetQuickEdit( m_pCbxQuickEdit->IsChecked() );
        aOptsItem.GetOptionsMisc().SetPickThrough( m_pCbxPickThrough->IsChecked() );
        aOptsItem.GetOptionsMisc().SetMasterPagePaintCaching( m_pCbxMasterPageCache->IsChecked() );
        aOptsItem.GetOptionsMisc().SetDragWithCopy( m_pCbxCopy->IsChecked() );
        aOptsItem.GetOptionsMisc().SetEnableSdremote( m_pCbxEnableSdremote->IsChecked() );
        aOptsItem.GetOptionsMisc().SetEnablePresenterScreen( m_pCbxEnablePresenterScreen->IsChecked() );
        aOptsItem.GetOptionsMisc().SetSummationOfParagraphs( m_pCbxCompatibility->IsChecked() );
        aOptsItem.GetOptionsMisc().SetPrinterIndependentLayout (
            m_pCbxUsePrinterMetrics->IsChecked()
            ? css::document::PrinterIndependentLayout::DISABLED
            : css::document::PrinterIndependentLayout::ENABLED);
        rAttrs->Put( aOptsItem );

        bModified = true;
    }

    // metric
    const sal_Int32 nMPos = m_pLbMetric->GetSelectedEntryPos();
    if ( m_pLbMetric->IsValueChangedFromSaved() )
    {
        sal_uInt16 nFieldUnit = static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(m_pLbMetric->GetEntryData( nMPos )));
        rAttrs->Put( SfxUInt16Item( GetWhich( SID_ATTR_METRIC ), nFieldUnit ) );
        bModified = true;
    }

    // tabulator space
    if( m_pMtrFldTabstop->IsValueChangedFromSaved() )
    {
        sal_uInt16 nWh = GetWhich( SID_ATTR_DEFTABSTOP );
        MapUnit eUnit = rAttrs->GetPool()->GetMetric( nWh );
        SfxUInt16Item aDef( nWh,static_cast<sal_uInt16>(GetCoreValue( *m_pMtrFldTabstop, eUnit )) );
        rAttrs->Put( aDef );
        bModified = true;
    }

    sal_Int32 nX, nY;
    if( SetScale( m_pCbScale->GetText(), nX, nY ) )
    {
        rAttrs->Put( SfxInt32Item( ATTR_OPTIONS_SCALE_X, nX ) );
        rAttrs->Put( SfxInt32Item( ATTR_OPTIONS_SCALE_Y, nY ) );

        bModified = true;
    }

    return bModified;
}

void SdTpOptionsMisc::Reset( const SfxItemSet* rAttrs )
{
    SdOptionsMiscItem aOptsItem( static_cast<const SdOptionsMiscItem&>( rAttrs->
                        Get( ATTR_OPTIONS_MISC ) ) );

    m_pCbxStartWithTemplate->Check( aOptsItem.GetOptionsMisc().IsStartWithTemplate() );
    m_pCbxMarkedHitMovesAlways->Check( aOptsItem.GetOptionsMisc().IsMarkedHitMovesAlways() );
    m_pCbxQuickEdit->Check( aOptsItem.GetOptionsMisc().IsQuickEdit() );
    m_pCbxPickThrough->Check( aOptsItem.GetOptionsMisc().IsPickThrough() );
    m_pCbxMasterPageCache->Check( aOptsItem.GetOptionsMisc().IsMasterPagePaintCaching() );
    m_pCbxCopy->Check( aOptsItem.GetOptionsMisc().IsDragWithCopy() );
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

    if ( rAttrs->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SfxUInt16Item& rItem = static_cast<const SfxUInt16Item&>(rAttrs->Get( nWhich ));
        long nFieldUnit = static_cast<long>(rItem.GetValue());

        for ( sal_Int32 i = 0; i < m_pLbMetric->GetEntryCount(); ++i )
        {
            if ( reinterpret_cast<sal_IntPtr>(m_pLbMetric->GetEntryData( i )) == nFieldUnit )
            {
                m_pLbMetric->SelectEntryPos( i );
                break;
            }
        }
    }

    // tabulator space
    nWhich = GetWhich( SID_ATTR_DEFTABSTOP );
    if( rAttrs->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        MapUnit eUnit = rAttrs->GetPool()->GetMetric( nWhich );
        const SfxUInt16Item& rItem = static_cast<const SfxUInt16Item&>(rAttrs->Get( nWhich ));
        SetMetricValue( *m_pMtrFldTabstop, rItem.GetValue(), eUnit );
    }
    m_pLbMetric->SaveValue();
    m_pMtrFldTabstop->SaveValue();
    //Scale
    sal_Int32 nX = static_cast<const SfxInt32Item&>( rAttrs->
                 Get( ATTR_OPTIONS_SCALE_X ) ).GetValue();
    sal_Int32 nY = static_cast<const SfxInt32Item&>( rAttrs->
                 Get( ATTR_OPTIONS_SCALE_Y ) ).GetValue();
    nWidth = static_cast<const SfxUInt32Item&>( rAttrs->
                    Get( ATTR_OPTIONS_SCALE_WIDTH ) ).GetValue();
    nHeight = static_cast<const SfxUInt32Item&>( rAttrs->
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

VclPtr<SfxTabPage> SdTpOptionsMisc::Create( TabPageParent pWindow,
                                            const SfxItemSet* rAttrs )
{
    return VclPtr<SdTpOptionsMisc>::Create( pWindow.pParent, *rAttrs );
}

IMPL_LINK_NOARG(SdTpOptionsMisc, SelectMetricHdl_Impl, ListBox&, void)
{
    sal_Int32 nPos = m_pLbMetric->GetSelectedEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        FieldUnit eUnit = static_cast<FieldUnit>(reinterpret_cast<sal_IntPtr>(m_pLbMetric->GetEntryData( nPos )));
        sal_Int64 nVal =
            m_pMtrFldTabstop->Denormalize( m_pMtrFldTabstop->GetValue( FieldUnit::TWIP ) );
        SetFieldUnit( *m_pMtrFldTabstop, eUnit );
        m_pMtrFldTabstop->SetValue( m_pMtrFldTabstop->Normalize( nVal ), FieldUnit::TWIP );
    }
}

void SdTpOptionsMisc::SetImpressMode()
{
#ifndef ENABLE_SDREMOTE_BLUETOOTH
    m_pCbxEnableSdremote->Hide();
#else
    (void) this; // loplugin:staticmethods
#endif
}

void    SdTpOptionsMisc::SetDrawMode()
{
    m_pScaleFrame->Show();
    m_pNewDocumentFrame->Hide();
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

OUString SdTpOptionsMisc::GetScale( sal_Int32 nX, sal_Int32 nY )
{
    return OUString::number(nX) + OUStringLiteral1(TOKEN) + OUString::number(nY);
}

bool SdTpOptionsMisc::SetScale( const OUString& aScale, sal_Int32& rX, sal_Int32& rY )
{
    if (aScale.isEmpty())
        return false;

    sal_Int32 nIdx {0};

    OUString aTmp(aScale.getToken(0, TOKEN, nIdx));
    if (nIdx<0)
        return false; // we expect another token!

    if (!comphelper::string::isdigitAsciiString(aTmp))
        return false;

    rX = static_cast<long>(aTmp.toInt32());
    if( rX == 0 )
        return false;

    aTmp = aScale.getToken(0, TOKEN, nIdx);
    if (nIdx>=0)
        return false; // we require just 2 tokens!

    if (!comphelper::string::isdigitAsciiString(aTmp))
        return false;

    rY = static_cast<long>(aTmp.toInt32());
    return rY != 0;
}

void SdTpOptionsMisc::UpdateCompatibilityControls()
{
    // Disable the compatibility controls by default.  Enable them only when
    // there is at least one open document.
    bool bIsEnabled = false;

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
                    // There is at least one model/document: Enable the compatibility controls.
                    bIsEnabled = true;
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

void SdTpOptionsMisc::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_SDMODE_FLAG, false);
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
