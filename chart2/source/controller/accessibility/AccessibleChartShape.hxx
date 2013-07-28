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
#ifndef _CHART2_ACCESSIBLECHARTSHAPE_HXX_
#define _CHART2_ACCESSIBLECHARTSHAPE_HXX_

#include "AccessibleBase.hxx"

#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <cppuhelper/implbase1.hxx>
#include <svx/AccessibleShapeTreeInfo.hxx>

namespace accessibility
{
class AccessibleShape;
}

namespace chart
{

namespace impl
{
typedef ::cppu::ImplInheritanceHelper1<
    AccessibleBase,
    ::com::sun::star::accessibility::XAccessibleExtendedComponent > AccessibleChartShape_Base;
}

class AccessibleChartShape :
    public impl::AccessibleChartShape_Base
{
public:
    AccessibleChartShape( const AccessibleElementInfo& rAccInfo,
        bool bMayHaveChildren, bool bAlwaysTransparent = false );
    virtual ~AccessibleChartShape();

    // ________ XServiceInfo ________
    virtual OUString SAL_CALL getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);

    // ________ XAccessibleContext ________
    virtual sal_Int32 SAL_CALL getAccessibleChildCount()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL
        getAccessibleChild( sal_Int32 i )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getAccessibleDescription()
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getAccessibleName()
        throw (::com::sun::star::uno::RuntimeException);

    // ________ XAccessibleComponent ________
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const ::com::sun::star::awt::Point& aPoint ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL grabFocus() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getForeground() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground() throw (::com::sun::star::uno::RuntimeException);

    // ________ XAccessibleExtendedComponent ________
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL getFont()
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getTitledBorderText()
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getToolTipText()
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::accessibility::AccessibleShape* m_pAccShape;
    ::accessibility::AccessibleShapeTreeInfo m_aShapeTreeInfo;
};

}  // namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
