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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_UNOSQLMESSAGE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_UNOSQLMESSAGE_HXX

#include <svtools/genericunodialog.hxx>
#include "moduledbu.hxx"
namespace dbaui
{

typedef ::svt::OGenericUnoDialog OSQLMessageDialogBase;
class OSQLMessageDialog
        :public OSQLMessageDialogBase
        ,public ::comphelper::OPropertyArrayUsageHelper< OSQLMessageDialog >
{
    OModuleClient m_aModuleClient;
protected:
    // <properties>
    ::com::sun::star::uno::Any  m_aException;
    OUString             m_sHelpURL;
    // </properties>

protected:
    OSQLMessageDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB);

public:
    // XTypeProvider
    virtual com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XServiceInfo - static methods
    static com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(void) throw( com::sun::star::uno::RuntimeException );
    static OUString getImplementationName_Static(void) throw( com::sun::star::uno::RuntimeException );
    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
            SAL_CALL Create(const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >&);

    // XPropertySet
    virtual com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;

protected:
    virtual void SAL_CALL initialize(com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > const & args) throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

// OPropertySetHelper overridables
    // (overwrittin these three, because we have some special handling for our property)
    virtual sal_Bool SAL_CALL convertFastPropertyValue( com::sun::star::uno::Any& _rConvertedValue, com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const com::sun::star::uno::Any& _rValue) throw(com::sun::star::lang::IllegalArgumentException) SAL_OVERRIDE;

// OGenericUnoDialog overridables
    virtual Dialog* createDialog(vcl::Window* _pParent) SAL_OVERRIDE;
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_UNOSQLMESSAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
