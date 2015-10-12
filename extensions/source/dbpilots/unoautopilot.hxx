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

#ifndef INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_UNOAUTOPILOT_HXX
#define INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_UNOAUTOPILOT_HXX

#include <svtools/genericunodialog.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/proparrhlp.hxx>
#include "componentmodule.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>


namespace dbp
{
    typedef ::svt::OGenericUnoDialog    OUnoAutoPilot_Base;
    template <class TYPE, class SERVICEINFO>
    class OUnoAutoPilot
            :public OUnoAutoPilot_Base
            ,public ::comphelper::OPropertyArrayUsageHelper< OUnoAutoPilot< TYPE, SERVICEINFO > >
            ,public OModuleResourceClient
    {
        explicit OUnoAutoPilot(const css::uno::Reference< css::uno::XComponentContext >& _rxORB)
            : OUnoAutoPilot_Base(_rxORB)
        {
        }


    protected:
        css::uno::Reference< css::beans::XPropertySet >   m_xObjectModel;

    public:
        // XTypeProvider
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override
        {
            return css::uno::Sequence<sal_Int8>();
        }

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override
        {
            return getImplementationName_Static();
        }

        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override
        {
            return getSupportedServiceNames_Static();
        }

        // XServiceInfo - static methods
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static() throw( css::uno::RuntimeException )
        {
            return SERVICEINFO::getServiceNames();
        }

        static OUString getImplementationName_Static() throw( css::uno::RuntimeException )
        {
            return SERVICEINFO::getImplementationName();
        }

        static css::uno::Reference< css::uno::XInterface >
                SAL_CALL Create(const css::uno::Reference< css::lang::XMultiServiceFactory >& _rxFactory)
        {
            return *(new OUnoAutoPilot<TYPE, SERVICEINFO>( comphelper::getComponentContext(_rxFactory) ));
        }

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) override
        {
            css::uno::Reference< css::beans::XPropertySetInfo >  xInfo( createPropertySetInfo( getInfoHelper() ) );
            return xInfo;
        }

        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override
        {
            return *this->getArrayHelper();
        }

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override
        {
            css::uno::Sequence< css::beans::Property > aProps;
            describeProperties(aProps);
            return new ::cppu::OPropertyArrayHelper(aProps);
        }

    protected:
    // OGenericUnoDialog overridables
        virtual VclPtr<Dialog> createDialog(vcl::Window* _pParent) override
        {
            return VclPtr<TYPE>::Create(_pParent, m_xObjectModel, m_aContext);
        }

        virtual void implInitialize(const css::uno::Any& _rValue) override
        {
            css::beans::PropertyValue aArgument;
            if (_rValue >>= aArgument)
                if (aArgument.Name == "ObjectModel")
                {
                    aArgument.Value >>= m_xObjectModel;
                    return;
                }

            OUnoAutoPilot_Base::implInitialize(_rValue);
        }
    };

}   // namespace dbp


#endif // INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_UNOAUTOPILOT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
