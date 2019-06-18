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
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/accessibleselectionhelper.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/compbase6.hxx>
#include <cppuhelper/compbase5.hxx>
#include <cppuhelper/basemutex.hxx>

#include <svx/rectenum.hxx>
#include <tools/gen.hxx>


namespace com { namespace sun { namespace star { namespace awt {
    struct Point;
    struct Rectangle;
    struct Size;
    class XFocusListener;
} } } };
class SvxPixelCtl;
class SvxPixelCtlAccessible;

typedef ::cppu::ImplHelper1<css::accessibility::XAccessible> OAccessibleHelper_Base;

class SvxPixelCtlAccessibleChild final : public ::comphelper::OAccessibleComponentHelper,
                                         public OAccessibleHelper_Base
{
public:
    SvxPixelCtlAccessibleChild(
                SvxPixelCtl& rWindow,
                bool bPixelColorOrBG,
                const tools::Rectangle& rBounds,
                const rtl::Reference<SvxPixelCtlAccessible>& xParent,
                long nIndexInParent );

    DECLARE_XINTERFACE( )
    DECLARE_XTYPEPROVIDER( )

    //XAccessibleComponent
    virtual void SAL_CALL grabFocus(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;

    //XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;

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

    // OCommonAccessibleComponent
    /// implements the calculation of the bounding rectangle
    virtual css::awt::Rectangle implGetBounds(  ) override;

    SvxPixelCtl& mrParentWindow;
    rtl::Reference<SvxPixelCtlAccessible> mxParent;
    bool m_bPixelColorOrBG;//Pixel Color Or BackGround Color
    tools::Rectangle const  maBoundingBox;
    /// index of child in parent
    long const              mnIndexInParent;
};

class SvxPixelCtlAccessible final : public ::comphelper::OAccessibleSelectionHelper,
                                    public OAccessibleHelper_Base
{
public:
    SvxPixelCtlAccessible(SvxPixelCtl* pPixelCtl);

    DECLARE_XINTERFACE( )
    DECLARE_XTYPEPROVIDER( )

    //XAccessibleComponent
    virtual void SAL_CALL grabFocus(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& aPoint ) override;

    //XAccessible
    //XAccessibleContext
    virtual sal_Int32 SAL_CALL getAccessibleChildCount(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleParent(  ) override;
    virtual sal_Int16 SAL_CALL getAccessibleRole(  ) override;
    virtual OUString SAL_CALL getAccessibleDescription(  ) override;
    virtual OUString SAL_CALL getAccessibleName(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet(  ) override;

    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext(  ) override;
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;

    css::uno::Reference< css::accessibility::XAccessible >
         CreateChild (long nIndex, Point mPoint);

private:
    virtual ~SvxPixelCtlAccessible() override;

    // OCommonAccessibleSelection
    // return if the specified child is visible => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
    virtual bool implIsSelected(sal_Int32 nAccessibleChildIndex) override;

    // select the specified child => watch for special ChildIndexes (ACCESSIBLE_SELECTION_CHILD_xxx)
    virtual void implSelect(sal_Int32 nAccessibleChildIndex, bool bSelect) override;

    // OCommonAccessibleComponent
    virtual css::awt::Rectangle implGetBounds() override;

    virtual void SAL_CALL disposing() override;

    SvxPixelCtl* mpPixelCtl;
    css::uno::Reference< css::accessibility::XAccessible> m_xCurChild;

public:
    void NotifyChild(long nIndex, bool bSelect, bool bCheck);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
