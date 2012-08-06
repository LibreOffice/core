/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _VCL_UNOWRAP_HXX
#define _VCL_UNOWRAP_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <com/sun/star/uno/Reference.h>

class XWindowPeer;
class XToolkit;
class EventList;
class Window;
class OutputDevice;
class MouseEvent;
class CommandEvent;
class KeyEvent;
class Rectangle;
class Menu;

namespace com {
namespace sun {
namespace star {
namespace awt {
    class XGraphics;
    class XToolkit;
    class XWindowPeer;
}
namespace lang {
    class XMultiServiceFactory;
}
} } }

namespace com {
namespace sun {
namespace star {
namespace accessibility {
    class XAccessible;
}}}}

class VCL_DLLPUBLIC UnoWrapperBase
{
public:
    virtual void                Destroy() = 0;

    // Toolkit
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > GetVCLToolkit() = 0;

    // Graphics
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >    CreateGraphics( OutputDevice* pOutDev ) = 0;
    virtual void                ReleaseAllGraphics( OutputDevice* pOutDev ) = 0;

    // Window
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer> GetWindowInterface( Window* pWindow, sal_Bool bCreate ) = 0;
    virtual void                SetWindowInterface( Window* pWindow, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xIFace ) = 0;

    virtual void                WindowDestroyed( Window* pWindow ) = 0;

    // Accessibility
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                CreateAccessible( Menu* pMenu, sal_Bool bIsMenuBar ) = 0;

protected:
    ~UnoWrapperBase() {}
};

#endif  // _VCL_UNOWRAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
