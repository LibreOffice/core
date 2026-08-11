/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/script/XInvocation.hpp>

namespace com::sun::star::uno
{
class XComponentContext;
}

namespace
{
// A service that accepts any method calls and returns the name of the method as a string. This is
// intended to test that language bindings can handle objects that implement XInvocation. This
// should intentionally not implement XInvocation2 so that we can be sure that the language bindings
// handle that common case.
class MethodToString : public cppu::WeakImplHelper<css::script::XInvocation>
{
public:
    css::uno::Reference<css::beans::XIntrospectionAccess> SAL_CALL getIntrospection() override;
    css::uno::Any SAL_CALL invoke(const OUString& aFunctionName,
                                  const css::uno::Sequence<css::uno::Any>& aParams,
                                  css::uno::Sequence<::sal_Int16>& aOutParamIndex,
                                  css::uno::Sequence<css::uno::Any>& aOutParam) override;
    void SAL_CALL setValue(const OUString& aPropertyName, const css::uno::Any& aValue) override;
    css::uno::Any SAL_CALL getValue(const OUString& aPropertyName) override;
    sal_Bool SAL_CALL hasMethod(const OUString& aName) override;
    sal_Bool SAL_CALL hasProperty(const OUString& aName) override;
};

css::uno::Reference<css::beans::XIntrospectionAccess> SAL_CALL MethodToString::getIntrospection()
{
    return css::uno::Reference<css::beans::XIntrospectionAccess>();
}

css::uno::Any SAL_CALL MethodToString::invoke(const OUString& aFunctionName,
                                              const css::uno::Sequence<css::uno::Any>&,
                                              css::uno::Sequence<::sal_Int16>&,
                                              css::uno::Sequence<css::uno::Any>&)
{
    return css::uno::Any(aFunctionName);
}

void SAL_CALL MethodToString::setValue(const OUString&, const css::uno::Any&) {}

css::uno::Any SAL_CALL MethodToString::getValue(const OUString&) { return css::uno::Any(); }

sal_Bool SAL_CALL MethodToString::hasMethod(const OUString&)
{
    // All methods are available
    return true;
}

sal_Bool SAL_CALL MethodToString::hasProperty(const OUString&) { return false; }
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_libreoffice_comp_embindtest_MethodToString_get_implementation(
    css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new MethodToString);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
