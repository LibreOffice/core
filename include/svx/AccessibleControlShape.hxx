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

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XModeChangeListener.hpp>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <svx/AccessibleShape.hxx>

namespace com::sun::star {
    namespace accessibility { class XAccessibleContext; }
    namespace awt { class XControl; }
    namespace beans { class XPropertySet; }
    namespace beans { class XPropertySetInfo; }
    namespace lang { class XComponent; }
    namespace lang { class XTypeProvider; }
    namespace uno { class XAggregation; }
}

namespace comphelper
{
    class OWrappedAccessibleChildrenManager;
}

namespace accessibility {

    class AccessibleShapeInfo;
    class AccessibleShapeTreeInfo;

    typedef ::cppu::ImplHelper4 <   css::beans::XPropertyChangeListener
                                ,   css::util::XModeChangeListener
                                                ,   css::container::XContainerListener
                                ,   css::accessibility::XAccessibleEventListener
                                >   AccessibleControlShape_Base;
/** @descr
*/
class SAL_DLLPUBLIC_RTTI AccessibleControlShape final
        :public AccessibleShape
        ,public AccessibleControlShape_Base
{
public:
    //=====  internal  ========================================================
    AccessibleControlShape(
        const AccessibleShapeInfo& rShapeInfo,
        const AccessibleShapeTreeInfo& rShapeTreeInfo);
    virtual ~AccessibleControlShape( ) override;

    const css::uno::Reference< css::beans::XPropertySet >&  GetControlModel( ) const { return m_xControlModel;}
    AccessibleControlShape* GetLabeledByControlShape();

private:
    //---  XAccessibleComponent  -------------------------------
    /// forward the focus to the contained control(in alive mode)
    virtual void SAL_CALL grabFocus( ) override;

    //---  XAccessibleContext  ---------------------------------
    virtual sal_Int64 SAL_CALL getAccessibleChildCount( ) override;
    virtual css::uno::Reference< css::accessibility::XAccessible > SAL_CALL getAccessibleChild( sal_Int64 i ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL getAccessibleRelationSet(  ) override;

    //---  XServiceInfo  ---------------------------------------
    virtual OUString SAL_CALL getImplementationName( ) override;

    //---  XInterface  -----------------------------------------
    DECLARE_XINTERFACE( )

    //---  XTypeProvider  --------------------------------------
    DECLARE_XTYPEPROVIDER( )

    //---  XPropertyChangeListener  ----------------------------
    virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& _rEvent ) override;

    //---  XComponent  -----------------------------------------
    virtual void SAL_CALL disposing( ) override;

    //---  XEventListener  -------------------------------------
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

    //---  XModeChangeListener  --------------------------------
    virtual void SAL_CALL modeChanged( const css::util::ModeChangeEvent& _rSource ) override;

    //---  XAccessibleEventListener ----------------------------
    virtual void SAL_CALL notifyEvent( const css::accessibility::AccessibleEventObject& aEvent ) override;

    // XVclContainerListener
    virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override;

    /** Initialize a new shape.  See the documentation of the base' constructor
        for the reason of this method's existence.
    */
    virtual void Init( ) override;

    /// Create a name string that contains the accessible name.
    virtual OUString
        CreateAccessibleBaseName( ) override;

    /** Create a unique name string that contains the accessible name.  The
        name consists of the base name and the index.
    */
    virtual OUString
        CreateAccessibleName( ) override;

    /// Create a description string that contains the accessible description.
    OUString CreateAccessibleDescription();

#ifdef DBG_UTIL
    /// Set the specified state
    virtual bool SetState( sal_Int64 _nState ) override;
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

    /** adjusts our AccessibleRole, depending on the control type we're working for

        <p>Only to be called during initialization</p>
    */
    void        adjustAccessibleRole( );

    /** initializes composed states of the context

        <p>Some of the states of our inner context need to be propagated to the "composed context", too
        (such as "checked" for check boxes). At lifetime, this is done by multiplexing state changes,
        at initialization time, this method is used.</p>
    */
    void        initializeComposedState( );

    AccessibleControlShape(const AccessibleControlShape&) = delete;
    AccessibleControlShape& operator= (const AccessibleControlShape&) = delete;

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

    rtl::Reference<::comphelper::OWrappedAccessibleChildrenManager>
                    m_pChildManager;

    bool        m_bListeningForName     : 1;    // are we currently listening for changes of the "Name" property?
    bool        m_bListeningForDesc     : 1;    // are we currently listening for changes of the "HelpText" property?
    bool        m_bMultiplexingStates   : 1;    // are we currently multiplexing state changes of the native context?
    bool        m_bDisposeNativeContext : 1;    // do we need to dispose mxNativeContextComponent?
    bool        m_bWaitingForControl    : 1;    // if we are created before our control exists, we need to wait for it to appear ...
};

} // end of namespace accessibility

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
