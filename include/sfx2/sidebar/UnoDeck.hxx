/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SFX2_SIDEBAR_UNODECK_HXX
#define INCLUDED_SFX2_SIDEBAR_UNODECK_HXX


#include <com/sun/star/ui/XDeck.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XPanels.hpp>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/weakref.hxx>

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/ResourceManager.hxx>

using namespace css;
using namespace sfx2::sidebar;

/** get the decks
*/
class SfxUnoDeck : public ::cppu::WeakImplHelper1< css::ui::XDeck >
{


public:

    SfxUnoDeck(const uno::Reference<frame::XFrame>& , const OUString&);
    virtual ~SfxUnoDeck() {};

    virtual OUString SAL_CALL getId() SAL_OVERRIDE;

    virtual OUString SAL_CALL  getTitle() SAL_OVERRIDE;
    virtual void SAL_CALL setTitle( const OUString& newTitle ) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isActive() SAL_OVERRIDE;
    virtual void SAL_CALL activate( const sal_Bool bActivate ) SAL_OVERRIDE;

    virtual uno::Reference<ui::XPanels> SAL_CALL getPanels() SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getOrderIndex() SAL_OVERRIDE;
    virtual void SAL_CALL setOrderIndex( const sal_Int32 newOrderIndex ) SAL_OVERRIDE;
    virtual void SAL_CALL moveFirst() SAL_OVERRIDE;
    virtual void SAL_CALL moveLast() SAL_OVERRIDE;
    virtual void SAL_CALL moveUp() SAL_OVERRIDE;
    virtual void SAL_CALL moveDown() SAL_OVERRIDE;

private:

    const uno::Reference<frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();

    const OUString mDeckId;
    const OUString mTitle;

    sal_Int32 GetMaxOrderIndex(ResourceManager::DeckContextDescriptorContainer aDecks);
    sal_Int32 GetMinOrderIndex(ResourceManager::DeckContextDescriptorContainer aDecks);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */