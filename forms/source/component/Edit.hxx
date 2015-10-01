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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_EDIT_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_EDIT_HXX

#include "EditBase.hxx"

#include <tools/link.hxx>
#include <cppuhelper/implbase3.hxx>

namespace dbtools { class FormattedColumnValue; }
struct ImplSVEvent;

namespace frm
{

class OEditModel : public OEditBaseModel
{
    ::std::unique_ptr< ::dbtools::FormattedColumnValue >
                                m_pValueFormatter;
    bool                    m_bMaxTextLenModified : 1;  // set to <TRUE/> when we change the MaxTextLen of the aggregate

    bool                    m_bWritingFormattedFake : 1; // are we writing something which should be interpreted as formatted upon reading?

public:
    DECLARE_DEFAULT_LEAF_XTOR( OEditModel );

protected:
    virtual css::uno::Sequence< css::uno::Type> _getTypes() SAL_OVERRIDE;

    void enableFormattedWriteFake() { m_bWritingFormattedFake = true; }
    void disableFormattedWriteFake() { m_bWritingFormattedFake = false; }
    bool lastReadWasFormattedFake() const { return (getLastReadVersion() & PF_FAKE_FORMATTED_FIELD) != 0; }

    friend class OFormattedFieldWrapper;
    friend class OFormattedModel;   // temporary

public:
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // XPropertySet
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;

    // XPersistObject
    virtual void SAL_CALL write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw ( css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw ( css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getServiceName() throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertySet
    using OBoundControlModel::getFastPropertyValue;

    // XReset
    virtual void SAL_CALL reset(  ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("com.sun.star.form.OEditModel"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const SAL_OVERRIDE;
    virtual void describeAggregateProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rAggregateProps
    ) const SAL_OVERRIDE;

    // XEventListener
    using OBoundControlModel::disposing;

protected:
    // OControlModel overridables
    virtual void writeAggregate( const css::uno::Reference< css::io::XObjectOutputStream >& _rxOutStream ) const SAL_OVERRIDE;
    virtual void readAggregate( const css::uno::Reference< css::io::XObjectInputStream >& _rxInStream ) SAL_OVERRIDE;

    // OBoundControlModel overridables
    virtual css::uno::Any   translateDbColumnToControlValue( ) SAL_OVERRIDE;
    virtual bool            commitControlValueToDbColumn( bool _bPostReset ) SAL_OVERRIDE;

    virtual css::uno::Any   getDefaultForReset() const SAL_OVERRIDE;

    virtual void            onConnectedDbColumn( const css::uno::Reference< css::uno::XInterface >& _rxForm ) SAL_OVERRIDE;
    virtual void            onDisconnectedDbColumn() SAL_OVERRIDE;

    virtual bool            approveDbColumnType( sal_Int32 _nColumnType ) SAL_OVERRIDE;

    virtual void            resetNoBroadcast() SAL_OVERRIDE;

protected:
    virtual sal_uInt16 getPersistenceFlags() const SAL_OVERRIDE;

    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    bool    implActsAsRichText( ) const;
};


//= OEditControl

typedef ::cppu::ImplHelper3<    css::awt::XFocusListener,
                                css::awt::XKeyListener,
                                css::form::XChangeBroadcaster > OEditControl_BASE;

class OEditControl : public OBoundControl
                      ,public OEditControl_BASE
{
    ::cppu::OInterfaceContainerHelper
                        m_aChangeListeners;

    OUString         m_aHtmlChangeValue;
    ImplSVEvent *    m_nKeyEvent;

public:
    OEditControl(const css::uno::Reference< css::uno::XComponentContext>& _rxContext);
    virtual ~OEditControl();

    DECLARE_UNO3_AGG_DEFAULTS(OEditControl, OBoundControl)
    virtual css::uno::Any SAL_CALL queryAggregation(const css::uno::Type& _rType) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Sequence< css::uno::Type> _getTypes() SAL_OVERRIDE;

// OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& _rSource) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("com.sun.star.form.OEditControl"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

// css::form::XChangeBroadcaster
    virtual void SAL_CALL addChangeListener(const css::uno::Reference< css::form::XChangeListener>& _rxListener) throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeChangeListener(const css::uno::Reference< css::form::XChangeListener>& _rxListener) throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// css::awt::XFocusListener
    virtual void SAL_CALL focusGained( const css::awt::FocusEvent& e ) throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL focusLost( const css::awt::FocusEvent& e ) throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// css::awt::XKeyListener
    virtual void SAL_CALL keyPressed(const css::awt::KeyEvent& e) throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL keyReleased(const css::awt::KeyEvent& e) throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XControl
    virtual void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& _rxToolkit, const css::uno::Reference< css::awt::XWindowPeer >& _rxParent ) throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

private:
    DECL_LINK_TYPED( OnKeyPressed, void*, void );
};

}

#endif // INCLUDED_FORMS_SOURCE_COMPONENT_EDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
