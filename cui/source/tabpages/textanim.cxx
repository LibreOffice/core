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
#include <svx/dialogs.hrc>

#ifndef _SVX_TEXTANIM_CXX
#define _SVX_TEXTANIM_CXX
#endif
#include "textanim.hxx"
#include "textanim.hrc"
#include "textattr.hxx"
#include <dialmgr.hxx>
#include "svx/dlgutil.hxx"

static sal_uInt16 pRanges[] =
{
    SDRATTR_TEXT_ANIKIND,
    SDRATTR_TEXT_ANIAMOUNT,
    0
};

/*************************************************************************
|*
|* constructor of the tab dialog: adds pages to the dialog
|*
\************************************************************************/

SvxTextTabDialog::SvxTextTabDialog( Window* pParent,
                                const SfxItemSet* pAttr,
                                const SdrView* pSdrView ) :
        SfxTabDialog        ( pParent, CUI_RES( RID_SVXDLG_TEXT ), pAttr ),
        rOutAttrs           ( *pAttr ),
        pView               ( pSdrView )
{
    FreeResource();

    AddTabPage( RID_SVXPAGE_TEXTATTR, SvxTextAttrPage::Create, 0);
    AddTabPage( RID_SVXPAGE_TEXTANIMATION, SvxTextAnimationPage::Create, 0);
}

/*************************************************************************
|*
|* PageCreated()
|*
\************************************************************************/

void SvxTextTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    switch( nId )
    {
        case RID_SVXPAGE_TEXTATTR:
        {
            ( (SvxTextAttrPage&) rPage ).SetView( pView );
            ( (SvxTextAttrPage&) rPage ).Construct();
        }
        break;

        case RID_SVXPAGE_TEXTANIMATION:
        break;

        default:
        break;
    }
}


/*************************************************************************
|*
|* Page
|*
\************************************************************************/

SvxTextAnimationPage::SvxTextAnimationPage( Window* pWindow, const SfxItemSet& rInAttrs ) :
                SfxTabPage      ( pWindow, CUI_RES( RID_SVXPAGE_TEXTANIMATION ),
                                  rInAttrs ),
                aFlEffect       ( this, CUI_RES(FL_EFFECT)),
                aFtEffects      ( this, CUI_RES(FT_EFFECTS)),
                aLbEffect       ( this, CUI_RES( LB_EFFECT ) ),
                //aCtlEffect        ( this, CUI_RES( CTL_EFFECT ) ),
                aFtDirection    ( this, CUI_RES(FT_DIRECTION) ),
                aBtnUp          ( this, CUI_RES( BTN_UP ) ),
                aBtnLeft        ( this, CUI_RES( BTN_LEFT ) ),
                aBtnRight       ( this, CUI_RES( BTN_RIGHT ) ),
                aBtnDown        ( this, CUI_RES( BTN_DOWN ) ),

                aFlProperties   ( this, CUI_RES(FL_PROPERTIES)),
                aTsbStartInside ( this, CUI_RES( TSB_START_INSIDE ) ),
                aTsbStopInside  ( this, CUI_RES( TSB_STOP_INSIDE ) ),

                aFtCount        ( this, CUI_RES(FT_COUNT)),
                aTsbEndless     ( this, CUI_RES( TSB_ENDLESS ) ),
                aNumFldCount    ( this, CUI_RES( NUM_FLD_COUNT ) ),

                aFtAmount       ( this, CUI_RES(FT_AMOUNT)),
                aTsbPixel       ( this, CUI_RES( TSB_PIXEL ) ),
                aMtrFldAmount   ( this, CUI_RES( MTR_FLD_AMOUNT ) ),

                aFtDelay        ( this, CUI_RES(FT_DELAY)),
                aTsbAuto        ( this, CUI_RES( TSB_AUTO ) ),
                aMtrFldDelay    ( this, CUI_RES( MTR_FLD_DELAY ) ),

                rOutAttrs       ( rInAttrs ),
                eAniKind        ( SDRTEXTANI_NONE )
{
    FreeResource();

    eFUnit = GetModuleFieldUnit( rInAttrs );
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    eUnit = pPool->GetMetric( SDRATTR_TEXT_LEFTDIST );

    Construct();

    aLbEffect.SetSelectHdl(
        LINK( this, SvxTextAnimationPage, SelectEffectHdl_Impl ) );
    aTsbEndless.SetClickHdl(
        LINK( this, SvxTextAnimationPage, ClickEndlessHdl_Impl ) );
    aTsbAuto.SetClickHdl(
        LINK( this, SvxTextAnimationPage, ClickAutoHdl_Impl ) );
    aTsbPixel.SetClickHdl(
        LINK( this, SvxTextAnimationPage, ClickPixelHdl_Impl ) );

    Link aLink( LINK( this, SvxTextAnimationPage, ClickDirectionHdl_Impl ) );
    aBtnUp.SetClickHdl( aLink );
    aBtnLeft.SetClickHdl( aLink );
    aBtnRight.SetClickHdl( aLink );
    aBtnDown.SetClickHdl( aLink );

    aNumFldCount.SetAccessibleRelationLabeledBy( &aTsbEndless );
    aMtrFldAmount.SetAccessibleRelationLabeledBy( &aTsbPixel );
    aMtrFldDelay.SetAccessibleRelationLabeledBy( &aTsbAuto );

    aBtnUp.SetAccessibleRelationLabeledBy( &aFtDirection );
    aBtnLeft.SetAccessibleRelationLabeledBy( &aFtDirection );
    aBtnRight.SetAccessibleRelationLabeledBy( &aFtDirection );
    aBtnDown.SetAccessibleRelationLabeledBy( &aFtDirection );

    aBtnUp.SetAccessibleRelationMemberOf( &aFlEffect );
    aBtnLeft.SetAccessibleRelationMemberOf( &aFlEffect );
    aBtnRight.SetAccessibleRelationMemberOf( &aFlEffect );
    aBtnDown.SetAccessibleRelationMemberOf( &aFlEffect );

    aTsbEndless.SetAccessibleRelationLabeledBy( &aFtCount );
    aTsbPixel.SetAccessibleRelationLabeledBy( &aFtAmount );
    aTsbAuto.SetAccessibleRelationLabeledBy( &aFtDelay );
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SvxTextAnimationPage::~SvxTextAnimationPage()
{
}

/*************************************************************************
|*
|* reads the passed item set
|*
\************************************************************************/

void SvxTextAnimationPage::Reset( const SfxItemSet& rAttrs )
{
    const SfxItemPool* pPool = rAttrs.GetPool();

    // animation type
    const SfxPoolItem* pItem = GetItem( rAttrs, SDRATTR_TEXT_ANIKIND );

    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANIKIND );
    if( pItem )
    {
        eAniKind = ( ( const SdrTextAniKindItem* )pItem )->GetValue();
        aLbEffect.SelectEntryPos( sal::static_int_cast< sal_uInt16 >(eAniKind) );
    }
    else
        aLbEffect.SetNoSelection();
    aLbEffect.SaveValue();

    // animation direction
    pItem = GetItem( rAttrs, SDRATTR_TEXT_ANIDIRECTION );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANIDIRECTION );
    if( pItem )
    {
        SdrTextAniDirection eValue = ( ( const SdrTextAniDirectionItem* )pItem )->GetValue();
        SelectDirection( eValue );
    }
    else
    {
        aBtnUp.Check( sal_False );
        aBtnLeft.Check( sal_False );
        aBtnRight.Check( sal_False );
        aBtnDown.Check( sal_False );
    }
    aBtnUp.SaveValue();
    aBtnLeft.SaveValue();
    aBtnRight.SaveValue();
    aBtnDown.SaveValue();

    // Start inside
    pItem = GetItem( rAttrs, SDRATTR_TEXT_ANISTARTINSIDE );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANISTARTINSIDE );
    if( pItem )
    {
        aTsbStartInside.EnableTriState( sal_False );
        sal_Bool bValue = ( ( const SdrTextAniStartInsideItem* )pItem )->GetValue();
        if( bValue )
            aTsbStartInside.SetState( STATE_CHECK );
        else
            aTsbStartInside.SetState( STATE_NOCHECK );
    }
    else
        aTsbStartInside.SetState( STATE_DONTKNOW );
    aTsbStartInside.SaveValue();

    // Stop inside
    pItem = GetItem( rAttrs, SDRATTR_TEXT_ANISTOPINSIDE );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANISTOPINSIDE );
    if( pItem )
    {
        aTsbStopInside.EnableTriState( sal_False );
        sal_Bool bValue = ( ( const SdrTextAniStopInsideItem* )pItem )->GetValue();
        if( bValue )
            aTsbStopInside.SetState( STATE_CHECK );
        else
            aTsbStopInside.SetState( STATE_NOCHECK );
    }
    else
        aTsbStopInside.SetState( STATE_DONTKNOW );
    aTsbStopInside.SaveValue();

    // quantity
    pItem = GetItem( rAttrs, SDRATTR_TEXT_ANICOUNT );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANICOUNT );
    if( pItem )
    {
        aTsbEndless.EnableTriState( sal_False );
        long nValue = (long) ( ( const SdrTextAniCountItem* )pItem )->GetValue();
        aNumFldCount.SetValue( nValue );
        if( nValue == 0 )
        {
            if( eAniKind == SDRTEXTANI_SLIDE )
            {
                aTsbEndless.SetState( STATE_NOCHECK );
                aTsbEndless.Enable( sal_False );
            }
            else
            {
                aTsbEndless.SetState( STATE_CHECK );
                aNumFldCount.SetEmptyFieldValue();
            }
        }
        else
            aTsbEndless.SetState( STATE_NOCHECK );
    }
    else
    {
        aNumFldCount.SetEmptyFieldValue();
        aTsbEndless.SetState( STATE_DONTKNOW );
    }
    aTsbEndless.SaveValue();
    aNumFldCount.SaveValue();

    // delay
    pItem = GetItem( rAttrs, SDRATTR_TEXT_ANIDELAY );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANIDELAY );
    if( pItem )
    {
        aTsbAuto.EnableTriState( sal_False );
        long nValue = (long) ( ( const SdrTextAniDelayItem* )pItem )->GetValue();
        aMtrFldDelay.SetValue( nValue );
        if( nValue == 0 )
        {
            aTsbAuto.SetState( STATE_CHECK );
            aMtrFldDelay.SetEmptyFieldValue();
        }
        else
            aTsbAuto.SetState( STATE_NOCHECK );
    }
    else
    {
        aMtrFldDelay.SetEmptyFieldValue();
        aTsbAuto.SetState( STATE_DONTKNOW );
    }
    aTsbAuto.SaveValue();
    aMtrFldDelay.SaveValue();

    // step size
    pItem = GetItem( rAttrs, SDRATTR_TEXT_ANIAMOUNT );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANIAMOUNT );
    if( pItem )
    {
        aTsbPixel.EnableTriState( sal_False );
        long nValue = (long) ( ( const SdrTextAniAmountItem* )pItem )->GetValue();
        if( nValue <= 0 )
        {
            aTsbPixel.SetState( STATE_CHECK );
            nValue = -nValue;
            if( nValue == 0 )
                nValue++;
            aMtrFldAmount.SetUnit( FUNIT_CUSTOM );
            aMtrFldAmount.SetDecimalDigits( 0 );

            aMtrFldAmount.SetSpinSize( 1 );
            aMtrFldAmount.SetMin( 1 );
            aMtrFldAmount.SetFirst( 1 );
            aMtrFldAmount.SetMax( 100 );
            aMtrFldAmount.SetLast( 100 );

            aMtrFldAmount.SetValue( nValue );
        }
        else
        {
            aTsbPixel.SetState( STATE_NOCHECK );
            aMtrFldAmount.SetUnit( eFUnit );
            aMtrFldAmount.SetDecimalDigits( 2 );

            aMtrFldAmount.SetSpinSize( 10 );
            aMtrFldAmount.SetMin( 1 );
            aMtrFldAmount.SetFirst( 1 );
            aMtrFldAmount.SetMax( 10000 );
            aMtrFldAmount.SetLast( 10000 );

            SetMetricValue( aMtrFldAmount, nValue, eUnit );
        }
    }
    else
    {
        aMtrFldAmount.Disable();
        aMtrFldAmount.SetEmptyFieldValue();
        aTsbPixel.SetState( STATE_DONTKNOW );
    }
    aTsbPixel.SaveValue();
    aMtrFldAmount.SaveValue();


    SelectEffectHdl_Impl( NULL );
    ClickEndlessHdl_Impl( NULL );
    ClickAutoHdl_Impl( NULL );
    //ClickPixelHdl_Impl( NULL );
}

/*************************************************************************
|*
|* fills the passed item set with dialog box attributes
|*
\************************************************************************/

sal_Bool SvxTextAnimationPage::FillItemSet( SfxItemSet& rAttrs)
{
    sal_Bool bModified = sal_False;
    sal_uInt16 nPos;
    TriState eState;

    // animation type
    nPos = aLbEffect.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND &&
        nPos != aLbEffect.GetSavedValue() )
    {
        rAttrs.Put( SdrTextAniKindItem( (SdrTextAniKind) nPos ) );
        bModified = sal_True;
    }

    // animation direction
    if( aBtnUp.GetSavedValue() != aBtnUp.IsChecked() ||
        aBtnLeft.GetSavedValue() != aBtnLeft.IsChecked() ||
        aBtnRight.GetSavedValue() != aBtnRight.IsChecked() ||
        aBtnDown.GetSavedValue() != aBtnDown.IsChecked() )
    {
        SdrTextAniDirection eValue = (SdrTextAniDirection) GetSelectedDirection();
        rAttrs.Put( SdrTextAniDirectionItem( eValue ) );
        bModified = sal_True;
    }

    // Start inside
    eState = aTsbStartInside.GetState();
    if( eState != aTsbStartInside.GetSavedValue() )
    {
        rAttrs.Put( SdrTextAniStartInsideItem( (sal_Bool) STATE_CHECK == eState ) );
        bModified = sal_True;
    }

    // Stop inside
    eState = aTsbStopInside.GetState();
    if( eState != aTsbStopInside.GetSavedValue() )
    {
        rAttrs.Put( SdrTextAniStopInsideItem( (sal_Bool) STATE_CHECK == eState ) );
        bModified = sal_True;
    }

    // quantity
    eState = aTsbEndless.GetState();
    String aStr = aNumFldCount.GetText();
    if( eState != aTsbEndless.GetSavedValue() ||
        aStr != aNumFldCount.GetSavedValue() )
    {
        sal_Int64 nValue = 0;
        if( eState == STATE_CHECK /*#89844#*/ && aTsbEndless.IsEnabled())
            bModified = sal_True;
        else
        {
            if( aStr != aNumFldCount.GetSavedValue() )
            {
                nValue = aNumFldCount.GetValue();
                bModified = sal_True;
            }
        }
        if( bModified )
            rAttrs.Put( SdrTextAniCountItem( (sal_uInt16) nValue ) );
    }

    // delay
    eState = aTsbAuto.GetState();
    aStr = aMtrFldDelay.GetText();
    if( eState != aTsbAuto.GetSavedValue() ||
        aStr != aMtrFldDelay.GetSavedValue() )
    {
        sal_Int64 nValue = 0;
        if( eState == STATE_CHECK )
            bModified = sal_True;
        else
        {
            if( aStr != aMtrFldDelay.GetSavedValue() )
            {
                nValue = aMtrFldDelay.GetValue();
                bModified = sal_True;
            }
        }
        if( bModified )
            rAttrs.Put( SdrTextAniDelayItem( (sal_uInt16) nValue ) );
    }

    // step size
    eState = aTsbPixel.GetState();
    aStr = aMtrFldAmount.GetText();
    if( eState != aTsbPixel.GetSavedValue() ||
        aStr != aMtrFldAmount.GetSavedValue() )
    {
        sal_Int64 nValue = 0;
        if( eState == STATE_CHECK )
        {
            nValue = aMtrFldAmount.GetValue();
            nValue = -nValue;
        }
        else
        {
            nValue = GetCoreValue( aMtrFldAmount, eUnit );
        }
        rAttrs.Put( SdrTextAniAmountItem( (sal_Int16) nValue ) );

        bModified = sal_True;
    }

    return( bModified );
}

void SvxTextAnimationPage::Construct()
{
}

sal_uInt16* SvxTextAnimationPage::GetRanges()
{
    return( pRanges );
}

/*************************************************************************
|*
|* creates the page
|*
\************************************************************************/

SfxTabPage* SvxTextAnimationPage::Create( Window* pWindow,
                const SfxItemSet& rAttrs )
{
    return( new SvxTextAnimationPage( pWindow, rAttrs ) );
}

IMPL_LINK_NOARG(SvxTextAnimationPage, SelectEffectHdl_Impl)
{
    sal_uInt16 nPos = aLbEffect.GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        eAniKind = (SdrTextAniKind) nPos;
        switch( eAniKind )
        {
            case SDRTEXTANI_NONE:
            {
                aFtDirection.Disable();
                aBtnUp.Disable();
                aBtnLeft.Disable();
                aBtnRight.Disable();
                aBtnDown.Disable();
                aTsbStartInside.Disable();
                aTsbStopInside.Disable();

                aTsbEndless.Disable();
                aNumFldCount.Disable();
                aFtCount.Disable();

                aTsbAuto.Disable();
                aMtrFldDelay.Disable();
                aFtDelay.Disable();

                aTsbPixel.Disable();
                aMtrFldAmount.Disable();
                aFtAmount.Disable();
            }
            break;

            case SDRTEXTANI_BLINK:
            case SDRTEXTANI_SCROLL:
            case SDRTEXTANI_ALTERNATE:
            case SDRTEXTANI_SLIDE:
            {
                if( eAniKind == SDRTEXTANI_SLIDE )
                {
                    aTsbStartInside.Disable();
                    aTsbStopInside.Disable();

                    aTsbEndless.Disable();
                    aNumFldCount.Enable();
                    aNumFldCount.SetValue( aNumFldCount.GetValue() );
                }
                else
                {
                    aTsbStartInside.Enable();
                    aTsbStopInside.Enable();

                    aTsbEndless.Enable();
                    ClickEndlessHdl_Impl( NULL );
                }
                aFtCount.Enable();

                aTsbAuto.Enable();
                aFtDelay.Enable();
                ClickAutoHdl_Impl( NULL );

                if( eAniKind == SDRTEXTANI_BLINK )
                {
                    aFtDirection.Disable();
                    aBtnUp.Disable();
                    aBtnLeft.Disable();
                    aBtnRight.Disable();
                    aBtnDown.Disable();

                    aTsbPixel.Disable();
                    aMtrFldAmount.Disable();
                    aFtAmount.Disable();
                }
                else
                {
                    aFtDirection.Enable();
                    aBtnUp.Enable();
                    aBtnLeft.Enable();
                    aBtnRight.Enable();
                    aBtnDown.Enable();

                    aTsbPixel.Enable();
                    aMtrFldAmount.Enable();
                    aFtAmount.Enable();
                }
            }
            break;
        }

    }
    return( 0L );
}

IMPL_LINK_NOARG(SvxTextAnimationPage, ClickEndlessHdl_Impl)
{

    if( eAniKind != SDRTEXTANI_SLIDE )
    {
        TriState eState = aTsbEndless.GetState();
        if( eState != STATE_NOCHECK )
        {
            aNumFldCount.Disable();
            aNumFldCount.SetEmptyFieldValue();
        }
        else
        {
            aNumFldCount.Enable();
            aNumFldCount.SetValue( aNumFldCount.GetValue() );
        }
    }
    return( 0L );
}

IMPL_LINK_NOARG(SvxTextAnimationPage, ClickAutoHdl_Impl)
{
    TriState eState = aTsbAuto.GetState();
    if( eState != STATE_NOCHECK )
    {
        aMtrFldDelay.Disable();
        aMtrFldDelay.SetEmptyFieldValue();
    }
    else
    {
        aMtrFldDelay.Enable();
        aMtrFldDelay.SetValue( aMtrFldDelay.GetValue() );
    }

    return( 0L );
}

IMPL_LINK_NOARG(SvxTextAnimationPage, ClickPixelHdl_Impl)
{
    TriState eState = aTsbPixel.GetState();
    if( eState == STATE_CHECK )
    {
        sal_Int64 nValue = aMtrFldAmount.GetValue() / 10;
        aMtrFldAmount.Enable();
        aMtrFldAmount.SetUnit( FUNIT_CUSTOM );
        //SetFieldUnit( aMtrFldAmount, FUNIT_CUSTOM );
        aMtrFldAmount.SetDecimalDigits( 0 );

        aMtrFldAmount.SetSpinSize( 1 );
        aMtrFldAmount.SetMin( 1 );
        aMtrFldAmount.SetFirst( 1 );
        aMtrFldAmount.SetMax( 100 );
        aMtrFldAmount.SetLast( 100 );

        aMtrFldAmount.SetValue( nValue );
    }
    else if( eState == STATE_NOCHECK )
    {
        sal_Int64 nValue = aMtrFldAmount.GetValue() * 10;
        aMtrFldAmount.Enable();
        aMtrFldAmount.SetUnit( eFUnit );
        //SetFieldUnit( aMtrFldAmount, eFUnit );
        aMtrFldAmount.SetDecimalDigits( 2 );

        aMtrFldAmount.SetSpinSize( 10 );
        aMtrFldAmount.SetMin( 1 );
        aMtrFldAmount.SetFirst( 1 );
        aMtrFldAmount.SetMax( 10000 );
        aMtrFldAmount.SetLast( 10000 );

        aMtrFldAmount.SetValue( nValue );
    }

    return( 0L );
}

IMPL_LINK( SvxTextAnimationPage, ClickDirectionHdl_Impl, ImageButton *, pBtn )
{
    aBtnUp.Check( pBtn == &aBtnUp );
    aBtnLeft.Check( pBtn == &aBtnLeft );
    aBtnRight.Check( pBtn == &aBtnRight );
    aBtnDown.Check( pBtn == &aBtnDown );

    return( 0L );
}

void SvxTextAnimationPage::SelectDirection( SdrTextAniDirection nValue )
{
    aBtnUp.Check( nValue == SDRTEXTANI_UP );
    aBtnLeft.Check( nValue == SDRTEXTANI_LEFT );
    aBtnRight.Check( nValue == SDRTEXTANI_RIGHT );
    aBtnDown.Check( nValue == SDRTEXTANI_DOWN );
}

sal_uInt16 SvxTextAnimationPage::GetSelectedDirection()
{
    sal_uInt16 nValue = 0;

    if( aBtnUp.IsChecked() )
        nValue = SDRTEXTANI_UP;
    else if( aBtnLeft.IsChecked() )
        nValue = SDRTEXTANI_LEFT;
    else if( aBtnRight.IsChecked() )
        nValue = SDRTEXTANI_RIGHT;
    else if( aBtnDown.IsChecked() )
        nValue = SDRTEXTANI_DOWN;

    return( nValue );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
