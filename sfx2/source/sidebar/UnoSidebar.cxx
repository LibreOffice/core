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

#include <sfx2/sidebar/UnoSidebar.hxx>
#include <sfx2/sidebar/Tools.hxx>

#include <sfx2/sidebar/ResourceManager.hxx>

#include <sfx2/sidebar/UnoDecks.hxx>

#include <com/sun/star/frame/XDispatch.hpp>


using namespace css;
using namespace ::sfx2::sidebar;

using ::com::sun::star::uno::RuntimeException;

SfxUnoSidebar::SfxUnoSidebar(const uno::Reference<frame::XFrame>& rFrame):
xFrame(rFrame)
{
}

SidebarController* SfxUnoSidebar::getSidebarController()
{
    return SidebarController::GetSidebarControllerForFrame(xFrame);
}

void SAL_CALL SfxUnoSidebar::showDecks ( const sal_Bool bVisible )
{
    SidebarController* pSidebarController = getSidebarController();

    if (pSidebarController)
    {
        if (bVisible)
            pSidebarController->RequestOpenDeck();
        else
            pSidebarController->RequestCloseDeck();
    }
}

void SAL_CALL SfxUnoSidebar::setVisible ( const sal_Bool bVisible )
{
    SidebarController* pSidebarController = getSidebarController();

    if ( (bVisible && !pSidebarController) || (!bVisible && pSidebarController) )
    {
        const util::URL aURL (Tools::GetURL(".uno:Sidebar"));
        uno::Reference<frame::XDispatch> mxDispatch (Tools::GetDispatch(xFrame, aURL));
        if (mxDispatch.is())
                mxDispatch->dispatch(aURL, uno::Sequence<beans::PropertyValue>());
    }

}

sal_Bool SAL_CALL SfxUnoSidebar::isVisible()
{
    SidebarController* pSidebarController = getSidebarController();

    return pSidebarController != NULL;
}

uno::Reference<frame::XFrame> SAL_CALL SfxUnoSidebar::getFrame() throw (uno::RuntimeException, std::exception)
{
    if (! xFrame.is() )
        throw uno::RuntimeException();

    return xFrame;
}

uno::Reference<ui::XDecks> SAL_CALL SfxUnoSidebar::getDecks()
{
    uno::Reference<ui::XDecks> decks = new SfxUnoDecks(xFrame);
    return decks;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
