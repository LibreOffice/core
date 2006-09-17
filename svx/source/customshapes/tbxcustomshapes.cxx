/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbxcustomshapes.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:08:07 $
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
#include "precompiled_svx.hxx"

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _SVX_SVXIDS_HRC
#include <svxids.hrc>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <sfx2/imagemgr.hxx>
#include <vcl/svapp.hxx>

#ifndef _SVX_TBXCUSTOMSHAPES_HXX
#include "tbxcustomshapes.hxx"
#endif

SFX_IMPL_TOOLBOX_CONTROL(SvxTbxCtlCustomShapes, SfxBoolItem);

/*************************************************************************
|*
|*
|*
\************************************************************************/

SvxTbxCtlCustomShapes::SvxTbxCtlCustomShapes( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    m_aSubTbxResName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/" ) ),
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    switch( nSlotId )
    {
        default :
        {
            DBG_ASSERT( false, "SvxTbxCtlCustomShapes: unknown slot executed. ?" );
        }
        case SID_DRAWTBX_CS_BASIC :
        {
            m_aCommand = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:BasicShapes.diamond" ) );
            m_aSubTbName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "basicshapes" ) );
        }
        break;

        case SID_DRAWTBX_CS_SYMBOL :
        {
            m_aCommand = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:SymbolShapes.smiley" ) );
            m_aSubTbName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "symbolshapes" ) );
        }
        break;

        case SID_DRAWTBX_CS_ARROW :
        {
            m_aCommand = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ArrowShapes.left-right-arrow" ) );
            m_aSubTbName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "arrowshapes" ) );
        }
        break;
        case SID_DRAWTBX_CS_FLOWCHART :
        {
            m_aCommand = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FlowChartShapes.flowchart-internal-storage" ) );
            m_aSubTbName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "flowchartshapes" ) );
        }
        break;
        case SID_DRAWTBX_CS_CALLOUT :
        {
            m_aCommand = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CalloutShapes.round-rectangular-callout" ) );
            m_aSubTbName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "calloutshapes" ) );
        }
        break;
        case SID_DRAWTBX_CS_STAR :
        {
            m_aCommand = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:StarShapes.star5" ) );
            m_aSubTbName = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "starshapes" ) );
        }
        break;
    }
    m_aSubTbxResName += m_aSubTbName;
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

/*************************************************************************
|*
|* Benachrichtigung, wenn sich der Applikationsstatus geaendert hat
|*
\************************************************************************/

void SvxTbxCtlCustomShapes::StateChanged( USHORT nSID, SfxItemState eState,
                                  const SfxPoolItem* pState )
{
    SfxToolBoxControl::StateChanged( nSID, eState, pState );
}

/*************************************************************************
|*
|* Wenn man ein PopupWindow erzeugen will
|*
\************************************************************************/

SfxPopupWindowType SvxTbxCtlCustomShapes::GetPopupWindowType() const
{
    return( m_aCommand.getLength() == 0 ? SFX_POPUPWINDOW_ONCLICK : SFX_POPUPWINDOW_ONTIMEOUT);
}

/*************************************************************************
|*
|* Hier wird das Fenster erzeugt
|* Lage der Toolbox mit GetToolBox() abfragbar
|* rItemRect sind die Screen-Koordinaten
|*
\************************************************************************/

SfxPopupWindow* SvxTbxCtlCustomShapes::CreatePopupWindow()
{
    createAndPositionSubToolBar( m_aSubTbxResName );
    return NULL;
}

// -----------------------------------------------------------------------

void SvxTbxCtlCustomShapes::Select( BOOL bMod1 )
{
     if ( m_aCommand.getLength() > 0 )
    {
        com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > aParamSeq( 0 );
        Dispatch( m_aCommand, aParamSeq );
    }
}


::sal_Bool SAL_CALL SvxTbxCtlCustomShapes::opensSubToolbar() throw (::com::sun::star::uno::RuntimeException)
{
    // We control a sub-toolbar therefor, we have to return true.
    return sal_True;
}

::rtl::OUString SAL_CALL SvxTbxCtlCustomShapes::getSubToolbarName() throw (::com::sun::star::uno::RuntimeException)
{
    // Provide the controlled sub-toolbar name, so we are notified whenever
    // this toolbar executes a function.
    return m_aSubTbName;
}

void SAL_CALL SvxTbxCtlCustomShapes::functionSelected( const ::rtl::OUString& rCommand ) throw (::com::sun::star::uno::RuntimeException)
{
    // remind the new command
    m_aCommand = rCommand;
    // Our sub-toolbar wants to execute a function.
    // We have to change the image of our toolbar button to reflect the new function.
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( !m_bDisposed )
    {
        if ( m_aCommand.getLength() > 0 )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame( getFrameInterface());
            Image aImage = GetImage( xFrame, m_aCommand, hasBigImages(), isHighContrast() );
            if ( !!aImage )
                GetToolBox().SetItemImage( GetId(), aImage );
        }
    }
}

void SAL_CALL SvxTbxCtlCustomShapes::updateImage(  ) throw (::com::sun::star::uno::RuntimeException)
{
    // We should update the button image of our parent (toolbar).
    // Use the stored command to set the correct current image.
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_aCommand.getLength() > 0 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame( getFrameInterface());
        Image aImage = GetImage( xFrame, m_aCommand, hasBigImages(), isHighContrast() );
        if ( !!aImage )
            GetToolBox().SetItemImage( GetId(), aImage );
    }
}

