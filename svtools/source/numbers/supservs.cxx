/*************************************************************************
 *
 *  $RCSfile: supservs.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-07 17:35:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVTOOLS_NUMBERS_SUPPLIERSERVICE_HXX_
#include "supservs.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef SVTOOLS_STRMADPT_HXX
#include <strmadpt.hxx>
#endif
#ifndef SVTOOLS_INSTRM_HXX
#include "instrm.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::vos;

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
    OGuard aGuard(Application::GetSolarMutex());
        // the mutex of the base class .... hope we have a real (i.e. own) mutex sometimes

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
            eNewFormatterLanguage = ConvertIsoNamesToLanguage(aLocale.Language, aLocale.Country);
        }
#if DBG_UTIL
        else
        {
            DBG_ERROR("SvNumberFormatsSupplierServiceObject::initialize : unknown argument !");
        }
#endif
    }

    m_pOwnFormatter = new SvNumberFormatter(m_xORB, eNewFormatterLanguage);
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
    OGuard aGuard(Application::GetSolarMutex());
    implEnsureFormatter();

    Reference< XOutputStream > xStream(_rxOutStream.get());
    SvLockBytesRef aLockBytes = new SvOutputStreamOpenLockBytes(xStream);
    SvStream aSvOutputSteam(aLockBytes);

    m_pOwnFormatter->Save(aSvOutputSteam);
}

//-------------------------------------------------------------------------
void SAL_CALL SvNumberFormatsSupplierServiceObject::read( const Reference< XObjectInputStream >& _rxInStream ) throw(IOException, RuntimeException)
{
    OGuard aGuard(Application::GetSolarMutex());
    implEnsureFormatter();

    Reference< XInputStream > xStream(_rxInStream.get());
    SvInputStream aSvInputSteam(xStream);

    m_pOwnFormatter->Load(aSvInputSteam);
}

//-------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL SvNumberFormatsSupplierServiceObject::getNumberFormatSettings() throw(RuntimeException)
{
    OGuard aGuard(Application::GetSolarMutex());
    implEnsureFormatter();
    return SvNumberFormatsSupplierObj::getNumberFormatSettings();
}

//-------------------------------------------------------------------------
Reference< XNumberFormats > SAL_CALL SvNumberFormatsSupplierServiceObject::getNumberFormats() throw(RuntimeException)
{
    OGuard aGuard(Application::GetSolarMutex());
    implEnsureFormatter();
    return SvNumberFormatsSupplierObj::getNumberFormats();
}

//-------------------------------------------------------------------------
void SvNumberFormatsSupplierServiceObject::implEnsureFormatter()
{
    if (!m_pOwnFormatter)
    {
        DBG_ERROR("SvNumberFormatsSupplierServiceObject::implEnsureFormatter : forced to initialize with a default language !");
            // you should use XMultiServiceFactory::createInstanceWithArguments (with an Locale as parameter)
        m_pOwnFormatter = new SvNumberFormatter(m_xORB, LANGUAGE_ENGLISH_US);
        SetNumberFormatter(m_pOwnFormatter);
    }
}

