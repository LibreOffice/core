/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SFX2_SIDEBAR_SIDEBAR_HXX
#define INCLUDED_SFX2_SIDEBAR_SIDEBAR_HXX

#include <sfx2/dllapi.h>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/ui/XSidebarProvider.hpp>
#include <com/sun/star/ui/XSidebar.hpp>

#include <com/sun/star/awt/XWindow2.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/weakref.hxx>

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/SidebarDockingWindow.hxx>

#include <sfx2/sidebar/UnoDecks.hxx>
#include <com/sun/star/ui/XDecks.hpp>

/** get the sidebar for a given frame
*/
class SfxUnoSidebar : public ::cppu::WeakImplHelper < css::ui::XSidebarProvider >
{

private:

    const css::uno::Reference<css::frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();

public:

    SfxUnoSidebar(const css::uno::Reference<css::frame::XFrame>&);

    virtual void SAL_CALL showDecks (const sal_Bool bVisible)
                                    throw(css::uno::RuntimeException, std::exception) override;


    virtual void SAL_CALL setVisible (const sal_Bool bVisible)
                                    throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL isVisible()
                                    throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::frame::XFrame> SAL_CALL getFrame()
                                throw( css::uno::RuntimeException, std::exception ) override;

    virtual css::uno::Reference<css::ui::XDecks> SAL_CALL getDecks()
                                    throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::ui::XSidebar> SAL_CALL getSidebar()
                                    throw(css::uno::RuntimeException, std::exception) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
