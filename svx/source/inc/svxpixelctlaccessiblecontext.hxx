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
#ifndef INCLUDED_SVX_SOURCE_INC_SVXPIXELCTLACCESSIBLECONTEXT_HXX
#define INCLUDED_SVX_SOURCE_INC_SVXPIXELCTLACCESSIBLECONTEXT_HXX

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/accessibleselectionhelper.hxx>
#include <cppuhelper/implbase.hxx>

#include <rtl/ref.hxx>
#include <tools/gen.hxx>


namespace com::sun::star::awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
}
class SvxPixelCtl;
class SvxPixelCtlAccessible;

class SvxPixelCtlAccessibleChild final : public cppu::ImplInheritanceHelper<
                                             ::comphelper::OAccessibleComponentHelper,
                                             css::accessibility::XAccessible>
{
public:
    SvxPixelCtlAccessibleChild(
                SvxPixelCtl& rWindow,
                bool bPixelColorOrBG,
                const tools::Rectangle& rBounds,
                rtl::Reference<SvxPixelCtlAccessible> xParent,
                tools::Long nIndexInParent );

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

    void CheckChild();
    void SelectChild( bool bSelect);
    void ChangePixelColorOrBG(bool bPixelColorOrBG){ m_bPixelColorOrBG = bPixelColorOrBG ;}
    OUString GetName() const;

private:
    virtual ~SvxPixelCtlAccessibleChild() override;

    virtual void SAL_CALL disposing() override;

    // OAccessibleComponentHelper
    /// implements the calculation of the bounding rectangle
    virtual css::awt::Rectangle implGetBounds(  ) override;

    SvxPixelCtl& mrParentWindow;
    rtl::Reference<SvxPixelCtlAccessible> mxParent;
    bool m_bPixelColorOrBG;//Pixel Color Or BackGround Color
    tools::Rectangle  maBoundingBox;
    /// index of child in parent
    tools::Long mnIndexInParent;
};

class SvxPixelCtlAccessible final : public cppu::ImplInheritanceHelper<
                                        ::comphelper::OAccessibleSelectionHelper,
                                        css::accessibility::XAccessible>
{
public:
    SvxPixelCtlAccessible(SvxPixelCtl* pPixelCtl);

    //XAccessibleComponent
    virtual void SAL_CALL grabFocus(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;

    //XAccessible
    //XAccessibleContext
    virtual sal_Int64 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual sal_Int64 SAL_CALL getAccessibleStateSet(  ) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    rtl::Reference<SvxPixelCtlAccessibleChild>
         CreateChild (tools::Long nIndex, Point mPoint);

private:
    virtual ~SvxPixelCtlAccessible() override;

    // OCommonAccessibleSelection
    // return if the specified child is visible => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
    virtual bool implIsSelected(sal_Int64 nAccessibleChildIndex) override;

    // select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
    virtual void implSelect(sal_Int64 nAccessibleChildIndex, bool bSelect) override;

    // OAccessibleComponentHelper
    virtual css::awt::Rectangle implGetBounds() override;

    virtual void SAL_CALL disposing() override;

    SvxPixelCtl* mpPixelCtl;
    rtl::Reference<SvxPixelCtlAccessibleChild> m_xCurChild;

public:
    void NotifyChild(tools::Long nIndex, bool bSelect, bool bCheck);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
