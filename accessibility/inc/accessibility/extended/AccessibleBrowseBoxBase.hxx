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


#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEBROWSEBOXBASE_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEBROWSEBOXBASE_HXX

#include <sal/config.h>

#include <boost/noncopyable.hpp>
#include <svtools/AccessibleBrowseBoxObjType.hxx>
#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/compbase.hxx>
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



namespace vcl { class Window; }

namespace utl {
    class AccessibleStateSetHelper;
}

namespace svt {
    class IAccessibleTableProvider;
}



namespace accessibility {



typedef ::cppu::WeakAggComponentImplHelper<
            css::accessibility::XAccessibleContext,
            css::accessibility::XAccessibleComponent,
            css::accessibility::XAccessibleEventBroadcaster,
            css::awt::XFocusListener,
            css::lang::XServiceInfo >
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
        implSetName() (in Ctor) or later via
        setAccessibleName() and setAccessibleDescription() (these methods
        notify the listeners about the change).
        @param rxParent  XAccessible interface of the parent object.
        @param rBrowseBox  The BrowseBox control.
        @param eNameText  The constant for the name text.
        @param eDescrText  The constant for the description text. */
    AccessibleBrowseBoxBase(
        const css::uno::Reference<
                  css::accessibility::XAccessible >& rxParent,
        ::svt::IAccessibleTableProvider&                  rBrowseBox,
        const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
        ::svt::AccessibleBrowseBoxObjType  eObjType );

    /** Constructor sets specified name and description.
        @param rxParent  XAccessible interface of the parent object.
        @param rBrowseBox  The BrowseBox control.
        @param rName  The name of this object.
        @param rDescription  The description text of this object. */
    AccessibleBrowseBoxBase(
        const css::uno::Reference<
                  css::accessibility::XAccessible >& rxParent,
        ::svt::IAccessibleTableProvider&                  rBrowseBox,
        const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
        ::svt::AccessibleBrowseBoxObjType  eObjType,
        const OUString&      rName,
        const OUString&      rDescription );

protected:
    virtual ~AccessibleBrowseBoxBase();

    /** Commits DeFunc event to listeners and cleans up members. */
    virtual void SAL_CALL disposing() override;

public:
    // XAccessibleContext -----------------------------------------------------

    /** @return  A reference to the parent accessible object. */
    virtual css::uno::Reference<css::accessibility::XAccessible > SAL_CALL getAccessibleParent()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return  The index of this object among the parent's children. */
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return
            The description of this object.
    */
    virtual OUString SAL_CALL getAccessibleDescription()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return
            The name of this object.
    */
    virtual OUString SAL_CALL getAccessibleName()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return
            The relation set (the BrowseBox does not have one).
    */
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return  The set of current states. */
    virtual css::uno::Reference< css::accessibility::XAccessibleStateSet > SAL_CALL getAccessibleStateSet()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return  The parent's locale. */
    virtual css::lang::Locale SAL_CALL getLocale()
        throw ( css::accessibility::IllegalAccessibleComponentStateException,
                css::uno::RuntimeException, std::exception ) override;

    /** @return
            The role of this object. Panel, ROWHEADER, COLUMNHEADER, TABLE, TABLE_CELL are supported.
    */
    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /*  Derived classes have to implement:
        -   getAccessibleChildCount,
        -   getAccessibleChild,
        -   getAccessibleRole.
        Derived classes may overwrite getAccessibleIndexInParent to increase
        performance. */

    // XAccessibleComponent ---------------------------------------------------

    /** @return
        <TRUE/>, if the point lies within the bounding box of this object. */
    virtual sal_Bool SAL_CALL containsPoint( const css::awt::Point& rPoint )
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return  The bounding box of this object. */
    virtual css::awt::Rectangle SAL_CALL getBounds()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return
        The upper left corner of the bounding box relative to the parent. */
    virtual css::awt::Point SAL_CALL getLocation()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return
        The upper left corner of the bounding box in screen coordinates. */
    virtual css::awt::Point SAL_CALL getLocationOnScreen()
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return  The size of the bounding box. */
    virtual css::awt::Size SAL_CALL getSize()
        throw ( css::uno::RuntimeException, std::exception ) override;

    virtual sal_Int32 SAL_CALL getForeground(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (css::uno::RuntimeException, std::exception) override;

    // XFocusListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL focusGained( const css::awt::FocusEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL focusLost( const css::awt::FocusEvent& e ) throw (css::uno::RuntimeException, std::exception) override;

    /*  Derived classes have to implement:
        -   getAccessibleAt,
        -   grabFocus. */

    /** @return
            The accessible child rendered under the given point.
    */
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleAtPoint( const css::awt::Point& rPoint )
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XAccessibleEventBroadcaster --------------------------------------------

    /** Adds a new event listener */
    virtual void SAL_CALL addAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener>& rxListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** Removes an event listener. */
    virtual void SAL_CALL removeAccessibleEventListener(
            const css::uno::Reference< css::accessibility::XAccessibleEventListener>& rxListener )
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XTypeProvider ----------------------------------------------------------

    /** @return  An unique implementation ID. */
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw ( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo -----------------------------------------------------------

    /** @return  Whether the specified service is supported by this class. */
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName )
        throw ( css::uno::RuntimeException, std::exception ) override;

    /** @return  A list of all supported services. */
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw ( css::uno::RuntimeException, std::exception ) override;

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
            const css::uno::Any& rNewValue,
            const css::uno::Any& rOldValue );
    /** @return  <TRUE/>, if the object is not disposed or disposing. */
    bool isAlive() const;

protected:
    // internal virtual methods -----------------------------------------------

    /** Determines whether the BrowseBox control is really showing inside of
        its parent accessible window. Derived classes may implement different
        behaviour.
        @attention  This method requires locked mutex's and a living object.
        @return  <TRUE/>, if the object is really showing. */
    bool implIsShowing();

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
        throw ( css::lang::DisposedException );

    /** @return  The ::osl::Mutex member provided by the class OBaseMutex. */
    inline ::osl::Mutex& getOslMutex();

    /** Changes the name of the object (flat assignment, no notify).
        @attention  This method requires a locked mutex. */
    inline void implSetName( const OUString& rName );

    /** Locks all mutex's and calculates the bounding box relative to the
        parent window.
        @return  The bounding box (VCL rect.) relative to the parent object. */
    Rectangle getBoundingBox()
        throw ( css::lang::DisposedException );
    /** Locks all mutex's and calculates the bounding box in screen
        coordinates.
        @return  The bounding box (VCL rect.) in screen coordinates. */
    Rectangle getBoundingBoxOnScreen()
        throw ( css::lang::DisposedException );

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
    css::uno::Reference< css::accessibility::XAccessible > mxParent;
    /** The VCL BrowseBox control. */
    ::svt::IAccessibleTableProvider* mpBrowseBox;

    /** This is the window which get all the nice focus events
    */
    css::uno::Reference< css::awt::XWindow > m_xFocusWindow;

private:
    /** Localized name. */
    OUString maName;
    /** Localized description text. */
    OUString maDescription;

    /** The type of this object (for names, descriptions, state sets, ...). */
    ::svt::AccessibleBrowseBoxObjType meObjType;

    ::comphelper::AccessibleEventNotifier::TClientId    m_aClientId;
};


// a version of AccessibleBrowseBoxBase which implements not only the XAccessibleContext,
// but also the XAccessible

typedef ::cppu::ImplHelper  <   css::accessibility::XAccessible
                            >   BrowseBoxAccessibleElement_Base;

class BrowseBoxAccessibleElement
            :public AccessibleBrowseBoxBase
            ,public BrowseBoxAccessibleElement_Base
            ,private boost::noncopyable
{
protected:
    /** Constructor sets specified name and description. If the constant of a
        text is BBTEXT_NONE, the derived class has to set the text via
        implSetName() (in Ctor) or later via
        setAccessibleName() and setAccessibleDescription() (these methods
        notify the listeners about the change).

        @param rxParent  XAccessible interface of the parent object.
        @param rBrowseBox  The BrowseBox control.
        @param eNameText  The constant for the name text.
        @param eDescrText  The constant for the description text.
    */
    BrowseBoxAccessibleElement(
        const css::uno::Reference<
                  css::accessibility::XAccessible >& rxParent,
        ::svt::IAccessibleTableProvider&                  rBrowseBox,
        const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
        ::svt::AccessibleBrowseBoxObjType  eObjType );

    /** Constructor sets specified name and description.

        @param rxParent  XAccessible interface of the parent object.
        @param rBrowseBox  The BrowseBox control.
        @param rName  The name of this object.
        @param rDescription  The description text of this object.
    */
    BrowseBoxAccessibleElement(
        const css::uno::Reference<
                  css::accessibility::XAccessible >& rxParent,
        ::svt::IAccessibleTableProvider&                  rBrowseBox,
        const css::uno::Reference< css::awt::XWindow >& _xFocusWindow,
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
    virtual css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL getAccessibleContext()
        throw ( css::uno::RuntimeException, std::exception ) override;
};


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

inline void AccessibleBrowseBoxBase::implSetName(
        const OUString& rName )
{
    maName = rName;
}

} // namespace accessibility



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
