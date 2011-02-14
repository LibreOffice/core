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

#ifndef _TOOLKIT_HELPER_UNOWRAPPER_HXX_
#define _TOOLKIT_HELPER_UNOWRAPPER_HXX_

#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <toolkit/helper/accessibilityclient.hxx>

#include <vcl/unowrap.hxx>
#include <vcl/window.hxx>

//  ----------------------------------------------------
//  class UnoWrapper
//  ----------------------------------------------------

class UnoWrapper : public UnoWrapperBase
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>  mxToolkit;
    ::toolkit::AccessibilityClient                                      maAccessibleFactoryAccess;

public:
    UnoWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>& rxToolkit );

    virtual void        Destroy();

    // Toolkit
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit> GetVCLToolkit();

    // Graphics
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics> CreateGraphics( OutputDevice* pOutDev );
    virtual void                ReleaseAllGraphics( OutputDevice* pOutDev );

    // Window
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> GetWindowInterface( Window* pWindow, sal_Bool bCreate );
    virtual void                SetWindowInterface( Window* pWindow, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> xIFace );

    void                WindowDestroyed( Window* pWindow );
    void                WindowEvent_Move( Window* pWindow );
    void                WindowEvent_Resize( Window* pWindow );
    void                WindowEvent_Show( Window* pWindow, sal_Bool bShow );
    void                WindowEvent_Close( Window* pWindow );
    void                WindowEvent_Minimize( Window* pWindow );
    void                WindowEvent_Normalize( Window* pWindow );
    void                WindowEvent_Activate( Window* pWindow, sal_Bool bActivated );
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

private:
    virtual ~UnoWrapper();
};

#endif // _TOOLKIT_HELPER_UNOWRAPPER_HXX_

