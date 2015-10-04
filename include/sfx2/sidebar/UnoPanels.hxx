/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_SIDEBAR_PANELS_HXX
#define INCLUDED_SFX2_SIDEBAR_PANELS_HXX


#include <com/sun/star/ui/XPanels.hpp>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/weakref.hxx>

#include <sfx2/sidebar/SidebarController.hxx>

/** get the decks
*/
class SfxUnoPanels : public ::cppu::WeakImplHelper < css::ui::XPanels >
{

public:

    SfxUnoPanels(const css::uno::Reference<css::frame::XFrame>& , const OUString&);
    virtual ~SfxUnoPanels() {};

// XPanels
    virtual OUString SAL_CALL getDeckId()
                                throw(css::uno::RuntimeException, std::exception) override;

// XNameAccess

    virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
                                throw(css::container::NoSuchElementException,
                                      css::lang::WrappedTargetException,
                                      css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
                                throw(css::uno::RuntimeException, std::exception) override;

// XIndexAccess

    virtual sal_Int32 SAL_CALL getCount()
                                throw(css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
                                throw(css::lang::IndexOutOfBoundsException,
                                      css::lang::WrappedTargetException,
                                      css::uno::RuntimeException, std::exception) override;

// XElementAccess
    virtual css::uno::Type SAL_CALL getElementType()
                                throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements()
                                throw(css::uno::RuntimeException, std::exception) override;

private:

    const css::uno::Reference<css::frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();
    const OUString& mDeckId;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
