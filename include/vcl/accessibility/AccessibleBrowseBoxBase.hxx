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

#include <sal/config.h>

#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <comphelper/accessiblecomponenthelper.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <comphelper/uno3.hxx>
#include <vcl/accessibility/AccessibleBrowseBoxObjType.hxx>


namespace vcl { class Window; }

namespace vcl {
    class IAccessibleTableProvider;
}

/** The BrowseBox accessible objects inherit from this base class. It
    implements basic functionality for various Accessibility interfaces. */
class VCL_DLLPUBLIC AccessibleBrowseBoxBase
    : public cppu::ImplInheritanceHelper<comphelper::OAccessibleComponentHelper,
                                         css::accessibility::XAccessible, css::awt::XFocusListener,
                                         css::lang::XServiceInfo>
{
public:
    /** Constructor sets specified name and description. If the constant of a
        text is BBTEXT_NONE, the derived class has to set the text via
        implSetName() (in Ctor) or later via
        setAccessibleName() and setAccessibleDescription() (these methods
        notify the listeners about the change).

        @param rxParent         XAccessible interface of the parent object.
        @param rBrowseBox       The BrowseBox control.
        @param _xFocusWindow    The window that gets all the focus events.
        @param eObjType         Object type */
    AccessibleBrowseBoxBase(
        const css::uno::Reference<css::accessibility::XAccessible>& xParent,
        ::vcl::IAccessibleTableProvider& rBrowseBox,
        const css::uno::Reference<css::awt::XWindow>& xFocusWindow,
        AccessibleBrowseBoxObjType eObjType );

    /** Constructor sets specified name and description.
        @param rxParent         XAccessible interface of the parent object.
        @param rBrowseBox       The BrowseBox control.
        @param _xFocusWindow    The window that gets all the focus events.
        @param eObjType         Object type
        @param rName            The name of this object.
        @param rDescription     The description text of this object. */
    AccessibleBrowseBoxBase(
        css::uno::Reference< css::accessibility::XAccessible > xParent,
        ::vcl::IAccessibleTableProvider& rBrowseBox,
        css::uno::Reference< css::awt::XWindow >  _xFocusWindow,
        AccessibleBrowseBoxObjType eObjType,
        OUString  rName,
        OUString  rDescription );

protected:
    AccessibleBrowseBoxBase(const AccessibleBrowseBoxBase&) = delete;
    AccessibleBrowseBoxBase& operator=(const AccessibleBrowseBoxBase&) = delete;

    /** Commits DeFunc event to listeners and cleans up members. */
    virtual void SAL_CALL disposing() override;

    virtual css::awt::Rectangle implGetBounds() override;

public:
    // XAccessible
    css::uno::Reference<css::accessibility::XAccessibleContext>
        SAL_CALL getAccessibleContext() override;

    // XAccessibleContext
    /** @return  A reference to the parent accessible object. */
    virtual css::uno::Reference<css::accessibility::XAccessible > SAL_CALL getAccessibleParent() override;

    /** @return  The index of this object among the parent's children. */
    virtual sal_Int64 SAL_CALL getAccessibleIndexInParent() override;

    /** @return
            The description of this object.
    */
    virtual OUString SAL_CALL getAccessibleDescription() override;

    /** @return
            The name of this object.
    */
    virtual OUString SAL_CALL getAccessibleName() override;

    /** @return
            The relation set (the BrowseBox does not have one).
    */
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet() override;

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

    // XFocusListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
    virtual void SAL_CALL focusGained( const css::awt::FocusEvent& e ) override;
    virtual void SAL_CALL focusLost( const css::awt::FocusEvent& e ) override;

    /*  Derived classes have to implement:
        -   getAccessibleAt,
        -   grabFocus. */

    /** @return
            The accessible child rendered under the given point.
    */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& rPoint ) override;

    // XTypeProvider

    /** @return  a unique implementation ID. */
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;

    // XServiceInfo

    /** @return  Whether the specified service is supported by this class. */
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName ) override;

    /** @return  A list of all supported services. */
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    /*  Derived classes have to implement:
        -   getImplementationName. */

    // helper methods

    /** @return  The BrowseBox object type. */
    inline AccessibleBrowseBoxObjType getType() const;

    /** Changes the name of the object and notifies listeners. */
    void setAccessibleName( const OUString& rName );
    /** Changes the description of the object and notifies listeners. */
    void setAccessibleDescription( const OUString& rDescription );

    /** Commits an event to all listeners. */
    void commitEvent(
            sal_Int16 nEventId,
            const css::uno::Any& rNewValue,
            const css::uno::Any& rOldValue );
    /** @return  TRUE, if the object is not disposed or disposing. */
    bool isAlive() const;

protected:
    // internal virtual methods

    /** Determines whether the BrowseBox control is really showing inside of
        its parent accessible window. Derived classes may implement different
        behaviour.
        @attention  This method requires locked mutex's and a living object.
        @return  TRUE, if the object is really showing. */
    bool implIsShowing();

    /** Derived classes return the bounding box relative to the parent window.
        @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) relative to the parent window. */
    virtual tools::Rectangle implGetBoundingBox() = 0;

    /** Creates a bitset of states of the
        current object. This method calls FillStateSet at the BrowseBox which
        fills it with more states depending on the object type. Derived classes
        may overwrite this method and add more states.
        @attention  This method requires locked mutex's.
    */
    virtual sal_Int64 implCreateStateSet();

    // internal helper methods

    /** Changes the name of the object (flat assignment, no notify).
        @attention  This method requires a locked mutex. */
    inline void implSetName( const OUString& rName );

public:
    /** @return  The osl::Mutex member provided by the class BaseMutex. */
    ::osl::Mutex&    getMutex( ) { return m_aMutex; }

    /** @throws <type>DisposedException</type>  If the object is not alive. */
    void ensureIsAlive() const;


protected:
    // members

    /** The parent accessible object. */
    css::uno::Reference< css::accessibility::XAccessible > mxParent;
    /** The VCL BrowseBox control. */
    ::vcl::IAccessibleTableProvider* mpBrowseBox;

    /** This is the window which get all the nice focus events
    */
    css::uno::Reference< css::awt::XWindow > m_xFocusWindow;

private:
    /** Localized name. */
    OUString maName;
    /** Localized description text. */
    OUString maDescription;

    /** The type of this object (for names, descriptions, state sets, ...). */
    AccessibleBrowseBoxObjType meObjType;
};

// a helper class for protecting methods which need to lock the solar mutex in addition to the own mutex

class SolarMethodGuard : public SolarMutexGuard, public osl::MutexGuard
{
public:
    SolarMethodGuard( osl::Mutex& rMutex )
        :SolarMutexGuard( )
        ,osl::MutexGuard( rMutex )
    {
    }
};

// inlines

inline AccessibleBrowseBoxObjType AccessibleBrowseBoxBase::getType() const
{
    return meObjType;
}

inline void AccessibleBrowseBoxBase::implSetName(
        const OUString& rName )
{
    maName = rName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
