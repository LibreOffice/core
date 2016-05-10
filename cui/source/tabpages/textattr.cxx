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
#include <cuires.hrc>
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

#include "textattr.hxx"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <sfx2/request.hxx>
#include <svx/ofaitem.hxx>
#include <editeng/writingmodeitem.hxx>

const sal_uInt16 SvxTextAttrPage::pRanges[] =
{
      SDRATTR_MISC_FIRST
    , SDRATTR_TEXT_HORZADJUST
    , SDRATTR_TEXT_WORDWRAP
    , SDRATTR_TEXT_AUTOGROWSIZE
    , 0
};

/*************************************************************************
|*
|* dialog (page) for copying objects
|*
\************************************************************************/
SvxTextAttrPage::SvxTextAttrPage(vcl::Window* pWindow, const SfxItemSet& rInAttrs)
    : SvxTabPage(pWindow,"TextAttributesPage","cui/ui/textattrtabpage.ui", rInAttrs)
    , rOutAttrs(rInAttrs)
    , pView(nullptr)
    , bAutoGrowSizeEnabled(false)
    , bContourEnabled(false)
    , bAutoGrowWidthEnabled(false)
    , bAutoGrowHeightEnabled(false)
    , bWordWrapTextEnabled(false)
    , bFitToSizeEnabled(false)
{
    get(m_pTsbAutoGrowWidth,"TSB_AUTOGROW_WIDTH");
    get(m_pTsbAutoGrowHeight,"TSB_AUTOGROW_HEIGHT");
    get(m_pTsbFitToSize,"TSB_FIT_TO_SIZE");
    get(m_pTsbContour,"TSB_CONTOUR");
    get(m_pTsbWordWrapText,"TSB_WORDWRAP_TEXT");
    get(m_pTsbAutoGrowSize,"TSB_AUTOGROW_SIZE");
    get(m_pFlDistance,"FL_DISTANCE");
    get(m_pMtrFldLeft,"MTR_FLD_LEFT");
    get(m_pMtrFldRight,"MTR_FLD_RIGHT");
    get(m_pMtrFldTop,"MTR_FLD_TOP");
    get(m_pMtrFldBottom,"MTR_FLD_BOTTOM");
    get(m_pFlPosition,"FL_POSITION");
    get(m_pCtlPosition,"CTL_POSITION");
    m_pCtlPosition->SetControlSettings(RP_MM, 240, 100),
    get(m_pTsbFullWidth,"TSB_FULL_WIDTH");


    FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit( *m_pMtrFldLeft, eFUnit );
    SetFieldUnit( *m_pMtrFldRight, eFUnit );
    SetFieldUnit( *m_pMtrFldTop, eFUnit );
    SetFieldUnit( *m_pMtrFldBottom, eFUnit );

    Link<Button*,void> aLink( LINK( this, SvxTextAttrPage, ClickHdl_Impl ) );
    m_pTsbAutoGrowWidth->SetClickHdl( aLink );
    m_pTsbAutoGrowHeight->SetClickHdl( aLink );
    m_pTsbFitToSize->SetClickHdl( aLink );
    m_pTsbContour->SetClickHdl( aLink );

    m_pTsbFullWidth->SetClickHdl(LINK( this, SvxTextAttrPage, ClickFullWidthHdl_Impl ) );
}

SvxTextAttrPage::~SvxTextAttrPage()
{
    disposeOnce();
}

void SvxTextAttrPage::dispose()
{
    m_pTsbAutoGrowWidth.clear();
    m_pTsbAutoGrowHeight.clear();
    m_pTsbFitToSize.clear();
    m_pTsbContour.clear();
    m_pTsbWordWrapText.clear();
    m_pTsbAutoGrowSize.clear();
    m_pFlDistance.clear();
    m_pMtrFldLeft.clear();
    m_pMtrFldRight.clear();
    m_pMtrFldTop.clear();
    m_pMtrFldBottom.clear();
    m_pFlPosition.clear();
    m_pCtlPosition.clear();
    m_pTsbFullWidth.clear();
    SvxTabPage::dispose();
}

/*************************************************************************
|*
|* reads the passed item set
|*
\************************************************************************/

void SvxTextAttrPage::Reset( const SfxItemSet* rAttrs )
{
    SfxItemPool* pPool = rAttrs->GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    SfxMapUnit eUnit = pPool->GetMetric( SDRATTR_TEXT_LEFTDIST );

    const SfxPoolItem* pItem = GetItem( *rAttrs, SDRATTR_TEXT_LEFTDIST );

    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_LEFTDIST );
    if( pItem )
    {
        long nValue = static_cast<const SdrMetricItem*>( pItem )->GetValue();
        SetMetricValue( *m_pMtrFldLeft, nValue, eUnit );
    }
    else
        m_pMtrFldLeft->SetText( "" );
    m_pMtrFldLeft->SaveValue();

    pItem = GetItem( *rAttrs, SDRATTR_TEXT_RIGHTDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_RIGHTDIST );
    if( pItem )
    {
        long nValue = static_cast<const SdrMetricItem*>( pItem )->GetValue();
        SetMetricValue( *m_pMtrFldRight, nValue, eUnit );
    }
    else
        m_pMtrFldRight->SetText( "" );
    m_pMtrFldRight->SaveValue();

    pItem = GetItem( *rAttrs, SDRATTR_TEXT_UPPERDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_UPPERDIST );
    if( pItem )
    {
        long nValue = static_cast<const SdrMetricItem*>( pItem )->GetValue();
        SetMetricValue( *m_pMtrFldTop, nValue, eUnit );
    }
    else
        m_pMtrFldTop->SetText( "" );
    m_pMtrFldTop->SaveValue();

    pItem = GetItem( *rAttrs, SDRATTR_TEXT_LOWERDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_LOWERDIST );
    if( pItem )
    {
        long nValue = static_cast<const SdrMetricItem*>(pItem)->GetValue();
        SetMetricValue( *m_pMtrFldBottom, nValue, eUnit );
    }
    else
        m_pMtrFldBottom->SetText( "" );
    m_pMtrFldBottom->SaveValue();

    // adjust to height
    if ( rAttrs->GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) != SfxItemState::DONTCARE )
    {
        m_pTsbAutoGrowHeight->SetState( static_cast<const SdrOnOffItem&>( rAttrs->Get( SDRATTR_TEXT_AUTOGROWHEIGHT ) ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pTsbAutoGrowHeight->EnableTriState( false );
    }
    else
        m_pTsbAutoGrowHeight->SetState( TRISTATE_INDET );
    m_pTsbAutoGrowHeight->SaveValue();

    // adjust to width
    if ( rAttrs->GetItemState( SDRATTR_TEXT_AUTOGROWWIDTH ) != SfxItemState::DONTCARE )
    {
        m_pTsbAutoGrowWidth->SetState( static_cast<const SdrOnOffItem&>( rAttrs->Get( SDRATTR_TEXT_AUTOGROWWIDTH ) ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pTsbAutoGrowWidth->EnableTriState( false );
    }
    else
        m_pTsbAutoGrowWidth->SetState( TRISTATE_INDET );
    m_pTsbAutoGrowWidth->SaveValue();

    // autogrowsize
    if ( rAttrs->GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) != SfxItemState::DONTCARE )
    {
        m_pTsbAutoGrowSize->SetState( static_cast<const SdrOnOffItem&>( rAttrs->Get( SDRATTR_TEXT_AUTOGROWHEIGHT ) ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pTsbAutoGrowSize->EnableTriState( false );
    }
    else
        m_pTsbAutoGrowSize->SetState( TRISTATE_INDET );
    m_pTsbAutoGrowSize->SaveValue();

    // wordwrap text
    if ( rAttrs->GetItemState( SDRATTR_TEXT_WORDWRAP ) != SfxItemState::DONTCARE )
    {
        m_pTsbWordWrapText->SetState( static_cast<const SdrOnOffItem&>( rAttrs->Get( SDRATTR_TEXT_WORDWRAP ) ).
                        GetValue() ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pTsbWordWrapText->EnableTriState( false );
    }
    else
        m_pTsbWordWrapText->SetState( TRISTATE_INDET );
    m_pTsbWordWrapText->SaveValue();


    // #103516# Do the setup based on states of hor/ver adjust
    // Setup center field and FullWidth
    SfxItemState eVState = rAttrs->GetItemState( SDRATTR_TEXT_VERTADJUST );
    SfxItemState eHState = rAttrs->GetItemState( SDRATTR_TEXT_HORZADJUST );

    if(SfxItemState::DONTCARE != eVState && SfxItemState::DONTCARE != eHState)
    {
        // VertAdjust and HorAdjust are unequivocal, thus
        SdrTextVertAdjust eTVA = (SdrTextVertAdjust)static_cast<const SdrTextVertAdjustItem&>(rAttrs->Get(SDRATTR_TEXT_VERTADJUST)).GetValue();
        SdrTextHorzAdjust eTHA = (SdrTextHorzAdjust)static_cast<const SdrTextHorzAdjustItem&>(rAttrs->Get(SDRATTR_TEXT_HORZADJUST)).GetValue();
        RECT_POINT eRP = RP_LB;

        m_pTsbFullWidth->EnableTriState( false );

        // Translate item values into local anchor position.
        switch (eTVA)
        {
            case SDRTEXTVERTADJUST_TOP:
            {
                switch (eTHA)
                {
                    case SDRTEXTHORZADJUST_LEFT: eRP = RP_LT; break;
                    case SDRTEXTHORZADJUST_BLOCK:
                    case SDRTEXTHORZADJUST_CENTER: eRP = RP_MT; break;
                    case SDRTEXTHORZADJUST_RIGHT: eRP = RP_RT; break;
                }
                break;
            }
            case SDRTEXTVERTADJUST_BLOCK:
            case SDRTEXTVERTADJUST_CENTER:
            {
                switch (eTHA)
                {
                    case SDRTEXTHORZADJUST_LEFT: eRP = RP_LM; break;
                    case SDRTEXTHORZADJUST_BLOCK:
                    case SDRTEXTHORZADJUST_CENTER: eRP = RP_MM; break;
                    case SDRTEXTHORZADJUST_RIGHT: eRP = RP_RM; break;
                }
                break;
            }
            case SDRTEXTVERTADJUST_BOTTOM:
            {
                switch (eTHA)
                {
                    case SDRTEXTHORZADJUST_LEFT: eRP = RP_LB; break;
                    case SDRTEXTHORZADJUST_BLOCK:
                    case SDRTEXTHORZADJUST_CENTER: eRP = RP_MB; break;
                    case SDRTEXTHORZADJUST_RIGHT: eRP = RP_RB; break;
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
            ClickFullWidthHdl_Impl(nullptr);
            m_pTsbFullWidth->SetState(TRISTATE_TRUE);
        }

        m_pCtlPosition->SetActualRP( eRP );
    }
    else
    {
        // VertAdjust or HorAdjust is not unequivocal
        m_pCtlPosition->Reset();

        m_pCtlPosition->SetState(CTL_STATE::NOVERT);
        m_pCtlPosition->DoCompletelyDisable(true);

        m_pTsbFullWidth->SetState(TRISTATE_INDET);
        m_pFlPosition->Enable( false );
    }

    // adjust to border
    if ( rAttrs->GetItemState( SDRATTR_TEXT_FITTOSIZE ) != SfxItemState::DONTCARE )
    {
        SdrFitToSizeType eFTS = (SdrFitToSizeType)
                    static_cast<const SdrTextFitToSizeTypeItem&>( rAttrs->Get( SDRATTR_TEXT_FITTOSIZE ) ).GetValue();
        m_pTsbFitToSize->SetState( eFTS == SDRTEXTFIT_NONE ? TRISTATE_FALSE : TRISTATE_TRUE );
        m_pTsbFitToSize->EnableTriState( false );
    }
    else
        m_pTsbFitToSize->SetState( TRISTATE_INDET );
    m_pTsbFitToSize->SaveValue();

    if( rAttrs->GetItemState( SDRATTR_TEXT_CONTOURFRAME ) != SfxItemState::DONTCARE )
    {
        bool bContour = static_cast<const SdrOnOffItem&>( rAttrs->Get( SDRATTR_TEXT_CONTOURFRAME ) ).GetValue();
        m_pTsbContour->SetState( bContour ? TRISTATE_TRUE : TRISTATE_FALSE );
        m_pTsbContour->EnableTriState( false );
    }
    else
        m_pTsbContour->SetState( TRISTATE_INDET );
    m_pTsbContour->SaveValue();

    ClickHdl_Impl( nullptr );
}

/*************************************************************************
|*
|* fills the passed item set with dialog box attributes
|*
\************************************************************************/

bool SvxTextAttrPage::FillItemSet( SfxItemSet* rAttrs)
{
    SfxItemPool* pPool = rAttrs->GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    SfxMapUnit eUnit = pPool->GetMetric( SDRATTR_TEXT_LEFTDIST );

    sal_Int32    nValue;
    TriState eState;

    if( m_pMtrFldLeft->IsValueChangedFromSaved() )
    {
        nValue = GetCoreValue( *m_pMtrFldLeft, eUnit );
        rAttrs->Put( makeSdrTextLeftDistItem( nValue ) );
    }

    if( m_pMtrFldRight->IsValueChangedFromSaved() )
    {
        nValue = GetCoreValue( *m_pMtrFldRight, eUnit );
        rAttrs->Put( makeSdrTextRightDistItem( nValue ) );
    }

    if( m_pMtrFldTop->IsValueChangedFromSaved() )
    {
        nValue = GetCoreValue( *m_pMtrFldTop, eUnit );
        rAttrs->Put( makeSdrTextUpperDistItem( nValue ) );
    }

    if( m_pMtrFldBottom->IsValueChangedFromSaved() )
    {
        nValue = GetCoreValue( *m_pMtrFldBottom, eUnit );
        rAttrs->Put( makeSdrTextLowerDistItem( nValue ) );
    }

    eState = m_pTsbAutoGrowHeight->GetState();
    if( m_pTsbAutoGrowHeight->IsValueChangedFromSaved() )
    {
        rAttrs->Put( makeSdrTextAutoGrowHeightItem( TRISTATE_TRUE == eState ) );
    }

    eState = m_pTsbAutoGrowWidth->GetState();
    if( m_pTsbAutoGrowWidth->IsValueChangedFromSaved() )
    {
        rAttrs->Put( makeSdrTextAutoGrowWidthItem( TRISTATE_TRUE == eState ) );
    }

    eState = m_pTsbAutoGrowSize->GetState();
    if( m_pTsbAutoGrowSize->IsValueChangedFromSaved() )
    {
        rAttrs->Put( makeSdrTextAutoGrowHeightItem( TRISTATE_TRUE == eState ) );
    }

    eState = m_pTsbWordWrapText->GetState();
    if( m_pTsbWordWrapText->IsValueChangedFromSaved() )
    {
        rAttrs->Put( makeSdrTextWordWrapItem( TRISTATE_TRUE == eState ) );
    }

    eState = m_pTsbContour->GetState();
    if( m_pTsbContour->IsValueChangedFromSaved() )
    {
        rAttrs->Put( makeSdrTextContourFrameItem( TRISTATE_TRUE == eState ) );
    }

    eState = m_pTsbFitToSize->GetState();
    if( m_pTsbFitToSize->IsValueChangedFromSaved() )
    {
        SdrFitToSizeType eFTS;
        switch( eState )
        {
            default: ; //prevent warning
                OSL_FAIL( "svx::SvxTextAttrPage::FillItemSet(), unhandled state!" );
                SAL_FALLTHROUGH;
            case TRISTATE_FALSE: eFTS = SDRTEXTFIT_NONE; break;
            case TRISTATE_TRUE: eFTS = SDRTEXTFIT_AUTOFIT; break;
        }
        rAttrs->Put( SdrTextFitToSizeTypeItem( eFTS ) );
    }

    // centered
    RECT_POINT eRP = m_pCtlPosition->GetActualRP();
    SdrTextVertAdjust eTVA, eOldTVA;
    SdrTextHorzAdjust eTHA, eOldTHA;

    switch( eRP )
    {
        default:
        case RP_LT: eTVA = SDRTEXTVERTADJUST_TOP;
                    eTHA = SDRTEXTHORZADJUST_LEFT; break;
        case RP_LM: eTVA = SDRTEXTVERTADJUST_CENTER;
                    eTHA = SDRTEXTHORZADJUST_LEFT; break;
        case RP_LB: eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    eTHA = SDRTEXTHORZADJUST_LEFT; break;
        case RP_MT: eTVA = SDRTEXTVERTADJUST_TOP;
                    eTHA = SDRTEXTHORZADJUST_CENTER; break;
        case RP_MM: eTVA = SDRTEXTVERTADJUST_CENTER;
                    eTHA = SDRTEXTHORZADJUST_CENTER; break;
        case RP_MB: eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    eTHA = SDRTEXTHORZADJUST_CENTER; break;
        case RP_RT: eTVA = SDRTEXTVERTADJUST_TOP;
                    eTHA = SDRTEXTHORZADJUST_RIGHT; break;
        case RP_RM: eTVA = SDRTEXTVERTADJUST_CENTER;
                    eTHA = SDRTEXTHORZADJUST_RIGHT; break;
        case RP_RB: eTVA = SDRTEXTVERTADJUST_BOTTOM;
                    eTHA = SDRTEXTHORZADJUST_RIGHT; break;
    }

    // #103516# Do not change values if adjust controls were disabled.
    bool bIsDisabled(m_pCtlPosition->IsCompletelyDisabled());

    if(!bIsDisabled)
    {
        if( m_pTsbFullWidth->GetState() == TRISTATE_TRUE )
        {
            if (IsTextDirectionLeftToRight())
                eTHA = SDRTEXTHORZADJUST_BLOCK;
            else
                eTVA = SDRTEXTVERTADJUST_BLOCK;
        }

        if ( rOutAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST ) != SfxItemState::DONTCARE )
        {
            eOldTVA = (SdrTextVertAdjust)
                        static_cast<const SdrTextVertAdjustItem&>( rOutAttrs.Get( SDRATTR_TEXT_VERTADJUST ) ).GetValue();
            if( eOldTVA != eTVA )
                rAttrs->Put( SdrTextVertAdjustItem( eTVA ) );
        }
        else
            rAttrs->Put( SdrTextVertAdjustItem( eTVA ) );

        if ( rOutAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST ) != SfxItemState::DONTCARE )
        {
            eOldTHA = (SdrTextHorzAdjust)
                        static_cast<const SdrTextHorzAdjustItem&>( rOutAttrs.Get( SDRATTR_TEXT_HORZADJUST ) ).GetValue();
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
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );

    bFitToSizeEnabled = bContourEnabled = true;
    bWordWrapTextEnabled = bAutoGrowSizeEnabled = bAutoGrowWidthEnabled = bAutoGrowHeightEnabled = false;

    const SdrMarkList& rMarkList = pView->GetMarkedObjectList();
    if( rMarkList.GetMarkCount() == 1 )
    {
        const SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
        SdrObjKind eKind = (SdrObjKind) pObj->GetObjIdentifier();
        if( pObj->GetObjInventor() == SdrInventor )
        {
            switch( eKind )
            {
                case OBJ_TEXT :
                case OBJ_TITLETEXT :
                case OBJ_OUTLINETEXT :
                case OBJ_CAPTION :
                {
                    if(pObj->HasText())
                    {
                        // contour NOT possible for pure text objects
                        bContourEnabled = false;

                        // adjusting width and height is ONLY possible for pure text objects
                        bAutoGrowWidthEnabled = bAutoGrowHeightEnabled = true;
                    }
                }
                break;
                case OBJ_CUSTOMSHAPE :
                {
                    bFitToSizeEnabled = bContourEnabled = false;
                    bAutoGrowSizeEnabled = true;
                    bWordWrapTextEnabled = true;
                }
                break;
                default: ;//prevent warning
            }
        }
    }
    m_pTsbAutoGrowHeight->Enable( bAutoGrowHeightEnabled );
    m_pTsbAutoGrowWidth->Enable( bAutoGrowWidthEnabled );
    m_pTsbFitToSize->Enable( bFitToSizeEnabled );
    m_pTsbContour->Enable( bContourEnabled );
    m_pTsbAutoGrowSize->Enable( bAutoGrowSizeEnabled );
    m_pTsbWordWrapText->Enable( bWordWrapTextEnabled );
}

VclPtr<SfxTabPage> SvxTextAttrPage::Create( vcl::Window* pWindow,
                                            const SfxItemSet* rAttrs )
{
    return VclPtr<SvxTextAttrPage>::Create( pWindow, *rAttrs );
}

/** Check whether we have to uncheck the "Full width" check box.
*/
void SvxTextAttrPage::PointChanged( vcl::Window*, RECT_POINT eRP )
{
    if (m_pTsbFullWidth->GetState() == TRISTATE_TRUE)
    {
        // Depending on write direction and currently checked anchor we have
        // to uncheck the "full width" button.
        if (IsTextDirectionLeftToRight())
            switch( eRP )
            {
                case RP_LT:
                case RP_LM:
                case RP_LB:
                case RP_RT:
                case RP_RM:
                case RP_RB:
                    m_pTsbFullWidth->SetState( TRISTATE_FALSE );
                break;
                default: ;//prevent warning
            }
        else
            switch (eRP)
            {
                case RP_LT:
                case RP_MT:
                case RP_RT:
                case RP_LB:
                case RP_MB:
                case RP_RB:
                    m_pTsbFullWidth->SetState( TRISTATE_FALSE );
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
IMPL_LINK_NOARG_TYPED(SvxTextAttrPage, ClickFullWidthHdl_Impl, Button*, void)
{
    if( m_pTsbFullWidth->GetState() == TRISTATE_TRUE )
    {
        if (IsTextDirectionLeftToRight())
        {
            // Move text anchor to horizontal middle axis.
            switch( m_pCtlPosition->GetActualRP() )
            {
                case RP_LT:
                case RP_RT:
                    m_pCtlPosition->SetActualRP( RP_MT );
                    break;

                case RP_LM:
                case RP_RM:
                    m_pCtlPosition->SetActualRP( RP_MM );
                    break;

                case RP_LB:
                case RP_RB:
                    m_pCtlPosition->SetActualRP( RP_MB );
                    break;
                default: ;//prevent warning
            }
        }
        else
        {
            // Move text anchor to vertical middle axis.
            switch( m_pCtlPosition->GetActualRP() )
            {
                case RP_LT:
                case RP_LB:
                    m_pCtlPosition->SetActualRP( RP_LM );
                    break;

                case RP_MT:
                case RP_MB:
                    m_pCtlPosition->SetActualRP( RP_MM );
                    break;

                case RP_RT:
                case RP_RB:
                    m_pCtlPosition->SetActualRP( RP_RM );
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

IMPL_LINK_NOARG_TYPED(SvxTextAttrPage, ClickHdl_Impl, Button*, void)
{
    bool bAutoGrowWidth  = m_pTsbAutoGrowWidth->GetState() == TRISTATE_TRUE;
    bool bAutoGrowHeight = m_pTsbAutoGrowHeight->GetState() == TRISTATE_TRUE;
    bool bFitToSize      = m_pTsbFitToSize->GetState() == TRISTATE_TRUE;
    bool bContour        = m_pTsbContour->GetState() == TRISTATE_TRUE;

    m_pTsbContour->Enable( !bFitToSize &&
                        !( ( bAutoGrowWidth && bAutoGrowWidthEnabled ) || ( bAutoGrowHeight && bAutoGrowHeightEnabled ) ) &&
                        bContourEnabled );

    m_pTsbAutoGrowWidth->Enable( !bFitToSize &&
                              !( bContour && bContourEnabled ) &&
                              bAutoGrowWidthEnabled );

    m_pTsbAutoGrowHeight->Enable( !bFitToSize &&
                               !( bContour && bContourEnabled ) &&
                               bAutoGrowHeightEnabled );

    m_pTsbFitToSize->Enable( !( ( bAutoGrowWidth && bAutoGrowWidthEnabled ) || ( bAutoGrowHeight && bAutoGrowHeightEnabled ) ) &&
                          !( bContour && bContourEnabled ) &&
                          bFitToSizeEnabled );

    // #101901# enable/disable metric fields and decorations dependent of contour
    m_pFlDistance->Enable(!bContour);

    if( bContour && bContourEnabled )
    {
        m_pMtrFldLeft->SetValue( 0 );
        m_pMtrFldRight->SetValue( 0 );
        m_pMtrFldTop->SetValue( 0 );
        m_pMtrFldBottom->SetValue( 0 );
    }

    // #103516# Do the setup based on states of hor/ver adjust
    SfxItemState eVState = rOutAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
    SfxItemState eHState = rOutAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );
    bool bHorAndVer(SfxItemState::DONTCARE == eVState || SfxItemState::DONTCARE == eHState);

    // #83698# enable/disable text anchoring dependent of contour
    m_pFlPosition->Enable(!bContour && !bHorAndVer);
}


bool SvxTextAttrPage::IsTextDirectionLeftToRight() const
{
    // Determine the text writing direction with left to right as default.
    bool bLeftToRightDirection = true;
    SfxItemState eState = rOutAttrs.GetItemState(SDRATTR_TEXTDIRECTION);

    if(SfxItemState::DONTCARE != eState)
    {
        const SvxWritingModeItem& rItem = static_cast<const SvxWritingModeItem&> (
            rOutAttrs.Get (SDRATTR_TEXTDIRECTION));
        if (rItem.GetValue() == css::text::WritingMode_TB_RL)
            bLeftToRightDirection = false;
    }
    return bLeftToRightDirection;
}

void SvxTextAttrPage::PageCreated(const SfxAllItemSet& aSet)
{
    const OfaPtrItem* pViewItem = aSet.GetItem<OfaPtrItem>(SID_SVXTEXTATTRPAGE_VIEW, false);

    if (pViewItem)
        SetView( static_cast<SdrView *>(pViewItem->GetValue()));

    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
