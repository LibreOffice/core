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

::std::function<void (const css::awt::Rectangle& rRepaintBox)>
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
    if (pDescriptor.get()==nullptr || ! pDescriptor->mbIsOpaque)
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
    if (pDescriptor.get()==nullptr || ! pDescriptor->mbIsOpaque)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
