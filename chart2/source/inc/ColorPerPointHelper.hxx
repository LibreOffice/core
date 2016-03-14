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

#ifndef INCLUDED_CHART2_SOURCE_INC_COLORPERPOINTHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_COLORPERPOINTHELPER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{

/**
*/

class OOO_DLLPUBLIC_CHARTTOOLS ColorPerPointHelper
{
public:
    static bool hasPointOwnColor(
        const css::uno::Reference< css::beans::XPropertySet >& xDataSeriesProperties
        , sal_Int32 nPointIndex
        , const css::uno::Reference< css::beans::XPropertySet >& xDataPointProperties //may be NULL this is just for performance
         );

    // returns true if AttributedDataPoints contains nPointIndex and the
    // property Color is DEFAULT
    SAL_DLLPRIVATE static bool hasPointOwnProperties(
        const css::uno::Reference< css::beans::XPropertySet >& xSeriesProperties
        , sal_Int32 nPointIndex );
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
