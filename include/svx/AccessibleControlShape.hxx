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

    typedef ::cppu::ImplHelper4 <   css::beans::XPropertyChangeListener
                                ,   css::util::XModeChangeListener
                                                ,   css::container::XContainerListener
                                ,   css::accessibility::XAccessibleEventListener
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

    css::uno::Reference< css::beans::XPropertySet > SAL_CALL  GetControlModel( ) { return m_xControlModel;} ;
    AccessibleControlShape* SAL_CALL GetLabeledByControlShape();
protected:
    //---  XAccessible  ----------------------------------------
    virtual css::uno::Reference< css::accessibility::XAccessibleContext> SAL_CALL getAccessibleContext( ) throw(css::uno::RuntimeException, std::exception) override;

    //---  XAccessibleComponent  -------------------------------
    /// forward the focus to the contained control(in alive mode)
    virtual void SAL_CALL grabFocus( ) throw(css::uno::RuntimeException, std::exception) override;

    //---  XAccessibleContext  ---------------------------------
    virtual sal_Int32 SAL_CALL getAccessibleChildCount( ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int32 i ) throw(css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) throw (css::uno::RuntimeException, std::exception) override;

    //---  XServiceInfo  ---------------------------------------
    virtual OUString SAL_CALL getImplementationName( ) throw(css::uno::RuntimeException, std::exception) override;

    //---  XInterface  -----------------------------------------
    DECLARE_XINTERFACE( )

    //---  XTypeProvider  --------------------------------------
    DECLARE_XTYPEPROVIDER( )

    //---  XPropertyChangeListener  ----------------------------
    virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& _rEvent ) throw(css::uno::RuntimeException, std::exception) override;

    //---  XComponent  -----------------------------------------
    virtual void SAL_CALL disposing( ) override;

    //---  XEventListener  -------------------------------------
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw(css::uno::RuntimeException, std::exception) override;

    //---  XModeChangeListener  --------------------------------
    virtual void SAL_CALL modeChanged( const css::util::ModeChangeEvent& _rSource ) throw(css::uno::RuntimeException, std::exception) override;

    //---  XAccessibleEventListener ----------------------------
    virtual void SAL_CALL notifyEvent( const css::accessibility::AccessibleEventObject& aEvent ) throw(css::uno::RuntimeException, std::exception) override;

    //---  document::XEventListener ----------------------------
    using AccessibleShape::notifyEvent;

    // XVclContainerListener
    virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;

protected:
    /** Initialize a new shape.  See the documentation of the base' constructor
        for the reason of this method's existence.
    */
    virtual void Init( ) override;

    /// Create a name string that contains the accessible name.
    virtual OUString
        CreateAccessibleBaseName( )
        throw(css::uno::RuntimeException) override;

    /** Create a unique name string that contains the accessible name.  The
        name consists of the base name and the index.
    */
    virtual OUString
        CreateAccessibleName( )
        throw(css::uno::RuntimeException, std::exception) override;

    /// Create a description string that contains the accessible description.
    virtual OUString
        CreateAccessibleDescription( )
        throw(css::uno::RuntimeException, std::exception) override;

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
    css::uno::Reference< css::beans::XPropertySet >
                    m_xControlModel;
    css::uno::Reference< css::beans::XPropertySetInfo >
                    m_xModelPropsMeta;          // cache this for performance reasons
    css::uno::Reference< css::awt::XControl >
                    m_xUnoControl;              // our UNO control

    css::uno::WeakReference< css::accessibility::XAccessibleContext >
                    m_aControlContext;      // the AccessibleContext of the control
    css::uno::Reference< css::uno::XAggregation >
                    m_xControlContextProxy; // the proxy for "aggregating" the AccessibleContext of the control
    css::uno::Reference< css::lang::XTypeProvider >
                    m_xControlContextTypeAccess;    // cached interface of our aggregate
    css::uno::Reference< css::lang::XComponent >
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
