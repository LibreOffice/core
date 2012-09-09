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

#ifndef ACC_ACCESSIBLETOOLPANELDECKTABBARITEM_HXX
#define ACC_ACCESSIBLETOOLPANELDECKTABBARITEM_HXX

#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase1.hxx>

#include <boost/scoped_ptr.hpp>

namespace svt
{
    class IToolPanelDeck;
    class PanelTabBar;
}

//......................................................................................................................
namespace accessibility
{
//......................................................................................................................

    //==================================================================================================================
    //= AccessibleToolPanelDeckTabBarItem
    //==================================================================================================================
    class AccessibleToolPanelDeckTabBarItem_Impl;
    typedef ::comphelper::OAccessibleExtendedComponentHelper AccessibleToolPanelDeckTabBarItem_Base;
    class AccessibleToolPanelDeckTabBarItem : public AccessibleToolPanelDeckTabBarItem_Base
    {
    public:
        AccessibleToolPanelDeckTabBarItem(
            const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rAccessibleParent,
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
        virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getAccessibleDescription(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getAccessibleName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleComponent
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL grabFocus(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

        // XAccessibleExtendedComponent
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getTitledBorderText(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual OUString SAL_CALL getToolTipText(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        // OCommonAccessibleComponent
        virtual ::com::sun::star::awt::Rectangle SAL_CALL implGetBounds(  ) throw (::com::sun::star::uno::RuntimeException);

        // OComponentHelper
        virtual void SAL_CALL disposing();

    protected:
        ::boost::scoped_ptr< AccessibleToolPanelDeckTabBarItem_Impl >   m_pImpl;
    };

//......................................................................................................................
} // namespace accessibility
//......................................................................................................................

#endif // ACC_ACCESSIBLETOOLPANELDECKTABBARITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
