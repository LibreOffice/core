/*************************************************************************
 *
 *  $RCSfile: window.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 14:31:36 $
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

#ifndef __FRAMEWORK_PATTERN_WINDOW_HXX_
#define __FRAMEWORK_PATTERN_WINDOW_HXX_

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_______________________________________________
// interface includes

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

//_______________________________________________
// other includes

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif

#ifndef _SV_SYSWIN_HXX
#include <vcl/syswin.hxx>
#endif

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//_______________________________________________
// namespaces

#ifndef css
namespace css = ::com::sun::star;
#endif

namespace framework{
    namespace pattern{
        namespace window{

//_______________________________________________
// definitions

//-----------------------------------------------
::rtl::OUString getWindowState(const css::uno::Reference< css::awt::XWindow >& xWindow)
{
    if (!xWindow.is())
        return ::rtl::OUString();

    // SOLAR SAFE -> ----------------------------
    ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());

    ByteString sWindowState;
    Window*    pWindow     = VCLUnoHelper::GetWindow(xWindow);
    // check for system window is neccessary to guarantee correct pointer cast!
    if (pWindow!=NULL && pWindow->IsSystemWindow())
    {
        ULONG nMask  = WINDOWSTATE_MASK_ALL;
              nMask &= ~(WINDOWSTATE_MASK_MINIMIZED);
        sWindowState = ((SystemWindow*)pWindow)->GetWindowState(nMask);
    }

    aSolarGuard.clear();
    // <- SOLAR SAFE ----------------------------

    return B2U_ENC(sWindowState,RTL_TEXTENCODING_UTF8);
}

//-----------------------------------------------
void setWindowState(const css::uno::Reference< css::awt::XWindow >& xWindow     ,
                    const ::rtl::OUString&                          sWindowState)
{
    if (
        (!xWindow.is()            ) ||
        (!sWindowState.getLength())
       )
        return;

    // SOLAR SAFE -> ----------------------------
    ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());

    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    // check for system window is neccessary to guarantee correct pointer cast!
    if (
        (pWindow                  ) &&
        (pWindow->IsSystemWindow()) &&
        (
            // dont overwrite a might existing minimized mode!
            (pWindow->GetType() != WINDOW_WORKWINDOW) ||
            (!((WorkWindow*)pWindow)->IsMinimized() )
        )
       )
    {
        ((SystemWindow*)pWindow)->SetWindowState(U2B_ENC(sWindowState,RTL_TEXTENCODING_UTF8));
    }

    aSolarGuard.clear();
    // <- SOLAR SAFE ----------------------------
}

        } // namespace config
    } // namespace pattern
} // namespace framework

#endif // __FRAMEWORK_PATTERN_WINDOW_HXX_
