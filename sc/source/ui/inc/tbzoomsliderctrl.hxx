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
#ifndef _SC_ZOOMSLIDERTBCONTRL_HXX
#define _SC_ZOOMSLIDERTBCONTRL_HXX

#ifndef _WINDOW_HXX        //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX   //autogen
#include <svl/poolitem.hxx>
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
    ScZoomSliderControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx );
    ~ScZoomSliderControl();

    virtual void    StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState );
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

    sal_uInt16          Offset2Zoom( long nOffset ) const;
    long            Zoom2Offset( sal_uInt16 nZoom ) const;
    void            DoPaint( const Rectangle& rRect );

public:
    ScZoomSliderWnd( Window* pParent, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProvider >& rDispatchProvider,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _xFrame , sal_uInt16 nCurrentZoom );
    ~ScZoomSliderWnd();
    void            UpdateFromItem( const SvxZoomSliderItem* pZoomSliderItem );

protected:
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Paint( const Rectangle& rRect );
};
#endif
