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

#include "unoadmin.hxx"
#include "dbu_reghelper.hxx"
#include "advancedsettingsdlg.hxx"
#include <comphelper/processfactory.hxx>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    // OAdvancedSettingsDialog
    class OAdvancedSettingsDialog
            :public ODatabaseAdministrationDialog
            ,public ::comphelper::OPropertyArrayUsageHelper< OAdvancedSettingsDialog >
    {

    protected:
        OAdvancedSettingsDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB);

    public:
        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo - static methods
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >&);

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
    protected:
    // OGenericUnoDialog overridables
        virtual Dialog* createDialog(Window* _pParent);
    };

    OAdvancedSettingsDialog::OAdvancedSettingsDialog(const Reference< XComponentContext >& _rxORB)
        :ODatabaseAdministrationDialog(_rxORB)
    {
    }
    Sequence<sal_Int8> SAL_CALL OAdvancedSettingsDialog::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId aId;
        return aId.getImplementationId();
    }

    Reference< XInterface > SAL_CALL OAdvancedSettingsDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
    {
        return *(new OAdvancedSettingsDialog( comphelper::getComponentContext(_rxFactory) ));
    }

    OUString SAL_CALL OAdvancedSettingsDialog::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    OUString OAdvancedSettingsDialog::getImplementationName_Static() throw(RuntimeException)
    {
        return OUString("org.openoffice.comp.dbu.OAdvancedSettingsDialog");
    }

    ::comphelper::StringSequence SAL_CALL OAdvancedSettingsDialog::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }

    ::comphelper::StringSequence OAdvancedSettingsDialog::getSupportedServiceNames_Static() throw(RuntimeException)
    {
        ::comphelper::StringSequence aSupported(1);
        aSupported.getArray()[0] = OUString("com.sun.star.sdb.AdvancedDatabaseSettingsDialog");
        return aSupported;
    }

    Reference<XPropertySetInfo>  SAL_CALL OAdvancedSettingsDialog::getPropertySetInfo() throw(RuntimeException)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    ::cppu::IPropertyArrayHelper& OAdvancedSettingsDialog::getInfoHelper()
    {
        return *const_cast<OAdvancedSettingsDialog*>(this)->getArrayHelper();
    }

    ::cppu::IPropertyArrayHelper* OAdvancedSettingsDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }
    Dialog* OAdvancedSettingsDialog::createDialog(Window* _pParent)
    {
        AdvancedSettingsDialog* pDlg = new AdvancedSettingsDialog(_pParent, m_pDatasourceItems, m_aContext, m_aInitialSelection);
        return pDlg;
    }

}   // namespace dbaui

extern "C" void SAL_CALL createRegistryInfo_OAdvancedSettingsDialog()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OAdvancedSettingsDialog > aAutoRegistration;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
