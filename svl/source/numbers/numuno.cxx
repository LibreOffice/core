/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"
#ifndef GCC
#endif

#define _ZFORLIST_DECLARE_TABLE

#include <tools/color.hxx>
#include <tools/debug.hxx>
#include <vos/mutex.hxx>
#include <osl/mutex.hxx>
#include <rtl/uuid.h>

#include <svl/numuno.hxx>
#include "numfmuno.hxx"
#include <svl/zforlist.hxx>

using namespace com::sun::star;

//------------------------------------------------------------------------

class SvNumFmtSuppl_Impl
{
public:
    SvNumberFormatter*                  pFormatter;
    mutable ::comphelper::SharedMutex   aMutex;

    SvNumFmtSuppl_Impl(SvNumberFormatter* p) :
        pFormatter(p) {}
};

//------------------------------------------------------------------------

// Default-ctor fuer getReflection
SvNumberFormatsSupplierObj::SvNumberFormatsSupplierObj()
{
    pImpl = new SvNumFmtSuppl_Impl(NULL);
}

SvNumberFormatsSupplierObj::SvNumberFormatsSupplierObj(SvNumberFormatter* pForm)
{
    pImpl = new SvNumFmtSuppl_Impl(pForm);
}

SvNumberFormatsSupplierObj::~SvNumberFormatsSupplierObj()
{
    delete pImpl;
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
    //  der alte Numberformatter ist ungueltig geworden, nicht mehr darauf zugreifen!
    pImpl->pFormatter = pNew;
}

void SvNumberFormatsSupplierObj::NumberFormatDeleted(sal_uInt32)
{
    //  Basis-Implementierung tut nix...
}

void SvNumberFormatsSupplierObj::SettingsChanged()
{
    //  Basis-Implementierung tut nix...
}

// XNumberFormatsSupplier

uno::Reference<beans::XPropertySet> SAL_CALL SvNumberFormatsSupplierObj::getNumberFormatSettings()
                                        throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( pImpl->aMutex );

    return new SvNumberFormatSettingsObj( *this, pImpl->aMutex );
}

uno::Reference<util::XNumberFormats> SAL_CALL SvNumberFormatsSupplierObj::getNumberFormats()
                                        throw(uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( pImpl->aMutex );

    return new SvNumberFormatsObj( *this, pImpl->aMutex );
}

// XUnoTunnel

sal_Int64 SAL_CALL SvNumberFormatsSupplierObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

// static
const uno::Sequence<sal_Int8>& SvNumberFormatsSupplierObj::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
SvNumberFormatsSupplierObj* SvNumberFormatsSupplierObj::getImplementation(
                                const uno::Reference<util::XNumberFormatsSupplier> xObj )
{
    SvNumberFormatsSupplierObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<SvNumberFormatsSupplierObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething( getUnoTunnelId() )));
    return pRet;
}


//------------------------------------------------------------------------



