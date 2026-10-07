/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "addin.hxx"
#include "anyjson.hxx"
#include "bridge.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cpo/uno/XComponentContext.hpp>
#include <cpo/uno/XInterface.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>

using namespace css;
using namespace cpo;
using namespace co::pythoncompute;

constexpr OUString ADDIN_SERVICE = u"com.sun.star.sheet.AddIn"_ustr;
constexpr OUString MY_SERVICE = u"org.collaboraoffice.sheet.addin.PythonComputeFunctions"_ustr;

extern "C" SAL_DLLPUBLIC_EXPORT cpo::uno::XInterface*
scaddins_CollaboraOfficePythonComputeAddIn_get_implementation(
    cpo::uno::XComponentContext*, cpo::uno::Sequence<cpo::uno::Any> const&)
{
    return cppu::acquire(new ScaPythonComputeAddIn());
}

ScaPythonComputeAddIn::ScaPythonComputeAddIn()
{
    m_aLocale.Language = u"en"_ustr;
    m_aLocale.Country = u"US"_ustr;
}

ScaPythonComputeAddIn::~ScaPythonComputeAddIn() = default;

cpo::uno::Any ScaPythonComputeAddIn::getPy(const OUString& aCode,
                                           const cpo::uno::Sequence<cpo::uno::Any>& aData)
{
    // Calc calls add-ins with SolarMutex held; the bridge state relies on it.
    DBG_TESTSOLARMUTEX();
    if (aCode.isEmpty())
        throw lang::IllegalArgumentException(u"PY() requires non-empty code"_ustr, getXWeak(), 0);
    SAL_INFO("scaddins.pythoncompute",
             "getPy codeLen=" << aCode.getLength() << " dataArity=" << aData.getLength());
    // Exception boundary: the IllegalArgumentException above is the only one that reaches
    // Calc; anything from startCompute (bad_alloc, JSON writer) becomes #N/A for this call.
    rtl::Reference<PythonComputeVolatileResult> xVol;
    try
    {
        xVol = startCompute(aCode, aData);
    }
    catch (...)
    {
        SAL_WARN("scaddins.pythoncompute", "getPy: exception -> #N/A");
        return makeFormulaErrorAny(FormulaError::NotAvailable);
    }
    // Prefer XVolatileResult (unambiguous); WeakImplHelper has multiple XInterface bases.
    cpo::uno::Any aRet;
    aRet <<= uno::Reference<sheet::XVolatileResult>(xVol);
    return aRet;
}

cpo::uno::Any ScaPythonComputeAddIn::getPython(const OUString& aCode,
                                               const cpo::uno::Sequence<cpo::uno::Any>& aData)
{
    return getPy(aCode, aData);
}

OUString ScaPythonComputeAddIn::getServiceName() { return MY_SERVICE; }

OUString ScaPythonComputeAddIn::getImplementationName()
{
    return u"org.collaboraoffice.sheet.addin.PythonComputeFunctionsImpl"_ustr;
}

bool ScaPythonComputeAddIn::supportsService(const OUString& aServiceName)
{
    return cppu::supportsService(this, aServiceName);
}

cpo::uno::Sequence<OUString> ScaPythonComputeAddIn::getSupportedServiceNames()
{
    return { ADDIN_SERVICE, MY_SERVICE };
}

void ScaPythonComputeAddIn::setLocale(const lang::Locale& eLocale) { m_aLocale = eLocale; }

lang::Locale ScaPythonComputeAddIn::getLocale() { return m_aLocale; }

OUString ScaPythonComputeAddIn::getProgrammaticFuntionName(const OUString& aDisplayName)
{
    const OUString s = aDisplayName.toAsciiUpperCase();
    if (s == "PY")
        return u"getPy"_ustr;
    if (s == "PYTHON")
        return u"getPython"_ustr;
    return {};
}

OUString ScaPythonComputeAddIn::getDisplayFunctionName(const OUString& aProgrammaticName)
{
    if (aProgrammaticName == "getPy")
        return u"PY"_ustr;
    if (aProgrammaticName == "getPython")
        return u"PYTHON"_ustr;
    return aProgrammaticName;
}

OUString ScaPythonComputeAddIn::getFunctionDescription(const OUString& aProgrammaticName)
{
    if (aProgrammaticName == "getPy" || aProgrammaticName == "getPython")
        return u"Executes Python via the remote compute service (interim #BUSY!). "
               "Assign output to result."_ustr;
    return {};
}

OUString ScaPythonComputeAddIn::getDisplayArgumentName(const OUString& aProgrammaticName,
                                                       sal_Int32 nArgument)
{
    if (aProgrammaticName != "getPy" && aProgrammaticName != "getPython")
        return {};
    switch (nArgument)
    {
        case 0:
            return u"code"_ustr;
        case 1:
            return u"data"_ustr;
        default:
            return {};
    }
}

OUString ScaPythonComputeAddIn::getArgumentDescription(const OUString& aProgrammaticName,
                                                       sal_Int32 nArgument)
{
    if (aProgrammaticName != "getPy" && aProgrammaticName != "getPython")
        return {};
    switch (nArgument)
    {
        case 0:
            return u"Python source. Assign the return value to result."_ustr;
        case 1:
            return u"Optional cell range(s) injected as data."_ustr;
        default:
            return {};
    }
}

OUString ScaPythonComputeAddIn::getProgrammaticCategoryName(const OUString&)
{
    return u"Add-In"_ustr;
}

OUString ScaPythonComputeAddIn::getDisplayCategoryName(const OUString& aProgrammaticName)
{
    return getProgrammaticCategoryName(aProgrammaticName);
}

cpo::uno::Sequence<sheet::LocalizedName>
ScaPythonComputeAddIn::getCompatibilityNames(const OUString& aProgrammaticName)
{
    sheet::LocalizedName aName;
    aName.Locale = m_aLocale;
    if (aProgrammaticName == "getPy")
        aName.Name = u"PY"_ustr;
    else if (aProgrammaticName == "getPython")
        aName.Name = u"PYTHON"_ustr;
    else
        return {};
    return { aName };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
