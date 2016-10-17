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

namespace comphelper
{

/**
 * Creates a beans::PropertyValue easily, i.e. you can write:
 *
 * function(comphelper::makePropertyValue("Foo", nBar));
 */
template<typename T>
css::beans::PropertyValue makePropertyValue(const OUString& rName, const T& rValue)
{
    return {rName, 0, css::uno::makeAny(rValue), css::beans::PropertyState_DIRECT_VALUE};
}

template<>
inline css::beans::PropertyValue makePropertyValue(const OUString& rName, const css::uno::Any& rValue)
{
    return {rName, 0, rValue, css::beans::PropertyState_DIRECT_VALUE};
}

}

#endif // INCLUDED_COMPHELPER_PROPERTYVALUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
