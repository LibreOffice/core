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


#ifndef ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXBASE_HXX
#define ACCESSIBILITY_EXT_ACCESSIBLEBROWSEBOXBASE_HXX

#include <svtools/AccessibleBrowseBoxObjType.hxx>
#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/compbase5.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/awt/XFocusListener.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <comphelper/uno3.hxx>

// ============================================================================

class Window;

namespace utl {
    class AccessibleStateSetHelper;
}

namespace svt {
    class IAccessibleTableProvider;
}

// ============================================================================

namespace accessibility {

// ============================================================================

typedef ::cppu::WeakAggComponentImplHelper5<
            ::com::sun::star::accessibility::XAccessibleContext,
            ::com::sun::star::accessibility::XAccessibleComponent,
            ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
        ::com::sun::star::awt::XFocusListener,
            ::com::sun::star::lang::XServiceInfo >
        AccessibleBrowseBoxImplHelper;

/** The BrowseBox accessible objects inherit from this base class. It
    implements basic functionality for various Accessibility interfaces and
    the event broadcaster and contains the ::osl::Mutex. */
class AccessibleBrowseBoxBase :
    public ::comphelper::OBaseMutex,
    public AccessibleBrowseBoxImplHelper
{
public:
    /** Constructor sets specified name and description. If the constant of a
        text is BBTEXT_NONE, the derived class has to set the text via
        implSetName() and implSetDescription() (in Ctor) or later via
        setAccessibleName() and setAccessibleDescription() (these methods
        notify the listeners about the change).
        @param rxParent  XAccessible interface of the parent object.
        @param rBrowseBox  The BrowseBox control.
        @param eNameText  The constant for the name text.
        @param eDescrText  The constant for the description text. */
    AccessibleBrowseBoxBase(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rxParent,
        ::svt::IAccessibleTableProvider&                  rBrowseBox,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
        ::svt::AccessibleBrowseBoxObjType  eObjType );

    /** Constructor sets specified name and description.
        @param rxParent  XAccessible interface of the parent object.
        @param rBrowseBox  The BrowseBox control.
        @param rName  The name of this object.
        @param rDescription  The description text of this object. */
    AccessibleBrowseBoxBase(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rxParent,
        ::svt::IAccessibleTableProvider&                  rBrowseBox,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
        ::svt::AccessibleBrowseBoxObjType  eObjType,
        const OUString&      rName,
        const OUString&      rDescription );

protected:
    virtual ~AccessibleBrowseBoxBase();

    /** Commits DeFunc event to listeners and cleans up members. */
    virtual void SAL_CALL disposing();

public:
    // XAccessibleContext -----------------------------------------------------

    /** @return  A reference to the parent accessible object. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleParent()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The index of this object among the parent's children. */
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return
            The description of this object.
    */
    virtual OUString SAL_CALL getAccessibleDescription()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return
            The name of this object.
    */
    virtual OUString SAL_CALL getAccessibleName()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return
            The relation set (the BrowseBox does not have one).
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL
    getAccessibleRelationSet()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The set of current states. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
    getAccessibleStateSet()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The parent's locale. */
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale()
        throw ( ::com::sun::star::accessibility::IllegalAccessibleComponentStateException,
                ::com::sun::star::uno::RuntimeException );

    /** @return
            The role of this object. Panel, ROWHEADER, COLUMNHEADER, TABLE, TABLE_CELL are supported.
    */
    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw ( ::com::sun::star::uno::RuntimeException );

    /*  Derived classes have to implement:
        -   getAccessibleChildCount,
        -   getAccessibleChild,
        -   getAccessibleRole.
        Derived classes may overwrite getAccessibleIndexInParent to increase
        performance. */

    // XAccessibleComponent ---------------------------------------------------

    /** @return
        <TRUE/>, if the point lies within the bounding box of this object. */
    virtual sal_Bool SAL_CALL containsPoint( const ::com::sun::star::awt::Point& rPoint )
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The bounding box of this object. */
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return
        The upper left corner of the bounding box relative to the parent. */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return
        The upper left corner of the bounding box in screen coordinates. */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  The size of the bounding box. */
    virtual ::com::sun::star::awt::Size SAL_CALL getSize()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  <TRUE/>, if the object is showing. */
    virtual sal_Bool SAL_CALL isShowing()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  <TRUE/>, if the object is visible. */
    virtual sal_Bool SAL_CALL isVisible()
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  <TRUE/>, if the object can accept the focus. */
    virtual sal_Bool SAL_CALL isFocusTraversable()
        throw ( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException);

    // XFocusListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent& e ) throw (::com::sun::star::uno::RuntimeException);

    /*  Derived classes have to implement:
        -   getAccessibleAt,
        -   grabFocus,
        -   getAccessibleKeyBinding. */

    /** @return
        No key bindings supported by default.
    */
    virtual ::com::sun::star::uno::Any SAL_CALL getAccessibleKeyBinding()
        throw ( ::com::sun::star::uno::RuntimeException );
    /** @return
            The accessible child rendered under the given point.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XAccessibleEventBroadcaster --------------------------------------------

    /** Adds a new event listener */
    using cppu::WeakAggComponentImplHelperBase::addEventListener;
    virtual void SAL_CALL addEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& rxListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    /** Removes an event listener. */
    using cppu::WeakAggComponentImplHelperBase::removeEventListener;
    virtual void SAL_CALL removeEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& rxListener )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XTypeProvider ----------------------------------------------------------

    /** @return  An unique implementation ID. */
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw ( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo -----------------------------------------------------------

    /** @return  Whether the specified service is supported by this class. */
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName )
        throw ( ::com::sun::star::uno::RuntimeException );

    /** @return  A list of all supported services. */
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw ( ::com::sun::star::uno::RuntimeException );

    /*  Derived classes have to implement:
        -   getImplementationName. */

    // helper methods ---------------------------------------------------------

    /** @return  The BrowseBox object type. */
    inline ::svt::AccessibleBrowseBoxObjType getType() const;

    /** Changes the name of the object and notifies listeners. */
    void setAccessibleName( const OUString& rName );
    /** Changes the description of the object and notifies listeners. */
    void setAccessibleDescription( const OUString& rDescription );

    /** Commits an event to all listeners. */
    void commitEvent(
            sal_Int16 nEventId,
            const ::com::sun::star::uno::Any& rNewValue,

    const ::com::sun::star::uno::Any& rOldValue );
    /** @return  <TRUE/>, if the object is not disposed or disposing. */
    sal_Bool isAlive() const;

protected:
    // internal virtual methods -----------------------------------------------

    /** Determines whether the BrowseBox control is really showing inside of
        its parent accessible window. Derived classes may implement different
        behaviour.
        @attention  This method requires locked mutex's and a living object.
        @return  <TRUE/>, if the object is really showing. */
    virtual sal_Bool implIsShowing();

    /** Derived classes return the bounding box relative to the parent window.
        @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) relative to the parent window. */
    virtual Rectangle implGetBoundingBox() = 0;
    /** Derived classes return the bounding box in screen coordinates.
        @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) in screen coordinates. */
    virtual Rectangle implGetBoundingBoxOnScreen() = 0;

    /** Creates a new AccessibleStateSetHelper and fills it with states of the
        current object. This method calls FillStateSet at the BrowseBox which
        fills it with more states depending on the object type. Derived classes
        may overwrite this method and add more states.
        @attention  This method requires locked mutex's.
        @return  A filled AccessibleStateSetHelper. */
    virtual ::utl::AccessibleStateSetHelper* implCreateStateSetHelper();

    // internal helper methods ------------------------------------------------

    /** @throws <type>DisposedException</type>  If the object is not alive. */
    void ensureIsAlive() const
        throw ( ::com::sun::star::lang::DisposedException );

    /** @return  The ::osl::Mutex member provided by the class OBaseMutex. */
    inline ::osl::Mutex& getOslMutex();
    /** @return  Pointer to the global ::osl::Mutex. */
    static inline ::osl::Mutex* getOslGlobalMutex();

    /** Changes the name of the object (flat assignment, no notify).
        @attention  This method requires a locked mutex. */
    inline void implSetName( const OUString& rName );
    /** Changes the description of the object (flat assignment, no notify).
        @attention  This method requires a locked mutex. */
    inline void implSetDescription( const OUString& rDescription );

    /** Locks all mutex's and calculates the bounding box relative to the
        parent window.
        @return  The bounding box (VCL rect.) relative to the parent object. */
    Rectangle getBoundingBox()
        throw ( ::com::sun::star::lang::DisposedException );
    /** Locks all mutex's and calculates the bounding box in screen
        coordinates.
        @return  The bounding box (VCL rect.) in screen coordinates. */
    Rectangle getBoundingBoxOnScreen()
        throw ( ::com::sun::star::lang::DisposedException );

    ::comphelper::AccessibleEventNotifier::TClientId getClientId() const { return m_aClientId; }
    void setClientId(::comphelper::AccessibleEventNotifier::TClientId _aNewClientId) { m_aClientId = _aNewClientId; }

public:
    // public versions of internal helper methods, with access control
    struct AccessControl { friend class SolarMethodGuard; private: AccessControl() { } };

    inline ::osl::Mutex&    getMutex( const AccessControl& ) { return getOslMutex(); }
    inline void             ensureIsAlive( const AccessControl& ) { ensureIsAlive(); }

protected:
    // members ----------------------------------------------------------------

    /** The parent accessible object. */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > mxParent;
    /** The VCL BrowseBox control. */
    ::svt::IAccessibleTableProvider* mpBrowseBox;

    /** This is the window which get all the nice focus events
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > m_xFocusWindow;

private:
    /** Localized name. */
    OUString maName;
    /** Localized description text. */
    OUString maDescription;

    /** The type of this object (for names, descriptions, state sets, ...). */
    ::svt::AccessibleBrowseBoxObjType meObjType;

    ::comphelper::AccessibleEventNotifier::TClientId    m_aClientId;
};

// ============================================================================
// a version of AccessibleBrowseBoxBase which implements not only the XAccessibleContext,
// but also the XAccessible

typedef ::cppu::ImplHelper1 <   ::com::sun::star::accessibility::XAccessible
                            >   BrowseBoxAccessibleElement_Base;

class BrowseBoxAccessibleElement
            :public AccessibleBrowseBoxBase
            ,public BrowseBoxAccessibleElement_Base
{
protected:
    /** Constructor sets specified name and description. If the constant of a
        text is BBTEXT_NONE, the derived class has to set the text via
        implSetName() and implSetDescription() (in Ctor) or later via
        setAccessibleName() and setAccessibleDescription() (these methods
        notify the listeners about the change).

        @param rxParent  XAccessible interface of the parent object.
        @param rBrowseBox  The BrowseBox control.
        @param eNameText  The constant for the name text.
        @param eDescrText  The constant for the description text.
    */
    BrowseBoxAccessibleElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rxParent,
        ::svt::IAccessibleTableProvider&                  rBrowseBox,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
        ::svt::AccessibleBrowseBoxObjType  eObjType );

    /** Constructor sets specified name and description.

        @param rxParent  XAccessible interface of the parent object.
        @param rBrowseBox  The BrowseBox control.
        @param rName  The name of this object.
        @param rDescription  The description text of this object.
    */
    BrowseBoxAccessibleElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rxParent,
        ::svt::IAccessibleTableProvider&                  rBrowseBox,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& _xFocusWindow,
        ::svt::AccessibleBrowseBoxObjType  eObjType,
        const OUString&      rName,
        const OUString&      rDescription );

public:
    // XInterface
    DECLARE_XINTERFACE( )
    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

protected:
    virtual ~BrowseBoxAccessibleElement();

protected:
    // XAccessible ------------------------------------------------------------

    /** @return  The XAccessibleContext interface of this object. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL
    getAccessibleContext()
        throw ( ::com::sun::star::uno::RuntimeException );

private:
    BrowseBoxAccessibleElement();                                               // never implemented
    BrowseBoxAccessibleElement( const BrowseBoxAccessibleElement& );            // never implemented
    BrowseBoxAccessibleElement& operator=( const BrowseBoxAccessibleElement& ); // never implemented
};

// ============================================================================
// a helper class for protecting methods which need to lock the solar mutex in addition to the own mutex

typedef ::osl::MutexGuard OslMutexGuard;

class SolarMethodGuard : public SolarMutexGuard, public OslMutexGuard
{
public:
    inline SolarMethodGuard( AccessibleBrowseBoxBase& _rOwner, bool _bEnsureAlive = true )
        :SolarMutexGuard( )
        ,OslMutexGuard( _rOwner.getMutex( AccessibleBrowseBoxBase::AccessControl() ) )
    {
        if ( _bEnsureAlive )
            _rOwner.ensureIsAlive( AccessibleBrowseBoxBase::AccessControl() );
    }
};

// inlines --------------------------------------------------------------------

inline ::svt::AccessibleBrowseBoxObjType AccessibleBrowseBoxBase::getType() const
{
    return meObjType;
}

inline ::osl::Mutex& AccessibleBrowseBoxBase::getOslMutex()
{
    return m_aMutex;
}

inline ::osl::Mutex* AccessibleBrowseBoxBase::getOslGlobalMutex()
{
    return ::osl::Mutex::getGlobalMutex();
}

inline void AccessibleBrowseBoxBase::implSetName(
        const OUString& rName )
{
    maName = rName;
}

inline void AccessibleBrowseBoxBase::implSetDescription(
        const OUString& rDescription )
{
    maDescription = rDescription;
}

// ============================================================================

} // namespace accessibility

// ============================================================================

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
