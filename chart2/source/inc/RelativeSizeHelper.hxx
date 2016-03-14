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
#ifndef INCLUDED_CHART2_SOURCE_INC_RELATIVESIZEHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_RELATIVESIZEHELPER_HXX

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS RelativeSizeHelper
{
public:
    static double calculate(
        double fValue,
        const css::awt::Size & rOldReferenceSize,
        const css::awt::Size & rNewReferenceSize );

    static void adaptFontSizes(
        const css::uno::Reference<
            css::beans::XPropertySet > & xTargetProperties,
        const css::awt::Size & rOldReferenceSize,
        const css::awt::Size & rNewReferenceSize );

private:
    RelativeSizeHelper() = delete;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_INC_RELATIVESIZEHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
