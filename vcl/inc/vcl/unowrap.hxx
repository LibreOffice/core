/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unowrap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:15:39 $
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

#ifndef _VCL_UNOWRAP_HXX
#define _VCL_UNOWRAP_HXX

#include <tools/solar.h>

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

class XWindowPeer;
class XToolkit;
class XVclToolkit;
class EventList;
class Window;
class OutputDevice;
class MouseEvent;
class CommandEvent;
class KeyEvent;
class Rectangle;
class XVclComponentPeer;
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
};

#endif  // _VCL_UNOWRAP_HXX
