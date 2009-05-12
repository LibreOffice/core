/*************************************************************************
*
*  OpenOffice.org - a multi-platform office productivity suite
*
*  $RCSfile: tbzoomsliderctrl.hxx,v $
*
*  $Revision: 1.1 $
*
*  last change: $Author: maoyg $ $Date: 2008/04/25 10:08:08 $
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
#ifndef _SC_ZOOMSLIDERTBCONTRL_HXX
#define _SC_ZOOMSLIDERTBCONTRL_HXX

#ifndef _WINDOW_HXX        //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX   //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SFXTBXCTRL_HXX    //autogen
#include <sfx2/tbxctrl.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#include <svx/zoomslideritem.hxx>


//class ScZoomSliderControl define
class ScZoomSliderControl: public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();
    ScZoomSliderControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx );
    ~ScZoomSliderControl();

    virtual void    StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState );
    virtual Window* CreateItemWindow( Window *pParent );
};

//========================================================================
// class ScZoomSliderWnd define---------------------------------------
//========================================================================
class ScZoomSliderWnd: public Window
{
private:
    struct                  ScZoomSliderWnd_Impl;
    ScZoomSliderWnd_Impl*   mpImpl;
    Size                    aLogicalSize;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider > m_xDispatchProvider;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >            m_xFrame;

    USHORT          Offset2Zoom( long nOffset ) const;
    long            Zoom2Offset( USHORT nZoom ) const;
    void            DoPaint( const Rectangle& rRect );

public:
    ScZoomSliderWnd( Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame , USHORT nCurrentZoom );
    ~ScZoomSliderWnd();
    void            UpdateFromItem( const SvxZoomSliderItem* pZoomSliderItem );

protected:
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Paint( const Rectangle& rRect );
};
#endif
