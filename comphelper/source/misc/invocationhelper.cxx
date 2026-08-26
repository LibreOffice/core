/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <comphelper/invocationhelper.hxx>

#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/theIntrospection.hpp>
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/lang/NoSuchMethodException.hpp>
#include <com/sun/star/script/CannotConvertException.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/script/XInvocation.hpp>

namespace comphelper
{
constexpr sal_Int32 ALLOWED_METHODS
    = css::beans::MethodConcept::ALL ^ css::beans::MethodConcept::DANGEROUS;
constexpr sal_Int32 ALLOWED_PROPERTIES
    = css::beans::PropertyConcept::ALL ^ css::beans::PropertyConcept::DANGEROUS;

InvocationHelper::InvocationHelper(const css::uno::Reference<css::uno::XComponentContext>& xContext)
    : m_xContext(xContext)
{
}

css::uno::Reference<css::beans::XIntrospectionAccess>
InvocationHelper::getIntrospection(css::script::XInvocation* pInvocation)
{
    ensureIntrospection(pInvocation);

    return m_xIntrospectionAccess;
}

css::uno::Sequence<css::uno::Any> InvocationHelper::prepareInvokeParams(
    const css::uno::Sequence<css::uno::Any>& aParams,
    const css::uno::Sequence<css::reflection::ParamInfo>& aParamInfos)
{
    css::uno::Sequence<css::uno::Any> aInvokeParams(aParamInfos.getLength());
    css::uno::Any* pInvokeParams = aInvokeParams.getArray();

    for (sal_Int32 nParam = 0, nParams = aParamInfos.getLength(); nParam < nParams; ++nParam)
    {
        const css::reflection::ParamInfo& rParam = aParamInfos[nParam];

        if (rParam.aMode == css::reflection::ParamMode_OUT)
        {
            // Initialize default value if the parameter is OUT
            rParam.aType->createObject(pInvokeParams[nParam]);
        }
        else
        {
            css::uno::Type aDestType(rParam.aType->getTypeClass(), rParam.aType->getName());

            try
            {
                pInvokeParams[nParam] = m_xTypeConverter->convertTo(aParams[nParam], aDestType);
            }
            catch (css::script::CannotConvertException& e)
            {
                // Add optional parameter index
                e.ArgumentIndex = nParam;
                throw;
            }
        }
    }

    return aInvokeParams;
}

css::uno::Any InvocationHelper::invoke(css::script::XInvocation* pInvocation,
                                       const OUString& aFunctionName,
                                       const css::uno::Sequence<css::uno::Any>& aParams,
                                       css::uno::Sequence<sal_Int16>& aOutParamIndex,
                                       css::uno::Sequence<css::uno::Any>& aOutParam)
{
    // This implementation is heavily inspired by Invocation_Impl::invoke

    ensureIntrospection(pInvocation);

    css::uno::Reference<css::reflection::XIdlMethod> xMethod;

    try
    {
        xMethod = m_xIntrospectionAccess->getMethod(aFunctionName, ALLOWED_METHODS);
    }
    catch (css::lang::NoSuchMethodException&)
    {
        throw css::lang::IllegalArgumentException(u"no such method"_ustr, pInvocation, 0);
    }

    css::uno::Sequence<css::reflection::ParamInfo> aParamInfos = xMethod->getParameterInfos();

    if (aParams.getLength() != aParamInfos.getLength())
    {
        throw css::lang::IllegalArgumentException(
            "incorrect number of parameters passed invoking function " + aFunctionName
                + ": expected " + OUString::number(aParamInfos.getLength()) + ", got "
                + OUString::number(aParams.getLength()),
            pInvocation, 1);
    }

    ensureTypeConverter();

    css::uno::Sequence<css::uno::Any> aInvokeParams = prepareInvokeParams(aParams, aParamInfos);

    css::uno::Reference<css::script::XInvocation> xInvocation(pInvocation);
    css::uno::Any aRet = xMethod->invoke(css::uno::Any(xInvocation), aInvokeParams);

    size_t nOutParams = std::count_if(
        std::cbegin(aParamInfos), std::cend(aParamInfos),
        [](const auto& rParamInfo) { return rParamInfo.aMode != css::reflection::ParamMode_IN; });
    aOutParamIndex.realloc(nOutParams);
    aOutParam.realloc(nOutParams);

    sal_Int16* pOutParamIndex = aOutParamIndex.getArray();
    css::uno::Any* pOutParam = aOutParam.getArray();

    for (sal_Int32 nParam = 0, nOutParam = 0, nParams = aParamInfos.getLength(); nParam < nParams;
         ++nParam)
    {
        if (aParamInfos[nParam].aMode == css::reflection::ParamMode_IN)
            continue;

        pOutParamIndex[nOutParam] = nParam;
        pOutParam[nOutParam] = aInvokeParams[nParam];
        nOutParam++;
    }

    return aRet;
}

void InvocationHelper::setValue(css::script::XInvocation* pInvocation,
                                const OUString& aPropertyName, const css::uno::Any& aValue)
{
    ensureIntrospection(pInvocation);

    css::uno::Reference<css::beans::XPropertySet> xPropertySet(m_xIntrospectionAccess,
                                                               css::uno::UNO_QUERY);

    if (!xPropertySet.is())
        throw css::beans::UnknownPropertyException(aPropertyName);

    css::beans::Property aProp;

    try
    {
        aProp = m_xIntrospectionAccess->getProperty(aPropertyName, ALLOWED_PROPERTIES);
    }
    catch (css::container::NoSuchElementException&)
    {
        throw css::beans::UnknownPropertyException(aPropertyName);
    }

    ensureTypeConverter();

    css::uno::Any aConvertedValue = m_xTypeConverter->convertTo(aValue, aProp.Type);

    try
    {
        return xPropertySet->setPropertyValue(aPropertyName, aConvertedValue);
    }
    catch (css::lang::WrappedTargetException& e)
    {
        throw css::reflection::InvocationTargetException(e.Message, pInvocation, e.TargetException);
    }
    catch (css::beans::PropertyVetoException& e)
    {
        throw css::reflection::InvocationTargetException(e.Message, pInvocation, css::uno::Any(e));
    }
}

css::uno::Any InvocationHelper::getValue(css::script::XInvocation* pInvocation,
                                         const OUString& aPropertyName)
{
    // Make sure the introspection access implements XPropertySet and that the property is one of
    // the ones allowed by hasProperty.

    if (!hasProperty(pInvocation, aPropertyName))
        throw css::beans::UnknownPropertyException(aPropertyName);

    css::uno::Reference<css::beans::XPropertySet> xPropertySet(m_xIntrospectionAccess,
                                                               css::uno::UNO_QUERY_THROW);

    try
    {
        return xPropertySet->getPropertyValue(aPropertyName);
    }
    catch (css::lang::WrappedTargetException& e)
    {
        throw css::reflection::InvocationTargetException(e.Message, pInvocation, e.TargetException);
    }
}

sal_Bool InvocationHelper::hasMethod(css::script::XInvocation* pInvocation, const OUString& aName)
{
    ensureIntrospection(pInvocation);

    return m_xIntrospectionAccess->hasMethod(aName, ALLOWED_METHODS);
}

sal_Bool InvocationHelper::hasProperty(css::script::XInvocation* pInvocation, const OUString& aName)
{
    ensureIntrospection(pInvocation);

    css::uno::Reference<css::beans::XPropertySet> xPropertySet(m_xIntrospectionAccess,
                                                               css::uno::UNO_QUERY);

    return xPropertySet.is() && m_xIntrospectionAccess->hasProperty(aName, ALLOWED_PROPERTIES);
}

void InvocationHelper::ensureTypeConverter()
{
    if (m_xTypeConverter.is())
        return;

    m_xTypeConverter = css::script::Converter::create(m_xContext);
}

void InvocationHelper::ensureIntrospection(css::script::XInvocation* pInvocation)
{
    if (m_xIntrospectionAccess.is())
        return;

    css::uno::Reference<css::beans::XIntrospection> xIntrospection
        = css::beans::theIntrospection::get(m_xContext);

    css::uno::Reference<css::script::XInvocation> xInvocation(pInvocation);
    m_xIntrospectionAccess = xIntrospection->inspect(css::uno::Any(xInvocation));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
