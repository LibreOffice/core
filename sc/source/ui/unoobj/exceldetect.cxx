/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "exceldetect.hxx"

using namespace com::sun::star;

ScExcelBiffDetect::ScExcelBiffDetect( const uno::Reference<uno::XComponentContext>& /*xContext*/ ) {}
ScExcelBiffDetect::~ScExcelBiffDetect() {}

OUString ScExcelBiffDetect::getImplementationName() throw (uno::RuntimeException)
{
    return impl_getStaticImplementationName();
}

sal_Bool ScExcelBiffDetect::supportsService( const OUString& aName ) throw (uno::RuntimeException)
{
    uno::Sequence<OUString> aSrvNames = getSupportedServiceNames();
    const OUString* pArray = aSrvNames.getConstArray();
    for (sal_Int32 i = 0; i < aSrvNames.getLength(); ++i, ++pArray)
    {
        if (*pArray == aName)
            return true;
    }
    return false;
}

uno::Sequence<OUString> ScExcelBiffDetect::getSupportedServiceNames() throw (uno::RuntimeException)
{
    return impl_getStaticSupportedServiceNames();
}

OUString ScExcelBiffDetect::detect( uno::Sequence<beans::PropertyValue>& lDescriptor )
    throw (uno::RuntimeException)
{
    return OUString();
}

uno::Sequence<OUString> ScExcelBiffDetect::impl_getStaticSupportedServiceNames()
{
    uno::Sequence<OUString> aNames(1);
    aNames[0] = "com.sun.star.frame.ExtendedTypeDetection";
    return aNames;
}

OUString ScExcelBiffDetect::impl_getStaticImplementationName()
{
    return OUString("com.sun.star.comp.calc.ExcelBiffFormatDetector");
}

uno::Reference<uno::XInterface> ScExcelBiffDetect::impl_createInstance(
    const uno::Reference<uno::XComponentContext>& xContext )
        throw (com::sun::star::uno::Exception)
{
    return static_cast<cppu::OWeakObject*>(new ScExcelBiffDetect(xContext));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
