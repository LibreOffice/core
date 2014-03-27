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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_GROUPBOX_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_GROUPBOX_HXX

#include "FormComponent.hxx"


namespace frm
{


// OGroupBoxModel

class OGroupBoxModel
        :public OControlModel
{
public:
    DECLARE_DEFAULT_LEAF_XTOR( OGroupBoxModel );

    // XServiceInfo
    IMPLEMENTATION_NAME(OGroupBoxModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPersistObject
    virtual OUString SAL_CALL    getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL
        write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL
        read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OControlModel's property handling
    virtual void describeAggregateProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
    ) const SAL_OVERRIDE;

protected:
    DECLARE_XCLONEABLE();
};


// OGroupBoxControl (nur aus Kompatibilitaet zur 5.0)

class OGroupBoxControl : public OControl
{
public:
    OGroupBoxControl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory);

    // XServiceInfo
    IMPLEMENTATION_NAME(OGroupBoxControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


}


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_GROUPBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
