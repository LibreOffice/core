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

#include "tools/urlobj.hxx"
#include "ucbhelper/content.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>

#define WRITER_TEXT_FILTER "Text"
#define CALC_TEXT_FILTER   "Text - txt - csv (StarCalc)"

using namespace ::com::sun::star;

namespace {

template<typename T>
void setPropValue(uno::Sequence<beans::PropertyValue>& rProps, sal_Int32 nPos, const char* pName, const T& rValue)
{
    if (nPos >= 0)
        rProps[nPos].Value <<= rValue;
    else
    {
        sal_Int32 n = rProps.getLength();
        rProps.realloc(n+1);
        rProps[n].Name = OUString::createFromAscii(pName);
        rProps[n].Value <<= rValue;
    }
}

}

PlainTextFilterDetect::PlainTextFilterDetect(const uno::Reference<uno::XComponentContext>& xCxt) :
    mxCxt(xCxt) {}

PlainTextFilterDetect::~PlainTextFilterDetect() {}

OUString SAL_CALL PlainTextFilterDetect::detect(uno::Sequence<beans::PropertyValue>& lDescriptor) throw (uno::RuntimeException)
{
    OUString aType;
    OUString aDocService;
    OUString aExt;
    OUString aUrl;

    sal_Int32 nFilter = -1;

    for (sal_Int32 i = 0, n = lDescriptor.getLength(); i < n; ++i)
    {
        if (lDescriptor[i].Name == "TypeName")
            lDescriptor[i].Value >>= aType;
        else if (lDescriptor[i].Name == "FilterName")
            nFilter = i;
        else if (lDescriptor[i].Name == "DocumentService")
            lDescriptor[i].Value >>= aDocService;
        else if (lDescriptor[i].Name == "URL")
        {
            lDescriptor[i].Value >>= aUrl;

            // Get the file name extension.
            INetURLObject aParser(aUrl);
            aExt = aParser.getExtension(
                INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET);
            aExt = aExt.toAsciiLowerCase();
        }
    }

    if (aType == "generic_Text")
    {
        // Generic text type.

        // Decide which filter to use based on the document service first,
        // then on extension if that's not available.

        if (aDocService == "com.sun.star.sheet.SpreadsheetDocument")
            // Open it in Calc.
            setPropValue(lDescriptor, nFilter, "FilterName", OUString(CALC_TEXT_FILTER));
        else if (aDocService == "com.sun.star.text.TextDocument")
            // Open it in Writer.
            setPropValue(lDescriptor, nFilter, "FilterName", OUString(WRITER_TEXT_FILTER));
        else if (aExt == "csv")
            setPropValue(lDescriptor, nFilter, "FilterName", OUString(CALC_TEXT_FILTER));
        else if (aExt == "txt")
            setPropValue(lDescriptor, nFilter, "FilterName", OUString(WRITER_TEXT_FILTER));
        else
            // No clue.  Open it in Writer by default.
            setPropValue(lDescriptor, nFilter, "FilterName", OUString(WRITER_TEXT_FILTER));

        return aType;
    }

    // failed!
    return OUString();
}

// XInitialization

void SAL_CALL PlainTextFilterDetect::initialize(const uno::Sequence<uno::Any>& /*aArguments*/)
    throw (uno::Exception, uno::RuntimeException)
{
}

OUString PlainTextFilterDetect_getImplementationName()
{
    return OUString("com.sun.star.comp.filters.PlainTextFilterDetect");
}

sal_Bool PlainTextFilterDetect_supportsService(const OUString& ServiceName)
{
    return ServiceName == "com.sun.star.document.ExtendedTypeDetection" ||
        ServiceName == "com.sun.star.comp.filters.PlainTextFilterDetect";
}

uno::Sequence<OUString> PlainTextFilterDetect_getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ExtendedTypeDetection";
    pArray[1] = "com.sun.star.comp.filters.PlainTextFilterDetect";
    return aRet;
}

uno::Reference<uno::XInterface> PlainTextFilterDetect_createInstance(
    const uno::Reference<uno::XComponentContext> & rCxt)
{
    return (cppu::OWeakObject*) new PlainTextFilterDetect(rCxt);
}

// XServiceInfo
OUString SAL_CALL PlainTextFilterDetect::getImplementationName()
    throw (uno::RuntimeException)
{
    return PlainTextFilterDetect_getImplementationName();
}

sal_Bool SAL_CALL PlainTextFilterDetect::supportsService(const OUString& rServiceName)
    throw (uno::RuntimeException)
{
    return PlainTextFilterDetect_supportsService(rServiceName);
}

uno::Sequence<OUString> SAL_CALL PlainTextFilterDetect::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return PlainTextFilterDetect_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
