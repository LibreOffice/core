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
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_FORMCONTROLLER_HXX
#define EXTENSIONS_SOURCE_PROPCTRLR_FORMCONTROLLER_HXX

#include "propcontroller.hxx"

#include <cppuhelper/propshlp.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/uno3.hxx>

//........................................................................
namespace pcr
{
//........................................................................

    //====================================================================
    //= ServiceDescriptor
    //====================================================================
    struct ServiceDescriptor
    {
        ::rtl::OUString
            ( *GetImplementationName )( void );
        ::com::sun::star::uno::Sequence< ::rtl::OUString >
            ( *GetSupportedServiceNames )( void );
    };

    //====================================================================
    //= FormController
    //====================================================================
    class FormController;
    typedef ::cppu::OPropertySetHelper                                  FormController_PropertyBase1;
    typedef ::comphelper::OPropertyArrayUsageHelper< FormController >   FormController_PropertyBase2;

    /** Legacy implementation of com.sun.star.form.PropertyBrowserController

        Nowadays only a wrapper around an ObjectInspector using a
        DefaultFormComponentInspectorModel.
    */
    class FormController    :public OPropertyBrowserController
                            ,public FormController_PropertyBase1
                            ,public FormController_PropertyBase2
    {
    private:
        ServiceDescriptor           m_aServiceDescriptor;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                                    m_xCurrentInspectee;
    public:
        FormController(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            ServiceDescriptor _aServiceDescriptor,
            bool _bUseFormFormComponentHandlers
        );

        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    protected:
        ~FormController();

        DECLARE_XINTERFACE()
        DECLARE_XTYPEPROVIDER()

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        // XPropertySet and friends
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any & rConvertedValue, ::com::sun::star::uno::Any & rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue
            )   throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue
            ) throw (::com::sun::star::uno::Exception);
        virtual void SAL_CALL getFastPropertyValue(
                ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle
            ) const;
    private:
        using FormController_PropertyBase1::getFastPropertyValue;
    };

    //====================================================================
    //= DialogController
    //====================================================================
    /** Legacy implementation of com.sun.star.awt.PropertyBrowserController
    */
    class DialogController
    {
    public:
        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    private:
        DialogController();                                     // never implemented
        DialogController( const DialogController& );            // never implemented
        DialogController& operator=( const DialogController& ); // never implemented
    };
//........................................................................
} // namespace pcr
//........................................................................

#endif // EXTENSIONS_SOURCE_PROPCTRLR_FORMCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
