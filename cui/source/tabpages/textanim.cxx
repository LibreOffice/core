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
#include <svx/dialogs.hrc>

#include "textanim.hxx"
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
        SfxTabDialog        ( pParent
                              ,"TextDialog"
                              ,"cui/ui/textdialog.ui"
                              , pAttr ),
        pView               ( pSdrView )
{
    m_nTextId = AddTabPage( "RID_SVXPAGE_TEXTATTR", SvxTextAttrPage::Create, 0);
    m_nTextAnimId = AddTabPage( "RID_SVXPAGE_TEXTANIMATION", SvxTextAnimationPage::Create, 0);
}

/*************************************************************************
|*
|* PageCreated()
|*
\************************************************************************/

void SvxTextTabDialog::PageCreated( sal_uInt16 nId, SfxTabPage &rPage )
{
    if (nId == m_nTextId)
        {
            ( (SvxTextAttrPage&) rPage ).SetView( pView );
            ( (SvxTextAttrPage&) rPage ).Construct();
        }
}


/*************************************************************************
|*
|* Page
|*
\************************************************************************/

SvxTextAnimationPage::SvxTextAnimationPage( Window* pWindow, const SfxItemSet& rInAttrs ) :
                SfxTabPage      ( pWindow
                                  ,"TextAnimation"
                                  ,"cui/ui/textanimtabpage.ui"
                                  ,rInAttrs ),
                rOutAttrs       ( rInAttrs ),
                eAniKind        ( SDRTEXTANI_NONE )
{
    get(m_pLbEffect, "LB_EFFECT");
    get(m_pBoxDirection,"boxDIRECTION");
    get(m_pBtnUp, "BTN_UP");
    get(m_pBtnLeft, "BTN_LEFT");
    get(m_pBtnRight, "BTN_RIGHT");
    get(m_pBtnDown, "BTN_DOWN");

    get(m_pFlProperties, "FL_PROPERTIES");
    get(m_pTsbStartInside, "TSB_START_INSIDE");
    get(m_pTsbStopInside, "TSB_STOP_INSIDE");

    get(m_pBoxCount, "boxCOUNT");
    get(m_pTsbEndless,"TSB_ENDLESS");
    get(m_pNumFldCount,"NUM_FLD_COUNT");

    get(m_pTsbPixel, "TSB_PIXEL");
    get(m_pMtrFldAmount, "MTR_FLD_AMOUNT");

    get(m_pTsbAuto, "TSB_AUTO");
    get(m_pMtrFldDelay, "MTR_FLD_DELAY");

    eFUnit = GetModuleFieldUnit( rInAttrs );
    SfxItemPool* pPool = rOutAttrs.GetPool();
    DBG_ASSERT( pPool, "Wo ist der Pool" );
    eUnit = pPool->GetMetric( SDRATTR_TEXT_LEFTDIST );

    m_pLbEffect->SetSelectHdl( LINK( this, SvxTextAnimationPage, SelectEffectHdl_Impl ) );
    m_pTsbEndless->SetClickHdl( LINK( this, SvxTextAnimationPage, ClickEndlessHdl_Impl ) );
    m_pTsbAuto->SetClickHdl( LINK( this, SvxTextAnimationPage, ClickAutoHdl_Impl ) );
    m_pTsbPixel->SetClickHdl( LINK( this, SvxTextAnimationPage, ClickPixelHdl_Impl ) );

    Link aLink( LINK( this, SvxTextAnimationPage, ClickDirectionHdl_Impl ) );
    m_pBtnUp->SetClickHdl( aLink );
    m_pBtnLeft->SetClickHdl( aLink );
    m_pBtnRight->SetClickHdl( aLink );
    m_pBtnDown->SetClickHdl( aLink );
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
        m_pLbEffect->SelectEntryPos( sal::static_int_cast< sal_Int32 >(eAniKind) );
    }
    else
        m_pLbEffect->SetNoSelection();
    m_pLbEffect->SaveValue();

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
        m_pBtnUp->Check( false );
        m_pBtnLeft->Check( false );
        m_pBtnRight->Check( false );
        m_pBtnDown->Check( false );
    }
    m_pBtnUp->SaveValue();
    m_pBtnLeft->SaveValue();
    m_pBtnRight->SaveValue();
    m_pBtnDown->SaveValue();

    // Start inside
    pItem = GetItem( rAttrs, SDRATTR_TEXT_ANISTARTINSIDE );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANISTARTINSIDE );
    if( pItem )
    {
        m_pTsbStartInside->EnableTriState( false );
        sal_Bool bValue = ( ( const SdrTextAniStartInsideItem* )pItem )->GetValue();
        if( bValue )
            m_pTsbStartInside->SetState( TRISTATE_TRUE );
        else
            m_pTsbStartInside->SetState( TRISTATE_FALSE );
    }
    else
        m_pTsbStartInside->SetState( TRISTATE_INDET );
    m_pTsbStartInside->SaveValue();

    // Stop inside
    pItem = GetItem( rAttrs, SDRATTR_TEXT_ANISTOPINSIDE );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANISTOPINSIDE );
    if( pItem )
    {
        m_pTsbStopInside->EnableTriState( false );
        sal_Bool bValue = ( ( const SdrTextAniStopInsideItem* )pItem )->GetValue();
        if( bValue )
            m_pTsbStopInside->SetState( TRISTATE_TRUE );
        else
            m_pTsbStopInside->SetState( TRISTATE_FALSE );
    }
    else
        m_pTsbStopInside->SetState( TRISTATE_INDET );
    m_pTsbStopInside->SaveValue();

    // quantity
    pItem = GetItem( rAttrs, SDRATTR_TEXT_ANICOUNT );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANICOUNT );
    if( pItem )
    {
        m_pTsbEndless->EnableTriState( false );
        long nValue = (long) ( ( const SdrTextAniCountItem* )pItem )->GetValue();
        m_pNumFldCount->SetValue( nValue );
        if( nValue == 0 )
        {
            if( eAniKind == SDRTEXTANI_SLIDE )
            {
                m_pTsbEndless->SetState( TRISTATE_FALSE );
                m_pTsbEndless->Enable( false );
            }
            else
            {
                m_pTsbEndless->SetState( TRISTATE_TRUE );
                m_pNumFldCount->SetEmptyFieldValue();
            }
        }
        else
            m_pTsbEndless->SetState( TRISTATE_FALSE );
    }
    else
    {
        m_pNumFldCount->SetEmptyFieldValue();
        m_pTsbEndless->SetState( TRISTATE_INDET );
    }
    m_pTsbEndless->SaveValue();
    m_pNumFldCount->SaveValue();

    // delay
    pItem = GetItem( rAttrs, SDRATTR_TEXT_ANIDELAY );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANIDELAY );
    if( pItem )
    {
        m_pTsbAuto->EnableTriState( false );
        long nValue = (long) ( ( const SdrTextAniDelayItem* )pItem )->GetValue();
        m_pMtrFldDelay->SetValue( nValue );
        if( nValue == 0 )
        {
            m_pTsbAuto->SetState( TRISTATE_TRUE );
            m_pMtrFldDelay->SetEmptyFieldValue();
        }
        else
            m_pTsbAuto->SetState( TRISTATE_FALSE );
    }
    else
    {
        m_pMtrFldDelay->SetEmptyFieldValue();
        m_pTsbAuto->SetState( TRISTATE_INDET );
    }
    m_pTsbAuto->SaveValue();
    m_pMtrFldDelay->SaveValue();

    // step size
    pItem = GetItem( rAttrs, SDRATTR_TEXT_ANIAMOUNT );
    if( !pItem )
        pItem = &pPool->GetDefaultItem( SDRATTR_TEXT_ANIAMOUNT );
    if( pItem )
    {
        m_pTsbPixel->EnableTriState( false );
        long nValue = (long) ( ( const SdrTextAniAmountItem* )pItem )->GetValue();
        if( nValue <= 0 )
        {
            m_pTsbPixel->SetState( TRISTATE_TRUE );
            nValue = -nValue;
            if( nValue == 0 )
                nValue++;
            m_pMtrFldAmount->SetUnit( FUNIT_CUSTOM );
            m_pMtrFldAmount->SetDecimalDigits( 0 );

            m_pMtrFldAmount->SetSpinSize( 1 );
            m_pMtrFldAmount->SetMin( 1 );
            m_pMtrFldAmount->SetFirst( 1 );
            m_pMtrFldAmount->SetMax( 100 );
            m_pMtrFldAmount->SetLast( 100 );

            m_pMtrFldAmount->SetValue( nValue );
        }
        else
        {
            m_pTsbPixel->SetState( TRISTATE_FALSE );
            m_pMtrFldAmount->SetUnit( eFUnit );
            m_pMtrFldAmount->SetDecimalDigits( 2 );

            m_pMtrFldAmount->SetSpinSize( 10 );
            m_pMtrFldAmount->SetMin( 1 );
            m_pMtrFldAmount->SetFirst( 1 );
            m_pMtrFldAmount->SetMax( 10000 );
            m_pMtrFldAmount->SetLast( 10000 );

            SetMetricValue( *m_pMtrFldAmount, nValue, eUnit );
        }
    }
    else
    {
        m_pMtrFldAmount->Disable();
        m_pMtrFldAmount->SetEmptyFieldValue();
        m_pTsbPixel->SetState( TRISTATE_INDET );
    }
    m_pTsbPixel->SaveValue();
    m_pMtrFldAmount->SaveValue();


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
    sal_Int32 nPos;
    TriState eState;

    // animation type
    nPos = m_pLbEffect->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND &&
        nPos != m_pLbEffect->GetSavedValue() )
    {
        rAttrs.Put( SdrTextAniKindItem( (SdrTextAniKind) nPos ) );
        bModified = sal_True;
    }

    // animation direction
    if( m_pBtnUp->GetSavedValue() != TriState(m_pBtnUp->IsChecked()) ||
        m_pBtnLeft->GetSavedValue() != TriState(m_pBtnLeft->IsChecked()) ||
        m_pBtnRight->GetSavedValue() != TriState(m_pBtnRight->IsChecked()) ||
        m_pBtnDown->GetSavedValue() != TriState(m_pBtnDown->IsChecked()) )
    {
        SdrTextAniDirection eValue = (SdrTextAniDirection) GetSelectedDirection();
        rAttrs.Put( SdrTextAniDirectionItem( eValue ) );
        bModified = sal_True;
    }

    // Start inside
    eState = m_pTsbStartInside->GetState();
    if( eState != m_pTsbStartInside->GetSavedValue() )
    {
        rAttrs.Put( SdrTextAniStartInsideItem( (sal_Bool) TRISTATE_TRUE == eState ) );
        bModified = sal_True;
    }

    // Stop inside
    eState = m_pTsbStopInside->GetState();
    if( eState != m_pTsbStopInside->GetSavedValue() )
    {
        rAttrs.Put( SdrTextAniStopInsideItem( (sal_Bool) TRISTATE_TRUE == eState ) );
        bModified = sal_True;
    }

    // quantity
    eState = m_pTsbEndless->GetState();
    OUString aStr = m_pNumFldCount->GetText();
    if( eState != m_pTsbEndless->GetSavedValue() ||
        aStr != m_pNumFldCount->GetSavedValue() )
    {
        sal_Int64 nValue = 0;
        if( eState == TRISTATE_TRUE /*#89844#*/ && m_pTsbEndless->IsEnabled())
            bModified = sal_True;
        else
        {
            if( aStr != m_pNumFldCount->GetSavedValue() )
            {
                nValue = m_pNumFldCount->GetValue();
                bModified = sal_True;
            }
        }
        if( bModified )
            rAttrs.Put( SdrTextAniCountItem( (sal_uInt16) nValue ) );
    }

    // delay
    eState = m_pTsbAuto->GetState();
    aStr = m_pMtrFldDelay->GetText();
    if( eState != m_pTsbAuto->GetSavedValue() ||
        aStr != m_pMtrFldDelay->GetSavedValue() )
    {
        sal_Int64 nValue = 0;
        if( eState == TRISTATE_TRUE )
            bModified = sal_True;
        else
        {
            if( aStr != m_pMtrFldDelay->GetSavedValue() )
            {
                nValue = m_pMtrFldDelay->GetValue();
                bModified = sal_True;
            }
        }
        if( bModified )
            rAttrs.Put( SdrTextAniDelayItem( (sal_uInt16) nValue ) );
    }

    // step size
    eState = m_pTsbPixel->GetState();
    aStr = m_pMtrFldAmount->GetText();
    if( eState != m_pTsbPixel->GetSavedValue() ||
        aStr != m_pMtrFldAmount->GetSavedValue() )
    {
        sal_Int64 nValue = 0;
        if( eState == TRISTATE_TRUE )
        {
            nValue = m_pMtrFldAmount->GetValue();
            nValue = -nValue;
        }
        else
        {
            nValue = GetCoreValue( *m_pMtrFldAmount, eUnit );
        }
        rAttrs.Put( SdrTextAniAmountItem( (sal_Int16) nValue ) );

        bModified = sal_True;
    }

    return( bModified );
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
    sal_Int32 nPos = m_pLbEffect->GetSelectEntryPos();
    if( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        eAniKind = (SdrTextAniKind) nPos;
        switch( eAniKind )
        {
            case SDRTEXTANI_NONE:
            {
                m_pBoxDirection->Disable();
                m_pFlProperties->Disable();
            }
            break;

            case SDRTEXTANI_BLINK:
            case SDRTEXTANI_SCROLL:
            case SDRTEXTANI_ALTERNATE:
            case SDRTEXTANI_SLIDE:
            {
                m_pFlProperties->Enable();
                if( eAniKind == SDRTEXTANI_SLIDE )
                {
                    m_pTsbStartInside->Disable();
                    m_pTsbStopInside->Disable();
                    m_pTsbEndless->Disable();
                    m_pNumFldCount->Enable();
                    m_pNumFldCount->SetValue( m_pNumFldCount->GetValue() );
                }
                else
                {
                    m_pTsbStartInside->Enable();
                    m_pTsbStopInside->Enable();
                    m_pTsbEndless->Enable();
                    ClickEndlessHdl_Impl( NULL );
                }

                m_pTsbAuto->Enable();
                ClickAutoHdl_Impl( NULL );

                if( eAniKind == SDRTEXTANI_BLINK )
                {
                    m_pBoxDirection->Disable();
                    m_pBoxCount->Disable();
                }
                else
                {
                    m_pBoxDirection->Enable();
                    m_pBoxCount->Enable();
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
        TriState eState = m_pTsbEndless->GetState();
        if( eState != TRISTATE_FALSE )
        {
            m_pNumFldCount->Disable();
            m_pNumFldCount->SetEmptyFieldValue();
        }
        else
        {
            m_pNumFldCount->Enable();
            m_pNumFldCount->SetValue( m_pNumFldCount->GetValue() );
        }
    }
    return( 0L );
}

IMPL_LINK_NOARG(SvxTextAnimationPage, ClickAutoHdl_Impl)
{
    TriState eState = m_pTsbAuto->GetState();
    if( eState != TRISTATE_FALSE )
    {
        m_pMtrFldDelay->Disable();
        m_pMtrFldDelay->SetEmptyFieldValue();
    }
    else
    {
        m_pMtrFldDelay->Enable();
        m_pMtrFldDelay->SetValue( m_pMtrFldDelay->GetValue() );
    }

    return( 0L );
}

IMPL_LINK_NOARG(SvxTextAnimationPage, ClickPixelHdl_Impl)
{
    TriState eState = m_pTsbPixel->GetState();
    if( eState == TRISTATE_TRUE )
    {
        sal_Int64 nValue = m_pMtrFldAmount->GetValue() / 10;
        m_pMtrFldAmount->Enable();
        m_pMtrFldAmount->SetUnit( FUNIT_CUSTOM );
        //SetFieldUnit( aMtrFldAmount, FUNIT_CUSTOM );
        m_pMtrFldAmount->SetDecimalDigits( 0 );

        m_pMtrFldAmount->SetSpinSize( 1 );
        m_pMtrFldAmount->SetMin( 1 );
        m_pMtrFldAmount->SetFirst( 1 );
        m_pMtrFldAmount->SetMax( 100 );
        m_pMtrFldAmount->SetLast( 100 );

        m_pMtrFldAmount->SetValue( nValue );
    }
    else if( eState == TRISTATE_FALSE )
    {
        sal_Int64 nValue = m_pMtrFldAmount->GetValue() * 10;
        m_pMtrFldAmount->Enable();
        m_pMtrFldAmount->SetUnit( eFUnit );
        //SetFieldUnit( aMtrFldAmount, eFUnit );
        m_pMtrFldAmount->SetDecimalDigits( 2 );

        m_pMtrFldAmount->SetSpinSize( 10 );
        m_pMtrFldAmount->SetMin( 1 );
        m_pMtrFldAmount->SetFirst( 1 );
        m_pMtrFldAmount->SetMax( 10000 );
        m_pMtrFldAmount->SetLast( 10000 );

        m_pMtrFldAmount->SetValue( nValue );
    }

    return( 0L );
}

IMPL_LINK( SvxTextAnimationPage, ClickDirectionHdl_Impl, ImageButton *, pBtn )
{
    m_pBtnUp->Check( pBtn == m_pBtnUp );
    m_pBtnLeft->Check( pBtn == m_pBtnLeft );
    m_pBtnRight->Check( pBtn == m_pBtnRight );
    m_pBtnDown->Check( pBtn == m_pBtnDown );

    return( 0L );
}

void SvxTextAnimationPage::SelectDirection( SdrTextAniDirection nValue )
{
    m_pBtnUp->Check( nValue == SDRTEXTANI_UP );
    m_pBtnLeft->Check( nValue == SDRTEXTANI_LEFT );
    m_pBtnRight->Check( nValue == SDRTEXTANI_RIGHT );
    m_pBtnDown->Check( nValue == SDRTEXTANI_DOWN );
}

sal_uInt16 SvxTextAnimationPage::GetSelectedDirection()
{
    sal_uInt16 nValue = 0;

    if( m_pBtnUp->IsChecked() )
        nValue = SDRTEXTANI_UP;
    else if( m_pBtnLeft->IsChecked() )
        nValue = SDRTEXTANI_LEFT;
    else if( m_pBtnRight->IsChecked() )
        nValue = SDRTEXTANI_RIGHT;
    else if( m_pBtnDown->IsChecked() )
        nValue = SDRTEXTANI_DOWN;

    return( nValue );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
