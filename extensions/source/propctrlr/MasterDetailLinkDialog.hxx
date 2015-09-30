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
#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_MASTERDETAILLINKDIALOG_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_MASTERDETAILLINKDIALOG_HXX
#include <svtools/genericunodialog.hxx>
#include "modulepcr.hxx"

namespace pcr
{


    class MasterDetailLinkDialog;
    typedef ::svt::OGenericUnoDialog                                            MasterDetailLinkDialog_DBase;
    typedef ::comphelper::OPropertyArrayUsageHelper< MasterDetailLinkDialog >   MasterDetailLinkDialog_PBase;

    class MasterDetailLinkDialog :   public MasterDetailLinkDialog_DBase
                                    ,public MasterDetailLinkDialog_PBase
                                    ,public PcrClient
    {
    public:
        MasterDetailLinkDialog(const css::uno::Reference< css::uno::XComponentContext >& _rxContext);

        // XServiceInfo - static methods
        static css::uno::Sequence< OUString > getSupportedServiceNames_static() throw( css::uno::RuntimeException );
        static OUString getImplementationName_static() throw( css::uno::RuntimeException );
        static css::uno::Reference< css::uno::XInterface >
                SAL_CALL Create(const css::uno::Reference< css::uno::XComponentContext >&);
    private:
        // XTypeProvider
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;

        // OGenericUnoDialog overridables
        virtual VclPtr<Dialog> createDialog(vcl::Window* _pParent) SAL_OVERRIDE;
        virtual void implInitialize(const css::uno::Any& _rValue) SAL_OVERRIDE;

        css::uno::Reference< css::beans::XPropertySet> m_xDetail;
        css::uno::Reference< css::beans::XPropertySet> m_xMaster;
        OUString m_sExplanation;
        OUString m_sDetailLabel;
        OUString m_sMasterLabel;
    };


}   // namespace pcr

#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_MASTERDETAILLINKDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
