/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_PROPERTYVALUE_HXX
#define INCLUDED_COMPHELPER_PROPERTYVALUE_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <type_traits>
#include <utility>

namespace comphelper
{

/**
 * Creates a beans::PropertyValue easily, i.e. you can write:
 *
 * function(comphelper::makePropertyValue("Foo", nBar));
 */
template<typename T, typename std::enable_if<!std::is_same<T, css::uno::Any>::value &&
                                             !std::is_same<T, sal_Int16>::value, int>::type = 0>
css::beans::PropertyValue makePropertyValue(const OUString& rName, T&& rValue)
{
    return {rName, 0, css::uno::makeAny<T>(std::forward<T>(rValue)), css::beans::PropertyState_DIRECT_VALUE};
}

// workaround for MSVC 2013: if makePropertyValue<sal_Int16> is called with sal_Int32 rValue
// it cannot convert argument from ´sal_Int32´ to ´sal_Int16 &&´
inline css::beans::PropertyValue makePropertyValue(const OUString& rName, const sal_Int16& rValue)
{
    return {rName, 0, css::uno::makeAny<sal_Int16>(rValue), css::beans::PropertyState_DIRECT_VALUE};
}

inline css::beans::PropertyValue makePropertyValue(const OUString& rName, const css::uno::Any& rValue)
{
    return {rName, 0, rValue, css::beans::PropertyState_DIRECT_VALUE};
}

}
#endif // INCLUDED_COMPHELPER_PROPERTYVALUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
