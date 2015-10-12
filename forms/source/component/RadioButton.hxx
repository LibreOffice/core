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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_RADIOBUTTON_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_RADIOBUTTON_HXX

#include "refvaluecomponent.hxx"


namespace frm
{

class ORadioButtonModel     :public OReferenceValueComponent
{
public:
    DECLARE_DEFAULT_LEAF_XTOR( ORadioButtonModel );

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("com.sun.star.form.ORadioButtonModel"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

    // OPropertySetHelper
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue )
                throw (css::uno::Exception, std::exception) override;

    // XPersistObject
    virtual OUString SAL_CALL    getServiceName() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL
        read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw(css::io::IOException, css::uno::RuntimeException, std::exception) override;

    // OPropertyChangeListener
    virtual void _propertyChanged(const css::beans::PropertyChangeEvent& evt) throw(css::uno::RuntimeException) override;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const override;

protected:
    // OBoundControlModel overridables
    virtual css::uno::Any   translateDbColumnToControlValue( ) override;
    virtual bool            commitControlValueToDbColumn( bool _bPostReset ) override;
    virtual css::uno::Any   translateExternalValueToControlValue( const css::uno::Any& _rExternalValue ) const override;

protected:
    void SetSiblingPropsTo(const OUString& rPropName, const css::uno::Any& rValue);

    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

private:
    void setControlSource();
};

class ORadioButtonControl: public OBoundControl
{
public:
    explicit ORadioButtonControl(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("com.sun.star.form.ORadioButtonControl"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;

protected:
    // XControl
    virtual void SAL_CALL createPeer(const css::uno::Reference<css::awt::XToolkit>& Toolkit, const css::uno::Reference<css::awt::XWindowPeer>& Parent) throw (css::uno::RuntimeException, std::exception) override;
};


}


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_RADIOBUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
