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
#include "supservs.hxx"
#include <com/sun/star/lang/Locale.hpp>
#include <comphelper/sharedmutex.hxx>
#include <i18npool/mslangid.hxx>
#include <tools/debug.hxx>
#include <vos/mutex.hxx>
#include <tools/stream.hxx>
#include <svl/strmadpt.hxx>
#include <svl/instrm.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::vos;
using namespace ::utl;

#define PERSISTENT_SERVICE_NAME     ::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatsSupplier");

//-------------------------------------------------------------------------
Reference< XInterface > SAL_CALL SvNumberFormatsSupplierServiceObject_CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return static_cast< ::cppu::OWeakObject* >(new SvNumberFormatsSupplierServiceObject(_rxFactory));
}

//-------------------------------------------------------------------------
SvNumberFormatsSupplierServiceObject::SvNumberFormatsSupplierServiceObject(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB)
    :m_pOwnFormatter(NULL)
    ,m_xORB(_rxORB)
{
}

//-------------------------------------------------------------------------
SvNumberFormatsSupplierServiceObject::~SvNumberFormatsSupplierServiceObject()
{
    if (m_pOwnFormatter)
    {
        delete m_pOwnFormatter;
        m_pOwnFormatter = NULL;
    }
}

//-------------------------------------------------------------------------
Any SAL_CALL SvNumberFormatsSupplierServiceObject::queryAggregation( const Type& _rType ) throw (RuntimeException)
{
    Any aReturn = ::cppu::queryInterface(_rType,
        static_cast< XInitialization* >(this),
        static_cast< XPersistObject* >(this),
        static_cast< XServiceInfo* >(this)
    );

    if (!aReturn.hasValue())
        aReturn = SvNumberFormatsSupplierObj::queryAggregation(_rType);

    return aReturn;
}

//-------------------------------------------------------------------------
void SAL_CALL SvNumberFormatsSupplierServiceObject::initialize( const Sequence< Any >& _rArguments ) throw(Exception, RuntimeException)
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

    Type aExpectedArgType = ::getCppuType(static_cast<Locale*>(NULL));
    LanguageType eNewFormatterLanguage = LANGUAGE_ENGLISH_US;
        // the default

    const Any* pArgs = _rArguments.getConstArray();
    for (sal_Int32 i=0; i<_rArguments.getLength(); ++i, ++pArgs)
    {
        if (pArgs->getValueType().equals(aExpectedArgType))
        {
            Locale aLocale;
            *pArgs >>= aLocale;
            eNewFormatterLanguage = MsLangId::convertLocaleToLanguage( aLocale);
        }
#ifdef DBG_UTIL
        else
        {
            DBG_ERROR("SvNumberFormatsSupplierServiceObject::initialize : unknown argument !");
        }
#endif
    }

    m_pOwnFormatter = new SvNumberFormatter(m_xORB, eNewFormatterLanguage);
    m_pOwnFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_FORMAT_INTL );
    SetNumberFormatter(m_pOwnFormatter);
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL SvNumberFormatsSupplierServiceObject::getImplementationName(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.uno.util.numbers.SvNumberFormatsSupplierServiceObject");
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL SvNumberFormatsSupplierServiceObject::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aServices = getSupportedServiceNames();
    const ::rtl::OUString* pServices = aServices.getConstArray();
    for (sal_Int32 i=0; i<aServices.getLength(); ++i, ++pServices)
        if (pServices->equals(_rServiceName))
            return sal_True;

    return sal_False;
}

//-------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL SvNumberFormatsSupplierServiceObject::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(1);
    aSupported.getArray()[0] = PERSISTENT_SERVICE_NAME;
    return aSupported;
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL SvNumberFormatsSupplierServiceObject::getServiceName(  ) throw(RuntimeException)
{
    return PERSISTENT_SERVICE_NAME;
}

//-------------------------------------------------------------------------
void SAL_CALL SvNumberFormatsSupplierServiceObject::write( const Reference< XObjectOutputStream >& _rxOutStream ) throw(IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( getSharedMutex() );
    implEnsureFormatter();

    Reference< XOutputStream > xStream(_rxOutStream.get());
    SvLockBytesRef aLockBytes = new SvOutputStreamOpenLockBytes(xStream);
    SvStream aSvOutputSteam(aLockBytes);

    m_pOwnFormatter->Save(aSvOutputSteam);
}

//-------------------------------------------------------------------------
void SAL_CALL SvNumberFormatsSupplierServiceObject::read( const Reference< XObjectInputStream >& _rxInStream ) throw(IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( getSharedMutex() );
    implEnsureFormatter();

    Reference< XInputStream > xStream(_rxInStream.get());
    SvInputStream aSvInputSteam(xStream);

    m_pOwnFormatter->Load(aSvInputSteam);
}

//-------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL SvNumberFormatsSupplierServiceObject::getNumberFormatSettings() throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( getSharedMutex() );
    implEnsureFormatter();
    return SvNumberFormatsSupplierObj::getNumberFormatSettings();
}

//-------------------------------------------------------------------------
Reference< XNumberFormats > SAL_CALL SvNumberFormatsSupplierServiceObject::getNumberFormats() throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( getSharedMutex() );
    implEnsureFormatter();
    return SvNumberFormatsSupplierObj::getNumberFormats();
}

//-------------------------------------------------------------------------
sal_Int64 SAL_CALL SvNumberFormatsSupplierServiceObject::getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw (RuntimeException)
{
    sal_Int64 nReturn = SvNumberFormatsSupplierObj::getSomething( aIdentifier );
    if ( nReturn )
        // if somebody accesses internals then we should have the formatter
        implEnsureFormatter();
    return nReturn;
}

//-------------------------------------------------------------------------
void SvNumberFormatsSupplierServiceObject::implEnsureFormatter()
{
    if (!m_pOwnFormatter)
    {
        // get the office's UI locale
        SvtSysLocale aSysLocale;
        Locale aOfficeLocale = aSysLocale.GetLocaleData().getLocale();

        // initi with this locale
        Sequence< Any > aFakedInitProps( 1 );
        aFakedInitProps[0] <<= aOfficeLocale;

        initialize( aFakedInitProps );
    }
}

