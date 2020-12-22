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

SdTpOptionsSnap::SdTpOptionsSnap(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SvxGridTabPage(pPage, pController, rInAttrs)
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
    aOptsItem.GetOptionsSnap().SetAngle(Degree10(m_xMtrFldAngle->get_value(FieldUnit::DEGREE) * 10));
    aOptsItem.GetOptionsSnap().SetEliminatePolyPointLimitAngle(Degree10(m_xMtrFldBezAngle->get_value(FieldUnit::DEGREE) * 10));

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
    m_xMtrFldAngle->set_value(aOptsItem.GetOptionsSnap().GetAngle().get(), FieldUnit::DEGREE);
    m_xMtrFldBezAngle->set_value(aOptsItem.GetOptionsSnap().GetEliminatePolyPointLimitAngle().get(), FieldUnit::DEGREE);

    ClickRotateHdl_Impl(*m_xCbxRotate);
}

std::unique_ptr<SfxTabPage> SdTpOptionsSnap::Create( weld::Container* pPage, weld::DialogController* pController,
                                            const SfxItemSet* rAttrs )
{
    return std::make_unique<SdTpOptionsSnap>(pPage, pController, *rAttrs);
}

/*************************************************************************
|*
|*  TabPage to adjust the content options
|*
\************************************************************************/
SdTpOptionsContents::SdTpOptionsContents(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "modules/simpress/ui/sdviewpage.ui", "SdViewPage", &rInAttrs)
    , m_xCbxRuler(m_xBuilder->weld_check_button("ruler"))
    , m_xCbxDragStripes(m_xBuilder->weld_check_button("dragstripes"))
    , m_xCbxHandlesBezier(m_xBuilder->weld_check_button("handlesbezier"))
    , m_xCbxMoveOutline(m_xBuilder->weld_check_button("moveoutline"))
{
}

SdTpOptionsContents::~SdTpOptionsContents()
{
}

bool SdTpOptionsContents::FillItemSet( SfxItemSet* rAttrs )
{
    bool bModified = false;

    if( m_xCbxRuler->get_state_changed_from_saved() ||
        m_xCbxMoveOutline->get_state_changed_from_saved() ||
        m_xCbxDragStripes->get_state_changed_from_saved() ||
        m_xCbxHandlesBezier->get_state_changed_from_saved() )
    {
        SdOptionsLayoutItem aOptsItem;

        aOptsItem.GetOptionsLayout().SetRulerVisible( m_xCbxRuler->get_active() );
        aOptsItem.GetOptionsLayout().SetMoveOutline( m_xCbxMoveOutline->get_active() );
        aOptsItem.GetOptionsLayout().SetDragStripes( m_xCbxDragStripes->get_active() );
        aOptsItem.GetOptionsLayout().SetHandlesBezier( m_xCbxHandlesBezier->get_active() );

        rAttrs->Put( aOptsItem );
        bModified = true;
    }
    return bModified;
}

void SdTpOptionsContents::Reset( const SfxItemSet* rAttrs )
{
    SdOptionsLayoutItem aLayoutItem( static_cast<const SdOptionsLayoutItem&>( rAttrs->
                        Get( ATTR_OPTIONS_LAYOUT ) ) );

    m_xCbxRuler->set_active( aLayoutItem.GetOptionsLayout().IsRulerVisible() );
    m_xCbxMoveOutline->set_active( aLayoutItem.GetOptionsLayout().IsMoveOutline() );
    m_xCbxDragStripes->set_active( aLayoutItem.GetOptionsLayout().IsDragStripes() );
    m_xCbxHandlesBezier->set_active( aLayoutItem.GetOptionsLayout().IsHandlesBezier() );

    m_xCbxRuler->save_state();
    m_xCbxMoveOutline->save_state();
    m_xCbxDragStripes->save_state();
    m_xCbxHandlesBezier->save_state();
}

std::unique_ptr<SfxTabPage> SdTpOptionsContents::Create( weld::Container* pPage, weld::DialogController* pController,
                                                const SfxItemSet* rAttrs )
{
    return std::make_unique<SdTpOptionsContents>(pPage, pController, *rAttrs);
}

/*************************************************************************
|*
|*  TabPage to adjust the misc options
|*
\************************************************************************/
#define TABLE_COUNT 12
#define TOKEN ':'

SdTpOptionsMisc::SdTpOptionsMisc(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "modules/simpress/ui/optimpressgeneralpage.ui", "OptSavePage", &rInAttrs)
    , nWidth(0)
    , nHeight(0)
    , m_xCbxQuickEdit(m_xBuilder->weld_check_button("qickedit"))
    , m_xCbxPickThrough(m_xBuilder->weld_check_button("textselected"))
    , m_xNewDocumentFrame(m_xBuilder->weld_frame("newdocumentframe"))
    , m_xCbxStartWithTemplate(m_xBuilder->weld_check_button("startwithwizard"))
    , m_xCbxMasterPageCache(m_xBuilder->weld_check_button("backgroundback"))
    , m_xCbxCopy(m_xBuilder->weld_check_button("copywhenmove"))
    , m_xCbxMarkedHitMovesAlways(m_xBuilder->weld_check_button("objalwymov"))
    , m_xPresentationFrame(m_xBuilder->weld_frame("presentationframe"))
    , m_xLbMetric(m_xBuilder->weld_combo_box("units"))
    , m_xMtrFldTabstop(m_xBuilder->weld_metric_spin_button("metricFields", FieldUnit::MM))
    , m_xCbxEnableSdremote(m_xBuilder->weld_check_button("enremotcont"))
    , m_xCbxEnablePresenterScreen(m_xBuilder->weld_check_button("enprsntcons"))
    , m_xCbxUsePrinterMetrics(m_xBuilder->weld_check_button("printermetrics"))
    , m_xCbxCompatibility(m_xBuilder->weld_check_button("cbCompatibility"))
    , m_xScaleFrame(m_xBuilder->weld_frame("scaleframe"))
    , m_xCbScale(m_xBuilder->weld_combo_box("scaleBox"))
    , m_xNewDocLb(m_xBuilder->weld_label("newdoclbl"))
    , m_xFiInfo1(m_xBuilder->weld_label("info1"))
    , m_xMtrFldOriginalWidth(m_xBuilder->weld_metric_spin_button("metricWidthFields", FieldUnit::MM))
    , m_xWidthLb(m_xBuilder->weld_label("widthlbl"))
    , m_xHeightLb(m_xBuilder->weld_label("heightlbl"))
    , m_xFiInfo2(m_xBuilder->weld_label("info2"))
    , m_xMtrFldOriginalHeight(m_xBuilder->weld_metric_spin_button("metricHeightFields", FieldUnit::MM))
    , m_xCbxDistort(m_xBuilder->weld_check_button("distortcb"))
    , m_xMtrFldInfo1(m_xBuilder->weld_metric_spin_button("metricInfo1Fields", FieldUnit::MM))
    , m_xMtrFldInfo2(m_xBuilder->weld_metric_spin_button("metricInfo2Fields", FieldUnit::MM))
{
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

    SetFieldUnit( *m_xMtrFldTabstop , eFUnit );

    // Impress is default mode, let' hide the entire scale frame etc.
    m_xCbxDistort->hide();
    m_xScaleFrame->hide();

    // fill ListBox with metrics
    for (sal_uInt32 i = 0; i < SvxFieldUnitTable::Count(); ++i)
    {
        OUString sMetric = SvxFieldUnitTable::GetString(i);
        sal_uInt32 nFieldUnit = sal_uInt32(SvxFieldUnitTable::GetValue(i));
        m_xLbMetric->append(OUString::number(nFieldUnit), sMetric);
    }
    m_xLbMetric->connect_changed( LINK( this, SdTpOptionsMisc, SelectMetricHdl_Impl ) );

    SetFieldUnit( *m_xMtrFldOriginalWidth, eFUnit );
    SetFieldUnit( *m_xMtrFldOriginalHeight, eFUnit );
    m_xMtrFldOriginalWidth->set_max(999999999, FieldUnit::NONE);
    m_xMtrFldOriginalHeight->set_max(999999999, FieldUnit::NONE);

    // temporary fields for info texts (for formatting/calculation)
    m_xMtrFldInfo1->set_unit( eFUnit );
    m_xMtrFldInfo1->set_max(999999999, FieldUnit::NONE);
    m_xMtrFldInfo1->set_digits( 2 );
    m_xMtrFldInfo2->set_unit( eFUnit );
    m_xMtrFldInfo2->set_max(999999999, FieldUnit::NONE);
    m_xMtrFldInfo2->set_digits( 2 );

    // determine PoolUnit
    SfxItemPool* pPool = rInAttrs.GetPool();
    DBG_ASSERT( pPool, "Where is the Pool?" );
    ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // Fill the CB
    sal_uInt16 aTable[ TABLE_COUNT ] =
        { 1, 2, 4, 5, 8, 10, 16, 20, 30, 40, 50, 100 };

    for( sal_uInt16 i = 0; i < TABLE_COUNT; i++ )
        m_xCbScale->append_text( GetScale( 1, aTable[i] ) );
    for( sal_uInt16 i = 1; i < TABLE_COUNT; i++ )
        m_xCbScale->append_text( GetScale(  aTable[i], 1 ) );
}

SdTpOptionsMisc::~SdTpOptionsMisc()
{
}

void SdTpOptionsMisc::ActivatePage( const SfxItemSet& rSet )
{
    // We have to call save_state again since it can happen that the value
    // has no effect on other TabPages
    m_xLbMetric->save_value();
    // change metric if necessary (since TabPage is in the Dialog where
    // the metric is set)
    const SfxPoolItem* pAttr = nullptr;
    if( SfxItemState::SET != rSet.GetItemState( SID_ATTR_METRIC , false,
                                    &pAttr ))
        return;

    const SfxUInt16Item* pItem = static_cast<const SfxUInt16Item*>(pAttr);

    FieldUnit eFUnit = static_cast<FieldUnit>(static_cast<tools::Long>(pItem->GetValue()));

    if( eFUnit == m_xMtrFldOriginalWidth->get_unit() )
        return;

    // set metrics
    sal_Int64 nVal = m_xMtrFldOriginalWidth->denormalize( m_xMtrFldOriginalWidth->get_value( FieldUnit::TWIP ) );
    SetFieldUnit( *m_xMtrFldOriginalWidth, eFUnit, true );
    m_xMtrFldOriginalWidth->set_value( m_xMtrFldOriginalWidth->normalize( nVal ), FieldUnit::TWIP );

    nVal = m_xMtrFldOriginalHeight->denormalize( m_xMtrFldOriginalHeight->get_value( FieldUnit::TWIP ) );
    SetFieldUnit( *m_xMtrFldOriginalHeight, eFUnit, true );
    m_xMtrFldOriginalHeight->set_value( m_xMtrFldOriginalHeight->normalize( nVal ), FieldUnit::TWIP );

    if( nWidth == 0 || nHeight == 0 )
        return;

    m_xMtrFldInfo1->set_unit( eFUnit );
    m_xMtrFldInfo2->set_unit( eFUnit );

    SetMetricValue( *m_xMtrFldInfo1, nWidth, ePoolUnit );
    aInfo1 = m_xMtrFldInfo1->get_text();
    m_xFiInfo1->set_label( aInfo1 );

    SetMetricValue( *m_xMtrFldInfo2, nHeight, ePoolUnit );
    aInfo2 = m_xMtrFldInfo2->get_text();
    m_xFiInfo2->set_label( aInfo2 );
}

DeactivateRC SdTpOptionsMisc::DeactivatePage( SfxItemSet* pActiveSet )
{
    // check parser
    sal_Int32 nX, nY;
    if( SetScale( m_xCbScale->get_active_text(), nX, nY ) )
    {
        if( pActiveSet )
            FillItemSet( pActiveSet );
        return DeactivateRC::LeavePage;
    }

    std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
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

    if( m_xCbxStartWithTemplate->get_state_changed_from_saved()     ||
        m_xCbxMarkedHitMovesAlways->get_state_changed_from_saved()  ||
        m_xCbxQuickEdit->get_state_changed_from_saved()             ||
        m_xCbxPickThrough->get_state_changed_from_saved()           ||
        m_xCbxMasterPageCache->get_state_changed_from_saved()       ||
        m_xCbxCopy->get_state_changed_from_saved()                  ||
        m_xCbxEnableSdremote->get_state_changed_from_saved()        ||
        m_xCbxEnablePresenterScreen->get_state_changed_from_saved() ||
        m_xCbxCompatibility->get_state_changed_from_saved()         ||
        m_xCbxUsePrinterMetrics->get_state_changed_from_saved()     ||
        m_xCbxDistort->get_state_changed_from_saved())
    {
        SdOptionsMiscItem aOptsItem;

        aOptsItem.GetOptionsMisc().SetStartWithTemplate( m_xCbxStartWithTemplate->get_active() );
        aOptsItem.GetOptionsMisc().SetMarkedHitMovesAlways( m_xCbxMarkedHitMovesAlways->get_active() );
        aOptsItem.GetOptionsMisc().SetQuickEdit( m_xCbxQuickEdit->get_active() );
        aOptsItem.GetOptionsMisc().SetPickThrough( m_xCbxPickThrough->get_active() );
        aOptsItem.GetOptionsMisc().SetMasterPagePaintCaching( m_xCbxMasterPageCache->get_active() );
        aOptsItem.GetOptionsMisc().SetDragWithCopy( m_xCbxCopy->get_active() );
        aOptsItem.GetOptionsMisc().SetEnableSdremote( m_xCbxEnableSdremote->get_active() );
        aOptsItem.GetOptionsMisc().SetEnablePresenterScreen( m_xCbxEnablePresenterScreen->get_active() );
        aOptsItem.GetOptionsMisc().SetSummationOfParagraphs( m_xCbxCompatibility->get_active() );
        aOptsItem.GetOptionsMisc().SetPrinterIndependentLayout (
            m_xCbxUsePrinterMetrics->get_active()
            ? css::document::PrinterIndependentLayout::DISABLED
            : css::document::PrinterIndependentLayout::ENABLED);
        aOptsItem.GetOptionsMisc().SetCrookNoContortion( m_xCbxDistort->get_active() );
        rAttrs->Put( aOptsItem );

        bModified = true;
    }

    // metric
    if (m_xLbMetric->get_value_changed_from_saved())
    {
        const sal_Int32 nMPos = m_xLbMetric->get_active();
        sal_uInt16 nFieldUnit = m_xLbMetric->get_id(nMPos).toUInt32();
        rAttrs->Put( SfxUInt16Item( GetWhich( SID_ATTR_METRIC ), nFieldUnit ) );
        bModified = true;
    }

    // tabulator space
    if( m_xMtrFldTabstop->get_value_changed_from_saved() )
    {
        sal_uInt16 nWh = GetWhich( SID_ATTR_DEFTABSTOP );
        MapUnit eUnit = rAttrs->GetPool()->GetMetric( nWh );
        SfxUInt16Item aDef( nWh,static_cast<sal_uInt16>(GetCoreValue( *m_xMtrFldTabstop, eUnit )) );
        rAttrs->Put( aDef );
        bModified = true;
    }

    sal_Int32 nX, nY;
    if( SetScale( m_xCbScale->get_active_text(), nX, nY ) )
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

    m_xCbxStartWithTemplate->set_active( aOptsItem.GetOptionsMisc().IsStartWithTemplate() );
    m_xCbxMarkedHitMovesAlways->set_active( aOptsItem.GetOptionsMisc().IsMarkedHitMovesAlways() );
    m_xCbxQuickEdit->set_active( aOptsItem.GetOptionsMisc().IsQuickEdit() );
    m_xCbxPickThrough->set_active( aOptsItem.GetOptionsMisc().IsPickThrough() );
    m_xCbxMasterPageCache->set_active( aOptsItem.GetOptionsMisc().IsMasterPagePaintCaching() );
    m_xCbxCopy->set_active( aOptsItem.GetOptionsMisc().IsDragWithCopy() );
    m_xCbxEnableSdremote->set_active( aOptsItem.GetOptionsMisc().IsEnableSdremote() );
    m_xCbxEnablePresenterScreen->set_active( aOptsItem.GetOptionsMisc().IsEnablePresenterScreen() );
    m_xCbxCompatibility->set_active( aOptsItem.GetOptionsMisc().IsSummationOfParagraphs() );
    m_xCbxUsePrinterMetrics->set_active( aOptsItem.GetOptionsMisc().GetPrinterIndependentLayout()==1 );
    m_xCbxDistort->set_active( aOptsItem.GetOptionsMisc().IsCrookNoContortion() );
    m_xCbxStartWithTemplate->save_state();
    m_xCbxMarkedHitMovesAlways->save_state();
    m_xCbxQuickEdit->save_state();
    m_xCbxPickThrough->save_state();

    m_xCbxMasterPageCache->save_state();
    m_xCbxCopy->save_state();
    m_xCbxEnableSdremote->save_state();
    m_xCbxEnablePresenterScreen->save_state();
    m_xCbxCompatibility->save_state();
    m_xCbxUsePrinterMetrics->save_state();
    m_xCbxDistort->save_state();

    // metric
    sal_uInt16 nWhich = GetWhich( SID_ATTR_METRIC );
    m_xLbMetric->set_active(-1);

    if ( rAttrs->GetItemState( nWhich ) >= SfxItemState::DEFAULT )
    {
        const SfxUInt16Item& rItem = static_cast<const SfxUInt16Item&>(rAttrs->Get( nWhich ));
        sal_uInt32 nFieldUnit = static_cast<sal_uInt32>(rItem.GetValue());

        for (sal_Int32 i = 0, nEntryCount = m_xLbMetric->get_count(); i < nEntryCount; ++i)
        {
            if (m_xLbMetric->get_id(i).toUInt32() == nFieldUnit)
            {
                m_xLbMetric->set_active( i );
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
        SetMetricValue( *m_xMtrFldTabstop, rItem.GetValue(), eUnit );
    }
    m_xLbMetric->save_value();
    m_xMtrFldTabstop->save_value();
    //Scale
    sal_Int32 nX = static_cast<const SfxInt32Item&>( rAttrs->
                 Get( ATTR_OPTIONS_SCALE_X ) ).GetValue();
    sal_Int32 nY = static_cast<const SfxInt32Item&>( rAttrs->
                 Get( ATTR_OPTIONS_SCALE_Y ) ).GetValue();
    nWidth = static_cast<const SfxUInt32Item&>( rAttrs->
                    Get( ATTR_OPTIONS_SCALE_WIDTH ) ).GetValue();
    nHeight = static_cast<const SfxUInt32Item&>( rAttrs->
                    Get( ATTR_OPTIONS_SCALE_HEIGHT ) ).GetValue();

    m_xCbScale->set_entry_text( GetScale( nX, nY ) );

    m_xMtrFldOriginalWidth->hide();
    m_xMtrFldOriginalWidth->set_text( aInfo1 ); // empty
    m_xMtrFldOriginalHeight->hide();
    m_xMtrFldOriginalHeight->set_text( aInfo2 ); //empty
    m_xFiInfo1->hide();
    m_xFiInfo2->hide();

    UpdateCompatibilityControls ();
}

std::unique_ptr<SfxTabPage> SdTpOptionsMisc::Create( weld::Container* pPage, weld::DialogController* pController,
                                            const SfxItemSet* rAttrs )
{
    return std::make_unique<SdTpOptionsMisc>( pPage, pController, *rAttrs );
}

IMPL_LINK_NOARG(SdTpOptionsMisc, SelectMetricHdl_Impl, weld::ComboBox&, void)
{
    int nPos = m_xLbMetric->get_active();
    if (nPos != -1)
    {
        FieldUnit eUnit = static_cast<FieldUnit>(m_xLbMetric->get_id(nPos).toUInt32());
        sal_Int64 nVal =
            m_xMtrFldTabstop->denormalize(m_xMtrFldTabstop->get_value(FieldUnit::TWIP));
        SetFieldUnit( *m_xMtrFldTabstop, eUnit );
        m_xMtrFldTabstop->set_value( m_xMtrFldTabstop->normalize( nVal ), FieldUnit::TWIP );
    }
}

void SdTpOptionsMisc::SetImpressMode()
{
#ifndef ENABLE_SDREMOTE_BLUETOOTH
    m_xCbxEnableSdremote->hide();
#else
    (void) this; // loplugin:staticmethods
#endif
}

void SdTpOptionsMisc::SetDrawMode()
{
    m_xScaleFrame->show();
    m_xNewDocumentFrame->hide();
    m_xCbxEnableSdremote->hide();
    m_xCbxEnablePresenterScreen->hide();
    m_xCbxCompatibility->hide();
    m_xNewDocLb->hide();
    m_xCbScale->show();
    m_xPresentationFrame->hide();
    m_xMtrFldInfo1->hide();
    m_xMtrFldInfo2->hide();
    m_xWidthLb->hide();
    m_xHeightLb->hide();
    m_xFiInfo1->show();
    m_xMtrFldOriginalWidth->show();
    m_xFiInfo2->show();
    m_xMtrFldOriginalHeight->show();
    m_xCbxDistort->show();
    m_xCbxCompatibility->hide();
}

OUString SdTpOptionsMisc::GetScale( sal_Int32 nX, sal_Int32 nY )
{
    return OUString::number(nX) + OUStringChar(TOKEN) + OUString::number(nY);
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

    rX = static_cast<tools::Long>(aTmp.toInt32());
    if( rX == 0 )
        return false;

    aTmp = aScale.getToken(0, TOKEN, nIdx);
    if (nIdx>=0)
        return false; // we require just 2 tokens!

    if (!comphelper::string::isdigitAsciiString(aTmp))
        return false;

    rY = static_cast<tools::Long>(aTmp.toInt32());
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

            Reference<container::XEnumerationAccess> xComponents =
                xDesktop->getComponents();
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

    m_xCbxCompatibility->set_sensitive(bIsEnabled);
    m_xCbxUsePrinterMetrics->set_sensitive(bIsEnabled);
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
