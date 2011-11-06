/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef INCLUDED_FRACTION_HXX
#define INCLUDED_FRACTION_HXX

#include <sal/types.h>

namespace writerfilter {
namespace resourcemodel {

class Fraction
{
public:
    explicit Fraction(sal_Int32 nNumerator, sal_Int32 nDenominator = 1);
    explicit Fraction(const Fraction & a, const Fraction & b);
    virtual ~Fraction();

    void init(sal_Int32 nNumerator, sal_Int32 nDenominator);
    void assign(const Fraction & rFraction);

    Fraction inverse() const;

    Fraction operator=(const Fraction & rFraction);
    Fraction operator+(const Fraction & rFraction) const;
    Fraction operator-(const Fraction & rFraction) const;
    Fraction operator*(const Fraction & rFraction) const;
    Fraction operator/(const Fraction & rFraction) const;
    operator sal_Int32() const;
    operator float() const;

private:
    sal_Int32 mnNumerator;
    sal_Int32 mnDenominator;
};
}}
#endif // INCLUDED_FRACTION_HXX
