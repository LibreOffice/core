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

#include <controls/table/tablecontrol.hxx>

#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <tools/gen.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <comphelper/uno3.hxx>


namespace vcl { class Window; }


namespace accessibility {

/** The GridControl accessible objects inherit from this base class. It
    implements basic functionality. */
class AccessibleGridControlBase
    : public cppu::ImplInheritanceHelper<comphelper::OAccessibleComponentHelper,
                                         css::accessibility::XAccessible, css::lang::XServiceInfo>
{
public:
    /** Constructor.
        @param rxParent  XAccessible interface of the parent object.
        @param rTable    The Table control.
        @param eObjType  Type of accessible table control. */
    AccessibleGridControlBase(
        css::uno::Reference< css::accessibility::XAccessible > xParent,
        svt::table::TableControl& rTable,
        AccessibleTableControlObjType eObjType);

protected:
    virtual ~AccessibleGridControlBase() = default;
    virtual void SAL_CALL disposing() override;

public:
    // XAccessible
    /** @return  The XAccessibleContext interface of this object. */
    virtual css::uno::Reference<css::accessibility::XAccessibleContext> SAL_CALL
    getAccessibleContext() override;

    // XAccessibleContext

    /** @return  A reference to the parent accessible object. */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleParent() override;

    /** @return
            The description of this object.
    */
    virtual OUString SAL_CALL getAccessibleDescription() override;

    /** @return
            The name of this object.
    */
    virtual OUString SAL_CALL getAccessibleName() override;

    /** @return
            The relation set (the GridControl does not have one).
    */
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet() override;

    /** @return  The set of current states. */
    virtual sal_Int64 SAL_CALL getAccessibleStateSet() override;

    /** @return  The parent's locale. */
    virtual css::lang::Locale SAL_CALL getLocale() override;

    /** @return
            The role of this object. Panel, ROWHEADER, COLUMNHEADER, TABLE, TABLE_CELL are supported.
    */
    virtual sal_Int16 SAL_CALL getAccessibleRole() override;

    /*  Derived classes have to implement:
        -   getAccessibleChildCount,
        -   getAccessibleChild,
        -   getAccessibleRole.
        Derived classes may overwrite getAccessibleIndexInParent to increase
        performance. */

    // XAccessibleComponent
    virtual sal_Int32 SAL_CALL getForeground(  ) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) override;


    /*  Derived classes have to implement:
        -   getAccessibleAt,
        -   grabFocus. */

    /** @return
            The accessible child rendered under the given point.
    */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint( const css::awt::Point& rPoint ) override;

    // XTypeProvider

    /** @return  a unique implementation ID. */
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XServiceInfo

    /** @return  Whether the specified service is supported by this class. */
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;

    /** @return  a list of all supported services. */
    virtual css::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    /*  Derived classes have to implement:
        -   getImplementationName. */

    // helper methods

    /** @return  The GridControl object type. */
    inline AccessibleTableControlObjType getType() const;

    /** Commits an event to all listeners. */
    virtual void commitEvent(sal_Int16 nEventId, const css::uno::Any& rNewValue,
                             const css::uno::Any& rOldValue);

protected:
    virtual css::awt::Rectangle implGetBounds() override;

    // internal virtual methods

    /** Determines whether the Grid control is really showing inside of
        its parent accessible window. Derived classes may implement different
        behaviour.
        @attention  This method requires locked mutex's and a living object.
        @return  TRUE, if the object is really showing. */
    bool implIsShowing();

    /** Return the bounding box relative to the parent.
        @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) relative to the parent. */
    tools::Rectangle implGetBoundingBox();

    ///** Derived classes return the bounding box in screen coordinates.
    //    @attention  This method requires locked mutex's and a living object.
    //    @return  The bounding box (VCL rect.) in screen coordinates. */
    virtual AbsoluteScreenPixelRectangle implGetBoundingBoxOnScreen() = 0;

    /** Creates a bitset of states of the
        current object. This method calls FillStateSet at the GridControl which
        fills it with more states depending on the object type. Derived classes
        may overwrite this method and add more states.
        @attention  This method requires locked mutex's.
    */
    virtual sal_Int64 implCreateStateSet();

protected:
    // members

    /** The parent accessible object. */
    css::uno::Reference< css::accessibility::XAccessible > m_xParent;
    /** The SVT Table control. */
    svt::table::TableControl& m_aTable;
    /** The type of this object (for names, descriptions, state sets, ...). */
    AccessibleTableControlObjType m_eObjType;
};

// inlines

inline AccessibleTableControlObjType AccessibleGridControlBase::getType() const
{
    return m_eObjType;
}


} // namespace accessibility



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
