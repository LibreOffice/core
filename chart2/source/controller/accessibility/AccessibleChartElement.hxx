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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_ACCESSIBILITY_ACCESSIBLECHARTELEMENT_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_ACCESSIBILITY_ACCESSIBLECHARTELEMENT_HXX

#include "AccessibleBase.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <unotools/accessiblestatesethelper.hxx>

#include <vector>
#include <map>

namespace chart
{

/** Base class for all Chart Accessibility objects except the root node (see AccessibleChartView)

    This class contains a reference to the ChartModel, thus, components can easily access all core functionality.

    Usage Instructions:

    <ul>
     <li>define the getAccessibleName() method of XAccessibleContext</li>
     <li>set the ChartModel using SetChartModel() for the first node before
         creating any children</li>
     <li>override UpdateChildren()</li>
    </ul>
 */

namespace impl
{
typedef ::cppu::ImplInheritanceHelper<
        AccessibleBase,
        ::com::sun::star::accessibility::XAccessibleExtendedComponent
        > AccessibleChartElement_Base;
}

class AccessibleChartElement :
    public impl::AccessibleChartElement_Base
{
public:
    AccessibleChartElement( const AccessibleElementInfo & rAccInfo,
                            bool bMayHaveChildren,
                            bool bAlwaysTransparent = false );
    virtual ~AccessibleChartElement();

    // ________ AccessibleBase ________
    virtual bool ImplUpdateChildren() SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >
        ImplGetAccessibleChildById( sal_Int32 i ) const
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;
    virtual sal_Int32 ImplGetAccessibleChildCount() const
        throw (::com::sun::star::uno::RuntimeException) SAL_OVERRIDE;

    // ________ XAccessibleContext ________
    virtual OUString SAL_CALL getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getAccessibleDescription()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ________ XAccessibleExtendedComponent ________
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getTitledBorderText()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getToolTipText()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // the following interface is implemented in AccessibleBase, however it is
    // also a (non-virtual) base class of XAccessibleExtendedComponent Thus
    // these methods have to be overridden and forward to AccessibleBase

    // ________ XAccessibleComponent ________
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL grabFocus() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getForeground() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getBackground() throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // ________ XServiceInfo ________
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    bool                m_bHasText;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessibleContext >
                        m_xTextHelper;

    void InitTextEdit();
};

}  // namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
