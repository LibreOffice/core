/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_CONTROL_SHAPE_HXX
#define _SVX_ACCESSIBILITY_ACCESSIBLE_CONTROL_SHAPE_HXX

#include <svx/AccessibleShape.hxx>

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <cppuhelper/implbase3.hxx>
#include <comphelper/uno3.hxx>

namespace com { namespace sun { namespace star { namespace awt {
    class XControl;
} } } }

namespace comphelper
{
    class OWrappedAccessibleChildrenManager;
}

class SdrObject;
namespace accessibility {

    typedef ::cppu::ImplHelper4 <   ::com::sun::star::beans::XPropertyChangeListener
                                ,   ::com::sun::star::util::XModeChangeListener
                                                ,   ::com::sun::star::container::XContainerListener
                                ,   ::com::sun::star::accessibility::XAccessibleEventListener
                                >   AccessibleControlShape_Base;
/** @descr
*/
class AccessibleControlShape
        :public AccessibleShape
        ,public AccessibleControlShape_Base
{
public:
    //=====  internal  ========================================================
    AccessibleControlShape(
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo);
    virtual ~AccessibleControlShape( );

protected:
    //---  XAccessible  ----------------------------------------
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL getAccessibleContext( ) throw(::com::sun::star::uno::RuntimeException);

    //---  XAccessibleComponent  -------------------------------
    /// forward the focus to the contained control(in alive mode)
    virtual void SAL_CALL grabFocus( ) throw(::com::sun::star::uno::RuntimeException);

    //---  XAccessibleContext  ---------------------------------
    virtual sal_Int32 SAL_CALL getAccessibleChildCount( ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException);

    //---  XServiceInfo  ---------------------------------------
    virtual ::rtl::OUString SAL_CALL getImplementationName( ) throw(::com::sun::star::uno::RuntimeException);

    //---  XInterface  -----------------------------------------
    DECLARE_XINTERFACE( )

    //---  XTypeProvider  --------------------------------------
    DECLARE_XTYPEPROVIDER( )

    //---  XPropertyChangeListener  ----------------------------
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException);

    //---  XComponent  -----------------------------------------
    virtual void SAL_CALL disposing( );

    //---  XEventListener  -------------------------------------
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException);

    //---  XModeChangeListener  --------------------------------
    virtual void SAL_CALL modeChanged( const ::com::sun::star::util::ModeChangeEvent& _rSource ) throw(::com::sun::star::uno::RuntimeException);

    //---  XAccessibleEventListener ----------------------------
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

    //---  document::XEventListener ----------------------------
    using AccessibleShape::notifyEvent;

    // XVclContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

protected:
    /** Initialize a new shape.  See the documentation of the base' constructor
        for the reason of this method's existence.
    */
    virtual void Init( );

    /// Create a name string that contains the accessible name.
    virtual ::rtl::OUString
        CreateAccessibleBaseName( )
        throw(::com::sun::star::uno::RuntimeException);

    /** Create a unique name string that contains the accessible name.  The
        name consists of the base name and the index.
    */
    virtual ::rtl::OUString
        CreateAccessibleName( )
        throw(::com::sun::star::uno::RuntimeException);

    /// Create a description string that contains the accessible description.
    virtual ::rtl::OUString
        CreateAccessibleDescription( )
        throw(::com::sun::star::uno::RuntimeException);

#ifdef DBG_UTIL
    /// Set the specified state
    virtual sal_Bool SetState( sal_Int16 _nState );
#endif // DBG_UTIL

    /// (safely) reads the given property from the model of the UNO control
    ::rtl::OUString getControlModelStringProperty( const ::rtl::OUString& _rPropertyName ) const SAL_THROW(( ));

    /// ensure that our control model exists(will be retrieved upon need only)
    sal_Bool ensureControlModelAccess( ) SAL_THROW(( ));

    /// ensures that we're listening for the given property if(and only if!) necessary
    sal_Bool ensureListeningState( const sal_Bool _bCurrentlyListening, const sal_Bool _bNeedNewListening,
                const ::rtl::OUString& _rPropertyName );

    /// starts multiplexing the state changes of our aggregate context
    void    startStateMultiplexing( );
    /// stops multiplexing the state changes of our aggregate context
    void    stopStateMultiplexing( );

    /// retrieves the SdrObject of the shape we represent
    SdrObject*  getSdrObject( ) const;

    /** adjusts our AccessibleRole, depending on the control type we're working for

        <p>Only to be called during inituialization</p>
    */
    void        adjustAccessibleRole( );

    /** initializes composed states of the context

        <p>Some of the states of our inner context need to be propagated to the "composed context", too
        (such as "checked" for check boxes). At lifetime, this is done by multiplexing state changes,
        at initialization time, this method is used.</p>
    */
    void        initializeComposedState( );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    m_xControlModel;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                    m_xModelPropsMeta;          // cache this for performance reasons
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >
                    m_xUnoControl;              // our UNO control

    ::com::sun::star::uno::WeakReference< ::com::sun::star::accessibility::XAccessibleContext >
                    m_aControlContext;      // the AccessibleContext of the control
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >
                    m_xControlContextProxy; // the proxy for "aggregating" the AccessibleContext of the control
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider >
                    m_xControlContextTypeAccess;    // cached interface of our aggregate
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
                    m_xControlContextComponent;     // cached interface of our aggregate

    ::comphelper::OWrappedAccessibleChildrenManager*
                    m_pChildManager;

    sal_Bool        m_bListeningForName     : 1;    // are we currently listening for changes of the "Name" property?
    sal_Bool        m_bListeningForDesc     : 1;    // are we currently listening for changes of the "HelpText" property?
    sal_Bool        m_bMultiplexingStates   : 1;    // are we currently multiplexing state changes of the native context?
    sal_Bool        m_bDisposeNativeContext : 1;    // do we need to dispose mxNativeContextComponent?
    sal_Bool        m_bWaitingForControl    : 1;    // if we are created before our control exists, we need to wait for it to appear ...

private:
    /** Don't use the default constructor.  Use the public constructor that
        takes the original shape and the parent as arguments instead.
    */
    AccessibleControlShape( );

    /// Don't use the constructor. not implemented.
    AccessibleControlShape(const AccessibleControlShape&);

    /// Don't use the assignment operator. not implemented.
    AccessibleControlShape& operator= (const AccessibleControlShape&);
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
