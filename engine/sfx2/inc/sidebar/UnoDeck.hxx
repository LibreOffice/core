/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once


#include <com/sun/star/ui/XDeck.hpp>

#include <cppuhelper/implbase.hxx>

#include <sfx2/sidebar/ResourceManager.hxx>

namespace com::sun::star::frame { class XFrame; }
namespace sfx2::sidebar { class SidebarController; }

/** get the decks
*/
class SfxUnoDeck final : public cppu::WeakImplHelper<css::ui::XDeck>
{

public:

    SfxUnoDeck(css::uno::Reference<css::frame::XFrame>, OUString );

    virtual OUString getId() override;

    virtual OUString  getTitle() override;
    virtual void setTitle( const OUString& newTitle ) override;

    virtual bool isActive() override;
    virtual void activate( const bool bActivate ) override;

    virtual css::uno::Reference<css::ui::XPanels> getPanels() override;

    virtual sal_Int32 getOrderIndex() override;
    virtual void setOrderIndex( const sal_Int32 newOrderIndex ) override;
    virtual void moveFirst() override;
    virtual void moveLast() override;
    virtual void moveUp() override;
    virtual void moveDown() override;

private:

    const css::uno::Reference<css::frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();

    const OUString mDeckId;

    sal_Int32 GetMaxOrderIndex(const sfx2::sidebar::ResourceManager::DeckContextDescriptorContainer& rDecks);
    sal_Int32 GetMinOrderIndex(const sfx2::sidebar::ResourceManager::DeckContextDescriptorContainer& rDecks);

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
