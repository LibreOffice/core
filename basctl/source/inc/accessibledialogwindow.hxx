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

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <cppuhelper/implbase3.hxx>
#include <svl/lstner.hxx>
#include <tools/link.hxx>
#include <vcl/vclptr.hxx>

class VclSimpleEvent;
class VclWindowEvent;

namespace utl {
   class AccessibleStateSetHelper;
}

namespace basctl
{

class DialogWindow;
class DlgEditor;
class DlgEdModel;
class DlgEdObj;



typedef ::cppu::ImplHelper3 <
    css::accessibility::XAccessible,
    css::accessibility::XAccessibleSelection,
    css::lang::XServiceInfo > AccessibleDialogWindow_BASE;

class AccessibleDialogWindow final : public comphelper::OAccessibleExtendedComponentHelper,
                                public AccessibleDialogWindow_BASE,
                                public SfxListener
{
private:

    class ChildDescriptor
    {
    public:
        DlgEdObj*                                                 pDlgEdObj;
        css::uno::Reference< css::accessibility::XAccessible >    rxAccessible;

        ChildDescriptor( DlgEdObj* _pDlgEdObj );

        bool operator==( const ChildDescriptor& rDesc );
        bool operator<( const ChildDescriptor& rDesc ) const;
    };

    typedef std::vector< ChildDescriptor > AccessibleChildren;

    AccessibleChildren             m_aAccessibleChildren;
    VclPtr<basctl::DialogWindow>   m_pDialogWindow;
    DlgEdModel*                    m_pDlgEdModel;

    void                    UpdateFocused();
    void                    UpdateSelected();
    void                    UpdateBounds();

    bool                    IsChildVisible( const ChildDescriptor& rDesc );

    void                    InsertChild( const ChildDescriptor& rDesc );
    void                    RemoveChild( const ChildDescriptor& rDesc );
    void                    UpdateChild( const ChildDescriptor& rDesc );
    void                    UpdateChildren();
    void                    SortChildren();

    DECL_LINK( WindowEventListener, VclWindowEvent&, void );

    void            ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
    void            FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet );

    // OCommonAccessibleComponent
    virtual css::awt::Rectangle implGetBounds(  ) override;

    // XComponent
    virtual void SAL_CALL   disposing() override;

public:
    AccessibleDialogWindow (basctl::DialogWindow*);
    virtual ~AccessibleDialogWindow() override;

    // SfxListener
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    // XInterface
    DECLARE_XINTERFACE()

    // XTypeProvider
    DECLARE_XTYPEPROVIDER()

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    // XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

    // XAccessibleComponent
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;
    virtual void SAL_CALL grabFocus(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    // XAccessibleExtendedComponent
    virtual css::uno::Reference< css::awt::XFont > SAL_CALL getFont(  ) override;
    virtual OUString SAL_CALL getTitledBorderText(  ) override;
    virtual OUString SAL_CALL getToolTipText(  ) override;

    // XAccessibleSelection
    virtual void SAL_CALL selectAccessibleChild( sal_Int32 nChildIndex ) override;
    virtual sal_Bool SAL_CALL isAccessibleChildSelected( sal_Int32 nChildIndex ) override;
    virtual void SAL_CALL clearAccessibleSelection() override;
    virtual void SAL_CALL selectAllAccessibleChildren(  ) override;
    virtual sal_Int32 SAL_CALL getSelectedAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) override;
    virtual void SAL_CALL deselectAccessibleChild( sal_Int32 nChildIndex ) override;
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
