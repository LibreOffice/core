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

#ifndef INCLUDED_TOOLKIT_INC_HELPER_UNOWRAPPER_HXX
#define INCLUDED_TOOLKIT_INC_HELPER_UNOWRAPPER_HXX

#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>

#include <vcl/unowrap.hxx>
#include <vcl/window.hxx>

#include "helper/accessibilityclient.hxx"

//  class UnoWrapper


class UnoWrapper : public UnoWrapperBase
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>  mxToolkit;
    ::toolkit::AccessibilityClient                                      maAccessibleFactoryAccess;

public:
    UnoWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>& rxToolkit );

    virtual void        Destroy() SAL_OVERRIDE;

    // Toolkit
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit> GetVCLToolkit() SAL_OVERRIDE;

    // Graphics
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics> CreateGraphics( OutputDevice* pOutDev ) SAL_OVERRIDE;
    virtual void                ReleaseAllGraphics( OutputDevice* pOutDev ) SAL_OVERRIDE;

    // Window
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> GetWindowInterface( Window* pWindow, sal_Bool bCreate ) SAL_OVERRIDE;
    virtual void                SetWindowInterface( Window* pWindow, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> xIFace ) SAL_OVERRIDE;

    void                WindowDestroyed( Window* pWindow ) SAL_OVERRIDE;
    void                WindowEvent_Move( Window* pWindow );
    void                WindowEvent_Resize( Window* pWindow );
    void                WindowEvent_Show( Window* pWindow, bool bShow );
    void                WindowEvent_Close( Window* pWindow );
    void                WindowEvent_Minimize( Window* pWindow );
    void                WindowEvent_Normalize( Window* pWindow );
    void                WindowEvent_Activate( Window* pWindow, bool bActivated );
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
                        CreateAccessible( Menu* pMenu, sal_Bool bIsMenuBar ) SAL_OVERRIDE;

private:
    virtual ~UnoWrapper();
};

#endif // INCLUDED_TOOLKIT_INC_HELPER_UNOWRAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
