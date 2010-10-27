/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _DBAUI_UNOSQLMESSAGE_HXX_
#define _DBAUI_UNOSQLMESSAGE_HXX_

#include <svtools/genericunodialog.hxx>
#include "moduledbu.hxx"
//.........................................................................
namespace dbaui
{
//.........................................................................

typedef ::svt::OGenericUnoDialog OSQLMessageDialogBase;
class OSQLMessageDialog
        :public OSQLMessageDialogBase
        ,public ::comphelper::OPropertyArrayUsageHelper< OSQLMessageDialog >
{
    OModuleClient m_aModuleClient;
protected:
    // <properties>
    ::com::sun::star::uno::Any  m_aException;
    ::rtl::OUString             m_sHelpURL;
    // </properties>

protected:
    OSQLMessageDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

public:
    // XTypeProvider
    virtual com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(com::sun::star::uno::RuntimeException);
    virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(com::sun::star::uno::RuntimeException);

    // XServiceInfo - static methods
    static com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( com::sun::star::uno::RuntimeException );
    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
            SAL_CALL Create(const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >&);

    // XPropertySet
    virtual com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

protected:
// OPropertySetHelper overridables
    // (overwrittin these three, because we have some special handling for our property)
    virtual sal_Bool SAL_CALL convertFastPropertyValue( com::sun::star::uno::Any& _rConvertedValue, com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const com::sun::star::uno::Any& _rValue) throw(com::sun::star::lang::IllegalArgumentException);

// OGenericUnoDialog overridables
    virtual Dialog* createDialog(Window* _pParent);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_UNOSQLMESSAGE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
