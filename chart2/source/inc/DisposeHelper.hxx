/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_CHART2_SOURCE_INC_DISPOSEHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_DISPOSEHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XComponent.hpp>

namespace chart
{
namespace DisposeHelper
{

template<class T>
void Dispose(const T & xInterface)
{
    css::uno::Reference<css::lang::XComponent> xComponent(xInterface, css::uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}

template<class T>
void DisposeAndClear(css::uno::Reference<T> & rInterface)
{
    Dispose<css::uno::Reference<T>>(rInterface);
    rInterface.set(nullptr);
}

template<class Container>
void DisposeAllElements(Container & rContainer)
{
    for (const auto & rElement : rContainer)
    {
        Dispose<typename Container::value_type>(rElement);
    }
}

}} //  namespace chart::DisposeHelper

#endif // INCLUDED_CHART2_SOURCE_INC_DISPOSEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
