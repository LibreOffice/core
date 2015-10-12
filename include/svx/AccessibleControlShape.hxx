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

#ifndef INCLUDED_SVX_ACCESSIBLECONTROLSHAPE_HXX
#define INCLUDED_SVX_ACCESSIBLECONTROLSHAPE_HXX

#include <svx/AccessibleShape.hxx>

#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleEventListener.hpp>
#include <com/sun/star/util/XModeChangeBroadcaster.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <cppuhelper/implbase4.hxx>
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

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL  GetControlModel( ) { return m_xControlModel;} ;
    AccessibleControlShape* SAL_CALL GetLabeledByControlShape();
protected:
    //---  XAccessible  ----------------------------------------
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext> SAL_CALL getAccessibleContext( ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //---  XAccessibleComponent  -------------------------------
    /// forward the focus to the contained control(in alive mode)
    virtual void SAL_CALL grabFocus( ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //---  XAccessibleContext  ---------------------------------
    virtual sal_Int32 SAL_CALL getAccessibleChildCount( ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    //---  XServiceInfo  ---------------------------------------
    virtual OUString SAL_CALL getImplementationName( ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //---  XInterface  -----------------------------------------
    DECLARE_XINTERFACE( )

    //---  XTypeProvider  --------------------------------------
    DECLARE_XTYPEPROVIDER( )

    //---  XPropertyChangeListener  ----------------------------
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //---  XComponent  -----------------------------------------
    virtual void SAL_CALL disposing( ) override;

    //---  XEventListener  -------------------------------------
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //---  XModeChangeListener  --------------------------------
    virtual void SAL_CALL modeChanged( const ::com::sun::star::util::ModeChangeEvent& _rSource ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //---  XAccessibleEventListener ----------------------------
    virtual void SAL_CALL notifyEvent( const ::com::sun::star::accessibility::AccessibleEventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    //---  document::XEventListener ----------------------------
    using AccessibleShape::notifyEvent;

    // XVclContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    /** Initialize a new shape.  See the documentation of the base' constructor
        for the reason of this method's existence.
    */
    virtual void Init( ) override;

    /// Create a name string that contains the accessible name.
    virtual OUString
        CreateAccessibleBaseName( )
        throw(::com::sun::star::uno::RuntimeException) override;

    /** Create a unique name string that contains the accessible name.  The
        name consists of the base name and the index.
    */
    virtual OUString
        CreateAccessibleName( )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    /// Create a description string that contains the accessible description.
    virtual OUString
        CreateAccessibleDescription( )
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

#ifdef DBG_UTIL
    /// Set the specified state
    virtual bool SetState( sal_Int16 _nState ) override;
#endif // DBG_UTIL

    /// (safely) reads the given property from the model of the UNO control
    OUString getControlModelStringProperty( const OUString& _rPropertyName ) const;

    /// ensure that our control model exists(will be retrieved upon need only)
    bool ensureControlModelAccess( );

    /// ensures that we're listening for the given property if(and only if!) necessary
    bool ensureListeningState( const bool _bCurrentlyListening, const bool _bNeedNewListening,
                const OUString& _rPropertyName );

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

    bool        m_bListeningForName     : 1;    // are we currently listening for changes of the "Name" property?
    bool        m_bListeningForDesc     : 1;    // are we currently listening for changes of the "HelpText" property?
    bool        m_bMultiplexingStates   : 1;    // are we currently multiplexing state changes of the native context?
    bool        m_bDisposeNativeContext : 1;    // do we need to dispose mxNativeContextComponent?
    bool        m_bWaitingForControl    : 1;    // if we are created before our control exists, we need to wait for it to appear ...

private:
    AccessibleControlShape(const AccessibleControlShape&) = delete;

    AccessibleControlShape& operator= (const AccessibleControlShape&) = delete;
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
