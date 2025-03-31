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


#ifndef INCLUDED_SVX_SOURCE_INC_SVXRECTCTACCESSIBLECONTEXT_HXX
#define INCLUDED_SVX_SOURCE_INC_SVXRECTCTACCESSIBLECONTEXT_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/accessibleselectionhelper.hxx>
#include <rtl/ref.hxx>
#include <svx/rectenum.hxx>
#include <tools/gen.hxx>
#include <vector>

namespace com::sun::star::awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
}
namespace tools { class Rectangle; }
class SvxRectCtl;
class SvxRectCtlChildAccessibleContext;

class SvxRectCtlAccessibleContext final : public cppu::ImplInheritanceHelper<
                                              ::comphelper::OAccessibleSelectionHelper,
                                              css::accessibility::XAccessible>
{
public:
    // internal
    SvxRectCtlAccessibleContext(SvxRectCtl* pRepresentation);

    // XAccessibleComponent
    virtual void SAL_CALL grabFocus() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint(const css::awt::Point& rPoint) override;

    // XAccessible
    // XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL getAccessibleChild(sal_Int64 nIndex) override;
    virtual css::uno::Reference< css::accessibility::XAccessible> SAL_CALL getAccessibleParent() override;
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;
    virtual OUString SAL_CALL getAccessibleDescription() override;
    virtual OUString SAL_CALL getAccessibleName() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() override;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet() override;

    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL getAccessibleContext() override { return this; }
    virtual sal_Int32 SAL_CALL getForeground() override;
    virtual sal_Int32 SAL_CALL getBackground() override;

    /** Selects a new child by point.

        <p>If the child was not selected before, the state of the child will
        be updated. If the point is not invalid, the index will internally set to NOCHILDSELECTED</p>

        @param eButton
            Button which belongs to the child which should be selected.
    */
    void selectChild( RectPoint ePoint );
    void FireChildFocus( RectPoint eButton );

private:
    virtual ~SvxRectCtlAccessibleContext() override;

    // OCommonAccessibleSelection
    // return if the specified child is visible => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
    virtual bool implIsSelected(sal_Int64 nAccessibleChildIndex) override;

    // select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
    virtual void implSelect(sal_Int64 nAccessibleChildIndex, bool bSelect) override;

    // OAccessibleComponentHelper
    virtual css::awt::Rectangle implGetBounds() override;

    virtual void SAL_CALL disposing() override;

    void checkChildIndex(sal_Int64 nIndex);

    /** Selects a new child by index.

        <p>If the child was not selected before, the state of the child will
        be updated. If the index is invalid, the index will internally set to NOCHILDSELECTED</p>

        @param nIndexOfChild
            Index of the new child which should be selected.
    */
    void selectChild( tools::Long nIndexOfChild );

    /** Description of this object.  This is not a constant because it can
        be set from the outside.
    */
    OUString                     msDescription;

    /** Name of this object.
    */
    OUString                     msName;

    /// pointer to internal representation
    SvxRectCtl*                  mpRepr;

    /// array for all possible children
    std::vector<rtl::Reference<SvxRectCtlChildAccessibleContext>>  mvChildren;

    /// actual selected child
    tools::Long                                mnSelectedChild;
};

class SvxRectCtlChildAccessibleContext final : public cppu::ImplInheritanceHelper<
                                                   ::comphelper::OAccessibleComponentHelper,
                                                   css::accessibility::XAccessible,
                                                   css::accessibility::XAccessibleValue,
                                                   css::accessibility::XAccessibleAction>
{
public:
    SvxRectCtlChildAccessibleContext(
        const css::uno::Reference< css::accessibility::XAccessible>& rxParent,
        OUString aName, OUString aDescription,
        const tools::Rectangle& rBoundingBox,
        tools::Long nIndexInParent );

    // XAccessibleComponent
    virtual void SAL_CALL grabFocus() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& rPoint ) override;

    // XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount() override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 nIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent() override;
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;
    virtual OUString SAL_CALL getAccessibleDescription() override;
    virtual OUString SAL_CALL getAccessibleName() override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() override;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet() override;

    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL getAccessibleContext() override { return this; }

    virtual sal_Int32 SAL_CALL getForeground() override;
    virtual sal_Int32 SAL_CALL getBackground() override;

    // XAccessibleValue
    virtual css::uno::Any SAL_CALL
        getCurrentValue() override;

    virtual sal_Bool SAL_CALL
        setCurrentValue( const css::uno::Any& aNumber ) override;

    virtual css::uno::Any SAL_CALL
        getMaximumValue() override;

    virtual css::uno::Any SAL_CALL
        getMinimumValue() override;

    virtual css::uno::Any SAL_CALL
        getMinimumIncrement() override;

    // XAccessibleAction
    virtual sal_Int32 SAL_CALL getAccessibleActionCount( ) override;
    virtual sal_Bool SAL_CALL doAccessibleAction ( sal_Int32 nIndex ) override;
    virtual OUString SAL_CALL getAccessibleActionDescription ( sal_Int32 nIndex ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleKeyBinding > SAL_CALL getAccessibleActionKeyBinding( sal_Int32 nIndex ) override;

    // internal
    /// Sets the checked status
    void setStateChecked(bool bChecked);
    void FireFocusEvent();

private:
    virtual ~SvxRectCtlChildAccessibleContext() override;

    virtual void SAL_CALL disposing() override;

    // OAccessibleComponentHelper
    /// implements the calculation of the bounding rectangle
    virtual css::awt::Rectangle implGetBounds(  ) override;

    /** Description of this object.  This is not a constant because it can
        be set from the outside.  Furthermore, it changes according to the
        draw page's display mode.
    */
    OUString                     msDescription;

    /** Name of this object.  It changes according the draw page's
        display mode.
    */
    OUString                     msName;

    /// Reference to the parent object.
    css::uno::Reference< css::accessibility::XAccessible >
                                 mxParent;

    /// Bounding box
    tools::Rectangle             maBoundingBox;

    /// index of child in parent
    tools::Long                         mnIndexInParent;

    /// Indicates, if object is checked
    bool                         mbIsChecked;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
