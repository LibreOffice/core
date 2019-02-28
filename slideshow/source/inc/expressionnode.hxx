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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_EXPRESSIONNODE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_EXPRESSIONNODE_HXX

#include <memory>

/* Definition of ExpressionNode interface */

namespace slideshow
{
    namespace internal
    {
        /** Interface describing an abstract animation function.

            Use this interface to model time-dependent animation
            functions of one variable.
         */
        class AnimationFunction
        {
        public:
            virtual ~AnimationFunction() {}

            /** Operator to calculate function value.

                This method calculates the function value for the
                given time instant t.

                @param t
                Current time instant, must be in the range [0,1]

                @return the function value, typically in relative
                user coordinate space ([0,1] range).
             */
            virtual double operator()( double t ) const = 0;

        };

        /** Refinement of AnimationFunction

            This interface is used by the SMIL function parser, to
            collapse constant values into precalculated, single nodes.
        */
        class ExpressionNode : public AnimationFunction
        {
        public:
            /** Predicate whether this node is constant.

                This predicate returns true, if this node is
                neither time- nor ViewInfo dependent. This allows
                for certain obtimizations, i.e. not the full
                expression tree needs be represented by
                ExpressionNodes.

                @returns true, if this node is neither time- nor
                ViewInfo dependent
            */
            virtual bool isConstant() const = 0;
        };
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_EXPRESSIONNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
