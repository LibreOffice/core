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
#ifndef INCLUDED_SVL_NUMUNO_HXX
#define INCLUDED_SVL_NUMUNO_HXX

#include <svl/svldllapi.h>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <cppuhelper/implbase.hxx>
#include <memory>

class SvNumberFormatter;
class SvNumFmtSuppl_Impl;

namespace comphelper
{
    class SharedMutex;
}


//  SvNumberFormatsSupplierObj: aggregate to document,
//  construct with SvNumberFormatter

class SVL_DLLPUBLIC SvNumberFormatsSupplierObj : public cppu::WeakAggImplHelper<
                                    css::util::XNumberFormatsSupplier,
                                    css::lang::XUnoTunnel>
{
private:
    std::unique_ptr<SvNumFmtSuppl_Impl> pImpl;

public:
                                SvNumberFormatsSupplierObj();
                                SvNumberFormatsSupplierObj(SvNumberFormatter* pForm);
    virtual                     ~SvNumberFormatsSupplierObj() override;

    void                        SetNumberFormatter(SvNumberFormatter* pNew);
    SvNumberFormatter*          GetNumberFormatter() const;

                                // XNumberFormatsSupplier
    virtual css::uno::Reference< css::beans::XPropertySet > SAL_CALL
                                getNumberFormatSettings()
                                    throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::util::XNumberFormats > SAL_CALL
                                getNumberFormats()
                                    throw(css::uno::RuntimeException, std::exception) override;

                                // XUnoTunnel
    virtual sal_Int64 SAL_CALL  getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
                                        throw(css::uno::RuntimeException, std::exception) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();
    static SvNumberFormatsSupplierObj* getImplementation( const css::uno::Reference<
                                    css::util::XNumberFormatsSupplier>& rObj );

    ::comphelper::SharedMutex&  getSharedMutex() const;
};

#endif // INCLUDED_SVL_NUMUNO_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
