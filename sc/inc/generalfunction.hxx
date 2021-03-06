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
#pragma once

#include <com/sun/star/sheet/GeneralFunction.hpp>
#include <com/sun/star/sheet/GeneralFunction2.hpp>

/**
 * the css::sheet::GeneralFunction enum is extended by constants in GeneralFunction2, which causes some type-safety issues.
 * So abstract them behind the facade of this enum.
 */
enum class ScGeneralFunction
{
    /** nothing is calculated.
     */
    NONE = int(css::sheet::GeneralFunction_NONE),

    /** function is determined automatically.

        <p>If the values are all numerical, SUM is used, otherwise COUNT.</p>
     */
    AUTO = int(css::sheet::GeneralFunction_AUTO),

    /** sum of all numerical values is calculated.
     */
    SUM = int(css::sheet::GeneralFunction_SUM),

    /** all values, including non-numerical values, are counted.
     */
    COUNT = int(css::sheet::GeneralFunction_COUNT),

    /** average of all numerical values is calculated.
     */
    AVERAGE = int(css::sheet::GeneralFunction_AVERAGE),

    /** maximum value of all numerical values is calculated.
     */
    MAX = int(css::sheet::GeneralFunction_MAX),

    /** minimum value of all numerical values is calculated.
     */
    MIN = int(css::sheet::GeneralFunction_MIN),

    /** product of all numerical values is calculated.
     */
    PRODUCT = int(css::sheet::GeneralFunction_PRODUCT),

    /** numerical values are counted.
     */
    COUNTNUMS = int(css::sheet::GeneralFunction_COUNTNUMS),

    /** standard deviation is calculated based on a sample.
     */
    STDEV = int(css::sheet::GeneralFunction_STDEV),

    /** standard deviation is calculated based on the entire population.
     */
    STDEVP = int(css::sheet::GeneralFunction_STDEVP),

    /** variance is calculated based on a sample.
     */
    VAR = int(css::sheet::GeneralFunction_VAR),

    /** variance is calculated based on the entire population.
     */
    VARP = int(css::sheet::GeneralFunction_VARP),

    /**
    *   median of all numerical values is calculated.
    *   @since LibreOffice 5.3
    */
    MEDIAN = css::sheet::GeneralFunction2::MEDIAN
};
