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

#ifndef INCLUDED_SVX_SOURCE_INC_ACCESSIBLEFRAMESELECTOR_HXX
#define INCLUDED_SVX_SOURCE_INC_ACCESSIBLEFRAMESELECTOR_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <cppuhelper/implbase.hxx>
#include<comphelper/accessiblecomponenthelper.hxx>
#include <svx/framebordertype.hxx>

namespace svx {

class FrameSelector;

namespace a11y {

class AccFrameSelector final : public cppu::ImplInheritanceHelper<
                                   ::comphelper::OAccessibleComponentHelper,
                                   css::accessibility::XAccessible>
{
public:
    explicit            AccFrameSelector(FrameSelector& rFrameSel);
    virtual             ~AccFrameSelector() override;

    //XAccessibleComponent
    virtual void SAL_CALL grabFocus(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;

    //XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet(  ) override;
    virtual css::awt::Point SAL_CALL getLocationOnScreen() override;

    //XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    void    Invalidate();

private:
    // OAccessibleComponentHelper
    /// implements the calculation of the bounding rectangle
    virtual css::awt::Rectangle implGetBounds(  ) override;

    /// @throws css::uno::RuntimeException
    void                IsValid();

    FrameSelector*      mpFrameSel;
};

class AccFrameSelectorChild final : public cppu::ImplInheritanceHelper<
                                        ::comphelper::OAccessibleComponentHelper,
                                        css::accessibility::XAccessible>
{
public:
    explicit            AccFrameSelectorChild( FrameSelector& rFrameSel, FrameBorderType eBorder );

    virtual             ~AccFrameSelectorChild() override;

    //XAccessibleComponent
    virtual void SAL_CALL grabFocus(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;

    //XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet(  ) override;

    //XAccessible
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;

    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    void NotifyAccessibleEvent(const sal_Int16 _nEventId, const css::uno::Any& _rOldValue, const css::uno::Any& _rNewValue)
    {
        ::comphelper::OAccessibleComponentHelper::NotifyAccessibleEvent(_nEventId, _rOldValue, _rNewValue);
    }

    void    Invalidate();

private:
    // OAccessibleComponentHelper
    /// implements the calculation of the bounding rectangle
    virtual css::awt::Rectangle implGetBounds(  ) override;

    /// @throws css::uno::RuntimeException
    void                IsValid();

    FrameSelector*      mpFrameSel;

    FrameBorderType     meBorder;
};


}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
