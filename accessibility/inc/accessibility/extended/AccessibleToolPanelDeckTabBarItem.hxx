/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLETOOLPANELDECKTABBARITEM_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLETOOLPANELDECKTABBARITEM_HXX

#include <comphelper/accessiblecomponenthelper.hxx>

#include <memory>

namespace svt
{
    class IToolPanelDeck;
    class PanelTabBar;
}

namespace accessibility
{

    class AccessibleToolPanelDeckTabBarItem_Impl;
    typedef ::comphelper::OAccessibleExtendedComponentHelper AccessibleToolPanelDeckTabBarItem_Base;
    class AccessibleToolPanelDeckTabBarItem : public AccessibleToolPanelDeckTabBarItem_Base
    {
    public:
        AccessibleToolPanelDeckTabBarItem(
            const css::uno::Reference< css::accessibility::XAccessible >& i_rAccessibleParent,
            ::svt::IToolPanelDeck& i_rPanelDeck,
            ::svt::PanelTabBar& i_rTabBar,
            const size_t i_nItemPos
        );

        using AccessibleToolPanelDeckTabBarItem_Base::NotifyAccessibleEvent;
        using AccessibleToolPanelDeckTabBarItem_Base::lateInit;

    protected:
        virtual ~AccessibleToolPanelDeckTabBarItem();

    public:
        // XAccessibleContext
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getAccessibleName(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleComponent
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL grabFocus(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XAccessibleExtendedComponent
        virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getTitledBorderText(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getToolTipText(  ) throw (css::uno::RuntimeException, std::exception) override;

    protected:
        // OCommonAccessibleComponent
        virtual css::awt::Rectangle implGetBounds(  ) throw (css::uno::RuntimeException) override;

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

    protected:
        std::unique_ptr< AccessibleToolPanelDeckTabBarItem_Impl >   m_xImpl;
    };


} // namespace accessibility


#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLETOOLPANELDECKTABBARITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
