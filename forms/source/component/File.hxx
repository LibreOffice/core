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

#ifndef _FORMS_FILE_HXX_
#define _FORMS_FILE_HXX_

#include "FormComponent.hxx"

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OFileControlModel
//==================================================================
class OFileControlModel
                :public OControlModel
                ,public ::com::sun::star::form::XReset
{
    ::cppu::OInterfaceContainerHelper       m_aResetListeners;
    OUString                         m_sDefaultValue;

protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    DECLARE_DEFAULT_LEAF_XTOR( OFileControlModel );

    DECLARE_UNO3_AGG_DEFAULTS(OFileControlModel, OControlModel);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    IMPLEMENTATION_NAME(OFileControlModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

    // XPropertySet and friends
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);

    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::lang::IllegalArgumentException);

    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 _nHandle ) const;

    // XPersistObject
    virtual OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com ::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com ::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XReset
    virtual void SAL_CALL reset() throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeResetListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException);

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const;

    // prevent method hiding
    using OControlModel::disposing;
    using OControlModel::getFastPropertyValue;

protected:
    virtual void _reset();

    DECLARE_XCLONEABLE();
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FORMS_FILE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
