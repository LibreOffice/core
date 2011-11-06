/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
