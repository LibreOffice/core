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


#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/weakref.hxx>

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/Panel.hxx>
#include <sfx2/sidebar/Deck.hxx>
#include <sfx2/sidebar/ResourceManager.hxx>


/** get the Panel
*/
class SfxUnoPanel : public ::cppu::WeakImplHelper1< css::ui::XPanel >
{

public:

    SfxUnoPanel(const css::uno::Reference<css::frame::XFrame>& , const OUString&, const OUString&);
    virtual ~SfxUnoPanel() {};

    virtual OUString SAL_CALL getId() SAL_OVERRIDE;

    virtual OUString SAL_CALL  getTitle() SAL_OVERRIDE;
    virtual void SAL_CALL setTitle( const OUString& newTitle ) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isExpanded() SAL_OVERRIDE;
    virtual void SAL_CALL expand( const sal_Bool bCollapseOther ) SAL_OVERRIDE;
    virtual void SAL_CALL collapse( ) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getOrderIndex() SAL_OVERRIDE;
    virtual void SAL_CALL setOrderIndex( const sal_Int32 newOrderIndex ) SAL_OVERRIDE;
    virtual void SAL_CALL moveFirst() SAL_OVERRIDE;
    virtual void SAL_CALL moveLast() SAL_OVERRIDE;
    virtual void SAL_CALL moveUp() SAL_OVERRIDE;
    virtual void SAL_CALL moveDown() SAL_OVERRIDE;

    css::uno::Reference<css::awt::XWindow> SAL_CALL getDialog() SAL_OVERRIDE;

private:

    const css::uno::Reference<css::frame::XFrame> xFrame;
    sfx2::sidebar::SidebarController* getSidebarController();

    const OUString mPanelId;
    const OUString mDeckId;

    sfx2::sidebar::Deck* mpDeck;
    sfx2::sidebar::Panel* mpPanel;

    sal_Int32 GetMaxOrderIndex(sfx2::sidebar::ResourceManager::PanelContextDescriptorContainer aPanels);
    sal_Int32 GetMinOrderIndex(sfx2::sidebar::ResourceManager::PanelContextDescriptorContainer aPanels);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */