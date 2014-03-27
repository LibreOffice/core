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


namespace frm
{


//= OEditModel

class OEditModel
                :public OEditBaseModel
{
    ::std::auto_ptr< ::dbtools::FormattedColumnValue >
                                m_pValueFormatter;
    sal_Bool                    m_bMaxTextLenModified : 1;  // set to <TRUE/> when we change the MaxTextLen of the aggregate

    sal_Bool                    m_bWritingFormattedFake : 1;
        // are we writing something which should be interpreted as formatted upon reading?

protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

    DECLARE_DEFAULT_LEAF_XTOR( OEditModel );

    void enableFormattedWriteFake() { m_bWritingFormattedFake = sal_True; }
    void disableFormattedWriteFake() { m_bWritingFormattedFake = sal_False; }
    sal_Bool lastReadWasFormattedFake() const { return (getLastReadVersion() & PF_FAKE_FORMATTED_FIELD) != 0; }

    friend InterfaceRef SAL_CALL OEditModel_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    friend class OFormattedFieldWrapper;
    friend class OFormattedModel;   // temporary

public:
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // XPropertySet
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const SAL_OVERRIDE;

    // XPersistObject
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPropertySet
    using OBoundControlModel::getFastPropertyValue;

    // XReset
    virtual void SAL_CALL reset(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    IMPLEMENTATION_NAME(OEditModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const SAL_OVERRIDE;
    virtual void describeAggregateProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
    ) const SAL_OVERRIDE;

    // XEventListener
    using OBoundControlModel::disposing;

protected:
    // OControlModel overridables
    virtual void writeAggregate( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream >& _rxOutStream ) const SAL_OVERRIDE;
    virtual void readAggregate( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream >& _rxInStream ) SAL_OVERRIDE;

    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( ) SAL_OVERRIDE;
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset ) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const SAL_OVERRIDE;

    virtual void            onConnectedDbColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm ) SAL_OVERRIDE;
    virtual void            onDisconnectedDbColumn() SAL_OVERRIDE;

    virtual sal_Bool        approveDbColumnType( sal_Int32 _nColumnType ) SAL_OVERRIDE;

    virtual void            resetNoBroadcast() SAL_OVERRIDE;

protected:
    virtual sal_uInt16 getPersistenceFlags() const SAL_OVERRIDE;

    DECLARE_XCLONEABLE();

private:
    bool    implActsAsRichText( ) const;
};


//= OEditControl

typedef ::cppu::ImplHelper3<    ::com::sun::star::awt::XFocusListener,
                                ::com::sun::star::awt::XKeyListener,
                                ::com::sun::star::form::XChangeBroadcaster > OEditControl_BASE;

class OEditControl : public OBoundControl
                      ,public OEditControl_BASE
{
    ::cppu::OInterfaceContainerHelper
                        m_aChangeListeners;

    OUString         m_aHtmlChangeValue;
    sal_uInt32              m_nKeyEvent;

public:
    OEditControl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext);
    virtual ~OEditControl();

    DECLARE_UNO3_AGG_DEFAULTS(OEditControl, OBoundControl);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes() SAL_OVERRIDE;

// OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OEditControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

// ::com::sun::star::form::XChangeBroadcaster
    virtual void SAL_CALL addChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeChangeListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XChangeListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// ::com::sun::star::awt::XFocusListener
    virtual void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent& e ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent& e ) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// ::com::sun::star::awt::XKeyListener
    virtual void SAL_CALL keyPressed(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL keyReleased(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XControl
    virtual void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& _rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& _rxParent ) throw ( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

private:
    DECL_LINK( OnKeyPressed, void* );
};


}


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_EDIT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
