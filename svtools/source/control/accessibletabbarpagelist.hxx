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

#pragma once


#include "accessibletabbarbase.hxx"
#include "accessibletabbarpage.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>

#include <vector>

namespace accessibility
{
    class AccessibleTabBarPageList final : public cppu::ImplInheritanceHelper<
                                               AccessibleTabBarBase,
                                               css::accessibility::XAccessible,
                                               css::accessibility::XAccessibleSelection,
                                               css::lang::XServiceInfo>
    {
    private:
        typedef std::vector< rtl::Reference< AccessibleTabBarPage > > AccessibleChildren;

        AccessibleChildren      m_aAccessibleChildren;
        sal_Int32               m_nIndexInParent;

        void                    UpdateShowing( bool bShowing );
        void                    UpdateSelected( sal_Int32 i, bool bSelected );
        void                    UpdatePageText( sal_Int32 i );

        void                    InsertChild( sal_Int32 i );
        void                    RemoveChild( sal_Int32 i );
        void                    MoveChild( sal_Int32 i, sal_Int32 j );

        virtual void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
        void            FillAccessibleStateSet( sal_Int64& rStateSet );

        // OAccessibleComponentHelper
        virtual css::awt::Rectangle implGetBounds(  ) override;

        // XComponent
        virtual void SAL_CALL   disposing() override;

    public:
        AccessibleTabBarPageList( TabBar* pTabBar, sal_Int32 nIndexInParent );

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XAccessible
        virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

        // XAccessibleContext
        virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
        virtual sal_Int64 SAL_CALL getAccessibleIndexInParent(  ) override;
        virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
        virtual OUString SAL_CALL getAccessibleDescription(  ) override;
        virtual OUString SAL_CALL getAccessibleName(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
        virtual sal_Int64 SAL_CALL getAccessibleStateSet(  ) override;
        virtual css::lang::Locale SAL_CALL getLocale(  ) override;

        // XAccessibleComponent
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
        virtual void SAL_CALL grabFocus(  ) override;
        virtual sal_Int32 SAL_CALL getForeground(  ) override;
        virtual sal_Int32 SAL_CALL getBackground(  ) override;

        // XAccessibleExtendedComponent
        virtual OUString SAL_CALL getTitledBorderText(  ) override;
        virtual OUString SAL_CALL getToolTipText(  ) override;

        // XAccessibleSelection
        virtual void SAL_CALL selectAccessibleChild( sal_Int64 nChildIndex ) override;
        virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int64 nChildIndex ) override;
        virtual void SAL_CALL clearAccessibleSelection(  ) override;
        virtual void SAL_CALL selectAllAccessibleChildren(  ) override;
        virtual sal_Int64 SAL_CALL getSelectedAccessibleChildCount(  ) override;
        virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int64 nSelectedChildIndex ) override;
        virtual void SAL_CALL deselectAccessibleChild( sal_Int64 nChildIndex ) override;

    private:
        rtl::Reference< AccessibleTabBarPage > getAccessibleChildImpl( sal_Int64 i );
    };


}   // namespace accessibility



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
