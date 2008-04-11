/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FullScreenPane.cxx,v $
 *
 * $Revision: 1.3 $
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

#include "precompiled_sd.hxx"

#include "FullScreenPane.hxx"
#include "ViewShellBase.hxx"
#include <cppcanvas/vclfactory.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/topfrm.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/util/URL.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sd { namespace framework {

FullScreenPane::FullScreenPane (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<XResourceId>& rxPaneId,
    const ::Window* pViewShellWindow)
    : FrameWindowPane(rxPaneId,NULL),
      mxComponentContext(rxComponentContext),
      mpWorkWindow(new WorkWindow(NULL, 0))
{
    if ( ! rxPaneId.is())
        throw lang::IllegalArgumentException();

    sal_Int32 nScreenNumber = 1;
    ExtractArguments(rxPaneId, nScreenNumber);

    if (mpWorkWindow.get() == NULL)
        return;

    // Create a new top-leve window that is displayed full screen.
    mpWorkWindow->ShowFullScreenMode(TRUE, nScreenNumber);
    mpWorkWindow->SetMenuBarMode(MENUBAR_MODE_HIDE);
    mpWorkWindow->SetBorderStyle(WINDOW_BORDER_REMOVEBORDER);
    mpWorkWindow->SetBackground(Wallpaper());
    mpWorkWindow->Show();

    // Set title and icon of the new window to those of the current window
    // of the view shell.
    if (pViewShellWindow != NULL)
    {
        const SystemWindow* pSystemWindow = pViewShellWindow->GetSystemWindow();
        mpWorkWindow->SetText(pSystemWindow->GetText());
        mpWorkWindow->SetIcon(pSystemWindow->GetIcon());
    }

    // For some reason the VCL canvas can not paint into a WorkWindow.
    // Therefore a child window is created that covers the WorkWindow
    // completely.
    mpWindow = new ::Window(mpWorkWindow.get());
    mpWindow->SetPosSizePixel(Point(0,0), mpWorkWindow->GetSizePixel());
    mpWindow->Show();
    mpWindow->SetBackground(Wallpaper());
    mxWindow = VCLUnoHelper::GetInterface(mpWindow);

    // Create the canvas.
    mxCanvas = CreateCanvas();

    mpWindow->GrabFocus();
}




FullScreenPane::~FullScreenPane (void) throw()
{
}




void SAL_CALL FullScreenPane::disposing (void)
{
    // We have created the window pointed to by mpWindow, we delete it.
    if (mpWindow != NULL)
    {
        delete mpWindow;
    }

    mpWorkWindow.reset();

    FrameWindowPane::disposing();
}




//-----------------------------------------------------------------------------

Reference<rendering::XCanvas> FullScreenPane::CreateCanvas (void)
    throw (RuntimeException)
{
    ::Window* pWindow = VCLUnoHelper::GetWindow(mxWindow);
    if (pWindow != NULL)
    {
        Sequence<Any> aArg (5);

        // common: first any is VCL pointer to window (for VCL canvas)
        aArg[0] = makeAny(reinterpret_cast<sal_Int64>(pWindow));
        aArg[1] = Any();
        aArg[2] = makeAny(::com::sun::star::awt::Rectangle());
        aArg[3] = makeAny(sal_False);
        aArg[4] = makeAny(mxWindow);

        Reference<lang::XMultiServiceFactory> xFactory (
            mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
        return Reference<rendering::XCanvas>(
            xFactory->createInstanceWithArguments(
                OUString::createFromAscii("com.sun.star.rendering.VCLCanvas"),
                aArg),
            UNO_QUERY);
    }
    else
        throw RuntimeException();
}




void FullScreenPane::ExtractArguments (
    const Reference<XResourceId>& rxPaneId,
    sal_Int32& rnScreenNumberReturnValue)
{
    // Extract arguments from the resource URL.
    const util::URL aURL = rxPaneId->getFullResourceURL();
    sal_Int32 nIndex = 0;
    while (nIndex >= 0)
    {
        const OUString aToken = aURL.Arguments.getToken(0, '&', nIndex);
        if (aToken.getLength() > 0)
        {
            // Split at the first '='.
            const sal_Int32 nAssign = aToken.indexOf('=');
            const OUString sKey = aToken.copy(0, nAssign);
            const OUString sValue = aToken.copy(nAssign+1);

            if (sKey.compareToAscii("ScreenNumber") == 0)
            {
                rnScreenNumberReturnValue = sValue.toInt32();
            }
        }
    }
}


} } // end of namespace sd::framework
