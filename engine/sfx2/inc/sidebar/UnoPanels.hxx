/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once


#include <com/sun/star/ui/XPanels.hpp>

#include <cppuhelper/implbase.hxx>

namespace com::sun::star::frame { class XFrame; }
namespace sfx2::sidebar { class SidebarController; }

/** get the decks
*/
class SfxUnoPanels final : public cppu::WeakImplHelper<css::ui::XPanels>
{

public:

    SfxUnoPanels(css::uno::Reference<css::frame::XFrame>, const OUString&);

// XPanels
    virtual OUString getDeckId() override;

// XNameAccess

    virtual cpo::uno::Any getByName( const OUString& aName ) override;

    virtual cpo::uno::Sequence< OUString > getElementNames() override;

    virtual bool hasByName( const OUString& aName ) override;

// XIndexAccess

    virtual sal_Int32 getCount() override;

    virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

// XElementAccess
    virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

private:

    const css::uno::Reference<css::frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();
    const OUString& mDeckId;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
