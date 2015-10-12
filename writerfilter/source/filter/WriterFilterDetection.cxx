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

#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/storagehelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sot/storage.hxx>

using namespace ::com::sun::star;

/// File format detection service for DOCX.
class WriterFilterDetection : public cppu::WeakImplHelper
    <
    document::XExtendedFilterDetection,
    lang::XServiceInfo
    >
{
    uno::Reference<uno::XComponentContext> m_xContext;

public:
    explicit WriterFilterDetection(const uno::Reference<uno::XComponentContext>& rxContext);
    virtual ~WriterFilterDetection();

    //XExtendedFilterDetection
    virtual OUString SAL_CALL detect(uno::Sequence<beans::PropertyValue>& Descriptor) throw (uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) throw (uno::RuntimeException, std::exception) override;
    virtual uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw (uno::RuntimeException, std::exception) override;
};

uno::Sequence<OUString> SAL_CALL WriterFilterDetection_getSupportedServiceNames() throw (uno::RuntimeException);

WriterFilterDetection::WriterFilterDetection(const uno::Reference<uno::XComponentContext>& rxContext)
    : m_xContext(rxContext)
{
}

WriterFilterDetection::~WriterFilterDetection()
{
}

OUString WriterFilterDetection::detect(uno::Sequence<beans::PropertyValue>& rDescriptor) throw (uno::RuntimeException, std::exception)
{
    OUString sTypeName;
    bool bWord = false;
    sal_Int32 nPropertyCount = rDescriptor.getLength();
    const beans::PropertyValue* pValues = rDescriptor.getConstArray();
    OUString sURL;
    uno::Reference<io::XStream> xStream;
    uno::Reference<io::XInputStream> xInputStream;
    for (sal_Int32 nProperty = 0; nProperty < nPropertyCount; ++nProperty)
    {
        if (pValues[nProperty].Name == "TypeName")
            rDescriptor[nProperty].Value >>= sTypeName;
        else if (pValues[nProperty].Name == "URL")
            pValues[nProperty].Value >>= sURL;
        else if (pValues[nProperty].Name == "Stream")
            pValues[nProperty].Value >>= xStream;
        else if (pValues[nProperty].Name == "InputStream")
            pValues[nProperty].Value >>= xInputStream;
    }
    try
    {
        uno::Reference<embed::XStorage> xDocStorage;
        if (sURL == "private:stream")
            xDocStorage = comphelper::OStorageHelper::GetStorageFromInputStream(xInputStream);
        else
            xDocStorage = comphelper::OStorageHelper::GetStorageFromURL(sURL, embed::ElementModes::READ);
        if (xDocStorage.is())
        {
            uno::Sequence<OUString> aNames = xDocStorage->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            for (sal_Int32 nName = 0; nName < aNames.getLength(); ++nName)
            {
                if (pNames[nName] == "word")
                {
                    bWord = true;
                    if (sTypeName.isEmpty())
                        sTypeName = "writer_MS_Word_2007";
                    break;
                }
            }
        }
    }
    catch (const uno::Exception&)
    {
        SAL_WARN("writerfilter", "exception while opening storage");
    }
    if (!bWord)
        sTypeName.clear();
    return sTypeName;
}

uno::Sequence<OUString> WriterFilterDetection_getSupportedServiceNames() throw (uno::RuntimeException)
{
    uno::Sequence<OUString> aRet =
    {
        OUString("com.sun.star.document.ExtendedTypeDetection")
    };
    return aRet;
}

OUString WriterFilterDetection::getImplementationName() throw (uno::RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.Writer.WriterFilterDetector");
}

sal_Bool WriterFilterDetection::supportsService(const OUString& rServiceName) throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> WriterFilterDetection::getSupportedServiceNames() throw (uno::RuntimeException, std::exception)
{
    return WriterFilterDetection_getSupportedServiceNames();
}

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface* SAL_CALL com_sun_star_comp_Writer_WriterFilterDetector_get_implementation(uno::XComponentContext* pComp, uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new WriterFilterDetection(pComp));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
