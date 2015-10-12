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

#ifndef INCLUDED_SVL_SOURCE_NUMBERS_SUPSERVS_HXX
#define INCLUDED_SVL_SOURCE_NUMBERS_SUPSERVS_HXX

#include <svl/numuno.hxx>
#include <svl/zforlist.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

/**
 * SvNumberFormatsSupplierServiceObject - a number formats supplier which
 * - can be instantiated as an service
 * - works with it's own SvNumberFormatter instance
 * - can be initialized (::com::sun::star::lang::XInitialization)
 * with a specific language (i.e. ::com::sun::star::lang::Locale)
 */
class SvNumberFormatsSupplierServiceObject
            :public SvNumberFormatsSupplierObj
            ,public ::com::sun::star::lang::XInitialization
            ,public ::com::sun::star::lang::XServiceInfo
{
protected:
    SvNumberFormatter*  m_pOwnFormatter;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        m_xORB;

public:
    explicit SvNumberFormatsSupplierServiceObject(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB);
    virtual ~SvNumberFormatsSupplierServiceObject();

    // XInterface
    virtual void SAL_CALL acquire() throw() override { SvNumberFormatsSupplierObj::acquire(); }
    virtual void SAL_CALL release() throw() override { SvNumberFormatsSupplierObj::release(); }
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override
        { return SvNumberFormatsSupplierObj::queryInterface(_rType); }

    // XAggregation
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XInitialization
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XNumberFormatsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
                getNumberFormatSettings() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormats > SAL_CALL
                getNumberFormats() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XUnoTunnler
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    void implEnsureFormatter();
};


#endif // INCLUDED_SVL_SOURCE_NUMBERS_SUPSERVS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
