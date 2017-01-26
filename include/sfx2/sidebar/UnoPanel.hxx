/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SFX2_SIDEBAR_UNOPANEL_HXX
#define INCLUDED_SFX2_SIDEBAR_UNOPANEL_HXX

#include <com/sun/star/ui/XPanel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XWindow.hpp>


#include <cppuhelper/compbase.hxx>

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/Deck.hxx>
#include <sfx2/sidebar/ResourceManager.hxx>


/** get the Panel
*/
class SfxUnoPanel : public cppu::WeakImplHelper<css::ui::XPanel>
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
    VclPtr<sfx2::sidebar::Panel> mpPanel;

    sal_Int32 GetMaxOrderIndex(sfx2::sidebar::ResourceManager::PanelContextDescriptorContainer aPanels);
    sal_Int32 GetMinOrderIndex(sfx2::sidebar::ResourceManager::PanelContextDescriptorContainer aPanels);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
