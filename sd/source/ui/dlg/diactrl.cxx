/*************************************************************************
 *
 *  $RCSfile: diactrl.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 12:24:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_PRESENTATION_FADEEFFECT_HPP_
#include <com/sun/star/presentation/FadeEffect.hpp>
#endif

#include <svx/dialogs.hrc>

#include "sdattr.hxx"
#include "strings.hrc"

#define _SD_DIACTRL_CXX
#include "diactrl.hxx"

#include "sdresid.hxx"
#include "app.hrc"
#include "res_bmp.hrc"

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif

using namespace ::com::sun::star;

SFX_IMPL_TOOLBOX_CONTROL( SdTbxCtlDiaEffect, DiaEffectItem )
SFX_IMPL_TOOLBOX_CONTROL( SdTbxCtlDiaPages,  SfxUInt16Item )

/*************************************************************************
|*
|* Konstruktor fuer DiaEffectControl
|*
\************************************************************************/

DiaEffectControl::DiaEffectControl( Window* pParent,
                                    const uno::Reference< frame::XFrame >& rFrame,
                                    WinBits nStyle ) :
        Window          ( pParent, nStyle ),
        m_xFrame        ( rFrame ),
        aFtDescr        ( this, 0 ),
        aLbEffect       ( this, WinBits( WB_BORDER | WB_DROPDOWN ) )
{
    // Child-Windows (Controls) setzen
    String  aStr( SdResId( STR_EFFECT ) );
    long    nTextHeight = this->GetTextHeight();
    Size    aSize( this->GetTextWidth( aStr )+6, nTextHeight+2 );


    //Size  aSize( LogicToPixel( pParent->GetTextSize( aStr ), MAP_APPFONT ) );

    aFtDescr.SetPosPixel( Point( 0, 4 ) );
    aFtDescr.SetSizePixel( aSize ); //( 40, 14 ) );
    aFtDescr.SetText( aStr );
    aFtDescr.Show();

    String aXStr; aXStr += sal_Unicode('X');
    Size aXSize( this->GetTextWidth( aXStr ), nTextHeight );
    aLbEffect.SetPosPixel( Point( aSize.Width(), 0 ) );
    aLbEffect.SetSizePixel( Size( aXSize.Width() * 21, aXSize.Height() * 10 ) ); // 155/180
    aLbEffect.Fill();
    aLbEffect.SelectEffect( presentation::FadeEffect_NONE );
    aLbEffect.SetHelpId( SID_DIA_EFFECT );
    aLbEffect.Show();

    Size aDescSize = aFtDescr.GetSizePixel();
    Size aAttrSize = aLbEffect.GetSizePixel();
    Point aAttrPnt = aLbEffect.GetPosPixel();
    SetSizePixel( Size( aAttrPnt.X() + aAttrSize.Width(),
                        Max( aAttrSize.Height(), aDescSize.Height() ) ) );

    //SetSizePixel( Size( 210, 20 ) );
    Show();

    aLbEffect.SetSelectHdl( LINK( this, DiaEffectControl, SelectDiaEffectHdl ) );

    SelectDiaEffectHdl( NULL );
}

//------------------------------------------------------------------------

DiaEffectControl::~DiaEffectControl()
{
}




void DiaEffectControl::GetFocus (void)
{
    Window::GetFocus();
    // Grab the focus to the slide effects control so that it can be
    // controlled with the keyboard.
    aLbEffect.GrabFocus();
}




//------------------------------------------------------------------------

IMPL_LINK( DiaEffectControl, SelectDiaEffectHdl, void *, p )
{
    DiaEffectItem aDiaEffectItem( aLbEffect.GetSelectedEffect() );

    if( p )
    {
        ::uno::Any a;
        ::uno::Sequence< ::beans::PropertyValue > aArgs( 1 );
        aDiaEffectItem.QueryValue( a );
        aArgs[0].Name   = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DiaEffect" ));
        aArgs[0].Value  = a;
        SfxToolBoxControl::Dispatch( ::uno::Reference< ::frame::XDispatchProvider >( m_xFrame->getController(), ::uno::UNO_QUERY ),
                                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DiaEffect" )),
                                     aArgs );
//        rBindings.GetDispatcher()->Execute( SID_DIA, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aDiaEffectItem, (void*) NULL, 0L );
    }

    return( 0L );
}

SFX_IMPL_TOOLBOX_CONTROL( SdTbxCtlDiaSpeed, DiaSpeedItem )

/*************************************************************************
|*
|* Konstruktor fuer DiaSpeedControl
|*
\************************************************************************/

DiaSpeedControl::DiaSpeedControl( Window* pParent,
                                  const uno::Reference< frame::XFrame >& rFrame,
                                  WinBits nStyle ) :
        Window          ( pParent, nStyle ),
        m_xFrame        ( rFrame ),
        aLbSpeed        ( this, WinBits( WB_BORDER | WB_DROPDOWN ) )
{
    String  aStrSlow( SdResId( STR_SLOW ) );
    String  aStrMed(  SdResId( STR_MEDIUM ) );
    String  aStrFast( SdResId( STR_FAST ) );
    Size    aSize( this->GetTextWidth( aStrSlow ), this->GetTextHeight() );
    if( this->GetTextWidth( aStrMed ) > aSize.Width() )
        aSize.Width() = this->GetTextWidth( aStrMed );
    if( this->GetTextWidth( aStrFast ) > aSize.Width() )
        aSize.Width() = this->GetTextWidth( aStrFast );
    aSize.Width() += 30; // Wegen DropDown-Button
    aSize.Height() *= 5; // Anzahl Eintraege + ListBox (2)

    // Child-Windows (Controls) setzen
    //aLbSpeed.SetPosPixel( Point( 0, 0 ) );
    aLbSpeed.SetSizePixel( aSize ); //( 90, 60 ) );
    aLbSpeed.InsertEntry( aStrSlow );
    aLbSpeed.InsertEntry( aStrMed );
    aLbSpeed.InsertEntry( aStrFast );
    aLbSpeed.SelectEntryPos( 0 );
    aLbSpeed.SetHelpId( SID_DIA_SPEED );
    aLbSpeed.Show();

    //SetSizePixel( Size( 95, 20 ) );
    SetSizePixel( aLbSpeed.GetSizePixel() );
    Show();

    aLbSpeed.SetSelectHdl( LINK( this, DiaSpeedControl, SelectDiaSpeedHdl ) );

    SelectDiaSpeedHdl( NULL );
}

//------------------------------------------------------------------------

DiaSpeedControl::~DiaSpeedControl()
{
}




void DiaSpeedControl::GetFocus (void)
{
    Window::GetFocus();
    // Grab the focus to the slide speed control so that it can be
    // controlled with the keyboard.
    aLbSpeed.GrabFocus();
}




//------------------------------------------------------------------------

IMPL_LINK( DiaSpeedControl, SelectDiaSpeedHdl, void *, p )
{
    FadeSpeed eFS = (FadeSpeed) aLbSpeed.GetSelectEntryPos();
    DiaSpeedItem aDiaSpeedItem( eFS );

    if( p )
    {
        ::uno::Any a;
        ::uno::Sequence< ::beans::PropertyValue > aArgs( 1 );
        aArgs[0].Name   = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DiaSpeed" ));
        aDiaSpeedItem.QueryValue( a );
        aArgs[0].Value  = a;
        SfxToolBoxControl::Dispatch( ::uno::Reference< ::frame::XDispatchProvider >( m_xFrame->getController(), ::uno::UNO_QUERY ),
                                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DiaSpeed" )),
                                     aArgs );
//      rBindings.GetDispatcher()->Execute(
//          SID_DIA, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aDiaSpeedItem, (void*) NULL, 0L );
    }
    return( 0L );
}

SFX_IMPL_TOOLBOX_CONTROL( SdTbxCtlDiaAuto, DiaAutoItem )

/*************************************************************************
|*
|* Konstruktor fuer DiaAutoControl
|*
\************************************************************************/

DiaAutoControl::DiaAutoControl( Window* pParent,
                                const uno::Reference< frame::XFrame >& rFrame,
                                WinBits nStyle ) :
        Window          ( pParent, nStyle ),
        m_xFrame        ( rFrame ),
        aFtDescr        ( this, 0 ),
        aLbAuto         ( this, WinBits( WB_BORDER | WB_DROPDOWN ) )
{
    // Child-Windows (Controls) setzen
    String  aStr( SdResId( STR_CHANGE ) );
    Size    aSize( this->GetTextWidth( aStr )+6, this->GetTextHeight()+2 );



    // Child-Windows (Controls) setzen
    aFtDescr.SetPosPixel( Point( 0, 4 ) );
    aFtDescr.SetSizePixel( aSize ); //( 55, 14 ) );
    aFtDescr.SetText( aStr );
    aFtDescr.Show();

    // Listbox
    String  aStrMan( SdResId( STR_MANUEL ) );
    String  aStrSemi( SdResId( STR_SEMIAUTO ) );
    String  aStrAuto( SdResId( STR_AUTO ) );
    Size    aLbSize( this->GetTextWidth( aStrMan ), this->GetTextHeight() );
    if( this->GetTextWidth( aStrAuto ) > aSize.Width() )
        aLbSize.Width() = this->GetTextWidth( aStrAuto );
    aLbSize.Width() += 30; // Wegen DropDown-Button
    aLbSize.Height() *= 5; // Anzahl Eintraege + ListBox (2)

    aLbAuto.SetPosPixel( Point( aSize.Width(), 0 ) );
    aLbAuto.SetSizePixel( aLbSize ); //( 105, 52 ) );
    aLbAuto.InsertEntry( aStrMan );
    aLbAuto.InsertEntry( aStrSemi );
    aLbAuto.InsertEntry( aStrAuto );
    aLbAuto.SelectEntryPos( 0 );
    aLbAuto.SetHelpId( SID_DIA_AUTO );
    aLbAuto.Show();

    //SetSizePixel( Size( 175, 20 ) );
    Size aDescSize = aFtDescr.GetSizePixel();
    Size aAttrSize = aLbAuto.GetSizePixel();
    Point aAttrPnt = aLbAuto.GetPosPixel();
    SetSizePixel( Size( aAttrPnt.X() + aAttrSize.Width(),
                        Max( aAttrSize.Height(), aDescSize.Height() ) ) );
    Show();

    aLbAuto.SetSelectHdl( LINK( this, DiaAutoControl, SelectDiaAutoHdl ) );

    SelectDiaAutoHdl( NULL );
}

//------------------------------------------------------------------------

DiaAutoControl::~DiaAutoControl()
{
}




void DiaAutoControl::GetFocus (void)
{
    Window::GetFocus();
    // Grab the focus to the slide auto transition control so that it can be
    // controlled with the keyboard.
    aLbAuto.GrabFocus();
}




//------------------------------------------------------------------------

IMPL_LINK( DiaAutoControl, SelectDiaAutoHdl, void *, p )
{
    PresChange  eChange;
    const ULONG nPos = aLbAuto.GetSelectEntryPos();

    if( !nPos )
        eChange = PRESCHANGE_MANUAL;
    else if( 1 == nPos )
        eChange = PRESCHANGE_SEMIAUTO;
    else
        eChange = PRESCHANGE_AUTO;

    DiaAutoItem aDiaAutoItem( eChange );

/*
    if( bAuto )
    {
    // Time-Control disablen
    }
    else
    {
    // Time-Control enablen; ModifyHdl rufen
    }
*/

    if( p )
    {
        ::uno::Any a;
        ::uno::Sequence< ::beans::PropertyValue > aArgs( 1 );
        aArgs[0].Name   = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DiaAuto" ));
        aDiaAutoItem.QueryValue( a );
        aArgs[0].Value  = a;
        SfxToolBoxControl::Dispatch( ::uno::Reference< ::frame::XDispatchProvider >( m_xFrame->getController(), ::uno::UNO_QUERY ),
                                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DiaAuto" )),
                                     aArgs );

//        rBindings.GetDispatcher()->Execute(
//          SID_DIA, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aDiaAutoItem, (void*) NULL, 0L );
    }
    return( 0L );
}

SFX_IMPL_TOOLBOX_CONTROL( SdTbxCtlDiaTime, DiaTimeItem )

/*************************************************************************
|*
|* Konstruktor fuer DiaTimeControl
|*
\************************************************************************/

DiaTimeControl::DiaTimeControl( Window* pParent, const uno::Reference< frame::XFrame >& rFrame, WinBits nStyle ) :
        TimeField   ( pParent, nStyle ),
        m_xFrame    ( rFrame )
{
    SetModifyHdl( LINK( this, DiaTimeControl, ModifyDiaTimeHdl ) );
    SetGetFocusHdl( LINK( this, DiaTimeControl, GetFocusHdl ) );

    UniString aStrTmp( RTL_CONSTASCII_STRINGPARAM( "00:00:00" ), RTL_TEXTENCODING_ASCII_US );
    Size aSize( GetTextWidth( aStrTmp ) +30, GetTextHeight() +6 );


    SetSizePixel( aSize ); //( 75, 20 ) );
    SetFormat( TIMEF_SEC );
    SetMin( 0L );
    SetMax( Time( 12, 00, 00 ) );
    SetDuration( TRUE );
    Show();

    ModifyDiaTimeHdl( NULL );
}

/*************************************************************************
|*
|* 2. Konstruktor fuer DiaTimeControl
|*
\************************************************************************/

DiaTimeControl::DiaTimeControl( Window* pParent, const uno::Reference< frame::XFrame >& rFrame, ResId nRId ) :
        TimeField       ( pParent, nRId ),
        m_xFrame        ( rFrame )
{
    SetGetFocusHdl( LINK( this, DiaTimeControl, GetFocusHdl ) );
}

//------------------------------------------------------------------------

DiaTimeControl::~DiaTimeControl()
{
}

//------------------------------------------------------------------------

IMPL_LINK( DiaTimeControl, ModifyDiaTimeHdl, void *, p )
{
    Time aTime = GetTime();
    ULONG lTime = aTime.GetSec() + aTime.GetMin() * 60 + aTime.GetHour() * 3600;
    DiaTimeItem aDiaTimeItem( lTime );

    if( p )
    {
        ::uno::Any a;
        ::uno::Sequence< ::beans::PropertyValue > aArgs( 1 );
        aArgs[0].Name   = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DiaTime" ));
        aDiaTimeItem.QueryValue( a );
        aArgs[0].Value  = a;
        SfxToolBoxControl::Dispatch( ::uno::Reference< ::frame::XDispatchProvider >( m_xFrame->getController(), ::uno::UNO_QUERY ),
                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:DiaTime" )),
                                    aArgs );
//      rBindings.GetDispatcher()->Execute(
//          SID_DIA, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aDiaTimeItem, (void*) NULL, 0L );
    }

    return( 0L );
}

//------------------------------------------------------------------------

IMPL_LINK_INLINE_START( DiaTimeControl, GetFocusHdl, void *, p )
{
    SetSelection( GetText().Len() );

    return( 0L );
}
IMPL_LINK_INLINE_END( DiaTimeControl, GetFocusHdl, void *, p )

//------------------------------------------------------------------------

void DiaTimeControl::Down()
{
    if( IsEmptyTime() )
    {
        SetTime( Time( 0, 0, 0 ) );
        Reformat();
        SetSelection( GetText().Len() );
        Modify();
    }
    else
        TimeField::Down();
}

//------------------------------------------------------------------------

void DiaTimeControl::Up()
{
    if( IsEmptyTime() )
    {
        SetTime( Time( 0, 0, 0 ) );
        Reformat();
        SetSelection( GetText().Len() );
    }
    TimeField::Up();
}

//========================================================================
// SdPagesField
//========================================================================

SdPagesField::SdPagesField( Window* pParent,
                            const uno::Reference< frame::XFrame >& rFrame,
                            WinBits nBits ) :
    SvxMetricField  ( pParent, rFrame, nBits ),
    m_xFrame        ( rFrame )
{
    String aStr( SdResId( STR_SLIDE_PLURAL ) );
    SetCustomUnitText( aStr );

    // Groesse setzen
    aStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "XXX" ) );
    Size aSize( GetTextWidth( aStr )+20, GetTextHeight()+6 );


    SetSizePixel( aSize );

    // Parameter des MetricFields setzen
    SetUnit( FUNIT_CUSTOM );
    SetMin( 1 );
    SetFirst( 1 );
    SetMax( 15 );
    SetLast( 15 );
    SetSpinSize( 1 );
    SetDecimalDigits( 0 );
    Show();
}

// -----------------------------------------------------------------------

SdPagesField::~SdPagesField()
{
}

// -----------------------------------------------------------------------

void SdPagesField::Update( const SfxUInt16Item* pItem )
{
    if( pItem )
    {
        long nValue = (long) pItem->GetValue();
        SetValue( nValue );
        if( nValue == 1 )
            SetCustomUnitText( String( SdResId( STR_SLIDE_SINGULAR ) ) );
        else
            SetCustomUnitText( String( SdResId( STR_SLIDE_PLURAL ) ) );
    }
    else
        SetText( String() );
}

// -----------------------------------------------------------------------

void SdPagesField::Modify()
{
    SfxUInt16Item aItem( SID_PAGES_PER_ROW, (UINT16) GetValue() );

    ::uno::Any a;
    ::uno::Sequence< ::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name   = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PagesPerRow" ));
    aItem.QueryValue( a );
    aArgs[0].Value  = a;
    SfxToolBoxControl::Dispatch( ::uno::Reference< ::frame::XDispatchProvider >( m_xFrame->getController(), ::uno::UNO_QUERY ),
                                 rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PagesPerRow" )),
                                 aArgs );
/*
    rBindings.GetDispatcher()->Execute(
        SID_PAGES_PER_ROW, SFX_CALLMODE_SLOT | SFX_CALLMODE_RECORD, &aItem, 0L, 0L );
*/
}

/*************************************************************************

|*

|* Klassen fuer DiaAttribute

|*
\************************************************************************/

SdTbxCtlDiaEffect::SdTbxCtlDiaEffect( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

/*************************************************************************/

void SdTbxCtlDiaEffect::StateChanged( USHORT nSId,
                        SfxItemState eState, const SfxPoolItem* pState )
{
    // FadeEffecListBox vom Window (DiaEffectControl) holen
    FadeEffectLB* pFadeEffectLB = (FadeEffectLB*) &( ( (DiaEffectControl*)
                GetToolBox().GetItemWindow( GetId() ) )->aLbEffect );
    if ( pFadeEffectLB )
    {
        if( pState )
        {
            pFadeEffectLB->Enable();
            if ( IsInvalidItem( pState ) )
            {
                pFadeEffectLB->SetNoSelection();
            }
            else
            {
                presentation::FadeEffect eFE = (presentation::FadeEffect) ( (const DiaEffectItem*) pState )->GetValue();
                pFadeEffectLB->SelectEffect( eFE );
            }
        }
        else
        {
            pFadeEffectLB->Disable();
            pFadeEffectLB->SetNoSelection();
        }
    }
}

/*************************************************************************/

Window* SdTbxCtlDiaEffect::CreateItemWindow( Window *pParent )
{
    if( GetSlotId() == SID_DIA_EFFECT )
    {
        return( new DiaEffectControl( pParent, m_xFrame ) );
    }

    return( NULL );
}

/*************************************************************************/
/*************************************************************************/

SdTbxCtlDiaSpeed::SdTbxCtlDiaSpeed( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

/*************************************************************************/

void SdTbxCtlDiaSpeed::StateChanged( USHORT nSId,
                        SfxItemState eState, const SfxPoolItem* pState )
{
    // ListBox vom Window (DiaSpeedControl) holen
    ListBox* pLB = (ListBox*) &( ( (DiaSpeedControl*)
                GetToolBox().GetItemWindow( GetId() ) )->aLbSpeed );
    if ( pLB )
    {
        if( pState )
        {
            pLB->Enable();
            if ( IsInvalidItem( pState ) )
            {
                pLB->SetNoSelection();
            }
            else
            {
                FadeSpeed eFS = (FadeSpeed) ( (const DiaSpeedItem*) pState )->GetValue();
                pLB->SelectEntryPos( eFS );
            }
        }
        else
        {
            pLB->Disable();
            pLB->SetNoSelection();
        }
    }
}

/*************************************************************************/

Window* SdTbxCtlDiaSpeed::CreateItemWindow( Window *pParent )
{
    if( GetSlotId() == SID_DIA_SPEED )
    {
        return( new DiaSpeedControl( pParent, m_xFrame ) );
    }

    return( NULL );
}

/*************************************************************************/
/*************************************************************************/

SdTbxCtlDiaAuto::SdTbxCtlDiaAuto( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

/*************************************************************************/

void SdTbxCtlDiaAuto::StateChanged( USHORT nSId,
                        SfxItemState eState, const SfxPoolItem* pState )
{
    // ListBox vom Window (DiaAutoControl) holen
    ListBox* pLB = (ListBox*) &( ( (DiaAutoControl*)
                GetToolBox().GetItemWindow( GetId() ) )->aLbAuto );
    if ( pLB )
    {
        if( pState )
        {
            pLB->Enable();
            if ( IsInvalidItem( pState ) )
            {
                pLB->SetNoSelection();
            }
            else
            {
                const PresChange eChange = ( (const DiaAutoItem*) pState )->GetValue();

                if( eChange == PRESCHANGE_MANUAL )
                    pLB->SelectEntryPos( 0 ); // Manuell
                else if( eChange == PRESCHANGE_SEMIAUTO )
                    pLB->SelectEntryPos( 1 ); // Halbautomatisch
                else
                    pLB->SelectEntryPos( 2 ); // Automatisch
            }
        }
        else
        {
            pLB->Disable();
            pLB->SetNoSelection();
        }
    }
}

/*************************************************************************/

Window* SdTbxCtlDiaAuto::CreateItemWindow( Window *pParent )
{
    if( GetSlotId() == SID_DIA_AUTO )
    {
        return( new DiaAutoControl( pParent, m_xFrame ) );
    }

    return( NULL );
}

/*************************************************************************/
/*************************************************************************/

SdTbxCtlDiaTime::SdTbxCtlDiaTime( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

/*************************************************************************/

void SdTbxCtlDiaTime::StateChanged( USHORT nSId,
                        SfxItemState eState, const SfxPoolItem* pState )
{
    DiaTimeControl* pDTC = (DiaTimeControl*) GetToolBox().GetItemWindow( GetId() ); //SID_DIA_TIME );

    if ( pDTC )
    {
        if( pState )
        {
            pDTC->Enable();
            if ( IsInvalidItem( pState ) )
            {
                pDTC->SetEmptyTime();
            }
            else
            {
                Selection* pSelection = NULL;
                if( pDTC->HasChildPathFocus() )
                    pSelection = new Selection( pDTC->GetSelection() );

                ULONG lTime = ( ( const DiaTimeItem* ) pState )->GetValue();
                USHORT  nHour = (USHORT) ( lTime / 3600 );
                USHORT  nMin  = (USHORT) ( ( lTime % 3600 ) / 60 );
                USHORT  nSec  = (USHORT) ( ( lTime % 3600 ) % 60 );
                Time aTime( nHour, nMin, nSec);

                pDTC->SetTime( aTime );

                if( pSelection )
                {
                    pDTC->SetSelection( *pSelection );
                    delete pSelection;
                }
            }
        }
        else
        {
            pDTC->Disable();
            pDTC->SetEmptyTime();
        }
    }
}

/*************************************************************************/

Window* SdTbxCtlDiaTime::CreateItemWindow( Window *pParent )
{
    if( GetSlotId() == SID_DIA_TIME )
    {
        return( new DiaTimeControl( pParent, m_xFrame,
                WinBits( WB_BORDER | WB_SPIN | WB_REPEAT ) ) );
    }

    return( NULL );
}

/*************************************************************************
|*
|* SdTbxCtlDiaPages
|*
\************************************************************************/

SdTbxCtlDiaPages::SdTbxCtlDiaPages( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

//========================================================================

SdTbxCtlDiaPages::~SdTbxCtlDiaPages()
{
}

//========================================================================

void SdTbxCtlDiaPages::StateChanged( USHORT nSID,
                SfxItemState eState, const SfxPoolItem* pState )
{
    SdPagesField* pFld = (SdPagesField*) GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pFld, "Window not found" );

    if ( eState == SFX_ITEM_DISABLED )
    {
        pFld->Disable();
        pFld->SetText( String() );
    }
    else
    {
        pFld->Enable();

        if ( eState == SFX_ITEM_AVAILABLE )
        {
            DBG_ASSERT( pState->ISA(SfxUInt16Item), "falscher ItemType" )
            pFld->Update( (const SfxUInt16Item*) pState );
        }
        else
            pFld->Update( NULL );
    }
}

//========================================================================

Window* SdTbxCtlDiaPages::CreateItemWindow( Window* pParent )
{
    return new SdPagesField( pParent, m_xFrame );
}



