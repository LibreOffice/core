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


    //= MasterDetailLinkDialog

    class MasterDetailLinkDialog :   public MasterDetailLinkDialog_DBase
                                    ,public MasterDetailLinkDialog_PBase
                                    ,public PcrClient
    {
    public:
        MasterDetailLinkDialog(const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& _rxContext);

        // XServiceInfo - static methods
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static OUString getImplementationName_static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >&);
    private:
        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() SAL_OVERRIDE;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const SAL_OVERRIDE;

        // OGenericUnoDialog overridables
        virtual Dialog* createDialog(vcl::Window* _pParent) SAL_OVERRIDE;
        virtual void implInitialize(const com::sun::star::uno::Any& _rValue) SAL_OVERRIDE;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> m_xDetail;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> m_xMaster;
        OUString m_sExplanation;
        OUString m_sDetailLabel;
        OUString m_sMasterLabel;
    };


}   // namespace pcr

#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_MASTERDETAILLINKDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
