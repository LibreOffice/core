/*************************************************************************
 *
 *  $RCSfile: unowrapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-15 11:43:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
};

#endif // _TOOLKIT_HELPER_UNOWRAPPER_HXX_

