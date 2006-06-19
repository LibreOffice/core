/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unowrapper.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 22:57:32 $
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

#ifndef _TOOLKIT_HELPER_UNOWRAPPER_HXX_
#define _TOOLKIT_HELPER_UNOWRAPPER_HXX_

#ifndef _COM_SUN_STAR_AWT_XTOOLKIT_HPP_
#include <com/sun/star/awt/XToolkit.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XGRAPHICS_HPP_
#include <com/sun/star/awt/XGraphics.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif

#include <vcl/unowrap.hxx>
#include <vcl/window.hxx>

//  ----------------------------------------------------
//  class UnoWrapper
//  ----------------------------------------------------

class UnoWrapper : public UnoWrapperBase
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit> mxToolkit;

public:
    UnoWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>& rxToolkit );
    virtual ~UnoWrapper();

    void                                Destroy();

    // Toolkit
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit> GetVCLToolkit();

    // Graphics
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics> CreateGraphics( OutputDevice* pOutDev );
    virtual void                ReleaseAllGraphics( OutputDevice* pOutDev );

    // Window
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> GetWindowInterface( Window* pWindow, BOOL bCreate );
    virtual void                SetWindowInterface( Window* pWindow, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> xIFace );

    void                WindowDestroyed( Window* pWindow );
    void                WindowEvent_Move( Window* pWindow );
    void                WindowEvent_Resize( Window* pWindow );
    void                WindowEvent_Show( Window* pWindow, BOOL bShow );
    void                WindowEvent_Close( Window* pWindow );
    void                WindowEvent_Minimize( Window* pWindow );
    void                WindowEvent_Normalize( Window* pWindow );
    void                WindowEvent_Activate( Window* pWindow, BOOL bActivated );
    void                WindowEvent_MouseButtonUp( Window* pWindow, const MouseEvent& rEvt );
    void                WindowEvent_MouseButtonDown( Window* pWindow, const MouseEvent& rEvt );
    void                WindowEvent_MouseMove( Window* pWindow, const MouseEvent& rEvt );
    void                WindowEvent_Command( Window* pWindow, const CommandEvent& rCEvt );
    void                WindowEvent_KeyInput( Window* pWindow, const KeyEvent& rEvt );
    void                WindowEvent_KeyUp( Window* pWindow, const KeyEvent& rEvt );
    void                WindowEvent_GetFocus( Window* pWindow );
    void                WindowEvent_LoseFocus( Window* pWindow );
    void                WindowEvent_Paint( Window* pWindow, const Rectangle& rRect );

    // Accessibility
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                CreateAccessible( Menu* pMenu, sal_Bool bIsMenuBar );
};

#endif // _TOOLKIT_HELPER_UNOWRAPPER_HXX_

