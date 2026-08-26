/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#if defined LIBO_INTERNAL_ONLY

#include <comphelper/comphelperdllapi.h>

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

namespace com::sun::star::beans
{
class XIntrospectionAccess;
class XPropertySet;
}

namespace com::sun::star::reflection
{
struct ParamInfo;
}

namespace com::sun::star::script
{
class XInvocation;
class XTypeConverter;
}

namespace com::sun::star::uno
{
class XComponentContext;
}

namespace comphelper
{
/** Helper class to implement css::script::Invocation.

    The methods of this implementation use the css::beans::XIntrospection service to query the
    methods and properties advertised by the interfaces of the object. The implementation can use
    this to provide extra methods and properties. It should have an instance of this class as a
    private member and then call the helper methods passing ‘this’ as the first parameter when it
    encounters unhandled names in order to continue providing access to the base methods via the
    XInvocation interface.
*/
class COMPHELPER_DLLPUBLIC InvocationHelper
{
public:
    InvocationHelper(const css::uno::Reference<css::uno::XComponentContext>& xContext);

    // All the methods of XInvocation. The implementation can call these passing ‘this’ as the first
    // parameter.
    css::uno::Reference<css::beans::XIntrospectionAccess>
    getIntrospection(css::script::XInvocation* pInvocation);
    css::uno::Any invoke(css::script::XInvocation* pInvocation, const OUString& aFunctionName,
                         const css::uno::Sequence<css::uno::Any>& aParams,
                         css::uno::Sequence<sal_Int16>& aOutParamIndex,
                         css::uno::Sequence<css::uno::Any>& aOutParam);
    void setValue(css::script::XInvocation* pInvocation, const OUString& aPropertyName,
                  const css::uno::Any& aValue);
    css::uno::Any getValue(css::script::XInvocation* pInvocation, const OUString& aPropertyName);
    sal_Bool hasMethod(css::script::XInvocation* pInvocation, const OUString& aName);
    sal_Bool hasProperty(css::script::XInvocation* pInvocation, const OUString& aName);

private:
    css::uno::Reference<css::uno::XComponentContext> m_xContext;
    css::uno::Reference<css::script::XTypeConverter> m_xTypeConverter;
    css::uno::Reference<css::beans::XIntrospectionAccess> m_xIntrospectionAccess;

    void ensureTypeConverter();
    void ensureIntrospection(css::script::XInvocation* pInvocation);
    css::uno::Sequence<css::uno::Any>
    prepareInvokeParams(const css::uno::Sequence<css::uno::Any>& aParams,
                        const css::uno::Sequence<css::reflection::ParamInfo>& aParamInfos);
};
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
