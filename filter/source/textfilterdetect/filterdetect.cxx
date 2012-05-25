/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "filterdetect.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace ::com::sun::star;

PlainTextFilterDetect::PlainTextFilterDetect(const uno::Reference<lang::XMultiServiceFactory> &xMSF) :
    mxMSF(xMSF) {}

PlainTextFilterDetect::~PlainTextFilterDetect() {}

rtl::OUString SAL_CALL PlainTextFilterDetect::detect(uno::Sequence<beans::PropertyValue>& aArguments) throw (uno::RuntimeException)
{
    return rtl::OUString();
}

// XInitialization

void SAL_CALL PlainTextFilterDetect::initialize(const uno::Sequence<uno::Any>& aArguments)
    throw (uno::Exception, uno::RuntimeException)
{
    uno::Sequence<beans::PropertyValue> aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if (nLength && (aArguments[0] >>= aAnySeq))
    {
        const beans::PropertyValue * pValue = aAnySeq.getConstArray();
        for (sal_Int32 i = 0, n = aAnySeq.getLength(); i < n; ++i)
        {
            if (pValue[i].Name == "Type")
            {
                fprintf(stdout, "PlainTextFilterDetect::initialize:   type = '%s'\n",
                        rtl::OUStringToOString(pValue[i].Value.get<rtl::OUString>(), RTL_TEXTENCODING_UTF8).getStr());
            }
            else if (pValue[i].Name == "UserData")
            {
                fprintf(stdout, "PlainTextFilterDetect::initialize:   user data = '%s'\n",
                        rtl::OUStringToOString(pValue[i].Value.get<rtl::OUString>(), RTL_TEXTENCODING_UTF8).getStr());
            }
            else if (pValue[i].Name == "TemplateName")
            {
                fprintf(stdout, "PlainTextFilterDetect::initialize:   template name = '%s'\n",
                        rtl::OUStringToOString(pValue[i].Value.get<rtl::OUString>(), RTL_TEXTENCODING_UTF8).getStr());
            }
        }
    }
}

rtl::OUString PlainTextFilterDetect_getImplementationName()
{
    return rtl::OUString("com.sun.star.comp.filters.PlainTextFilterDetect");
}

sal_Bool PlainTextFilterDetect_supportsService(const rtl::OUString& ServiceName)
{
    return ServiceName == "com.sun.star.document.ExtendedTypeDetection" ||
        ServiceName == "com.sun.star.comp.filters.PlainTextFilterDetect";
}

uno::Sequence<rtl::OUString> PlainTextFilterDetect_getSupportedServiceNames()
{
    uno::Sequence<rtl::OUString> aRet(2);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ExtendedTypeDetection";
    pArray[1] = "com.sun.star.comp.filters.PlainTextFilterDetect";
    return aRet;
}

uno::Reference<uno::XInterface> PlainTextFilterDetect_createInstance(
    const uno::Reference<lang::XMultiServiceFactory> & rSMgr)
{
    return (cppu::OWeakObject*) new PlainTextFilterDetect(rSMgr);
}

// XServiceInfo
rtl::OUString SAL_CALL PlainTextFilterDetect::getImplementationName()
    throw (uno::RuntimeException)
{
    return PlainTextFilterDetect_getImplementationName();
}

sal_Bool SAL_CALL PlainTextFilterDetect::supportsService(const rtl::OUString& rServiceName)
    throw (uno::RuntimeException)
{
    return PlainTextFilterDetect_supportsService(rServiceName);
}

uno::Sequence<rtl::OUString> SAL_CALL PlainTextFilterDetect::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return PlainTextFilterDetect_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
