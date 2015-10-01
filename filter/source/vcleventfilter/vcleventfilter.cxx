/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "vcleventfilter.hxx"

#include <cppuhelper/factory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>

OUString VclEventFilter_getImplementationName()
    throw (css::uno::RuntimeException)
{
    return OUString ( "com.sun.star.comp.filters.VclEventFilter" );
}

css::uno::Sequence<OUString> SAL_CALL VclEventFilter_getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ImportFilter";
    pArray[1] = "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

css::uno::Reference<css::uno::XInterface> SAL_CALL VclEventFilter_createInstance(const css::uno::Reference<css::lang::XMultiServiceFactory> & rSMgr)
    throw (css::uno::Exception)
{
    return static_cast<cppu::OWeakObject*>(new VclEventFilter( rSMgr ));
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL vcleventfilter_component_getFactory(
        const sal_Char * pImplName, void * pServiceManager, void * /* pRegistryKey */ )
    {
        void * pRet = 0;

        OUString implName = OUString::createFromAscii( pImplName );
        if ( pServiceManager && implName.equals(VclEventFilter_getImplementationName()) )
        {
            css::uno::Reference<css::lang::XSingleServiceFactory> xFactory(
                cppu::createSingleFactory(
                static_cast<css::lang::XMultiServiceFactory*>(pServiceManager),
                OUString::createFromAscii( pImplName ),
                VclEventFilter_createInstance, VclEventFilter_getSupportedServiceNames()));

            if (xFactory.is())
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }

        return pRet;
    }
}

VclEventFilter::VclEventFilter(const css::uno::Reference<css::lang::XMultiServiceFactory> &r)
    : mxMSF(r)
{
}

VclEventFilter::~VclEventFilter()
{
}

// XExtendedTypeDetection
OUString VclEventFilter::detect(css::uno::Sequence<css::beans::PropertyValue>& rDescriptor)
    throw (css::uno::RuntimeException, std::exception)
{
    fprintf(stderr, "trying detect\n");

    sal_Int32 nLength = rDescriptor.getLength();
    const css::beans::PropertyValue *pValue = rDescriptor.getConstArray();
    for (sal_Int32 i = 0 ; i < nLength; ++i)
    {
        if (pValue[i].Name == "InputStream")
            pValue[i].Value >>= mxInputStream;
    }

    if (!mxInputStream.is())
        return OUString();

    css::uno::Sequence< sal_Int8 > aData;
    const size_t numBytes = 3;
    size_t numBytesRead = mxInputStream->readSomeBytes(aData, numBytes);

    if (numBytesRead != numBytes || aData[0] != 'V' || aData[1] != 'C' || aData[2] != 'L')
        return OUString();

    return OUString("filters_Vcl_Event_Document");
}

// XFilter
sal_Bool SAL_CALL VclEventFilter::filter(const css::uno::Sequence<css::beans::PropertyValue>& /*rDescriptor*/)
    throw (css::uno::RuntimeException, std::exception)
{
    fprintf(stderr, "ta-da");
    return true;
}

// XImporter
void SAL_CALL VclEventFilter::setTargetDocument(const css::uno::Reference<css::lang::XComponent>& xDoc)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    mxDoc = xDoc;
}

// XServiceInfo
OUString SAL_CALL VclEventFilter::getImplementationName()
    throw (css::uno::RuntimeException, std::exception)
{
    return VclEventFilter_getImplementationName();
}

sal_Bool SAL_CALL VclEventFilter::supportsService(const OUString& rServiceName)
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}

css::uno::Sequence< OUString > SAL_CALL VclEventFilter::getSupportedServiceNames(  )
    throw (css::uno::RuntimeException, std::exception)
{
    return VclEventFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
