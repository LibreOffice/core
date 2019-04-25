/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once


#include <com/sun/star/ui/XDecks.hpp>

#include <cppuhelper/implbase.hxx>

namespace com::sun::star::frame { class XFrame; }
namespace sfx2 { namespace sidebar { class SidebarController; } }

/** get the decks
*/
class SfxUnoDecks : public cppu::WeakImplHelper<css::ui::XDecks>
{

public:

    SfxUnoDecks(const css::uno::Reference<css::frame::XFrame>&);

// XNameAccess

    virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

// XIndexAccess

    virtual sal_Int32 SAL_CALL getCount() override;

    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

private:

    const css::uno::Reference<css::frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();

};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
