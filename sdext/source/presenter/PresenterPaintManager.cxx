/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterPaintManager.cxx,v $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "PresenterPaintManager.hxx"

#include "PresenterPaneContainer.hxx"
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdext { namespace presenter {

PresenterPaintManager::PresenterPaintManager (
    const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
    const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper,
    const rtl::Reference<PresenterPaneContainer>& rpPaneContainer)
    : mxParentWindow(rxParentWindow),
      mxParentWindowPeer(rxParentWindow, UNO_QUERY),
      mxPresenterHelper(rxPresenterHelper),
      mpPaneContainer(rpPaneContainer)
{
}




::boost::function<void(const css::awt::Rectangle& rRepaintBox)>
    PresenterPaintManager::GetInvalidator (
        const css::uno::Reference<css::awt::XWindow>& rxWindow,
        const bool bSynchronous)
{
    return ::boost::bind(
        static_cast<void (PresenterPaintManager::*)(
            const css::uno::Reference<css::awt::XWindow>&,
            const css::awt::Rectangle&,
            const bool)>(&PresenterPaintManager::Invalidate),
        this,
        rxWindow,
        _1,
        bSynchronous);
}




void PresenterPaintManager::Invalidate (
    const css::uno::Reference<css::awt::XWindow>& rxWindow,
    const bool bSynchronous)
{
    sal_Int16 nInvalidateMode (awt::InvalidateStyle::CHILDREN);
    if (bSynchronous)
        nInvalidateMode |= awt::InvalidateStyle::UPDATE;

    PresenterPaneContainer::SharedPaneDescriptor pDescriptor(
        mpPaneContainer->FindContentWindow(rxWindow));
    if (pDescriptor.get()==NULL || ! pDescriptor->mbIsOpaque)
        nInvalidateMode |= awt::InvalidateStyle::TRANSPARENT;
    else
        nInvalidateMode |= awt::InvalidateStyle::NOTRANSPARENT;

    Invalidate(rxWindow, nInvalidateMode);
}




void PresenterPaintManager::Invalidate (
    const css::uno::Reference<css::awt::XWindow>& rxWindow,
    const sal_Int16 nInvalidateFlags)
{
    if ((nInvalidateFlags & awt::InvalidateStyle::TRANSPARENT) != 0)
    {
        // Window is transparent and parent window(s) have to be painted as
        // well.  Invalidate the parent explicitly.
        if (mxPresenterHelper.is() && mxParentWindowPeer.is())
        {
            const awt::Rectangle aBBox (
                mxPresenterHelper->getWindowExtentsRelative(rxWindow, mxParentWindow));
            mxParentWindowPeer->invalidateRect(aBBox, nInvalidateFlags);
        }
    }
    else
    {
        Reference<awt::XWindowPeer> xPeer (rxWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->invalidate(nInvalidateFlags);
    }
}




void PresenterPaintManager::Invalidate (
    const css::uno::Reference<css::awt::XWindow>& rxWindow,
    const css::awt::Rectangle& rRepaintBox,
    const bool bSynchronous)
{
    sal_Int16 nInvalidateMode (awt::InvalidateStyle::CHILDREN);
    if (bSynchronous)
        nInvalidateMode |= awt::InvalidateStyle::UPDATE;

    PresenterPaneContainer::SharedPaneDescriptor pDescriptor(
        mpPaneContainer->FindContentWindow(rxWindow));
    if (pDescriptor.get()==NULL || ! pDescriptor->mbIsOpaque)
        nInvalidateMode |= awt::InvalidateStyle::TRANSPARENT;
    else
        nInvalidateMode |= awt::InvalidateStyle::NOTRANSPARENT;

    Invalidate(rxWindow, rRepaintBox, nInvalidateMode);
}




void PresenterPaintManager::Invalidate (
    const css::uno::Reference<css::awt::XWindow>& rxWindow,
    const css::awt::Rectangle& rRepaintBox,
    const sal_Int16 nInvalidateFlags)
{
    if ((nInvalidateFlags & awt::InvalidateStyle::TRANSPARENT) != 0)
    {
        // Window is transparent and parent window(s) have to be painted as
        // well.  Invalidate the parent explicitly.
        if (mxPresenterHelper.is() && mxParentWindowPeer.is())
        {
            const awt::Rectangle aBBox (
                mxPresenterHelper->getWindowExtentsRelative(rxWindow, mxParentWindow));
            mxParentWindowPeer->invalidateRect(
                awt::Rectangle(
                    rRepaintBox.X + aBBox.X,
                    rRepaintBox.Y + aBBox.Y,
                    rRepaintBox.Width,
                    rRepaintBox.Height),
                nInvalidateFlags);
        }
    }
    else
    {
        Reference<awt::XWindowPeer> xPeer (rxWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->invalidateRect(rRepaintBox, nInvalidateFlags);
    }
}

} }
