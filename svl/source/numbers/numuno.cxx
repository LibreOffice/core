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


#include <comphelper/servicehelper.hxx>

#include <svl/numuno.hxx>
#include "numfmuno.hxx"
#include <svl/zforlist.hxx>

using namespace com::sun::star;


class SvNumFmtSuppl_Impl
{
public:
    SvNumberFormatter*                  pFormatter;
    mutable ::comphelper::SharedMutex   aMutex;

    explicit SvNumFmtSuppl_Impl(SvNumberFormatter* p) :
        pFormatter(p) {}
};


// Default ctor for getReflection
SvNumberFormatsSupplierObj::SvNumberFormatsSupplierObj()
    : pImpl( new SvNumFmtSuppl_Impl(nullptr) )
{
}

SvNumberFormatsSupplierObj::SvNumberFormatsSupplierObj(SvNumberFormatter* pForm)
    : pImpl( new SvNumFmtSuppl_Impl(pForm) )
{
}

SvNumberFormatsSupplierObj::~SvNumberFormatsSupplierObj()
{
}

::comphelper::SharedMutex& SvNumberFormatsSupplierObj::getSharedMutex() const
{
    return pImpl->aMutex;
}

SvNumberFormatter* SvNumberFormatsSupplierObj::GetNumberFormatter() const
{
    return pImpl->pFormatter;
}

void SvNumberFormatsSupplierObj::SetNumberFormatter(SvNumberFormatter* pNew)
{
    // The old Numberformatter has been retired, do not access it anymore!
    pImpl->pFormatter = pNew;
}

// XNumberFormatsSupplier

uno::Reference<beans::XPropertySet> SAL_CALL SvNumberFormatsSupplierObj::getNumberFormatSettings()
                                        throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( pImpl->aMutex );

    return new SvNumberFormatSettingsObj( *this, pImpl->aMutex );
}

uno::Reference<util::XNumberFormats> SAL_CALL SvNumberFormatsSupplierObj::getNumberFormats()
                                        throw(uno::RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( pImpl->aMutex );

    return new SvNumberFormatsObj( *this, pImpl->aMutex );
}

// XUnoTunnel

sal_Int64 SAL_CALL SvNumberFormatsSupplierObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException, std::exception)
{
    if ( rId.getLength() == 16 &&
          0 == memcmp( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace
{
    class theSvNumberFormatsSupplierObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSvNumberFormatsSupplierObjUnoTunnelId > {};
}

// static
const uno::Sequence<sal_Int8>& SvNumberFormatsSupplierObj::getUnoTunnelId()
{
    return theSvNumberFormatsSupplierObjUnoTunnelId::get().getSeq();
}

// static
SvNumberFormatsSupplierObj* SvNumberFormatsSupplierObj::getImplementation(
                                const uno::Reference<util::XNumberFormatsSupplier>& rObj )
{
    SvNumberFormatsSupplierObj* pRet = nullptr;
    uno::Reference<lang::XUnoTunnel> xUT(rObj, uno::UNO_QUERY);
    if (xUT.is())
        pRet = reinterpret_cast<SvNumberFormatsSupplierObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething( getUnoTunnelId() )));
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
