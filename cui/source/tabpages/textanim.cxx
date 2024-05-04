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

#include <textanim.hxx>
#include <textattr.hxx>
#include <TextColumnsPage.hxx>
#include <svx/dlgutil.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdview.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/sdtaiitm.hxx>
#include <svx/sdtayitm.hxx>
#include <svtools/unitconv.hxx>

const WhichRangesContainer SvxTextAnimationPage::pRanges(
    svl::Items<SDRATTR_TEXT_ANIKIND, SDRATTR_TEXT_ANIAMOUNT>);

/*************************************************************************
|*
|* constructor of the tab dialog: adds pages to the dialog
|*
\************************************************************************/

SvxTextTabDialog::SvxTextTabDialog(weld::Window* pParent, const SfxItemSet* pAttr, const SdrView* pSdrView)
    : SfxTabDialogController(pParent, "cui/ui/textdialog.ui", "TextDialog", pAttr)
    , pView(pSdrView)
{
    AddTabPage("RID_SVXPAGE_TEXTATTR", SvxTextAttrPage::Create, nullptr);
    AddTabPage("RID_SVXPAGE_TEXTANIMATION", SvxTextAnimationPage::Create, nullptr);
    AddTabPage("RID_SVXPAGE_TEXTCOLUMNS", SvxTextColumnsPage::Create, nullptr);
}

/*************************************************************************
|*
|* PageCreated()
|*
\************************************************************************/

void SvxTextTabDialog::PageCreated(const OUString& rId, SfxTabPage &rPage)
{
    if (rId != "RID_SVXPAGE_TEXTATTR")
        return;

    SdrObjKind eKind = SdrObjKind::NONE;
    if (pView)
    {
        const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
        bool bHasMarked = rMarkList.GetMarkCount() > 0;
        if (bHasMarked)
        {
            if (rMarkList.GetMarkCount() == 1)
            {
                const SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                eKind = pObj->GetObjIdentifier();
            }
        }
    }
    static_cast<SvxTextAttrPage&>(rPage).SetObjKind(eKind);
    static_cast<SvxTextAttrPage&>(rPage).Construct();
}

/*************************************************************************
|*
|* Page
|*
\************************************************************************/
SvxTextAnimationPage::SvxTextAnimationPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs)
    : SfxTabPage(pPage, pController, "cui/ui/textanimtabpage.ui", "TextAnimation", &rInAttrs)
    , eAniKind(SdrTextAniKind::NONE)
    , m_aUpState(TRISTATE_INDET)
    , m_aLeftState(TRISTATE_INDET)
    , m_aRightState(TRISTATE_INDET)
    , m_aDownState(TRISTATE_INDET)
    , m_xLbEffect(m_xBuilder->weld_combo_box("LB_EFFECT"))
    , m_xBoxDirection(m_xBuilder->weld_widget("boxDIRECTION"))
    , m_xBtnUp(m_xBuilder->weld_toggle_button("BTN_UP"))
    , m_xBtnLeft(m_xBuilder->weld_toggle_button("BTN_LEFT"))
    , m_xBtnRight(m_xBuilder->weld_toggle_button("BTN_RIGHT"))
    , m_xBtnDown(m_xBuilder->weld_toggle_button("BTN_DOWN"))
    , m_xFlProperties(m_xBuilder->weld_frame("FL_PROPERTIES"))
    , m_xTsbStartInside(m_xBuilder->weld_check_button("TSB_START_INSIDE"))
    , m_xTsbStopInside(m_xBuilder->weld_check_button("TSB_STOP_INSIDE"))
    , m_xBoxCount(m_xBuilder->weld_widget("boxCOUNT"))
    , m_xTsbEndless(m_xBuilder->weld_check_button("TSB_ENDLESS"))
    , m_xNumFldCount(m_xBuilder->weld_spin_button("NUM_FLD_COUNT"))
    , m_xTsbPixel(m_xBuilder->weld_check_button("TSB_PIXEL"))
    , m_xMtrFldAmount(m_xBuilder->weld_metric_spin_button("MTR_FLD_AMOUNT", FieldUnit::PIXEL))
    , m_xTsbAuto(m_xBuilder->weld_check_button("TSB_AUTO"))
    , m_xMtrFldDelay(m_xBuilder->weld_metric_spin_button("MTR_FLD_DELAY", FieldUnit::MILLISECOND))
{
    eFUnit = GetModuleFieldUnit( rInAttrs );
    SfxItemPool* pPool = rInAttrs.GetPool();
    assert(pPool && "Where is the pool?");
    eUnit = pPool->GetMetric( SDRATTR_TEXT_LEFTDIST );

    m_xLbEffect->connect_changed( LINK( this, SvxTextAnimationPage, SelectEffectHdl_Impl ) );
    m_xTsbEndless->connect_toggled( LINK( this, SvxTextAnimationPage, ClickEndlessHdl_Impl ) );
    m_xTsbAuto->connect_toggled( LINK( this, SvxTextAnimationPage, ClickAutoHdl_Impl ) );
    m_xTsbPixel->connect_toggled( LINK( this, SvxTextAnimationPage, ClickPixelHdl_Impl ) );

    Link<weld::Button&,void> aLink( LINK( this, SvxTextAnimationPage, ClickDirectionHdl_Impl ) );
    m_xBtnUp->connect_clicked( aLink );
    m_xBtnLeft->connect_clicked( aLink );
    m_xBtnRight->connect_clicked( aLink );
    m_xBtnDown->connect_clicked( aLink );
}

SvxTextAnimationPage::~SvxTextAnimationPage()
{
}

/*************************************************************************
|*
|* reads the passed item set
|*
\************************************************************************/

void SvxTextAnimationPage::Reset( const SfxItemSet* rAttrs )
{
    const SfxItemPool* pPool = rAttrs->GetPool();

    // animation type
    const SfxPoolItem* pItem = GetItem( *rAttrs, SDRATTR_TEXT_ANIKIND );

    if( !pItem )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_TEXT_ANIKIND );

    eAniKind = static_cast<const SdrTextAniKindItem*>(pItem)->GetValue();
    m_xLbEffect->set_active(sal::static_int_cast<sal_Int32>(eAniKind));
    m_xLbEffect->save_value();

    // animation direction
    pItem = GetItem( *rAttrs, SDRATTR_TEXT_ANIDIRECTION );
    if( !pItem )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_TEXT_ANIDIRECTION );

    SelectDirection(static_cast<const SdrTextAniDirectionItem*>(pItem)->GetValue());
    m_aUpState = m_xBtnUp->get_state();
    m_aLeftState = m_xBtnLeft->get_state();
    m_aRightState = m_xBtnRight->get_state();
    m_aDownState = m_xBtnDown->get_state();

    // Start inside
    pItem = GetItem( *rAttrs, SDRATTR_TEXT_ANISTARTINSIDE );
    if( !pItem )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_TEXT_ANISTARTINSIDE );

    if (static_cast<const SdrTextAniStartInsideItem*>(pItem)->GetValue())
        m_xTsbStartInside->set_state(TRISTATE_TRUE);
    else
        m_xTsbStartInside->set_state(TRISTATE_FALSE);
    m_xTsbStartInside->save_state();

    // Stop inside
    pItem = GetItem( *rAttrs, SDRATTR_TEXT_ANISTOPINSIDE );
    if( !pItem )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_TEXT_ANISTOPINSIDE );

    if (static_cast<const SdrTextAniStopInsideItem*>(pItem)->GetValue())
        m_xTsbStopInside->set_state(TRISTATE_TRUE);
    else
        m_xTsbStopInside->set_state(TRISTATE_FALSE);
    m_xTsbStopInside->save_state();

    // quantity
    pItem = GetItem( *rAttrs, SDRATTR_TEXT_ANICOUNT );
    if( !pItem )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_TEXT_ANICOUNT );

    tools::Long nValue = static_cast<tools::Long>(static_cast<const SdrTextAniCountItem*>(pItem)->GetValue());
    m_xNumFldCount->set_value(nValue);
    if (nValue == 0)
    {
        if (eAniKind == SdrTextAniKind::Slide)
        {
            m_xTsbEndless->set_state(TRISTATE_FALSE);
            m_xTsbEndless->set_sensitive(false);
        }
        else
        {
            m_xTsbEndless->set_state(TRISTATE_TRUE);
            m_xNumFldCount->set_text("");
        }
    }
    else
        m_xTsbEndless->set_state(TRISTATE_FALSE);
    m_xTsbEndless->save_state();
    m_xNumFldCount->save_value();

    // delay
    pItem = GetItem( *rAttrs, SDRATTR_TEXT_ANIDELAY );
    if( !pItem )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_TEXT_ANIDELAY );

    nValue = static_cast<tools::Long>(static_cast<const SdrTextAniDelayItem*>(pItem)->GetValue());
    m_xMtrFldDelay->set_value(nValue, FieldUnit::NONE);
    if (nValue == 0)
    {
        m_xTsbAuto->set_state(TRISTATE_TRUE);
        m_xMtrFldDelay->set_text("");
    }
    else
        m_xTsbAuto->set_state(TRISTATE_FALSE);
    m_xTsbAuto->save_state();
    m_xMtrFldDelay->save_value();

    // step size
    pItem = GetItem( *rAttrs, SDRATTR_TEXT_ANIAMOUNT );
    if( !pItem )
        pItem = &pPool->GetUserOrPoolDefaultItem( SDRATTR_TEXT_ANIAMOUNT );

    nValue = static_cast<tools::Long>(static_cast<const SdrTextAniAmountItem*>(pItem)->GetValue());
    if (nValue <= 0)
    {
        m_xTsbPixel->set_state(TRISTATE_TRUE);
        nValue = -nValue;
        if (nValue == 0)
            nValue++;
        m_xMtrFldAmount->set_unit(FieldUnit::CUSTOM);
        m_xMtrFldAmount->set_digits(0);

        m_xMtrFldAmount->set_increments(1, 10, FieldUnit::NONE);
        m_xMtrFldAmount->set_range(1, 100, FieldUnit::NONE);
        m_xMtrFldAmount->set_value(nValue, FieldUnit::NONE);
    }
    else
    {
        m_xTsbPixel->set_state(TRISTATE_FALSE);
        m_xMtrFldAmount->set_unit(eFUnit);
        m_xMtrFldAmount->set_digits(2);

        m_xMtrFldAmount->set_increments(10, 100, FieldUnit::NONE);
        m_xMtrFldAmount->set_range(1, 10000, FieldUnit::NONE);

        SetMetricValue(*m_xMtrFldAmount, nValue, eUnit);
    }
    m_xTsbPixel->save_state();
    m_xMtrFldAmount->save_value();

    SelectEffectHdl_Impl(*m_xLbEffect);
    ClickEndlessHdl_Impl(*m_xTsbEndless);
    ClickAutoHdl_Impl(*m_xTsbAuto);
}

/*************************************************************************
|*
|* fills the passed item set with dialog box attributes
|*
\************************************************************************/

bool SvxTextAnimationPage::FillItemSet( SfxItemSet* rAttrs)
{
    bool bModified = false;
    TriState eState;

    // animation type
    int nPos = m_xLbEffect->get_active();
    if( nPos != -1 &&
        m_xLbEffect->get_value_changed_from_saved() )
    {
        rAttrs->Put( SdrTextAniKindItem( static_cast<SdrTextAniKind>(nPos) ) );
        bModified = true;
    }

    // animation direction
    if (m_aUpState != m_xBtnUp->get_state() ||
        m_aLeftState != m_xBtnLeft->get_state() ||
        m_aRightState != m_xBtnRight->get_state() ||
        m_aDownState != m_xBtnDown->get_state())
    {
        SdrTextAniDirection eValue = static_cast<SdrTextAniDirection>(GetSelectedDirection());
        rAttrs->Put( SdrTextAniDirectionItem( eValue ) );
        bModified = true;
    }

    // Start inside
    eState = m_xTsbStartInside->get_state();
    if (m_xTsbStartInside->get_state_changed_from_saved())
    {
        rAttrs->Put( SdrTextAniStartInsideItem( TRISTATE_TRUE == eState ) );
        bModified = true;
    }

    // Stop inside
    eState = m_xTsbStopInside->get_state();
    if (m_xTsbStopInside->get_state_changed_from_saved())
    {
        rAttrs->Put( SdrTextAniStopInsideItem( TRISTATE_TRUE == eState ) );
        bModified = true;
    }

    // quantity
    eState = m_xTsbEndless->get_state();
    if (m_xTsbEndless->get_state_changed_from_saved() ||
        m_xNumFldCount->get_value_changed_from_saved())
    {
        sal_Int64 nValue = 0;
        if( eState == TRISTATE_TRUE /*#89844#*/ && m_xTsbEndless->get_sensitive())
            bModified = true;
        else
        {
            if( m_xNumFldCount->get_value_changed_from_saved() )
            {
                nValue = m_xNumFldCount->get_value();
                bModified = true;
            }
        }
        if( bModified )
            rAttrs->Put( SdrTextAniCountItem( static_cast<sal_uInt16>(nValue) ) );
    }

    // delay
    eState = m_xTsbAuto->get_state();
    if (m_xTsbAuto->get_state_changed_from_saved() ||
        m_xMtrFldDelay->get_value_changed_from_saved())
    {
        sal_Int64 nValue = 0;
        if( eState == TRISTATE_TRUE )
            bModified = true;
        else
        {
            if( m_xMtrFldDelay->get_value_changed_from_saved() )
            {
                nValue = m_xMtrFldDelay->get_value(FieldUnit::NONE);
                bModified = true;
            }
        }
        if( bModified )
            rAttrs->Put( SdrTextAniDelayItem( static_cast<sal_uInt16>(nValue) ) );
    }

    // step size
    eState = m_xTsbPixel->get_state();
    if (m_xTsbPixel->get_state_changed_from_saved() ||
        m_xMtrFldAmount->get_value_changed_from_saved())
    {
        sal_Int64 nValue = 0;
        if( eState == TRISTATE_TRUE )
        {
            nValue = m_xMtrFldAmount->get_value(FieldUnit::NONE);
            nValue = -nValue;
        }
        else
        {
            nValue = GetCoreValue( *m_xMtrFldAmount, eUnit );
        }
        rAttrs->Put( SdrTextAniAmountItem( static_cast<sal_Int16>(nValue) ) );

        bModified = true;
    }

    return bModified;
}

/*************************************************************************
|*
|* creates the page
|*
\************************************************************************/

std::unique_ptr<SfxTabPage> SvxTextAnimationPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrs)
{
    return std::make_unique<SvxTextAnimationPage>(pPage, pController, *rAttrs);
}

IMPL_LINK_NOARG(SvxTextAnimationPage, SelectEffectHdl_Impl, weld::ComboBox&, void)
{
    int nPos = m_xLbEffect->get_active();
    if (nPos == -1)
        return;

    eAniKind = static_cast<SdrTextAniKind>(nPos);
    switch( eAniKind )
    {
        case SdrTextAniKind::NONE:
        {
            m_xBoxDirection->set_sensitive(false);
            m_xFlProperties->set_sensitive(false);
        }
        break;

        case SdrTextAniKind::Blink:
        case SdrTextAniKind::Scroll:
        case SdrTextAniKind::Alternate:
        case SdrTextAniKind::Slide:
        {
            m_xFlProperties->set_sensitive(true);
            if( eAniKind == SdrTextAniKind::Slide )
            {
                m_xTsbStartInside->set_sensitive(false);
                m_xTsbStopInside->set_sensitive(false);
                m_xTsbEndless->set_sensitive(false);
                m_xNumFldCount->set_sensitive(true);
                m_xNumFldCount->set_value(m_xNumFldCount->get_value());
            }
            else
            {
                m_xTsbStartInside->set_sensitive(true);
                m_xTsbStopInside->set_sensitive(true);
                m_xTsbEndless->set_sensitive(true);
                ClickEndlessHdl_Impl(*m_xTsbEndless);
            }

            m_xTsbAuto->set_sensitive(true);
            ClickAutoHdl_Impl(*m_xTsbAuto);

            if( eAniKind == SdrTextAniKind::Blink )
            {
                m_xBoxDirection->set_sensitive(false);
                m_xBoxCount->set_sensitive(false);
            }
            else
            {
                m_xBoxDirection->set_sensitive(true);
                m_xBoxCount->set_sensitive(true);
            }
        }
        break;
    }
}

IMPL_LINK_NOARG(SvxTextAnimationPage, ClickEndlessHdl_Impl, weld::Toggleable&, void)
{
    if( eAniKind == SdrTextAniKind::Slide )
        return;

    TriState eState = m_xTsbEndless->get_state();
    if( eState != TRISTATE_FALSE )
    {
        m_xNumFldCount->set_sensitive(false);
        m_xNumFldCount->set_text("");
    }
    else
    {
        m_xNumFldCount->set_sensitive(true);
        m_xNumFldCount->set_value(m_xNumFldCount->get_value());
    }
}

IMPL_LINK_NOARG(SvxTextAnimationPage, ClickAutoHdl_Impl, weld::Toggleable&, void)
{
    TriState eState = m_xTsbAuto->get_state();
    if( eState != TRISTATE_FALSE )
    {
        m_xMtrFldDelay->set_sensitive(false);
        m_xMtrFldDelay->set_text("");
    }
    else
    {
        m_xMtrFldDelay->set_sensitive(true);
        m_xMtrFldDelay->set_value(m_xMtrFldDelay->get_value(FieldUnit::NONE), FieldUnit::NONE); //to-do
    }
}

IMPL_LINK_NOARG(SvxTextAnimationPage, ClickPixelHdl_Impl, weld::Toggleable&, void)
{
    TriState eState = m_xTsbPixel->get_state();
    if (eState == TRISTATE_TRUE)
    {
        int nValue = m_xMtrFldAmount->get_value(FieldUnit::NONE) / 10;
        m_xMtrFldAmount->set_sensitive(true);
        m_xMtrFldAmount->set_unit(FieldUnit::CUSTOM);
        m_xMtrFldAmount->set_digits(0);

        m_xMtrFldAmount->set_increments(1, 10, FieldUnit::NONE);
        m_xMtrFldAmount->set_range(1, 100, FieldUnit::NONE);

        m_xMtrFldAmount->set_value(nValue, FieldUnit::NONE);
    }
    else if( eState == TRISTATE_FALSE )
    {
        int nValue = m_xMtrFldAmount->get_value(FieldUnit::NONE) * 10;
        m_xMtrFldAmount->set_sensitive(true);
        m_xMtrFldAmount->set_unit(eFUnit);
        m_xMtrFldAmount->set_digits(2);

        m_xMtrFldAmount->set_increments(10, 100, FieldUnit::NONE);
        m_xMtrFldAmount->set_range(1, 10000, FieldUnit::NONE);

        m_xMtrFldAmount->set_value(nValue, FieldUnit::NONE);
    }
}

IMPL_LINK(SvxTextAnimationPage, ClickDirectionHdl_Impl, weld::Button&, rBtn, void)
{
    m_xBtnUp->set_active(&rBtn == m_xBtnUp.get());
    m_xBtnLeft->set_active(&rBtn == m_xBtnLeft.get());
    m_xBtnRight->set_active(&rBtn == m_xBtnRight.get());
    m_xBtnDown->set_active(&rBtn == m_xBtnDown.get());
}

void SvxTextAnimationPage::SelectDirection( SdrTextAniDirection nValue )
{
    m_xBtnUp->set_active( nValue == SdrTextAniDirection::Up );
    m_xBtnLeft->set_active( nValue == SdrTextAniDirection::Left );
    m_xBtnRight->set_active( nValue == SdrTextAniDirection::Right );
    m_xBtnDown->set_active( nValue == SdrTextAniDirection::Down );
}

sal_uInt16 SvxTextAnimationPage::GetSelectedDirection() const
{
    SdrTextAniDirection nValue = SdrTextAniDirection::Left;

    if( m_xBtnUp->get_active() )
        nValue = SdrTextAniDirection::Up;
    else if( m_xBtnLeft->get_active() )
        nValue = SdrTextAniDirection::Left;
    else if( m_xBtnRight->get_active() )
        nValue = SdrTextAniDirection::Right;
    else if( m_xBtnDown->get_active() )
        nValue = SdrTextAniDirection::Down;

    return static_cast<sal_uInt16>(nValue);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
