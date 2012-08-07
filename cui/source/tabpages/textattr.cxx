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

#ifndef _SVX_TEXTATTR_CXX
#define _SVX_TEXTATTR_CXX
#endif
#include "textattr.hxx"
#include "textattr.hrc"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"
#include <sfx2/request.hxx>
#include <svx/ofaitem.hxx>
#include <editeng/writingmodeitem.hxx>

static sal_uInt16 pRanges[] =
{
    SDRATTR_MISC_FIRST, SDRATTR_TEXT_HORZADJUST,
    SDRATTR_TEXT_WORDWRAP, SDRATTR_TEXT_AUTOGROWSIZE,
    0
};

/*************************************************************************
|*
|* dialog (page) for copying objects
|*
\************************************************************************/

SvxTextAttrPage::SvxTextAttrPage( Window* pWindow, const SfxItemSet& rInAttrs ) :
                SvxTabPage      ( pWindow, CUI_RES( RID_SVXPAGE_TEXTATTR ),
                                  rInAttrs ),

                aFlText         ( this, CUI_RES( FL_TEXT ) ),
                aTsbAutoGrowWidth  ( this, CUI_RES( TSB_AUTOGROW_WIDTH ) ),
                aTsbAutoGrowHeight ( this, CUI_RES( TSB_AUTOGROW_HEIGHT ) ),
                aTsbFitToSize   ( this, CUI_RES( TSB_FIT_TO_SIZE ) ),
                aTsbContour     ( this, CUI_RES( TSB_CONTOUR ) ),
                aTsbWordWrapText( this, CUI_RES( TSB_WORDWRAP_TEXT ) ),
                aTsbAutoGrowSize( this, CUI_RES( TSB_AUTOGROW_SIZE ) ),
                aFlDistance     ( this, CUI_RES( FL_DISTANCE ) ),
                aFtLeft         ( this, CUI_RES( FT_LEFT ) ),
                aMtrFldLeft     ( this, CUI_RES( MTR_FLD_LEFT ) ),
                aFtRight        ( this, CUI_RES( FT_RIGHT ) ),
                aMtrFldRight    ( this, CUI_RES( MTR_FLD_RIGHT ) ),
                aFtTop          ( this, CUI_RES( FT_TOP ) ),
                aMtrFldTop      ( this, CUI_RES( MTR_FLD_TOP ) ),
                aFtBottom       ( this, CUI_RES( FT_BOTTOM ) ),
                aMtrFldBottom   ( this, CUI_RES( MTR_FLD_BOTTOM ) ),

                aFlSeparator    ( this, CUI_RES( FL_SEPARATOR ) ),

                aFlPosition     ( this, CUI_RES( FL_POSITION ) ),
                aCtlPosition    ( this, CUI_RES( CTL_POSITION ),
                                            RP_MM, 240, 100 ),
                aTsbFullWidth   ( this, CUI_RES( TSB_FULL_WIDTH ) ),

                rOutAttrs       ( rInAttrs )
{
    FreeResource();

    FieldUnit eFUnit = GetModuleFieldUnit( rInAttrs );
    SetFieldUnit( aMtrFldLeft, eFUnit );
    SetFieldUnit( aMtrFldRight, eFUnit );
    SetFieldUnit( aMtrFldTop, eFUnit );
    SetFieldUnit( aMtrFldBottom, eFUnit );

    Link aLink( LINK( this, SvxTextAttrPage, ClickHdl_Impl ) );
    aTsbAutoGrowWidth.SetClickHdl( aLink );
    aTsbAutoGrowHeight.SetClickHdl( aLink );
    aTsbFitToSize.SetClickHdl( aLink );
    aTsbContour.SetClickHdl( aLink );

    aTsbFullWidth.SetClickHdl(
        LINK( this, SvxTextAttrPage, ClickFullWidthHdl_Impl ) );

    aCtlPosition.SetAccessibleRelationMemberOf( &aFlPosition );
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
        SetMetricValue( aMtrFldLeft, nValue, eUnit );
    }
    else
        aMtrFldLeft.SetText( String() );
    aMtrFldLeft.SaveValue();

    pItem = GetItem( rAttrs, SDRATTR_TEXT_RIGHTDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_RIGHTDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrTextRightDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldRight, nValue, eUnit );
    }
    else
        aMtrFldRight.SetText( String() );
    aMtrFldRight.SaveValue();

    pItem = GetItem( rAttrs, SDRATTR_TEXT_UPPERDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_UPPERDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrTextUpperDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldTop, nValue, eUnit );
    }
    else
        aMtrFldTop.SetText( String() );
    aMtrFldTop.SaveValue();

    pItem = GetItem( rAttrs, SDRATTR_TEXT_LOWERDIST );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_LOWERDIST );
    if( pItem )
    {
        long nValue = ( ( const SdrTextLowerDistItem* )pItem )->GetValue();
        SetMetricValue( aMtrFldBottom, nValue, eUnit );
    }
    else
        aMtrFldBottom.SetText( String() );
    aMtrFldBottom.SaveValue();

    // adjust to height
    if ( rAttrs.GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) != SFX_ITEM_DONTCARE )
    {
        aTsbAutoGrowHeight.SetState( ( ( const SdrTextAutoGrowHeightItem& )rAttrs.Get( SDRATTR_TEXT_AUTOGROWHEIGHT ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        aTsbAutoGrowHeight.EnableTriState( sal_False );
    }
    else
        aTsbAutoGrowHeight.SetState( STATE_DONTKNOW );
    aTsbAutoGrowHeight.SaveValue();

    // adjust to width
    if ( rAttrs.GetItemState( SDRATTR_TEXT_AUTOGROWWIDTH ) != SFX_ITEM_DONTCARE )
    {
        aTsbAutoGrowWidth.SetState( ( ( const SdrTextAutoGrowWidthItem& )rAttrs.Get( SDRATTR_TEXT_AUTOGROWWIDTH ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        aTsbAutoGrowWidth.EnableTriState( sal_False );
    }
    else
        aTsbAutoGrowWidth.SetState( STATE_DONTKNOW );
    aTsbAutoGrowWidth.SaveValue();

    // autogrowsize
    if ( rAttrs.GetItemState( SDRATTR_TEXT_AUTOGROWSIZE ) != SFX_ITEM_DONTCARE )
    {
        aTsbAutoGrowSize.SetState( ( ( const SdrTextAutoGrowHeightItem& )rAttrs.Get( SDRATTR_TEXT_AUTOGROWHEIGHT ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        aTsbAutoGrowSize.EnableTriState( sal_False );
    }
    else
        aTsbAutoGrowSize.SetState( STATE_DONTKNOW );
    aTsbAutoGrowSize.SaveValue();

    // wordwrap text
    if ( rAttrs.GetItemState( SDRATTR_TEXT_WORDWRAP ) != SFX_ITEM_DONTCARE )
    {
        aTsbWordWrapText.SetState( ( ( const SdrTextWordWrapItem& )rAttrs.Get( SDRATTR_TEXT_WORDWRAP ) ).
                        GetValue() ? STATE_CHECK : STATE_NOCHECK );
        aTsbWordWrapText.EnableTriState( sal_False );
    }
    else
        aTsbWordWrapText.SetState( STATE_DONTKNOW );
    aTsbWordWrapText.SaveValue();


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

        aTsbFullWidth.EnableTriState( sal_False );

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
            aTsbFullWidth.SetState(STATE_CHECK);
        }

        aCtlPosition.SetActualRP( eRP );
    }
    else
    {
        // VertAdjust or HorAdjust is not unequivocal
        aCtlPosition.Reset();

        aCtlPosition.SetState(STATE_DONTKNOW);
        aCtlPosition.DoCompletelyDisable(sal_True);

        aTsbFullWidth.SetState(STATE_DONTKNOW);
        aTsbFullWidth.Enable( sal_False );
        aFlPosition.Enable( sal_False );
    }

    // adjust to border
    if ( rAttrs.GetItemState( SDRATTR_TEXT_FITTOSIZE ) != SFX_ITEM_DONTCARE )
    {
        SdrFitToSizeType eFTS = (SdrFitToSizeType)
                    ( ( const SdrTextFitToSizeTypeItem& )rAttrs.Get( SDRATTR_TEXT_FITTOSIZE ) ).GetValue();
        aTsbFitToSize.SetState( eFTS == SDRTEXTFIT_NONE ? STATE_NOCHECK : STATE_CHECK );
        aTsbFitToSize.EnableTriState( sal_False );
    }
    else
        aTsbFitToSize.SetState( STATE_DONTKNOW );
    aTsbFitToSize.SaveValue();

    if( rAttrs.GetItemState( SDRATTR_TEXT_CONTOURFRAME ) != SFX_ITEM_DONTCARE )
    {
        sal_Bool bContour = ( ( const SdrTextContourFrameItem& )rAttrs.Get( SDRATTR_TEXT_CONTOURFRAME ) ).GetValue();
        aTsbContour.SetState( bContour ? STATE_CHECK : STATE_NOCHECK );
        aTsbContour.EnableTriState( sal_False );
    }
    else
        aTsbContour.SetState( STATE_DONTKNOW );
    aTsbContour.SaveValue();

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

    if( aMtrFldLeft.GetText() != aMtrFldLeft.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldLeft, eUnit );
        rAttrs.Put( SdrTextLeftDistItem( nValue ) );
    }

    if( aMtrFldRight.GetText() != aMtrFldRight.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldRight, eUnit );
        rAttrs.Put( SdrTextRightDistItem( nValue ) );
    }

    if( aMtrFldTop.GetText() != aMtrFldTop.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldTop, eUnit );
        rAttrs.Put( SdrTextUpperDistItem( nValue ) );
    }

    if( aMtrFldBottom.GetText() != aMtrFldBottom.GetSavedValue() )
    {
        nValue = GetCoreValue( aMtrFldBottom, eUnit );
        rAttrs.Put( SdrTextLowerDistItem( nValue ) );
    }

    eState = aTsbAutoGrowHeight.GetState();
    if( eState != aTsbAutoGrowHeight.GetSavedValue() )
    {
        rAttrs.Put( SdrTextAutoGrowHeightItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    eState = aTsbAutoGrowWidth.GetState();
    if( eState != aTsbAutoGrowWidth.GetSavedValue() )
    {
        rAttrs.Put( SdrTextAutoGrowWidthItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    eState = aTsbAutoGrowSize.GetState();
    if( eState != aTsbAutoGrowSize.GetSavedValue() )
    {
        rAttrs.Put( SdrTextAutoGrowHeightItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    eState = aTsbWordWrapText.GetState();
    if( eState != aTsbWordWrapText.GetSavedValue() )
    {
        rAttrs.Put( SdrTextWordWrapItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    eState = aTsbContour.GetState();
    if( eState != aTsbContour.GetSavedValue() )
    {
        rAttrs.Put( SdrTextContourFrameItem( (sal_Bool) STATE_CHECK == eState ) );
    }

    eState = aTsbFitToSize.GetState();
    if( eState != aTsbFitToSize.GetSavedValue() )
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
    RECT_POINT eRP = aCtlPosition.GetActualRP();
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
    sal_Bool bIsDisabled(aCtlPosition.IsCompletelyDisabled());

    if(!bIsDisabled)
    {
        if( aTsbFullWidth.GetState() == STATE_CHECK )
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
                    if ( ((SdrTextObj*)pObj)->HasText() )
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
    aTsbAutoGrowHeight.Enable( bAutoGrowHeightEnabled );
    aTsbAutoGrowWidth.Enable( bAutoGrowWidthEnabled );
    aTsbFitToSize.Enable( bFitToSizeEnabled );
    aTsbContour.Enable( bContourEnabled );
    aTsbAutoGrowSize.Enable( bAutoGrowSizeEnabled );
    aTsbWordWrapText.Enable( bWordWrapTextEnabled );
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
    if (aTsbFullWidth.GetState() == STATE_CHECK)
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
                    aTsbFullWidth.SetState( STATE_NOCHECK );
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
                    aTsbFullWidth.SetState( STATE_NOCHECK );
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
    if( aTsbFullWidth.GetState() == STATE_CHECK )
    {
        if (IsTextDirectionLeftToRight())
        {
            // Move text anchor to horizontal middle axis.
            switch( aCtlPosition.GetActualRP() )
            {
                case RP_LT:
                case RP_RT:
                    aCtlPosition.SetActualRP( RP_MT );
                    break;

                case RP_LM:
                case RP_RM:
                    aCtlPosition.SetActualRP( RP_MM );
                    break;

                case RP_LB:
                case RP_RB:
                    aCtlPosition.SetActualRP( RP_MB );
                    break;
                default: ;//prevent warning
            }
        }
        else
        {
            // Move text anchor to vertical middle axis.
            switch( aCtlPosition.GetActualRP() )
            {
                case RP_LT:
                case RP_LB:
                    aCtlPosition.SetActualRP( RP_LM );
                    break;

                case RP_MT:
                case RP_MB:
                    aCtlPosition.SetActualRP( RP_MM );
                    break;

                case RP_RT:
                case RP_RB:
                    aCtlPosition.SetActualRP( RP_RM );
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
    sal_Bool bAutoGrowWidth  = aTsbAutoGrowWidth.GetState() == STATE_CHECK;
    sal_Bool bAutoGrowHeight = aTsbAutoGrowHeight.GetState() == STATE_CHECK;
    sal_Bool bFitToSize      = aTsbFitToSize.GetState() == STATE_CHECK;
    sal_Bool bContour        = aTsbContour.GetState() == STATE_CHECK;

    aTsbContour.Enable( !bFitToSize &&
                        !( ( bAutoGrowWidth && bAutoGrowWidthEnabled ) || ( bAutoGrowHeight && bAutoGrowHeightEnabled ) ) &&
                        bContourEnabled );

    aTsbAutoGrowWidth.Enable( !bFitToSize &&
                              !( bContour && bContourEnabled ) &&
                              bAutoGrowWidthEnabled );

    aTsbAutoGrowHeight.Enable( !bFitToSize &&
                               !( bContour && bContourEnabled ) &&
                               bAutoGrowHeightEnabled );

    aTsbFitToSize.Enable( !( ( bAutoGrowWidth && bAutoGrowWidthEnabled ) || ( bAutoGrowHeight && bAutoGrowHeightEnabled ) ) &&
                          !( bContour && bContourEnabled ) &&
                          bFitToSizeEnabled );

    // #101901# enable/disable metric fields and decorations dependent of contour
    aMtrFldLeft.Enable(!bContour);
    aMtrFldRight.Enable(!bContour);
    aMtrFldTop.Enable(!bContour);
    aMtrFldBottom.Enable(!bContour);
    aFlDistance.Enable(!bContour);
    aFtLeft.Enable(!bContour);
    aFtRight.Enable(!bContour);
    aFtTop.Enable(!bContour);
    aFtBottom.Enable(!bContour);

    if( bContour && bContourEnabled )
    {
        aMtrFldLeft.SetValue( 0 );
        aMtrFldRight.SetValue( 0 );
        aMtrFldTop.SetValue( 0 );
        aMtrFldBottom.SetValue( 0 );
    }

    // #103516# Do the setup based on states of hor/ver adjust
    SfxItemState eVState = rOutAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
    SfxItemState eHState = rOutAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );
    sal_Bool bHorAndVer(SFX_ITEM_DONTCARE == eVState || SFX_ITEM_DONTCARE == eHState);

    // #83698# enable/disable text anchoring dependent of contour
    aCtlPosition.Enable(!bContour && !bHorAndVer);
    aTsbFullWidth.Enable(!bContour && !bHorAndVer);
    aFlPosition.Enable(!bContour && !bHorAndVer);

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
