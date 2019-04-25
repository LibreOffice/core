/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <utility>

#include <com/sun/star/beans/PropertyValue.hpp>

namespace comphelper
{

/**
 * Creates a beans::PropertyValue easily, i.e. you can write:
 *
 * function(comphelper::makePropertyValue("Foo", nBar));
 *
 * instead of writing 3 extra lines to set the name and value of the beans::PropertyValue.
 */
template<typename T> css::beans::PropertyValue makePropertyValue(const OUString& rName, T&& rValue)
{
    css::beans::PropertyValue aValue;
    aValue.Name = rName;
    aValue.Value = css::uno::toAny(std::forward<T>(rValue));
    return aValue;
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
