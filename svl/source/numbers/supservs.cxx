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
#include <i18nlangtag/languagetag.hxx>
#include <svl/numformat.hxx>
#include <tools/debug.hxx>
#include <osl/mutex.hxx>
#include <utility>
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;


SvNumberFormatsSupplierServiceObject::SvNumberFormatsSupplierServiceObject(css::uno::Reference< css::uno::XComponentContext > _xORB)
    :m_xORB(std::move(_xORB))
{
}

SvNumberFormatsSupplierServiceObject::~SvNumberFormatsSupplierServiceObject()
{
}

Any SAL_CALL SvNumberFormatsSupplierServiceObject::queryAggregation( const Type& _rType )
{
    Any aReturn = ::cppu::queryInterface(_rType,
        static_cast< XInitialization* >(this),
        static_cast< XServiceInfo* >(this)
    );

    if (!aReturn.hasValue())
        aReturn = SvNumberFormatsSupplierObj::queryAggregation(_rType);

    return aReturn;
}

void SAL_CALL SvNumberFormatsSupplierServiceObject::initialize( const Sequence< Any >& _rArguments )
{
    ::osl::MutexGuard aGuard( getSharedMutex() );

    DBG_ASSERT(m_pOwnFormatter == nullptr,
        "SvNumberFormatsSupplierServiceObject::initialize : already initialized !");
        // maybe you already called a method which needed the formatter
        // you should use XMultiServiceFactory::createInstanceWithArguments to avoid that
    if (m_pOwnFormatter)
    {   // !!! this is only an emergency handling, normally this should not occur !!!
        m_pOwnFormatter.reset();
        SetNumberFormatter(m_pOwnFormatter.get());
    }

    Type aExpectedArgType = ::cppu::UnoType<css::lang::Locale>::get();
    LanguageType eNewFormatterLanguage = LANGUAGE_SYSTEM;
        // the default

    for (const Any& rArg : _rArguments)
    {
        if (rArg.getValueType().equals(aExpectedArgType))
        {
            css::lang::Locale aLocale;
            rArg >>= aLocale;
            eNewFormatterLanguage = LanguageTag::convertToLanguageType( aLocale, false);
        }
#ifdef DBG_UTIL
        else
        {
            OSL_FAIL("SvNumberFormatsSupplierServiceObject::initialize : unknown argument !");
        }
#endif
    }

    m_pOwnFormatter.reset( new SvNumberFormatter( m_xORB, eNewFormatterLanguage) );
    m_pOwnFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_FORMAT_INTL );
    SetNumberFormatter(m_pOwnFormatter.get());
}

OUString SAL_CALL SvNumberFormatsSupplierServiceObject::getImplementationName(  )
{
    return u"com.sun.star.uno.util.numbers.SvNumberFormatsSupplierServiceObject"_ustr;
}

sal_Bool SAL_CALL SvNumberFormatsSupplierServiceObject::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL SvNumberFormatsSupplierServiceObject::getSupportedServiceNames(  )
{
    return { u"com.sun.star.util.NumberFormatsSupplier"_ustr };
}

Reference< XPropertySet > SAL_CALL SvNumberFormatsSupplierServiceObject::getNumberFormatSettings()
{
    ::osl::MutexGuard aGuard( getSharedMutex() );
    implEnsureFormatter();
    return SvNumberFormatsSupplierObj::getNumberFormatSettings();
}

Reference< XNumberFormats > SAL_CALL SvNumberFormatsSupplierServiceObject::getNumberFormats()
{
    ::osl::MutexGuard aGuard( getSharedMutex() );
    implEnsureFormatter();
    return SvNumberFormatsSupplierObj::getNumberFormats();
}

sal_Int64 SAL_CALL SvNumberFormatsSupplierServiceObject::getSomething( const Sequence< sal_Int8 >& aIdentifier )
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
        initialize({ Any(aOfficeLocale) });
    }
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_uno_util_numbers_SvNumberFormatsSupplierServiceObject_get_implementation(css::uno::XComponentContext* context,
                                                                    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvNumberFormatsSupplierServiceObject(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
