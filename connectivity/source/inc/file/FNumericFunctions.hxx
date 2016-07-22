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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FNUMERICFUNCTIONS_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FNUMERICFUNCTIONS_HXX

#include "file/fcode.hxx"
#include "file/filedllapi.hxx"

namespace connectivity
{
    namespace file
    {
        /** ABS(X)
            Returns the absolute value of X:

        > SELECT ABS(2);
                -> 2
        > SELECT ABS(-32);
                -> 32

        */
        class OOp_Abs : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** SIGN(X)
            Returns the sign of the argument as -1, 0, or 1, depending on whether X is negative, zero, or positive:

            > SELECT SIGN(-32);
                    -> -1
            > SELECT SIGN(0);
                    -> 0
            > SELECT SIGN(234);
                    -> 1

        */
        class OOp_Sign : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** MOD(N,M)
            %
                Modulo (like the % operator in C). Returns the remainder of N divided by M:

            > SELECT MOD(234, 10);
                    -> 4
            > SELECT 253 % 7;
                    -> 1
            > SELECT MOD(29,9);
                    -> 2
            > SELECT 29 MOD 9;
                    -> 2
        */
        class OOp_Mod : public OBinaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const override;
        };

        /** FLOOR(X)
            Returns the largest integer value not greater than X:

        > SELECT FLOOR(1.23);
                -> 1
        > SELECT FLOOR(-1.23);
                -> -2

        */
        class OOp_Floor : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** CEILING(X)
            Returns the smallest integer value not less than X:

        > SELECT CEILING(1.23);
                -> 2
        > SELECT CEILING(-1.23);
                -> -1

        */
        class OOp_Ceiling : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** ROUND(X)
            ROUND(X,D)
            Returns the argument X, rounded to the nearest integer. With two arguments rounded to a number to D decimals.

            > SELECT ROUND(-1.23);
                    -> -1
            > SELECT ROUND(-1.58);
                    -> -2
            > SELECT ROUND(1.58);
                    -> 2
            > SELECT ROUND(1.298, 1);
                    -> 1.3
            > SELECT ROUND(1.298, 0);
                    -> 1
            > SELECT ROUND(23.298, -1);
                    -> 20
        */
        class OOp_Round : public ONthOperator
        {
        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const override;
        };

        /** EXP(X)
            Returns the value of e (the base of natural logarithms) raised to the power of X:

        > SELECT EXP(2);
                -> 7.389056
        > SELECT EXP(-2);
                -> 0.135335
        */
        class OOp_Exp : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** LN(X)
            Returns the natural logarithm of X:

        > SELECT LN(2);
                -> 0.693147
        > SELECT LN(-2);
                -> NULL

        */
        class OOp_Ln : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** LOG(X)
            LOG(B,X)
                If called with one parameter, this function returns the natural logarithm of X:

            > SELECT LOG(2);
                    -> 0.693147
            > SELECT LOG(-2);
                    -> NULL

                If called with two parameters, this function returns the logarithm of X for an arbitrary base B:

            > SELECT LOG(2,65536);
                    -> 16.000000
            > SELECT LOG(1,100);
                    -> NULL
        */
        class OOp_Log : public ONthOperator
        {
        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const override;
        };

        /** LOG10(X)
            Returns the base-10 logarithm of X:

        > SELECT LOG10(2);
                -> 0.301030
        > SELECT LOG10(100);
                -> 2.000000
        > SELECT LOG10(-100);
                -> NULL
        */
        class OOp_Log10 : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** POWER(X,Y)
                Returns the value of X raised to the power of Y:

            > SELECT POW(2,2);
                    -> 4.000000
            > SELECT POW(2,-2);
                    -> 0.250000
        */
        class OOp_Pow : public OBinaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const override;
        };

        /** SQRT(X)
            Returns the non-negative square root of X:

        > SELECT SQRT(4);
                -> 2.000000
        > SELECT SQRT(20);
                -> 4.472136
        */
        class OOp_Sqrt : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** PI()
            Returns the value of PI. The default shown number of decimals is 5, but  internally uses the full double precession for PI.

        > SELECT PI();
                -> 3.141593
        > SELECT PI()+0.000000000000000000;
                -> 3.141592653589793238

        */
        class OOp_Pi : public ONthOperator
        {
        protected:
            virtual ORowSetValue operate(const ::std::vector<ORowSetValue>& lhs) const override;
        };

        /** COS(X)
            Returns the cosine of X, where X is given in radians:

        > SELECT COS(PI());
                -> -1.000000
        */
        class OOp_Cos : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** SIN(X)
            Returns the sine of X, where X is given in radians:

        > SELECT SIN(PI());
                -> 0.000000

        */
        class OOp_Sin : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };
        /** TAN(X)
            Returns the tangent of X, where X is given in radians:

        > SELECT TAN(PI()+1);
                -> 1.557408
        */
        class OOp_Tan : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** ACOS(X)
            Returns the arc cosine of X, that is, the value whose cosine is X. Returns NULL if X is not in the range -1 to 1:

        > SELECT ACOS(1);
                -> 0.000000
        > SELECT ACOS(1.0001);
                -> NULL
        > SELECT ACOS(0);
                -> 1.570796
        */
        class OOp_ACos : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** ASIN(X)
            Returns the arc sine of X, that is, the value whose sine is X. Returns NULL if X is not in the range -1 to 1:

        > SELECT ASIN(0.2);
                -> 0.201358
        > SELECT ASIN('foo');
                -> 0.000000
        */
        class OOp_ASin : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** ATAN(X)
            Returns the arc tangent of X, that is, the value whose tangent is X:

        > SELECT ATAN(2);
                -> 1.107149
        > SELECT ATAN(-2);
                -> -1.107149
        */
        class OOp_ATan : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** ATAN2(Y,X)
            Returns the arc tangent of the two variables X and Y. It is similar to calculating the arc tangent of Y / X, except that the signs of both arguments are used to determine the quadrant of the result:

        > SELECT ATAN2(-2,2);
                -> -0.785398
        > SELECT ATAN2(PI(),0);
                -> 1.570796

        */
        class OOp_ATan2 : public OBinaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs,const ORowSetValue& rhs) const override;
        };

        /** DEGREES(X)
            Returns the argument X, converted from radians to degrees:

        > SELECT DEGREES(PI());
                -> 180.000000
        */
        class OOp_Degrees : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };

        /** RADIANS(X)
            Returns the argument X, converted from degrees to radians:

        > SELECT RADIANS(90);
                -> 1.570796

        */
        class OOp_Radians : public OUnaryOperator
        {
        protected:
            virtual ORowSetValue operate(const ORowSetValue& lhs) const override;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_FILE_FNUMERICFUNCTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
