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
#include <officecfg/Office/Impress.hxx>
#include <officecfg/Office/Draw.hxx>
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
#include <o3tl/string_view.hxx>

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
    bool bDrawMode = SvxGridTabPage::IsDrawMode();

    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());

    if (bDrawMode)
    {
        officecfg::Office::Draw::Snap::Object::SnapLine::set( m_xCbxSnapHelplines->get_active(), batch );
        officecfg::Office::Draw::Snap::Object::PageMargin::set( m_xCbxSnapBorder->get_active(), batch );
        officecfg::Office::Draw::Snap::Object::ObjectFrame::set( m_xCbxSnapFrame->get_active(), batch );
        officecfg::Office::Draw::Snap::Object::ObjectPoint::set( m_xCbxSnapPoints->get_active(), batch );
        officecfg::Office::Draw::Snap::Position::CreatingMoving::set( m_xCbxOrtho->get_active(), batch );
        officecfg::Office::Draw::Snap::Position::ExtendEdges::set( m_xCbxBigOrtho->get_active(), batch );
        officecfg::Office::Draw::Snap::Position::Rotating::set( m_xCbxRotate->get_active(), batch );
        officecfg::Office::Draw::Snap::Object::Range::set( static_cast<sal_Int16>(m_xMtrFldSnapArea->get_value(FieldUnit::PIXEL)), batch );
        officecfg::Office::Draw::Snap::Position::RotatingValue::set( static_cast<sal_Int32>(Degree100(m_xMtrFldAngle->get_value(FieldUnit::DEGREE))), batch );
        officecfg::Office::Draw::Snap::Position::PointReduction::set ( static_cast<sal_Int32>(Degree100(m_xMtrFldBezAngle->get_value(FieldUnit::DEGREE))), batch );
    }
    else
    {
        officecfg::Office::Impress::Snap::Object::SnapLine::set( m_xCbxSnapHelplines->get_active(), batch );
        officecfg::Office::Impress::Snap::Object::PageMargin::set( m_xCbxSnapBorder->get_active(), batch );
        officecfg::Office::Impress::Snap::Object::ObjectFrame::set( m_xCbxSnapFrame->get_active(), batch );
        officecfg::Office::Impress::Snap::Object::ObjectPoint::set( m_xCbxSnapPoints->get_active(), batch );
        officecfg::Office::Impress::Snap::Position::CreatingMoving::set( m_xCbxOrtho->get_active(), batch );
        officecfg::Office::Impress::Snap::Position::ExtendEdges::set( m_xCbxBigOrtho->get_active(), batch );
        officecfg::Office::Impress::Snap::Position::Rotating::set( m_xCbxRotate->get_active(), batch );
        officecfg::Office::Impress::Snap::Object::Range::set( static_cast<sal_Int16>(m_xMtrFldSnapArea->get_value(FieldUnit::PIXEL)), batch );
        officecfg::Office::Impress::Snap::Position::RotatingValue::set( static_cast<sal_Int32>(Degree100(m_xMtrFldAngle->get_value(FieldUnit::DEGREE))), batch );
        officecfg::Office::Impress::Snap::Position::PointReduction::set ( static_cast<sal_Int32>(Degree100(m_xMtrFldBezAngle->get_value(FieldUnit::DEGREE))), batch );
    }

    batch->commit();

    // we get a possible existing GridItem, this ensures that we do not set
    // some default values by accident
    return true;
}

void SdTpOptionsSnap::Reset( const SfxItemSet* rAttrs )
{
    SvxGridTabPage::Reset(rAttrs);

    bool bDrawMode = SvxGridTabPage::IsDrawMode();
    if (bDrawMode)
    {
        m_xCbxSnapHelplines->set_active( officecfg::Office::Draw::Snap::Object::SnapLine::get() );
        m_xCbxSnapBorder->set_active( officecfg::Office::Draw::Snap::Object::PageMargin::get() );
        m_xCbxSnapFrame->set_active( officecfg::Office::Draw::Snap::Object::ObjectFrame::get() );
        m_xCbxSnapPoints->set_active( officecfg::Office::Draw::Snap::Object::ObjectPoint::get() );
        m_xCbxOrtho->set_active( officecfg::Office::Draw::Snap::Position::CreatingMoving::get() );
        m_xCbxBigOrtho->set_active( officecfg::Office::Draw::Snap::Position::ExtendEdges::get() );
        m_xCbxRotate->set_active( officecfg::Office::Draw::Snap::Position::Rotating::get() );
        m_xMtrFldSnapArea->set_value( officecfg::Office::Draw::Snap::Object::Range::get(), FieldUnit::PIXEL);
        m_xMtrFldAngle->set_value( officecfg::Office::Draw::Snap::Position::RotatingValue::get(), FieldUnit::DEGREE);
        m_xMtrFldBezAngle->set_value( officecfg::Office::Draw::Snap::Position::PointReduction::get(), FieldUnit::DEGREE);
    }
    else
    {
        m_xCbxSnapHelplines->set_active( officecfg::Office::Impress::Snap::Object::SnapLine::get() );
        m_xCbxSnapBorder->set_active( officecfg::Office::Impress::Snap::Object::PageMargin::get() );
        m_xCbxSnapFrame->set_active( officecfg::Office::Impress::Snap::Object::ObjectFrame::get() );
        m_xCbxSnapPoints->set_active( officecfg::Office::Impress::Snap::Object::ObjectPoint::get() );
        m_xCbxOrtho->set_active( officecfg::Office::Impress::Snap::Position::CreatingMoving::get() );
        m_xCbxBigOrtho->set_active( officecfg::Office::Impress::Snap::Position::ExtendEdges::get() );
        m_xCbxRotate->set_active( officecfg::Office::Impress::Snap::Position::Rotating::get() );
        m_xMtrFldSnapArea->set_value( officecfg::Office::Impress::Snap::Object::Range::get(), FieldUnit::PIXEL);
        m_xMtrFldAngle->set_value( officecfg::Office::Impress::Snap::Position::RotatingValue::get(), FieldUnit::DEGREE);
        m_xMtrFldBezAngle->set_value( officecfg::Office::Impress::Snap::Position::PointReduction::get(), FieldUnit::DEGREE);
    }

    bool bReadOnly = bDrawMode ? officecfg::Office::Draw::Snap::Object::SnapLine::isReadOnly() :
        officecfg::Office::Impress::Snap::Object::SnapLine::isReadOnly();
    m_xCbxSnapHelplines->set_sensitive(!bReadOnly);
    m_xCbxSnapHelplinesImg->set_visible(bReadOnly);

    bReadOnly = bDrawMode ? officecfg::Office::Draw::Snap::Object::PageMargin::isReadOnly() :
        officecfg::Office::Impress::Snap::Object::PageMargin::isReadOnly();
    m_xCbxSnapBorder->set_sensitive(!bReadOnly);
    m_xCbxSnapBorderImg->set_visible(bReadOnly);

    bReadOnly = bDrawMode ? officecfg::Office::Draw::Snap::Object::ObjectFrame::isReadOnly() :
        officecfg::Office::Impress::Snap::Object::ObjectFrame::isReadOnly();
    m_xCbxSnapFrame->set_sensitive(!bReadOnly);
    m_xCbxSnapFrameImg->set_visible(bReadOnly);

    bReadOnly = bDrawMode ? officecfg::Office::Draw::Snap::Object::ObjectPoint::isReadOnly() :
        officecfg::Office::Impress::Snap::Object::ObjectPoint::isReadOnly();
    m_xCbxSnapPoints->set_sensitive(!bReadOnly);
    m_xCbxSnapPointsImg->set_visible(bReadOnly);

    bReadOnly = bDrawMode ? officecfg::Office::Draw::Snap::Position::CreatingMoving::isReadOnly() :
        officecfg::Office::Impress::Snap::Position::CreatingMoving::isReadOnly();
    m_xCbxOrtho->set_sensitive(!bReadOnly);
    m_xCbxOrthoImg->set_visible(bReadOnly);

    bReadOnly = bDrawMode ? officecfg::Office::Draw::Snap::Position::ExtendEdges::isReadOnly() :
        officecfg::Office::Impress::Snap::Position::ExtendEdges::isReadOnly();
    m_xCbxBigOrtho->set_sensitive(!bReadOnly);
    m_xCbxBigOrthoImg->set_visible(bReadOnly);

    bReadOnly = bDrawMode ? officecfg::Office::Draw::Snap::Position::Rotating::isReadOnly() :
        officecfg::Office::Impress::Snap::Position::Rotating::isReadOnly();
    m_xCbxRotate->set_sensitive(!bReadOnly);
    m_xCbxRotateImg->set_visible(bReadOnly);

    bReadOnly = bDrawMode ? officecfg::Office::Draw::Snap::Object::Range::isReadOnly() :
        officecfg::Office::Impress::Snap::Object::Range::isReadOnly();
    m_xMtrFldSnapArea->set_sensitive(!bReadOnly);
    m_xMtrFldSnapAreaImg->set_visible(bReadOnly);

    bReadOnly = bDrawMode ? officecfg::Office::Draw::Snap::Position::RotatingValue::isReadOnly() :
        officecfg::Office::Impress::Snap::Position::RotatingValue::isReadOnly();
    m_xMtrFldAngle->set_sensitive(!bReadOnly);

    bReadOnly = bDrawMode ? officecfg::Office::Draw::Snap::Position::PointReduction::isReadOnly() :
        officecfg::Office::Impress::Snap::Position::PointReduction::isReadOnly();
    m_xMtrFldBezAngle->set_sensitive(!bReadOnly);
    m_xMtrFldBezAngleImg->set_visible(bReadOnly);

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
    : SfxTabPage(pPage, pController, u"modules/simpress/ui/sdviewpage.ui"_ustr, u"SdViewPage"_ustr, &rInAttrs)
    , m_bDrawMode(false)
    , m_xCbxRuler(m_xBuilder->weld_check_button(u"ruler"_ustr))
    , m_xCbxRulerImg(m_xBuilder->weld_widget(u"lockruler"_ustr))
    , m_xCbxDragStripes(m_xBuilder->weld_check_button(u"dragstripes"_ustr))
    , m_xCbxDragStripesImg(m_xBuilder->weld_widget(u"lockdragstripes"_ustr))
    , m_xCbxHandlesBezier(m_xBuilder->weld_check_button(u"handlesbezier"_ustr))
    , m_xCbxHandlesBezierImg(m_xBuilder->weld_widget(u"lockhandlesbezier"_ustr))
    , m_xCbxMoveOutline(m_xBuilder->weld_check_button(u"moveoutline"_ustr))
    , m_xCbxMoveOutlineImg(m_xBuilder->weld_widget(u"lockmoveoutline"_ustr))
{
}

SdTpOptionsContents::~SdTpOptionsContents()
{
}

OUString SdTpOptionsContents::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"label1"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"ruler"_ustr, u"dragstripes"_ustr, u"handlesbezier"_ustr, u"moveoutline"_ustr };

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SdTpOptionsContents::FillItemSet( SfxItemSet* )
{
    bool bModified = false;

    if( m_xCbxRuler->get_state_changed_from_saved() ||
        m_xCbxMoveOutline->get_state_changed_from_saved() ||
        m_xCbxDragStripes->get_state_changed_from_saved() ||
        m_xCbxHandlesBezier->get_state_changed_from_saved() )
    {
        std::shared_ptr<comphelper::ConfigurationChanges> batch(
            comphelper::ConfigurationChanges::create());

        if (m_bDrawMode)
        {
            officecfg::Office::Draw::Layout::Display::Ruler::set(m_xCbxRuler->get_active(), batch);
            officecfg::Office::Draw::Layout::Display::Contour::set(m_xCbxMoveOutline->get_active(), batch);
            officecfg::Office::Draw::Layout::Display::Guide::set(m_xCbxDragStripes->get_active(), batch);
            officecfg::Office::Draw::Layout::Display::Bezier::set(m_xCbxHandlesBezier->get_active(), batch);
            batch->commit();
        }
        else
        {
            officecfg::Office::Impress::Layout::Display::Ruler::set(m_xCbxRuler->get_active(), batch);
            officecfg::Office::Impress::Layout::Display::Contour::set(m_xCbxMoveOutline->get_active(), batch);
            officecfg::Office::Impress::Layout::Display::Guide::set(m_xCbxDragStripes->get_active(), batch);
            officecfg::Office::Impress::Layout::Display::Bezier::set(m_xCbxHandlesBezier->get_active(), batch);
            batch->commit();
        }
        bModified = true;
    }
    return bModified;
}

void SdTpOptionsContents::Reset( const SfxItemSet* )
{
    if (m_bDrawMode)
    {
        m_xCbxRuler->set_active(officecfg::Office::Draw::Layout::Display::Ruler::get());
        m_xCbxMoveOutline->set_active(officecfg::Office::Draw::Layout::Display::Contour::get());
        m_xCbxDragStripes->set_active(officecfg::Office::Draw::Layout::Display::Guide::get());
        m_xCbxHandlesBezier->set_active(officecfg::Office::Draw::Layout::Display::Bezier::get());
    }
    else
    {
        m_xCbxRuler->set_active(officecfg::Office::Impress::Layout::Display::Ruler::get());
        m_xCbxMoveOutline->set_active(officecfg::Office::Impress::Layout::Display::Contour::get());
        m_xCbxDragStripes->set_active(officecfg::Office::Impress::Layout::Display::Guide::get());
        m_xCbxHandlesBezier->set_active(officecfg::Office::Impress::Layout::Display::Bezier::get());
    }

    bool bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Layout::Display::Ruler::isReadOnly() :
        officecfg::Office::Impress::Layout::Display::Ruler::isReadOnly();
    m_xCbxRuler->set_sensitive(!bReadOnly);
    m_xCbxRulerImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Layout::Display::Contour::isReadOnly() :
        officecfg::Office::Impress::Layout::Display::Contour::isReadOnly();
    m_xCbxMoveOutline->set_sensitive(!bReadOnly);
    m_xCbxMoveOutlineImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Layout::Display::Guide::isReadOnly() :
        officecfg::Office::Impress::Layout::Display::Guide::isReadOnly();
    m_xCbxDragStripes->set_sensitive(!bReadOnly);
    m_xCbxDragStripesImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Layout::Display::Bezier::isReadOnly() :
        officecfg::Office::Impress::Layout::Display::Bezier::isReadOnly();
    m_xCbxHandlesBezier->set_sensitive(!bReadOnly);
    m_xCbxHandlesBezierImg->set_visible(bReadOnly);

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

void SdTpOptionsContents::PageCreated( const SfxAllItemSet& aSet )
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_SDMODE_FLAG, false);
    if (pFlagItem)
    {
        sal_uInt32 nFlags = pFlagItem->GetValue();
        if ((nFlags & SD_DRAW_MODE) == SD_DRAW_MODE)
            SetDrawMode();
    }
}

/*************************************************************************
|*
|*  TabPage to adjust the misc options
|*
\************************************************************************/
#define TABLE_COUNT 12
#define TOKEN ':'

SdTpOptionsMisc::SdTpOptionsMisc(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, u"modules/simpress/ui/optimpressgeneralpage.ui"_ustr, u"OptSavePage"_ustr, &rInAttrs)
    , nWidth(0)
    , nHeight(0)
    , m_bDrawMode(false)
    , m_xCbxQuickEdit(m_xBuilder->weld_check_button(u"qickedit"_ustr))
    , m_xCbxQuickEditImg(m_xBuilder->weld_widget(u"lockqickedit"_ustr))
    , m_xCbxPickThrough(m_xBuilder->weld_check_button(u"textselected"_ustr))
    , m_xCbxPickThroughImg(m_xBuilder->weld_widget(u"locktextselected"_ustr))
    , m_xNewDocumentFrame(m_xBuilder->weld_frame(u"newdocumentframe"_ustr))
    , m_xCbxStartWithTemplate(m_xBuilder->weld_check_button(u"startwithwizard"_ustr))
    , m_xCbxStartWithTemplateImg(m_xBuilder->weld_widget(u"lockstartwithwizard"_ustr))
    , m_xCbxMasterPageCache(m_xBuilder->weld_check_button(u"backgroundback"_ustr))
    , m_xCbxMasterPageCacheImg(m_xBuilder->weld_widget(u"lockbackgroundback"_ustr))
    , m_xCbxCopy(m_xBuilder->weld_check_button(u"copywhenmove"_ustr))
    , m_xCbxCopyImg(m_xBuilder->weld_widget(u"lockcopywhenmove"_ustr))
    , m_xCbxMarkedHitMovesAlways(m_xBuilder->weld_check_button(u"objalwymov"_ustr))
    , m_xCbxMarkedHitMovesAlwaysImg(m_xBuilder->weld_widget(u"lockobjalwymov"_ustr))
    , m_xLbMetric(m_xBuilder->weld_combo_box(u"units"_ustr))
    , m_xLbMetricImg(m_xBuilder->weld_widget(u"lockunits"_ustr))
    , m_xMtrFldTabstop(m_xBuilder->weld_metric_spin_button(u"metricFields"_ustr, FieldUnit::MM))
    , m_xMtrFldTabstopImg(m_xBuilder->weld_widget(u"locktabstop"_ustr))
    , m_xCbxCompatibility(m_xBuilder->weld_check_button(u"cbCompatibility"_ustr))
    , m_xCbxCompatibilityImg(m_xBuilder->weld_widget(u"lockcbCompatibility"_ustr))
    , m_xScaleFrame(m_xBuilder->weld_frame(u"scaleframe"_ustr))
    , m_xCbScale(m_xBuilder->weld_combo_box(u"scaleBox"_ustr))
    , m_xCbScaleImg(m_xBuilder->weld_widget(u"lockscaleBox"_ustr))
    , m_xNewDocLb(m_xBuilder->weld_label(u"newdoclbl"_ustr))
    , m_xFiInfo1(m_xBuilder->weld_label(u"info1"_ustr))
    , m_xMtrFldOriginalWidth(m_xBuilder->weld_metric_spin_button(u"metricWidthFields"_ustr, FieldUnit::MM))
    , m_xWidthLb(m_xBuilder->weld_label(u"widthlbl"_ustr))
    , m_xHeightLb(m_xBuilder->weld_label(u"heightlbl"_ustr))
    , m_xFiInfo2(m_xBuilder->weld_label(u"info2"_ustr))
    , m_xMtrFldOriginalHeight(m_xBuilder->weld_metric_spin_button(u"metricHeightFields"_ustr, FieldUnit::MM))
    , m_xCbxDistort(m_xBuilder->weld_check_button(u"distortcb"_ustr))
    , m_xCbxDistortImg(m_xBuilder->weld_widget(u"lockdistortcb"_ustr))
    , m_xMtrFldInfo1(m_xBuilder->weld_metric_spin_button(u"metricInfo1Fields"_ustr, FieldUnit::MM))
    , m_xMtrFldInfo2(m_xBuilder->weld_metric_spin_button(u"metricInfo2Fields"_ustr, FieldUnit::MM))
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
    // tdf#148292 - avoid right frame to change position depending on width of this control
    m_xMtrFldTabstop->set_size_request(150, -1);
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
    assert(pPool &&  "Where is the Pool?");
    ePoolUnit = pPool->GetMetric( SID_ATTR_FILL_HATCH );

    // Fill the CB
    sal_uInt16 aTable[ TABLE_COUNT ] =
        { 1, 2, 4, 5, 8, 10, 16, 20, 30, 40, 50, 100 };

    for( sal_uInt16 i = TABLE_COUNT-1; i > 0 ; i-- )
        m_xCbScale->append_text( GetScale( 1, aTable[i] ) );
    for( sal_uInt16 i = 0; i < TABLE_COUNT; i++ )
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
    const SfxUInt16Item* pAttr = rSet.GetItemIfSet( SID_ATTR_METRIC , false );
    if( !pAttr )
        return;

    FieldUnit eFUnit = static_cast<FieldUnit>(static_cast<tools::Long>(pAttr->GetValue()));

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

OUString SdTpOptionsMisc::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[]
        = { u"newdoclbl"_ustr, u"label4"_ustr, u"label6"_ustr,   u"tabstoplabel"_ustr, u"label1"_ustr,    u"label7"_ustr, u"label2"_ustr,
            u"label5"_ustr,    u"label8"_ustr, u"widthlbl"_ustr, u"info1"_ustr,        u"heightlbl"_ustr, u"info2"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    OUString checkButton[] = { u"startwithwizard"_ustr, u"copywhenmove"_ustr, u"backgroundback"_ustr,
                               u"objalwymov"_ustr,      u"distortcb"_ustr,    u"cbCompatibility"_ustr,
                               u"qickedit"_ustr,     u"textselected"_ustr};

    for (const auto& check : checkButton)
    {
        if (const auto& pString = m_xBuilder->weld_check_button(check))
            sAllStrings += pString->get_label() + " ";
    }

    return sAllStrings.replaceAll("_", "");
}

bool SdTpOptionsMisc::FillItemSet( SfxItemSet* rAttrs )
{
    bool bModified = false;

    if( m_xCbxStartWithTemplate->get_state_changed_from_saved()         ||
        m_xCbxMarkedHitMovesAlways->get_state_changed_from_saved()      ||
        m_xCbxQuickEdit->get_state_changed_from_saved()                 ||
        m_xCbxPickThrough->get_state_changed_from_saved()               ||
        m_xCbxMasterPageCache->get_state_changed_from_saved()           ||
        m_xCbxCopy->get_state_changed_from_saved()                      ||
        m_xCbxCompatibility->get_state_changed_from_saved()             ||
        m_xCbxDistort->get_state_changed_from_saved())
    {
        SdOptionsMiscItem aOptsItem;

        aOptsItem.GetOptionsMisc().SetStartWithTemplate( m_xCbxStartWithTemplate->get_active() );
        aOptsItem.GetOptionsMisc().SetMarkedHitMovesAlways( m_xCbxMarkedHitMovesAlways->get_active() );
        aOptsItem.GetOptionsMisc().SetQuickEdit( m_xCbxQuickEdit->get_active() );
        aOptsItem.GetOptionsMisc().SetPickThrough( m_xCbxPickThrough->get_active() );
        aOptsItem.GetOptionsMisc().SetMasterPagePaintCaching( m_xCbxMasterPageCache->get_active() );
        aOptsItem.GetOptionsMisc().SetDragWithCopy( m_xCbxCopy->get_active() );
        aOptsItem.GetOptionsMisc().SetSummationOfParagraphs( m_xCbxCompatibility->get_active() );
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
        MapUnit eUnit = rAttrs->GetPool()->GetMetric( SID_ATTR_DEFTABSTOP );
        SfxUInt16Item aDef( SID_ATTR_DEFTABSTOP, static_cast<sal_uInt16>(GetCoreValue( *m_xMtrFldTabstop, eUnit )) );
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
    SdOptionsMiscItem aOptsItem( rAttrs->Get( ATTR_OPTIONS_MISC ) );

    bool bReadOnly = m_bDrawMode ? false : officecfg::Office::Impress::Misc::NewDoc::AutoPilot::isReadOnly();
    m_xCbxStartWithTemplate->set_active( aOptsItem.GetOptionsMisc().IsStartWithTemplate() );
    m_xCbxStartWithTemplate->set_sensitive(!bReadOnly);
    m_xCbxStartWithTemplateImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Misc::ObjectMoveable::isReadOnly() :
        officecfg::Office::Impress::Misc::ObjectMoveable::isReadOnly();
    m_xCbxMarkedHitMovesAlways->set_active( aOptsItem.GetOptionsMisc().IsMarkedHitMovesAlways() );
    m_xCbxMarkedHitMovesAlways->set_sensitive(!bReadOnly);
    m_xCbxMarkedHitMovesAlwaysImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Misc::TextObject::QuickEditing::isReadOnly() :
        officecfg::Office::Impress::Misc::TextObject::QuickEditing::isReadOnly();
    m_xCbxQuickEdit->set_active( aOptsItem.GetOptionsMisc().IsQuickEdit() );
    m_xCbxQuickEdit->set_sensitive(!bReadOnly);
    m_xCbxQuickEditImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Misc::TextObject::Selectable::isReadOnly() :
        officecfg::Office::Impress::Misc::TextObject::Selectable::isReadOnly();
    m_xCbxPickThrough->set_active( aOptsItem.GetOptionsMisc().IsPickThrough() );
    m_xCbxPickThrough->set_sensitive(!bReadOnly);
    m_xCbxPickThroughImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Misc::BackgroundCache::isReadOnly() :
        officecfg::Office::Impress::Misc::BackgroundCache::isReadOnly();
    m_xCbxMasterPageCache->set_active( aOptsItem.GetOptionsMisc().IsMasterPagePaintCaching() );
    m_xCbxMasterPageCache->set_sensitive(!bReadOnly);
    m_xCbxMasterPageCacheImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Misc::CopyWhileMoving::isReadOnly() :
        officecfg::Office::Impress::Misc::CopyWhileMoving::isReadOnly();
    m_xCbxCopy->set_active( aOptsItem.GetOptionsMisc().IsDragWithCopy() );
    m_xCbxCopy->set_sensitive(!bReadOnly);
    m_xCbxCopyImg->set_visible(bReadOnly);

    bReadOnly = m_bDrawMode ? false : officecfg::Office::Impress::Misc::Compatibility::AddBetween::isReadOnly();
    m_xCbxCompatibility->set_active( aOptsItem.GetOptionsMisc().IsSummationOfParagraphs() );
    m_xCbxCompatibility->set_sensitive(!bReadOnly);
    m_xCbxCompatibilityImg->set_visible(bReadOnly);

    m_xCbxDistort->set_active( aOptsItem.GetOptionsMisc().IsCrookNoContortion() );
    if (m_bDrawMode)
    {
        bReadOnly = officecfg::Office::Draw::Misc::NoDistort::isReadOnly();
        m_xCbxDistort->set_sensitive(!bReadOnly);
        m_xCbxDistortImg->set_visible(bReadOnly);
    }

    m_xCbxStartWithTemplate->save_state();
    m_xCbxMarkedHitMovesAlways->save_state();
    m_xCbxQuickEdit->save_state();
    m_xCbxPickThrough->save_state();

    m_xCbxMasterPageCache->save_state();
    m_xCbxCopy->save_state();
    m_xCbxCompatibility->save_state();
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
    constexpr auto nWhich2 = SID_ATTR_DEFTABSTOP;
    if( rAttrs->GetItemState( nWhich2 ) >= SfxItemState::DEFAULT )
    {
        MapUnit eUnit = rAttrs->GetPool()->GetMetric( nWhich2 );
        const SfxUInt16Item& rItem = rAttrs->Get( nWhich2 );
        SetMetricValue( *m_xMtrFldTabstop, rItem.GetValue(), eUnit );
    }

    if (SdOptionsGeneric::isMetricSystem())
    {
        bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Layout::Other::MeasureUnit::Metric::isReadOnly() :
            officecfg::Office::Impress::Layout::Other::MeasureUnit::Metric::isReadOnly();
    }
    else
    {
        bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Layout::Other::MeasureUnit::NonMetric::isReadOnly() :
            officecfg::Office::Impress::Layout::Other::MeasureUnit::NonMetric::isReadOnly();
    }
    m_xLbMetric->set_sensitive(!bReadOnly);
    m_xLbMetricImg->set_visible(bReadOnly);

    if (SdOptionsGeneric::isMetricSystem())
    {
        bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Layout::Other::TabStop::Metric::isReadOnly() :
            officecfg::Office::Impress::Layout::Other::TabStop::Metric::isReadOnly();
    }
    else
    {
        bReadOnly = m_bDrawMode ? officecfg::Office::Draw::Layout::Other::TabStop::NonMetric::isReadOnly() :
            officecfg::Office::Impress::Layout::Other::TabStop::NonMetric::isReadOnly();
    }
    m_xMtrFldTabstop->set_sensitive(!bReadOnly);
    m_xMtrFldTabstopImg->set_visible(bReadOnly);

    m_xLbMetric->save_value();
    m_xMtrFldTabstop->save_value();
    //Scale
    sal_Int32 nX = rAttrs->Get( ATTR_OPTIONS_SCALE_X ).GetValue();
    sal_Int32 nY = rAttrs->Get( ATTR_OPTIONS_SCALE_Y ).GetValue();
    nWidth = rAttrs->Get( ATTR_OPTIONS_SCALE_WIDTH ).GetValue();
    nHeight = rAttrs->Get( ATTR_OPTIONS_SCALE_HEIGHT ).GetValue();

    m_xCbScale->set_entry_text( GetScale( nX, nY ) );
    if (m_bDrawMode)
    {
        bReadOnly = officecfg::Office::Draw::Zoom::ScaleX::isReadOnly() &&
            officecfg::Office::Draw::Zoom::ScaleY::isReadOnly();
        m_xCbScale->set_sensitive(!bReadOnly);
        m_xCbScaleImg->set_visible(bReadOnly);
    }

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

void SdTpOptionsMisc::SetDrawMode()
{
    m_xScaleFrame->show();
    m_xNewDocumentFrame->hide();
    m_xCbxCompatibility->hide();
    m_xNewDocLb->hide();
    m_xCbScale->show();
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

    m_bDrawMode = true;
}

OUString SdTpOptionsMisc::GetScale( sal_Int32 nX, sal_Int32 nY )
{
    return OUString::number(nX) + OUStringChar(TOKEN) + OUString::number(nY);
}

bool SdTpOptionsMisc::SetScale( std::u16string_view aScale, sal_Int32& rX, sal_Int32& rY )
{
    if (aScale.empty())
        return false;

    sal_Int32 nIdx {0};

    std::u16string_view aTmp(o3tl::getToken(aScale, 0, TOKEN, nIdx));
    if (nIdx<0)
        return false; // we expect another token!

    if (!comphelper::string::isdigitAsciiString(aTmp))
        return false;

    rX = static_cast<tools::Long>(o3tl::toInt32(aTmp));
    if( rX == 0 )
        return false;

    aTmp = o3tl::getToken(aScale, 0, TOKEN, nIdx);
    if (nIdx>=0)
        return false; // we require just 2 tokens!

    if (!comphelper::string::isdigitAsciiString(aTmp))
        return false;

    rY = static_cast<tools::Long>(o3tl::toInt32(aTmp));
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

    m_xCbxCompatibility->set_sensitive(bIsEnabled && !officecfg::Office::Impress::Misc::Compatibility::AddBetween::isReadOnly());
}

void SdTpOptionsMisc::PageCreated(const SfxAllItemSet& aSet)
{
    const SfxUInt32Item* pFlagItem = aSet.GetItem<SfxUInt32Item>(SID_SDMODE_FLAG, false);
    if (pFlagItem)
    {
        sal_uInt32 nFlags=pFlagItem->GetValue();
        if ( ( nFlags & SD_DRAW_MODE ) == SD_DRAW_MODE )
            SetDrawMode();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
