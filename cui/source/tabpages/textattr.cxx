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
#include <tools/shl.hxx>
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

static sal_uInt16 pRanges[] =
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

SvxTextAttrPage::SvxTextAttrPage( Window* pWindow, const SfxItemSet& rInAttrs ) :
                SvxTabPage      ( pWindow
                                ,"TextAttributesPage"
                                ,"cui/ui/textattrtabpage.ui"
                                , rInAttrs ),
                rOutAttrs( rInAttrs )
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

    Link aLink( LINK( this, SvxTextAttrPage, ClickHdl_Impl ) );
    m_pTsbAutoGrowWidth->SetClickHdl( aLink );
    m_pTsbAutoGrowHeight->SetClickHdl( aLink );
    m_pTsbFitToSize->SetClickHdl( aLink );
    m_pTsbContour->SetClickHdl( aLink );

    m_pTsbFullWidth->SetClickHdl(LINK( this, SvxTextAttrPage, ClickFullWidthHdl_Impl ) );
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SvxTextAttrPage::~SvxTextAttrPage()
{
}

/*************************************************************************
|*
|* reads the passed item set
|*
\************************************************************************/

void SvxTextAttrPage::Reset( const SfxItemSet& rAttrs )
{
    SfxItemPool* pPool = rAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    SfxMapUnit eUnit = pPool->GetMetric( SDRATTR_TEXT_LEFTDIST );

    const SfxPoolItem* pItem = GetItem( rAttrs, SDRATTR_TEXT_LEFTDIST );

    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_LEFTDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrTextLeftDistItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldLeft, nValue, eUnit );
    }
    else
        m_pMtrFldLeft->SetText( "" );
    m_pMtrFldLeft->SaveValue();

    pItem = GetItem( rAttrs, SDRATTR_TEXT_RIGHTDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_RIGHTDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrTextRightDistItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldRight, nValue, eUnit );
    }
    else
        m_pMtrFldRight->SetText( "" );
    m_pMtrFldRight->SaveValue();

    pItem = GetItem( rAttrs, SDRATTR_TEXT_UPPERDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_UPPERDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrTextUpperDistItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldTop, nValue, eUnit );
    }
    else
        m_pMtrFldTop->SetText( "" );
    m_pMtrFldTop->SaveValue();

    pItem = GetItem( rAttrs, SDRATTR_TEXT_LOWERDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_LOWERDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrTextLowerDistItem* )pItem )->GetValue();
        SetMetricValue( *m_pMtrFldBottom, nValue, eUnit );
    }
    else
        m_pMtrFldBottom->SetText( "" );
    m_pMtrFldBottom->SaveValue();

    // adjust to height
    if ( rAttrs.GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbAutoGrowHeight->SetState( ( ( const SdrTextAutoGrowHeightItem& )rAttrs.Get( SDRATTR_TEXT_AUTOGROWHEIGHT ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        m_pTsbAutoGrowHeight->EnableTriState( sal_False );
    }
    else
        m_pTsbAutoGrowHeight->SetState( STATE_DONTKNOW );
    m_pTsbAutoGrowHeight->SaveValue();

    // adjust to width
    if ( rAttrs.GetItemState( SDRATTR_TEXT_AUTOGROWWIDTH ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbAutoGrowWidth->SetState( ( ( const SdrTextAutoGrowWidthItem& )rAttrs.Get( SDRATTR_TEXT_AUTOGROWWIDTH ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        m_pTsbAutoGrowWidth->EnableTriState( sal_False );
    }
    else
        m_pTsbAutoGrowWidth->SetState( STATE_DONTKNOW );
    m_pTsbAutoGrowWidth->SaveValue();

    // autogrowsize
    if ( rAttrs.GetItemState( SDRATTR_TEXT_AUTOGROWSIZE ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbAutoGrowSize->SetState( ( ( const SdrTextAutoGrowHeightItem& )rAttrs.Get( SDRATTR_TEXT_AUTOGROWHEIGHT ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        m_pTsbAutoGrowSize->EnableTriState( sal_False );
    }
    else
        m_pTsbAutoGrowSize->SetState( STATE_DONTKNOW );
    m_pTsbAutoGrowSize->SaveValue();

    // wordwrap text
    if ( rAttrs.GetItemState( SDRATTR_TEXT_WORDWRAP ) != SFX_ITEM_DONTCARE )
    {
        m_pTsbWordWrapText->SetState( ( ( const SdrTextWordWrapItem& )rAttrs.Get( SDRATTR_TEXT_WORDWRAP ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        m_pTsbWordWrapText->EnableTriState( sal_False );
    }
    else
        m_pTsbWordWrapText->SetState( STATE_DONTKNOW );
    m_pTsbWordWrapText->SaveValue();


    // #103516# Do the setup based on states of hor/ver adjust
    // Setup center field and FullWidth
    SfxItemState eVState = rAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
    SfxItemState eHState = rAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );

    if(SFX_ITEM_DONTCARE != eVState && SFX_ITEM_DONTCARE != eHState)
    {
        // VertAdjust and HorAdjust are unequivocal, thus
        SdrTextVertAdjust eTVA = (SdrTextVertAdjust)((const SdrTextVertAdjustItem&)rAttrs.Get(SDRATTR_TEXT_VERTADJUST)).GetValue();
        SdrTextHorzAdjust eTHA = (SdrTextHorzAdjust)((const SdrTextHorzAdjustItem&)rAttrs.Get(SDRATTR_TEXT_HORZADJUST)).GetValue();
        RECT_POINT eRP = RP_LB;

        m_pTsbFullWidth->EnableTriState( sal_False );

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
        sal_Bool bLeftToRight(IsTextDirectionLeftToRight());

        if((bLeftToRight && (SDRTEXTHORZADJUST_BLOCK == eTHA)) || (!bLeftToRight && (SDRTEXTVERTADJUST_BLOCK == eTVA)))
        {
            // Move anchor to valid position.
            ClickFullWidthHdl_Impl(NULL);
            m_pTsbFullWidth->SetState(STATE_CHECK);
        }

        m_pCtlPosition->SetActualRP( eRP );
    }
    else
    {
        // VertAdjust or HorAdjust is not unequivocal
        m_pCtlPosition->Reset();

        m_pCtlPosition->SetState(STATE_DONTKNOW);
        m_pCtlPosition->DoCompletelyDisable(sal_True);

        m_pTsbFullWidth->SetState(STATE_DONTKNOW);
        m_pFlPosition->Enable( sal_False );
    }

    // adjust to border
    if ( rAttrs.GetItemState( SDRATTR_TEXT_FITTOSIZE ) != SFX_ITEM_DONTCARE )
    {
        SdrFitToSizeType eFTS = (SdrFitToSizeType)
                    ( ( const SdrTextFitToSizeTypeItem& )rAttrs.Get( SDRATTR_TEXT_FITTOSIZE ) ).GetValue();
        m_pTsbFitToSize->SetState( eFTS == SDRTEXTFIT_NONE ? STATE_NOCHECK : STATE_CHECK );
        m_pTsbFitToSize->EnableTriState( sal_False );
    }
    else
        m_pTsbFitToSize->SetState( STATE_DONTKNOW );
    m_pTsbFitToSize->SaveValue();

    if( rAttrs.GetItemState( SDRATTR_TEXT_CONTOURFRAME ) != SFX_ITEM_DONTCARE )
    {
        sal_Bool bContour = ( ( const SdrTextContourFrameItem& )rAttrs.Get( SDRATTR_TEXT_CONTOURFRAME ) ).GetValue();
        m_pTsbContour->SetState( bContour ? STATE_CHECK : STATE_NOCHECK );
        m_pTsbContour->EnableTriState( sal_False );
    }
    else
        m_pTsbContour->SetState( STATE_DONTKNOW );
    m_pTsbContour->SaveValue();

    ClickHdl_Impl( NULL );
}

/*************************************************************************
|*
|* fills the passed item set with dialog box attributes
|*
\************************************************************************/

sal_Bool SvxTextAttrPage::FillItemSet( SfxItemSet& rAttrs)
{
    SfxItemPool* pPool = rAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    SfxMapUnit eUnit = pPool->GetMetric( SDRATTR_TEXT_LEFTDIST );

    sal_Int32    nValue;
    TriState eState;

    if( m_pMtrFldLeft->GetText() != m_pMtrFldLeft->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldLeft, eUnit );
        rAttrs.Put( SdrTextLeftDistItem( nValue ) );
    }

    if( m_pMtrFldRight->GetText() != m_pMtrFldRight->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldRight, eUnit );
        rAttrs.Put( SdrTextRightDistItem( nValue ) );
    }

    if( m_pMtrFldTop->GetText() != m_pMtrFldTop->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldTop, eUnit );
        rAttrs.Put( SdrTextUpperDistItem( nValue ) );
    }

    if( m_pMtrFldBottom->GetText() != m_pMtrFldBottom->GetSavedValue() )
    {
        nValue = GetCoreValue( *m_pMtrFldBottom, eUnit );
        rAttrs.Put( SdrTextLowerDistItem( nValue ) );
    }

    eState = m_pTsbAutoGrowHeight->GetState();
    if( eState != m_pTsbAutoGrowHeight->GetSavedValue() )
    {
        rAttrs.Put( SdrTextAutoGrowHeightItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    eState = m_pTsbAutoGrowWidth->GetState();
    if( eState != m_pTsbAutoGrowWidth->GetSavedValue() )
    {
        rAttrs.Put( SdrTextAutoGrowWidthItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    eState = m_pTsbAutoGrowSize->GetState();
    if( eState != m_pTsbAutoGrowSize->GetSavedValue() )
    {
        rAttrs.Put( SdrTextAutoGrowHeightItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    eState = m_pTsbWordWrapText->GetState();
    if( eState != m_pTsbWordWrapText->GetSavedValue() )
    {
        rAttrs.Put( SdrTextWordWrapItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    eState = m_pTsbContour->GetState();
    if( eState != m_pTsbContour->GetSavedValue() )
    {
        rAttrs.Put( SdrTextContourFrameItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    eState = m_pTsbFitToSize->GetState();
    if( eState != m_pTsbFitToSize->GetSavedValue() )
    {
        SdrFitToSizeType eFTS;
        switch( eState )
        {
            default: ; //prevent warning
                OSL_FAIL( "svx::SvxTextAttrPage::FillItemSet(), unhandled state!" );
            case STATE_NOCHECK: eFTS = SDRTEXTFIT_NONE; break;
            case STATE_CHECK: eFTS = SDRTEXTFIT_AUTOFIT; break;
        }
        rAttrs.Put( SdrTextFitToSizeTypeItem( eFTS ) );
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
    sal_Bool bIsDisabled(m_pCtlPosition->IsCompletelyDisabled());

    if(!bIsDisabled)
    {
        if( m_pTsbFullWidth->GetState() == STATE_CHECK )
        {
            if (IsTextDirectionLeftToRight())
                eTHA = SDRTEXTHORZADJUST_BLOCK;
            else
                eTVA = SDRTEXTVERTADJUST_BLOCK;
        }

        if ( rOutAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST ) != SFX_ITEM_DONTCARE )
        {
            eOldTVA = (SdrTextVertAdjust)
                        ( ( const SdrTextVertAdjustItem& )rOutAttrs.Get( SDRATTR_TEXT_VERTADJUST ) ).GetValue();
            if( eOldTVA != eTVA )
                rAttrs.Put( SdrTextVertAdjustItem( eTVA ) );
        }
        else
            rAttrs.Put( SdrTextVertAdjustItem( eTVA ) );

        if ( rOutAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST ) != SFX_ITEM_DONTCARE )
        {
            eOldTHA = (SdrTextHorzAdjust)
                        ( ( const SdrTextHorzAdjustItem& )rOutAttrs.Get( SDRATTR_TEXT_HORZADJUST ) ).GetValue();
            if( eOldTHA != eTHA )
                rAttrs.Put( SdrTextHorzAdjustItem( eTHA ) );
        }
        else
            rAttrs.Put( SdrTextHorzAdjustItem( eTHA ) );
    }

    return( sal_True );
}

void SvxTextAttrPage::Construct()
{
    DBG_ASSERT( pView, "Keine gueltige View Uebergeben!" );

    bFitToSizeEnabled = bContourEnabled = sal_True;
    bWordWrapTextEnabled = bAutoGrowSizeEnabled = bAutoGrowWidthEnabled = bAutoGrowHeightEnabled = sal_False;

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
                        bContourEnabled = sal_False;

                        // adjusting width and height is ONLY possible for pure text objects
                        bAutoGrowWidthEnabled = bAutoGrowHeightEnabled = sal_True;
                    }
                }
                break;
                case OBJ_CUSTOMSHAPE :
                {
                    bFitToSizeEnabled = bContourEnabled = sal_False;
                    bAutoGrowSizeEnabled = sal_True;
                    bWordWrapTextEnabled = sal_True;
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

/*************************************************************************
|*
|* creates the page
|*
\************************************************************************/

SfxTabPage* SvxTextAttrPage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxTextAttrPage( pWindow, rAttrs ) );
}

sal_uInt16* SvxTextAttrPage::GetRanges()
{
    return( pRanges );
}

/** Check whether we have to uncheck the "Full width" check box.
*/
void SvxTextAttrPage::PointChanged( Window*, RECT_POINT eRP )
{
    if (m_pTsbFullWidth->GetState() == STATE_CHECK)
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
                    m_pTsbFullWidth->SetState( STATE_NOCHECK );
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
                    m_pTsbFullWidth->SetState( STATE_NOCHECK );
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
IMPL_LINK_NOARG(SvxTextAttrPage, ClickFullWidthHdl_Impl)
{
    if( m_pTsbFullWidth->GetState() == STATE_CHECK )
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
    return( 0L );
}

/*************************************************************************
|*
|* enables/disables "size at text" or "adjust to frame"
|*
\************************************************************************/

IMPL_LINK_NOARG(SvxTextAttrPage, ClickHdl_Impl)
{
    sal_Bool bAutoGrowWidth  = m_pTsbAutoGrowWidth->GetState() == STATE_CHECK;
    sal_Bool bAutoGrowHeight = m_pTsbAutoGrowHeight->GetState() == STATE_CHECK;
    sal_Bool bFitToSize      = m_pTsbFitToSize->GetState() == STATE_CHECK;
    sal_Bool bContour        = m_pTsbContour->GetState() == STATE_CHECK;

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
    sal_Bool bHorAndVer(SFX_ITEM_DONTCARE == eVState || SFX_ITEM_DONTCARE == eHState);

    // #83698# enable/disable text anchoring dependent of contour
    m_pFlPosition->Enable(!bContour && !bHorAndVer);

    return( 0L );
}


bool SvxTextAttrPage::IsTextDirectionLeftToRight (void) const
{
    // Determine the text writing direction with left to right as default.
    bool bLeftToRightDirection = true;
    SfxItemState eState = rOutAttrs.GetItemState(SDRATTR_TEXTDIRECTION);

    if(SFX_ITEM_DONTCARE != eState)
    {
        const SvxWritingModeItem& rItem = static_cast<const SvxWritingModeItem&> (
            rOutAttrs.Get (SDRATTR_TEXTDIRECTION));
        if (rItem.GetValue() == com::sun::star::text::WritingMode_TB_RL)
            bLeftToRightDirection = false;
    }
    return bLeftToRightDirection;
}

void SvxTextAttrPage::PageCreated(SfxAllItemSet aSet)
{
    SFX_ITEMSET_ARG (&aSet,pViewItem,OfaPtrItem,SID_SVXTEXTATTRPAGE_VIEW,sal_False);

    if (pViewItem)
        SetView( static_cast<SdrView *>(pViewItem->GetValue()));

    Construct();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
