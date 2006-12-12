/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: diactrl.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:00:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

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

SFX_IMPL_TOOLBOX_CONTROL( SdTbxCtlDiaPages,  SfxUInt16Item )


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

void SdPagesField::UpdatePagesField( const SfxUInt16Item* pItem )
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

void SdTbxCtlDiaPages::StateChanged( USHORT,
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

        const SfxUInt16Item* pItem = 0;
        if ( eState == SFX_ITEM_AVAILABLE )
        {
            pItem = dynamic_cast< const SfxUInt16Item* >( pState );
            DBG_ASSERT( pItem, "sd::SdTbxCtlDiaPages::StateChanged(), wrong item type!" );
        }

        pFld->UpdatePagesField( pItem );
    }
}

//========================================================================

Window* SdTbxCtlDiaPages::CreateItemWindow( Window* pParent )
{
    return new SdPagesField( pParent, m_xFrame );
}



