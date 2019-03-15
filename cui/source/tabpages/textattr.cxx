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

#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <svx/svddef.hxx>
#include <svx/sdtditm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/sdtcfitm.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdview.hxx>
#include <svx/svdotext.hxx>
#include <svx/dialogs.hrc>

#include <textattr.hxx>
#include <svx/dlgutil.hxx>
#include <sfx2/request.hxx>
#include <svx/ofaitem.hxx>
#include <editeng/writingmodeitem.hxx>

using namespace ::com::sun::star;

const sal_uInt16 SvxTextAttrPage::pRanges[] =
{
      SDRATTR_MISC_FIRST
    , SDRATTR_TEXT_HORZADJUST
    , SDRATTR_TEXT_WORDWRAP
    , SDRATTR_TEXT_WORDWRAP
    , 0
};

/*************************************************************************
|*
|* dialog (page) for copying objects
|*
\************************************************************************/
SvxTextAttrPage::SvxTextAttrPage(TabPageParent pPage, const SfxItemSet& rInAttrs)
    : SvxTabPage(pPage, "cui/ui/textattrtabpage.ui", "TextAttributesPage", rInAttrs)
    , rOutAttrs(rInAttrs)
    , m_eObjKind(OBJ_NONE)
    , bAutoGrowSizeEnabled(false)
    , bContourEnabled(false)
    , bAutoGrowWidthEnabled(false)
    , bAutoGrowHeightEnabled(false)
    , bWordWrapTextEnabled(false)
    , bFitToSizeEnabled(false)
    , m_aCtlPosition(this)
    , m_xDrawingText(m_xBuilder->weld_widget("drawingtext"))
    , m_xCustomShapeText(m_xBuilder->weld_widget("customshapetext"))
    , m_xTsbAutoGrowWidth(m_xBuilder->weld_check_button("TSB_AUTOGROW_WIDTH"))
    , m_xTsbAutoGrowHeight(m_xBuilder->weld_check_button("TSB_AUTOGROW_HEIGHT"))
    , m_xTsbFitToSize(m_xBuilder->weld_check_button("TSB_FIT_TO_SIZE"))
    , m_xTsbContour(m_xBuilder->weld_check_button("TSB_CONTOUR"))
    , m_xTsbWordWrapText(m_xBuilder->weld_check_button("TSB_WORDWRAP_TEXT"))
    , m_xTsbAutoGrowSize(m_xBuilder->weld_check_button("TSB_AUTOGROW_SIZE"))
    , m_xFlDistance(m_xBuilder->weld_frame("FL_DISTANCE"))
    , m_xMtrFldLeft(m_xBuilder->weld_metric_spin_button("MTR_FLD_LEFT", FieldUnit::CM))
    , m_xMtrFldRight(m_xBuilder->weld_metric_spin_button("MTR_FLD_RIGHT", FieldUnit::CM))
    , m_xMtrFldTop(m_xBuilder->weld_metric_spin_button("MTR_FLD_TOP", FieldUnit::CM))
    , m_xMtrFldBottom(m_xBuilder->weld_metric_spin_button("MTR_FLD_BOTTOM", FieldUnit::CM))
    , m_xFlPosition(m_xBuilder->weld_frame("FL_POSITION"))
    , m_xCtlPosition(new weld::CustomWeld(*m_xBuilder, "CTL_POSITION", m_aCtlPosition))
    , m_xTsbFullWidth(m_xBuilder->weld_check_button("TSB_FULL_WIDTH"))
{
    m_aCtlPosition.SetControlSettings(RectPoint::MM, 240);

    FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit( *m_xMtrFldLeft, eFUnit );
    SetFieldUnit( *m_xMtrFldRight, eFUnit );
    SetFieldUnit( *m_xMtrFldTop, eFUnit );
    SetFieldUnit( *m_xMtrFldBottom, eFUnit );

    Link<weld::Button&,void> aLink( LINK( this, SvxTextAttrPage, ClickHdl_Impl ) );
    m_xTsbAutoGrowWidth->connect_clicked( aLink );
    m_xTsbAutoGrowHeight->connect_clicked( aLink );
    m_xTsbAutoGrowSize->connect_clicked( aLink );
    m_xTsbFitToSize->connect_clicked( aLink );
    m_xTsbContour->connect_clicked( aLink );

    m_xTsbFullWidth->connect_clicked(LINK( this, SvxTextAttrPage, ClickFullWidthHdl_Impl ) );
}

SvxTextAttrPage::~SvxTextAttrPage()
{
}

/*************************************************************************
|*
|* reads the passed item set
|*
\************************************************************************/

void SvxTextAttrPage::Reset( const SfxItemSet* rAttrs )
{
    SfxItemPool* pPool = rAttrs->GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    MapUnit eUnit = pPool->GetMetric( SDRATTR_TEXT_LEFTDIST );

    const SfxPoolItem* pItem = GetItem( *rAttrs, SDRATTR_TEXT_LEFTDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_LEFTDIST );

    SetMetricValue(*m_xMtrFldLeft, static_cast<const SdrMetricItem*>(pItem)->GetValue(), eUnit);
    m_xMtrFldLeft->save_value();

    pItem = GetItem( *rAttrs, SDRATTR_TEXT_RIGHTDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_RIGHTDIST );

    SetMetricValue(*m_xMtrFldRight, static_cast<const SdrMetricItem*>(pItem)->GetValue(), eUnit);
    m_xMtrFldRight->save_value();

    pItem = GetItem( *rAttrs, SDRATTR_TEXT_UPPERDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_UPPERDIST );

    SetMetricValue(*m_xMtrFldTop, static_cast<const SdrMetricItem*>(pItem)->GetValue(), eUnit);
    m_xMtrFldTop->save_value();

    pItem = GetItem( *rAttrs, SDRATTR_TEXT_LOWERDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_LOWERDIST );

    SetMetricValue(*m_xMtrFldBottom, static_cast<const SdrMetricItem*>(pItem)->GetValue(), eUnit);
    m_xMtrFldBottom->save_value();

    // adjust to height and autogrowsize
    if ( rAttrs->GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) != SfxItemState::DONTCARE )
    {
        m_xTsbAutoGrowHeight->set_state( rAttrs->Get( SDRATTR_TEXT_AUTOGROWHEIGHT ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );

        m_xTsbAutoGrowSize->set_state( rAttrs->Get( SDRATTR_TEXT_AUTOGROWHEIGHT ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    }
    else
    {
        m_xTsbAutoGrowHeight->set_state( TRISTATE_INDET );
        m_xTsbAutoGrowSize->set_state( TRISTATE_INDET );
    }
    m_xTsbAutoGrowHeight->save_state();
    m_xTsbAutoGrowSize->save_state();

    // adjust to width
    if ( rAttrs->GetItemState( SDRATTR_TEXT_AUTOGROWWIDTH ) != SfxItemState::DONTCARE )
    {
        m_xTsbAutoGrowWidth->set_state( rAttrs->Get( SDRATTR_TEXT_AUTOGROWWIDTH ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    }
    else
        m_xTsbAutoGrowWidth->set_state( TRISTATE_INDET );
    m_xTsbAutoGrowWidth->save_state();

    // wordwrap text
    if ( rAttrs->GetItemState( SDRATTR_TEXT_WORDWRAP ) != SfxItemState::DONTCARE )
    {
        m_xTsbWordWrapText->set_state( rAttrs->Get( SDRATTR_TEXT_WORDWRAP ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
    }
    else
        m_xTsbWordWrapText->set_state( TRISTATE_INDET );
    m_xTsbWordWrapText->save_state();


    // #103516# Do the setup based on states of hor/ver adjust
    // Setup center field and FullWidth
    SfxItemState eVState = rAttrs->GetItemState( SDRATTR_TEXT_VERTADJUST );
    SfxItemState eHState = rAttrs->GetItemState( SDRATTR_TEXT_HORZADJUST );

    if(SfxItemState::DONTCARE != eVState && SfxItemState::DONTCARE != eHState)
    {
        // VertAdjust and HorAdjust are unequivocal, thus
        SdrTextVertAdjust eTVA = rAttrs->Get(SDRATTR_TEXT_VERTADJUST).GetValue();
        SdrTextHorzAdjust eTHA = rAttrs->Get(SDRATTR_TEXT_HORZADJUST).GetValue();
        RectPoint eRP = RectPoint::LB;

        if (m_xTsbFullWidth->get_state() == TRISTATE_INDET)
            m_xTsbFullWidth->set_state(TRISTATE_FALSE);

        // Translate item values into local anchor position.
        switch (eTVA)
        {
            case SDRTEXTVERTADJUST_TOP:
            {
                switch (eTHA)
                {
                    case SDRTEXTHORZADJUST_LEFT: eRP = RectPoint::LT; break;
                    case SDRTEXTHORZADJUST_BLOCK:
                    case SDRTEXTHORZADJUST_CENTER: eRP = RectPoint::MT; break;
                    case SDRTEXTHORZADJUST_RIGHT: eRP = RectPoint::RT; break;
                }
                break;
            }
            case SDRTEXTVERTADJUST_BLOCK:
            case SDRTEXTVERTADJUST_CENTER:
            {
                switch (eTHA)
                {
                    case SDRTEXTHORZADJUST_LEFT: eRP = RectPoint::LM; break;
                    case SDRTEXTHORZADJUST_BLOCK:
                    case SDRTEXTHORZADJUST_CENTER: eRP = RectPoint::MM; break;
                    case SDRTEXTHORZADJUST_RIGHT: eRP = RectPoint::RM; break;
                }
                break;
            }
            case SDRTEXTVERTADJUST_BOTTOM:
            {
                switch (eTHA)
                {
                    case SDRTEXTHORZADJUST_LEFT: eRP = RectPoint::LB; break;
                    case SDRTEXTHORZADJUST_BLOCK:
                    case SDRTEXTHORZADJUST_CENTER: eRP = RectPoint::MB; break;
                    case SDRTEXTHORZADJUST_RIGHT: eRP = RectPoint::RB; break;
                }
                break;
            }
            default:
                break;
        }

        // See if we have to check the "full width" check button.
        bool bLeftToRight(IsTextDirectionLeftToRight());

        if((bLeftToRight && (SDRTEXTHORZADJUST_BLOCK == eTHA)) || (!bLeftToRight && (SDRTEXTVERTADJUST_BLOCK == eTVA)))
        {
            // Move anchor to valid position.
            ClickFullWidthHdl_Impl(*m_xTsbFullWidth);
            m_xTsbFullWidth->set_state(TRISTATE_TRUE);
        }

        m_aCtlPosition.SetActualRP( eRP );
    }
    else
    {
        // VertAdjust or HorAdjust is not unequivocal
        m_aCtlPosition.Reset();

        m_aCtlPosition.SetState(CTL_STATE::NOVERT);
        m_aCtlPosition.DoCompletelyDisable(true);

        m_xTsbFullWidth->set_state(TRISTATE_INDET);
        m_xFlPosition->set_sensitive( false );
    }

    // adjust to border
    if (rAttrs->GetItemState(SDRATTR_TEXT_FITTOSIZE) != SfxItemState::DONTCARE)
    {
        drawing::TextFitToSizeType const eFTS =
                    rAttrs->Get( SDRATTR_TEXT_FITTOSIZE ).GetValue();
        if (eFTS == drawing::TextFitToSizeType_AUTOFIT || eFTS == drawing::TextFitToSizeType_NONE)
            m_xTsbFitToSize->set_state( TRISTATE_FALSE );
        else
            m_xTsbFitToSize->set_state( TRISTATE_TRUE );
    }
    else
        m_xTsbFitToSize->set_state( TRISTATE_INDET );
    m_xTsbFitToSize->save_state();

    if( rAttrs->GetItemState( SDRATTR_TEXT_CONTOURFRAME ) != SfxItemState::DONTCARE )
    {
        bool bContour = rAttrs->Get( SDRATTR_TEXT_CONTOURFRAME ).GetValue();
        m_xTsbContour->set_state( bContour ? TRISTATE_TRUE : TRISTATE_FALSE );
    }
    else
        m_xTsbContour->set_state( TRISTATE_INDET );
    m_xTsbContour->save_state();

    ClickHdl_Impl(*m_xTsbContour);
}

/*************************************************************************
|*
|* fills the passed item set with dialog box attributes
|*
\************************************************************************/

bool SvxTextAttrPage::FillItemSet( SfxItemSet* rAttrs)
{
    SfxItemPool* pPool = rAttrs->GetPool();
    DBG_ASSERT( pPool, "Where is the pool?" );
    MapUnit eUnit = pPool->GetMetric( SDRATTR_TEXT_LEFTDIST );

    sal_Int32    nValue;
    TriState eState;

    if( m_xMtrFldLeft->get_value_changed_from_saved() )
    {
        nValue = GetCoreValue( *m_xMtrFldLeft, eUnit );
        rAttrs->Put( makeSdrTextLeftDistItem( nValue ) );
    }

    if( m_xMtrFldRight->get_value_changed_from_saved() )
    {
        nValue = GetCoreValue( *m_xMtrFldRight, eUnit );
        rAttrs->Put( makeSdrTextRightDistItem( nValue ) );
    }

    if( m_xMtrFldTop->get_value_changed_from_saved() )
    {
        nValue = GetCoreValue( *m_xMtrFldTop, eUnit );
        rAttrs->Put( makeSdrTextUpperDistItem( nValue ) );
    }

    if( m_xMtrFldBottom->get_value_changed_from_saved() )
    {
        nValue = GetCoreValue( *m_xMtrFldBottom, eUnit );
        rAttrs->Put( makeSdrTextLowerDistItem( nValue ) );
    }

    eState = m_xTsbAutoGrowHeight->get_state();
    if( m_xTsbAutoGrowHeight->get_state_changed_from_saved() )
    {
        rAttrs->Put( makeSdrTextAutoGrowHeightItem( TRISTATE_TRUE == eState ) );
    }

    eState = m_xTsbAutoGrowWidth->get_state();
    if( m_xTsbAutoGrowWidth->get_state_changed_from_saved() )
    {
        rAttrs->Put( makeSdrTextAutoGrowWidthItem( TRISTATE_TRUE == eState ) );
    }

    eState = m_xTsbAutoGrowSize->get_state();
    if( m_xTsbAutoGrowSize->get_state_changed_from_saved() )
    {
        rAttrs->Put( makeSdrTextAutoGrowHeightItem( TRISTATE_TRUE == eState ) );
    }

    eState = m_xTsbWordWrapText->get_state();
    if( m_xTsbWordWrapText->get_state_changed_from_saved() )
    {
        rAttrs->Put( makeSdrTextWordWrapItem( TRISTATE_TRUE == eState ) );
    }

    eState = m_xTsbContour->get_state();
    if( m_xTsbContour->get_state_changed_from_saved() )
    {
        rAttrs->Put( makeSdrTextContourFrameItem( TRISTATE_TRUE == eState ) );
    }

    eState = m_xTsbFitToSize->get_state();
    if( m_xTsbFitToSize->get_state_changed_from_saved() )
    {
        drawing::TextFitToSizeType eFTS;
        switch( eState )
        {
            default: ; //prevent warning
                OSL_FAIL( "svx::SvxTextAttrPage::FillItemSet(), unhandled state!" );
                [[fallthrough]];
            case TRISTATE_FALSE: eFTS = drawing::TextFitToSizeType_AUTOFIT; break;
            case TRISTATE_TRUE: eFTS = drawing::TextFitToSizeType_PROPORTIONAL; break;
        }
        rAttrs->Put( SdrTextFitToSizeTypeItem( eFTS ) );
    }

    // centered
    RectPoint eRP = m_aCtlPosition.GetActualRP();
    SdrTextVertAdjust eTVA, eOldTVA;
    SdrTextHorzAdjust eTHA, eOldTHA;

    switch( eRP )
    {
        default:
        case RectPoint::LT: eTVA = SDRTEXTVERTADJUST_TOP;
                    eTHA = SDRTEXTHORZADJUST_LEFT; break;
        case RectPoint::LM: eTVA = SDRTEXTVERTADJUST_CENTER;
                    eTHA = SDRTEXTHORZADJUST_LEFT; break;
        case RectPoint::LB: eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    eTHA = SDRTEXTHORZADJUST_LEFT; break;
        case RectPoint::MT: eTVA = SDRTEXTVERTADJUST_TOP;
                    eTHA = SDRTEXTHORZADJUST_CENTER; break;
        case RectPoint::MM: eTVA = SDRTEXTVERTADJUST_CENTER;
                    eTHA = SDRTEXTHORZADJUST_CENTER; break;
        case RectPoint::MB: eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    eTHA = SDRTEXTHORZADJUST_CENTER; break;
        case RectPoint::RT: eTVA = SDRTEXTVERTADJUST_TOP;
                    eTHA = SDRTEXTHORZADJUST_RIGHT; break;
        case RectPoint::RM: eTVA = SDRTEXTVERTADJUST_CENTER;
                    eTHA = SDRTEXTHORZADJUST_RIGHT; break;
        case RectPoint::RB: eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    eTHA = SDRTEXTHORZADJUST_RIGHT; break;
    }

    // #103516# Do not change values if adjust controls were disabled.
    bool bIsDisabled(m_aCtlPosition.IsCompletelyDisabled());

    if(!bIsDisabled)
    {
        if( m_xTsbFullWidth->get_state() == TRISTATE_TRUE )
        {
            if (IsTextDirectionLeftToRight())
                eTHA = SDRTEXTHORZADJUST_BLOCK;
            else
                eTVA = SDRTEXTVERTADJUST_BLOCK;
        }

        if ( rOutAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST ) != SfxItemState::DONTCARE )
        {
            eOldTVA = rOutAttrs.Get( SDRATTR_TEXT_VERTADJUST ).GetValue();
            if( eOldTVA != eTVA )
                rAttrs->Put( SdrTextVertAdjustItem( eTVA ) );
        }
        else
            rAttrs->Put( SdrTextVertAdjustItem( eTVA ) );

        if ( rOutAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST ) != SfxItemState::DONTCARE )
        {
            eOldTHA = rOutAttrs.Get( SDRATTR_TEXT_HORZADJUST ).GetValue();
            if( eOldTHA != eTHA )
                rAttrs->Put( SdrTextHorzAdjustItem( eTHA ) );
        }
        else
            rAttrs->Put( SdrTextHorzAdjustItem( eTHA ) );
    }

    return true;
}

void SvxTextAttrPage::Construct()
{
    switch (m_eObjKind)
    {
        case OBJ_NONE:
            // indeterminate, show them all
            bFitToSizeEnabled = bContourEnabled = bWordWrapTextEnabled =
            bAutoGrowSizeEnabled = bAutoGrowWidthEnabled = bAutoGrowHeightEnabled = true;
            m_xCustomShapeText->show();
            m_xDrawingText->show();
            break;
        case OBJ_TEXT:
        case OBJ_TITLETEXT:
        case OBJ_OUTLINETEXT:
        case OBJ_CAPTION:
            // contour NOT possible for pure text objects
            bContourEnabled = bWordWrapTextEnabled = bAutoGrowSizeEnabled = false;

            // adjusting width and height is ONLY possible for pure text objects
            bFitToSizeEnabled = bAutoGrowWidthEnabled = bAutoGrowHeightEnabled = true;
            m_xCustomShapeText->hide();
            m_xDrawingText->show();
            break;
        case OBJ_CUSTOMSHAPE:
            bFitToSizeEnabled = bContourEnabled = bAutoGrowWidthEnabled = bAutoGrowHeightEnabled = false;
            bWordWrapTextEnabled = bAutoGrowSizeEnabled = true;
            m_xDrawingText->hide();
            m_xCustomShapeText->show();
            break;
        default:
            bFitToSizeEnabled = bContourEnabled = true;
            bWordWrapTextEnabled = bAutoGrowSizeEnabled = bAutoGrowWidthEnabled = bAutoGrowHeightEnabled = false;
            m_xCustomShapeText->hide();
            m_xDrawingText->show();
            break;
    }

    m_xTsbAutoGrowHeight->set_visible( bAutoGrowHeightEnabled );
    m_xTsbAutoGrowWidth->set_visible( bAutoGrowWidthEnabled );
    m_xTsbFitToSize->set_visible( bFitToSizeEnabled );
    m_xTsbContour->set_visible( bContourEnabled );
    m_xTsbAutoGrowSize->set_visible( bAutoGrowSizeEnabled );
    m_xTsbWordWrapText->set_visible( bWordWrapTextEnabled );
}

VclPtr<SfxTabPage> SvxTextAttrPage::Create(TabPageParent pWindow, const SfxItemSet* rAttrs)
{
    return VclPtr<SvxTextAttrPage>::Create(pWindow, *rAttrs);
}

/** Check whether we have to uncheck the "Full width" check box.
*/
void SvxTextAttrPage::PointChanged(weld::DrawingArea*,  RectPoint eRP)
{
    if (m_xTsbFullWidth->get_state() == TRISTATE_TRUE)
    {
        // Depending on write direction and currently checked anchor we have
        // to uncheck the "full width" button.
        if (IsTextDirectionLeftToRight())
            switch( eRP )
            {
                case RectPoint::LT:
                case RectPoint::LM:
                case RectPoint::LB:
                case RectPoint::RT:
                case RectPoint::RM:
                case RectPoint::RB:
                    m_xTsbFullWidth->set_state( TRISTATE_FALSE );
                break;
                default: ;//prevent warning
            }
        else
            switch (eRP)
            {
                case RectPoint::LT:
                case RectPoint::MT:
                case RectPoint::RT:
                case RectPoint::LB:
                case RectPoint::MB:
                case RectPoint::RB:
                    m_xTsbFullWidth->set_state( TRISTATE_FALSE );
                break;
                default: ;//prevent warning
            }
    }
}


/*************************************************************************
|*
|* possibly changes the position of the position-control
|*
\************************************************************************/

/** When switching the "full width" check button on the text anchor may have
    to be moved to a valid and adjacent position.  This position depends on
    the current anchor position and the text writing direction.
*/
IMPL_LINK_NOARG(SvxTextAttrPage, ClickFullWidthHdl_Impl, weld::Button&, void)
{
    if( m_xTsbFullWidth->get_state() == TRISTATE_TRUE )
    {
        if (IsTextDirectionLeftToRight())
        {
            // Move text anchor to horizontal middle axis.
            switch( m_aCtlPosition.GetActualRP() )
            {
                case RectPoint::LT:
                case RectPoint::RT:
                    m_aCtlPosition.SetActualRP( RectPoint::MT );
                    break;

                case RectPoint::LM:
                case RectPoint::RM:
                    m_aCtlPosition.SetActualRP( RectPoint::MM );
                    break;

                case RectPoint::LB:
                case RectPoint::RB:
                    m_aCtlPosition.SetActualRP( RectPoint::MB );
                    break;
                default: ;//prevent warning
            }
        }
        else
        {
            // Move text anchor to vertical middle axis.
            switch( m_aCtlPosition.GetActualRP() )
            {
                case RectPoint::LT:
                case RectPoint::LB:
                    m_aCtlPosition.SetActualRP( RectPoint::LM );
                    break;

                case RectPoint::MT:
                case RectPoint::MB:
                    m_aCtlPosition.SetActualRP( RectPoint::MM );
                    break;

                case RectPoint::RT:
                case RectPoint::RB:
                    m_aCtlPosition.SetActualRP( RectPoint::RM );
                break;
                default: ;//prevent warning
            }
        }
    }
}

/*************************************************************************
|*
|* enables/disables "size at text" or "adjust to frame"
|*
\************************************************************************/

IMPL_LINK(SvxTextAttrPage, ClickHdl_Impl, weld::Button&, rButton, void)
{
    if (&rButton == m_xTsbAutoGrowSize.get())
    {
        m_xTsbAutoGrowHeight->set_state(m_xTsbAutoGrowSize->get_state());
        if (m_xTsbAutoGrowSize->get_state() == TRISTATE_TRUE)
        {
            m_xTsbFitToSize->set_state(TRISTATE_FALSE);
            m_xTsbContour->set_state(TRISTATE_FALSE);
        }
    }
    else if (&rButton == m_xTsbAutoGrowHeight.get())
        m_xTsbAutoGrowSize->set_state(m_xTsbAutoGrowHeight->get_state());

    bool bAutoGrowWidth  = m_xTsbAutoGrowWidth->get_state() == TRISTATE_TRUE;
    bool bAutoGrowHeight = m_xTsbAutoGrowHeight->get_state() == TRISTATE_TRUE;
    bool bFitToSize      = m_xTsbFitToSize->get_state() == TRISTATE_TRUE;
    bool bContour        = m_xTsbContour->get_state() == TRISTATE_TRUE;

    m_xTsbContour->set_sensitive( !bFitToSize &&
                        !( ( bAutoGrowWidth && bAutoGrowWidthEnabled ) || ( bAutoGrowHeight && bAutoGrowHeightEnabled ) ) &&
                        bContourEnabled );

    m_xTsbAutoGrowWidth->set_sensitive( !bFitToSize &&
                              !( bContour && bContourEnabled ) &&
                              bAutoGrowWidthEnabled );

    m_xTsbAutoGrowHeight->set_sensitive( !bFitToSize &&
                               !( bContour && bContourEnabled ) &&
                               bAutoGrowHeightEnabled );

    m_xTsbFitToSize->set_sensitive( !( ( bAutoGrowWidth && bAutoGrowWidthEnabled ) || ( bAutoGrowHeight && bAutoGrowHeightEnabled ) ) &&
                          !( bContour && bContourEnabled ) &&
                          bFitToSizeEnabled );

    // #101901# enable/disable metric fields and decorations dependent of contour
    m_xFlDistance->set_sensitive(!bContour);

    if( bContour && bContourEnabled )
    {
        m_xMtrFldLeft->set_value(0, FieldUnit::NONE);
        m_xMtrFldRight->set_value(0, FieldUnit::NONE);
        m_xMtrFldTop->set_value(0, FieldUnit::NONE);
        m_xMtrFldBottom->set_value(0, FieldUnit::NONE);
    }

    // #103516# Do the setup based on states of hor/ver adjust
    SfxItemState eVState = rOutAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
    SfxItemState eHState = rOutAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );
    bool bHorAndVer(SfxItemState::DONTCARE == eVState || SfxItemState::DONTCARE == eHState);

    // #83698# enable/disable text anchoring dependent of contour
    m_xFlPosition->set_sensitive(!bContour && !bHorAndVer);
}


bool SvxTextAttrPage::IsTextDirectionLeftToRight() const
{
    // Determine the text writing direction with left to right as default.
    bool bLeftToRightDirection = true;
    SfxItemState eState = rOutAttrs.GetItemState(SDRATTR_TEXTDIRECTION);

    if(SfxItemState::DONTCARE != eState)
    {
        const SvxWritingModeItem& rItem = rOutAttrs.Get(SDRATTR_TEXTDIRECTION);
        if (rItem.GetValue() == css::text::WritingMode_TB_RL)
            bLeftToRightDirection = false;
    }
    return bLeftToRightDirection;
}

void SvxTextAttrPage::PageCreated(const SfxAllItemSet& aSet)
{
    const CntUInt16Item* pObjTypeItem = aSet.GetItem<CntUInt16Item>(SID_SVXTEXTATTRPAGE_OBJKIND, false);

    if (pObjTypeItem)
        SetObjKind(static_cast<SdrObjKind>(pObjTypeItem->GetValue()));

    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
