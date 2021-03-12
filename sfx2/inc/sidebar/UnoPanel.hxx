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

#include <com/sun/star/ui/XPanel.hpp>


#include <cppuhelper/implbase.hxx>

#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/Deck.hxx>
#include <sfx2/sidebar/ResourceManager.hxx>

namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::frame { class XFrame; }
namespace sfx2::sidebar { class SidebarController; }


/** get the Panel
*/
class SfxUnoPanel final : public cppu::WeakImplHelper<css::ui::XPanel>
{

public:

    SfxUnoPanel(const css::uno::Reference<css::frame::XFrame>& , const OUString&, const OUString&);

    virtual OUString SAL_CALL getId() override;

    virtual OUString SAL_CALL  getTitle() override;
    virtual void SAL_CALL setTitle( const OUString& newTitle ) override;

    virtual sal_Bool SAL_CALL isExpanded() override;
    virtual void SAL_CALL expand( const sal_Bool bCollapseOther ) override;
    virtual void SAL_CALL collapse( ) override;

    virtual sal_Int32 SAL_CALL getOrderIndex() override;
    virtual void SAL_CALL setOrderIndex( const sal_Int32 newOrderIndex ) override;
    virtual void SAL_CALL moveFirst() override;
    virtual void SAL_CALL moveLast() override;
    virtual void SAL_CALL moveUp() override;
    virtual void SAL_CALL moveDown() override;

    virtual css::uno::Reference<css::awt::XWindow> SAL_CALL getDialog() override;

private:

    const css::uno::Reference<css::frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();

    const OUString mPanelId;
    const OUString mDeckId;

    VclPtr<sfx2::sidebar::Deck> mpDeck;
    std::weak_ptr<sfx2::sidebar::Panel> mxPanel;

    sal_Int32 GetMaxOrderIndex(sfx2::sidebar::ResourceManager::PanelContextDescriptorContainer aPanels);
    sal_Int32 GetMinOrderIndex(sfx2::sidebar::ResourceManager::PanelContextDescriptorContainer aPanels);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
