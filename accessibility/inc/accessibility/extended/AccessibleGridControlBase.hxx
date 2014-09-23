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


#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEGRIDCONTROLBASE_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_EXTENDED_ACCESSIBLEGRIDCONTROLBASE_HXX

#include <svtools/accessibletable.hxx>
#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <vcl/svapp.hxx>
#include <cppuhelper/compbase4.hxx>
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
#include <comphelper/accessibleeventnotifier.hxx>
#include <comphelper/uno3.hxx>



namespace vcl { class Window; }

namespace utl {
    class AccessibleStateSetHelper;
}



namespace accessibility {





typedef ::cppu::WeakAggComponentImplHelper4<
            ::com::sun::star::accessibility::XAccessibleContext,
            ::com::sun::star::accessibility::XAccessibleComponent,
            ::com::sun::star::accessibility::XAccessibleEventBroadcaster,
            ::com::sun::star::lang::XServiceInfo >
        AccessibleGridControlImplHelper;

/** The GridControl accessible objects inherit from this base class. It
    implements basic functionality for various Accessibility interfaces and
    the event broadcaster and contains the ::osl::Mutex. */
class AccessibleGridControlBase :
    public ::comphelper::OBaseMutex,
    public AccessibleGridControlImplHelper
{
public:
    /** Constructor sets specified name and description.
        @param rxParent  XAccessible interface of the parent object.
        @param rTable  The Table control.
        @param eNameText  The constant for the name text.
        @param eDescrText  The constant for the description text. */
    AccessibleGridControlBase(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessible >& rxParent,
        ::svt::table::IAccessibleTable& rTable,
        ::svt::table::AccessibleTableControlObjType  eObjType );

protected:
    virtual ~AccessibleGridControlBase();

    /** Commits DeFunc event to listeners and cleans up members. */
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

public:
    // XAccessibleContext -----------------------------------------------------

    /** @return  A reference to the parent accessible object. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleParent()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return  The index of this object among the parent's children. */
    virtual sal_Int32 SAL_CALL getAccessibleIndexInParent()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return
            The description of this object.
    */
    virtual OUString SAL_CALL getAccessibleDescription()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return
            The name of this object.
    */
    virtual OUString SAL_CALL getAccessibleName()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return
            The relation set (the GridControl does not have one).
    */
    virtual ::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL
        getAccessibleRelationSet()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return  The set of current states. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleStateSet > SAL_CALL
        getAccessibleStateSet()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return  The parent's locale. */
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale()
        throw ( ::com::sun::star::accessibility::IllegalAccessibleComponentStateException,
                ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return
            The role of this object. Panel, ROWHEADER, COLUMNHEADER, TABLE, TABLE_CELL are supported.
    */
    virtual sal_Int16 SAL_CALL getAccessibleRole()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

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
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return  The bounding box of this object. */
    virtual ::com::sun::star::awt::Rectangle SAL_CALL getBounds()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return
        The upper left corner of the bounding box relative to the parent. */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocation()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return
        The upper left corner of the bounding box in screen coordinates. */
    virtual ::com::sun::star::awt::Point SAL_CALL getLocationOnScreen()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return  The size of the bounding box. */
    virtual ::com::sun::star::awt::Size SAL_CALL getSize()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    virtual sal_Int32 SAL_CALL getForeground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL getBackground(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;


    /*  Derived classes have to implement:
        -   getAccessibleAt,
        -   grabFocus. */

    /** @return
            The accessible child rendered under the given point.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > SAL_CALL
    getAccessibleAtPoint( const ::com::sun::star::awt::Point& rPoint )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XAccessibleEventBroadcaster --------------------------------------------

    /** Adds a new event listener */
    virtual void SAL_CALL addAccessibleEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& rxListener )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** Removes an event listener. */
    virtual void SAL_CALL removeAccessibleEventListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::accessibility::XAccessibleEventListener>& rxListener )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XTypeProvider ----------------------------------------------------------

    /** @return  An unique implementation ID. */
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    // XServiceInfo -----------------------------------------------------------

    /** @return  Whether the specified service is supported by this class. */
    virtual sal_Bool SAL_CALL supportsService( const OUString& rServiceName )
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /** @return  A list of all supported services. */
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

    /*  Derived classes have to implement:
        -   getImplementationName. */

    // helper methods ---------------------------------------------------------

    /** @return  The GridControl object type. */
    inline ::svt::table::AccessibleTableControlObjType getType() const;

    /** Commits an event to all listeners. */
    void commitEvent(
            sal_Int16 nEventId,
            const ::com::sun::star::uno::Any& rNewValue,

    const ::com::sun::star::uno::Any& rOldValue );
    /** @return  <TRUE/>, if the object is not disposed or disposing. */
    bool isAlive() const;

protected:
    // internal virtual methods -----------------------------------------------

    /** Determines whether the Grid control is really showing inside of
        its parent accessible window. Derived classes may implement different
        behaviour.
        @attention  This method requires locked mutex's and a living object.
        @return  <TRUE/>, if the object is really showing. */
    virtual bool implIsShowing();

    /** Derived classes return the bounding box relative to the parent window.
        @attention  This method requires locked mutex's and a living object.
        @return  The bounding box (VCL rect.) relative to the parent window. */
    virtual Rectangle implGetBoundingBox() = 0;
    ///** Derived classes return the bounding box in screen coordinates.
    //    @attention  This method requires locked mutex's and a living object.
    //    @return  The bounding box (VCL rect.) in screen coordinates. */
    virtual Rectangle implGetBoundingBoxOnScreen() = 0;

    /** Creates a new AccessibleStateSetHelper and fills it with states of the
        current object. This method calls FillStateSet at the GridControl which
        fills it with more states depending on the object type. Derived classes
        may overwrite this method and add more states.
        @attention  This method requires locked mutex's.
        @return  A filled AccessibleStateSetHelper. */
    virtual ::utl::AccessibleStateSetHelper* implCreateStateSetHelper();

    // internal helper methods ------------------------------------------------

    /** @throws <type>DisposedException</type>  If the object is not alive. */
    void ensureIsAlive() const
        throw ( ::com::sun::star::lang::DisposedException );

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
    ///** Locks all mutex's and calculates the bounding box in screen
    //    coordinates.
    //    @return  The bounding box (VCL rect.) in screen coordinates. */
    Rectangle getBoundingBoxOnScreen()
        throw ( ::com::sun::star::lang::DisposedException );

    ::comphelper::AccessibleEventNotifier::TClientId getClientId() const { return m_aClientId; }
    void setClientId(::comphelper::AccessibleEventNotifier::TClientId _aNewClientId) { m_aClientId = _aNewClientId; }

protected:
    // members ----------------------------------------------------------------

    /** The parent accessible object. */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > m_xParent;
    /** The SVT Table control. */
    ::svt::table::IAccessibleTable& m_aTable;
    /** The type of this object (for names, descriptions, state sets, ...). */
    ::svt::table::AccessibleTableControlObjType m_eObjType;

private:
    /** Localized name. */
    OUString m_aName;
    /** Localized description text. */
    OUString m_aDescription;
    ::comphelper::AccessibleEventNotifier::TClientId    m_aClientId;
};


// a version of AccessibleGridControlBase which implements not only the XAccessibleContext,
// but also the XAccessible

typedef ::cppu::ImplHelper1 <   ::com::sun::star::accessibility::XAccessible
                            >   GridControlAccessibleElement_Base;

class GridControlAccessibleElement
            :public AccessibleGridControlBase
            ,public GridControlAccessibleElement_Base
{
protected:
    /** Constructor sets specified name and description.

        @param rxParent  XAccessible interface of the parent object.
        @param rTable  The Table control.
        @param eNameText  The constant for the name text.
        @param eDescrText  The constant for the description text.
    */
    GridControlAccessibleElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible >& rxParent,
            ::svt::table::IAccessibleTable& rTable,
        ::svt::table::AccessibleTableControlObjType  eObjType );

public:
    // XInterface
    DECLARE_XINTERFACE( )
    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )

protected:
    virtual ~GridControlAccessibleElement();

protected:
    // XAccessible ------------------------------------------------------------

    /** @return  The XAccessibleContext interface of this object. */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessibleContext > SAL_CALL
    getAccessibleContext()
        throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

private:
    GridControlAccessibleElement();                                             // never implemented
    GridControlAccessibleElement( const GridControlAccessibleElement& );        // never implemented
    GridControlAccessibleElement& operator=( const GridControlAccessibleElement& ); // never implemented
};

// inlines --------------------------------------------------------------------

inline ::svt::table::AccessibleTableControlObjType AccessibleGridControlBase::getType() const
{
    return m_eObjType;
}

inline void AccessibleGridControlBase::implSetName(
        const OUString& rName )
{
    m_aName = rName;
}

inline void AccessibleGridControlBase::implSetDescription(
        const OUString& rDescription )
{
    m_aDescription = rDescription;
}



} // namespace accessibility



#endif // ACCESSIBILITY_EXT_ACCESSIBILEGRIDCONTROLBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
