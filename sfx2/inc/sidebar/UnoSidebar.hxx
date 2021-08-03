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

#include <com/sun/star/ui/XSidebarProvider.hpp>

#include <cppuhelper/implbase.hxx>

namespace com::sun::star::frame { class XFrame; }
namespace com::sun::star::ui { class XDecks; }
namespace com::sun::star::ui { class XSidebar; }
namespace sfx2::sidebar { class SidebarController; }

/** get the sidebar for a given frame
*/
class SfxUnoSidebar final : public cppu::WeakImplHelper<css::ui::XSidebarProvider>
{

private:

    const css::uno::Reference<css::frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();

public:

    SfxUnoSidebar(const css::uno::Reference<css::frame::XFrame>&);

    virtual void SAL_CALL showDecks (const sal_Bool bVisible) override;


    virtual void SAL_CALL setVisible (const sal_Bool bVisible) override;

    virtual sal_Bool SAL_CALL isVisible() override;

    virtual css::uno::Reference<css::frame::XFrame> SAL_CALL getFrame() override;

    virtual css::uno::Reference<css::ui::XDecks> SAL_CALL getDecks() override;

    virtual css::uno::Reference<css::ui::XSidebar> SAL_CALL getSidebar() override;

};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
