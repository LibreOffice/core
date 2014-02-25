/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "filterdetect.hxx"

#include "tools/urlobj.hxx"
#include "ucbhelper/content.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/supportsservice.hxx>

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

OUString SAL_CALL PlainTextFilterDetect::detect(uno::Sequence<beans::PropertyValue>& lDescriptor) throw (uno::RuntimeException, std::exception)
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
        else if (aExt == "tsv")
            setPropValue(lDescriptor, nFilter, "FilterName", OUString(CALC_TEXT_FILTER));
        else if (aExt == "tab")
            setPropValue(lDescriptor, nFilter, "FilterName", OUString(CALC_TEXT_FILTER));
        else if (aExt == "xls")
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
    throw (uno::Exception, uno::RuntimeException, std::exception)
{
}

OUString PlainTextFilterDetect_getImplementationName()
{
    return OUString("com.sun.star.comp.filters.PlainTextFilterDetect");
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
    throw (uno::RuntimeException, std::exception)
{
    return PlainTextFilterDetect_getImplementationName();
}

sal_Bool SAL_CALL PlainTextFilterDetect::supportsService(const OUString& rServiceName)
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL PlainTextFilterDetect::getSupportedServiceNames()
    throw (uno::RuntimeException, std::exception)
{
    return PlainTextFilterDetect_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
