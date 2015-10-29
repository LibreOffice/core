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

#include "supservs.hxx"
#include <com/sun/star/lang/Locale.hpp>
#include <comphelper/sharedmutex.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <tools/stream.hxx>
#include <svl/instrm.hxx>

#include <strmadpt.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::utl;


SvNumberFormatsSupplierServiceObject::SvNumberFormatsSupplierServiceObject(const css::uno::Reference< css::uno::XComponentContext >& _rxORB)
    :m_pOwnFormatter(NULL)
    ,m_xORB(_rxORB)
{
}

SvNumberFormatsSupplierServiceObject::~SvNumberFormatsSupplierServiceObject()
{
    if (m_pOwnFormatter)
    {
        delete m_pOwnFormatter;
        m_pOwnFormatter = NULL;
    }
}

Any SAL_CALL SvNumberFormatsSupplierServiceObject::queryAggregation( const Type& _rType ) throw (RuntimeException, std::exception)
{
    Any aReturn = ::cppu::queryInterface(_rType,
        static_cast< XInitialization* >(this),
        static_cast< XServiceInfo* >(this)
    );

    if (!aReturn.hasValue())
        aReturn = SvNumberFormatsSupplierObj::queryAggregation(_rType);

    return aReturn;
}

void SAL_CALL SvNumberFormatsSupplierServiceObject::initialize( const Sequence< Any >& _rArguments ) throw(Exception, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( getSharedMutex() );

    DBG_ASSERT(m_pOwnFormatter == NULL,
        "SvNumberFormatsSupplierServiceObject::initialize : already initialized !");
        // maybe you already called a method which needed the formatter
        // you should use XMultiServiceFactory::createInstanceWithArguments to avoid that
    if (m_pOwnFormatter)
    {   // !!! this is only a emergency handling, normally this should not occur !!!
        delete m_pOwnFormatter;
        m_pOwnFormatter = NULL;
        SetNumberFormatter(m_pOwnFormatter);
    }

    Type aExpectedArgType = ::cppu::UnoType<css::lang::Locale>::get();
    LanguageType eNewFormatterLanguage = LANGUAGE_ENGLISH_US;
        // the default

    const Any* pArgs = _rArguments.getConstArray();
    for (sal_Int32 i=0; i<_rArguments.getLength(); ++i, ++pArgs)
    {
        if (pArgs->getValueType().equals(aExpectedArgType))
        {
            css::lang::Locale aLocale;
            *pArgs >>= aLocale;
            eNewFormatterLanguage = LanguageTag::convertToLanguageType( aLocale, false);
        }
#ifdef DBG_UTIL
        else
        {
            OSL_FAIL("SvNumberFormatsSupplierServiceObject::initialize : unknown argument !");
        }
#endif
    }

    m_pOwnFormatter = new SvNumberFormatter( m_xORB, eNewFormatterLanguage);
    m_pOwnFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_FORMAT_INTL );
    SetNumberFormatter(m_pOwnFormatter);
}

OUString SAL_CALL SvNumberFormatsSupplierServiceObject::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    return OUString("com.sun.star.uno.util.numbers.SvNumberFormatsSupplierServiceObject");
}

sal_Bool SAL_CALL SvNumberFormatsSupplierServiceObject::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL SvNumberFormatsSupplierServiceObject::getSupportedServiceNames(  ) throw(RuntimeException, std::exception)
{
    Sequence< OUString > aSupported(1);
    aSupported.getArray()[0] = "com.sun.star.util.NumberFormatsSupplier";
    return aSupported;
}

Reference< XPropertySet > SAL_CALL SvNumberFormatsSupplierServiceObject::getNumberFormatSettings() throw(RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( getSharedMutex() );
    implEnsureFormatter();
    return SvNumberFormatsSupplierObj::getNumberFormatSettings();
}

Reference< XNumberFormats > SAL_CALL SvNumberFormatsSupplierServiceObject::getNumberFormats() throw(RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( getSharedMutex() );
    implEnsureFormatter();
    return SvNumberFormatsSupplierObj::getNumberFormats();
}

sal_Int64 SAL_CALL SvNumberFormatsSupplierServiceObject::getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw (RuntimeException, std::exception)
{
    sal_Int64 nReturn = SvNumberFormatsSupplierObj::getSomething( aIdentifier );
    if ( nReturn )
        // if somebody accesses internals then we should have the formatter
        implEnsureFormatter();
    return nReturn;
}

void SvNumberFormatsSupplierServiceObject::implEnsureFormatter()
{
    if (!m_pOwnFormatter)
    {
        // get the office's UI locale
        SvtSysLocale aSysLocale;
        css::lang::Locale aOfficeLocale = aSysLocale.GetLocaleData().getLanguageTag().getLocale();

        // initialize with this locale
        Sequence< Any > aFakedInitProps( 1 );
        aFakedInitProps[0] <<= aOfficeLocale;

        initialize( aFakedInitProps );
    }
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_uno_util_numbers_SvNumberFormatsSupplierServiceObject_get_implementation(css::uno::XComponentContext* context,
                                                                    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvNumberFormatsSupplierServiceObject(context));
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
