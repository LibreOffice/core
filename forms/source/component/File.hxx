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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_FILE_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_FILE_HXX

#include "FormComponent.hxx"


namespace frm
{

class OFileControlModel
                :public OControlModel
                ,public css::form::XReset
{
    ::cppu::OInterfaceContainerHelper       m_aResetListeners;
    OUString                         m_sDefaultValue;

protected:
    virtual css::uno::Sequence< css::uno::Type> _getTypes() SAL_OVERRIDE;

public:
    DECLARE_DEFAULT_LEAF_XTOR( OFileControlModel );

    DECLARE_UNO3_AGG_DEFAULTS(OFileControlModel, OControlModel)
    virtual css::uno::Any SAL_CALL queryAggregation(const css::uno::Type& _rType) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("com.sun.star.form.OFileControlModel"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // XPropertySet and friends
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle) const SAL_OVERRIDE;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const css::uno::Any& rValue) throw ( css::uno::Exception, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL convertFastPropertyValue(css::uno::Any& rConvertedValue, css::uno::Any& rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue )
        throw(css::lang::IllegalArgumentException) SAL_OVERRIDE;

    virtual css::uno::Any getPropertyDefaultByHandle( sal_Int32 _nHandle ) const SAL_OVERRIDE;

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw ( ::com ::sun::star::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw ( ::com ::sun::star::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XReset
    virtual void SAL_CALL reset() throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addResetListener(const css::uno::Reference< css::form::XResetListener>& _rxListener) throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeResetListener(const css::uno::Reference< css::form::XResetListener>& _rxListener) throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const SAL_OVERRIDE;

    // prevent method hiding
    using OControlModel::disposing;
    using OControlModel::getFastPropertyValue;

protected:
    void _reset();

    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
};


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_FILE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
