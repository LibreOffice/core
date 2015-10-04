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

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/weakref.hxx>

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/ResourceManager.hxx>

/** get the decks
*/
class SfxUnoDeck : public ::cppu::WeakImplHelper < css::ui::XDeck >
{

public:

    SfxUnoDeck(const css::uno::Reference<css::frame::XFrame>& , const OUString&);
    virtual ~SfxUnoDeck() {};

    virtual OUString SAL_CALL getId()
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL  getTitle()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTitle( const OUString& newTitle )
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isActive()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL activate( const sal_Bool bActivate )
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::ui::XPanels> SAL_CALL getPanels()
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Int32 SAL_CALL getOrderIndex()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setOrderIndex( const sal_Int32 newOrderIndex )
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL moveFirst()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL moveLast()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL moveUp()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL moveDown()
                                throw(css::uno::RuntimeException, std::exception) override;

private:

    const css::uno::Reference<css::frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();

    const OUString mDeckId;

    sal_Int32 GetMaxOrderIndex(sfx2::sidebar::ResourceManager::DeckContextDescriptorContainer aDecks);
    sal_Int32 GetMinOrderIndex(sfx2::sidebar::ResourceManager::DeckContextDescriptorContainer aDecks);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
