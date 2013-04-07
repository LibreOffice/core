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

#ifndef INCLUDED_SLIDESHOW_SMILFUNCTIONPARSER_HXX
#define INCLUDED_SLIDESHOW_SMILFUNCTIONPARSER_HXX

#include "expressionnode.hxx"
#include "slideshowexceptions.hxx"

#include <basegfx/range/b2drectangle.hxx>

#include <boost/noncopyable.hpp>

namespace rtl
{
    class OUString;
}

/* Definition of SmilFunctionParser class */

namespace slideshow
{
    namespace internal
    {
        class SmilFunctionParser : private boost::noncopyable
        {
        public:
            /** Parse a string containing a SMIL value.

                This method parses a string representing
                a fixed value (i.e. a value that does not
                change by time). Due to the dynamic view
                capabilities of the presentation engine,
                this value can sometimes only be determined
                during runtime of the animation (because
                e.g. mixed screen/view coordinates are
                involved), and is thus still returned as an
                ExpressionNode object. An example for
                such a case is the "Width+1.0" string, which
                contains the width of the shape in user
                coordinate space, and the screen width
                in device coordinate space.

                The following grammar is accepted by this method:
                <code>
               identifier = 'pi'|'e'|'X'|'Y'|'Width'|'Height'

               function = 'abs'|'sqrt'|'sin'|'cos'|'tan'|'atan'|'acos'|'asin'|'exp'|'log'

               basic_expression =
                                number |
                                identifier |
                                function '(' additive_expression ')' |
                                '(' additive_expression ')'

               unary_expression =
                                   '-' basic_expression |
                                basic_expression

               multiplicative_expression =
                                   unary_expression ( ( '*' unary_expression )* |
                                                   ( '/' unary_expression )* )

               additive_expression =
                                   multiplicative_expression ( ( '+' multiplicative_expression )* |
                                                               ( '-' multiplicative_expression )* )

                </code>

                @param rSmilValue
                The string to parse

                @param rRelativeShapeBounds
                The bounds of the shape this SMIL value is to be
                evaluated for. The bounds must be <em>relative</em> to
                the page the shape is part of, i.e. within the [0,1]
                range. This is necessary, since the string might
                contain symbolic references to the shape bounding box.

                @throws ParseError if an invalid expression is given.

                @return the generated function object.
             */
            static ExpressionNodeSharedPtr parseSmilValue( const OUString&           rSmilValue,
                                                           const ::basegfx::B2DRectangle&   rRelativeShapeBounds ); // throw ParseError

            /** Parse a string containing a SMIL function.

                This method parses a string representing
                a possibly time-varying SMIL function.

                The following grammar is accepted by this method:
                <code>
               identifier = 't'|'pi'|'e'|'X'|'Y'|'Width'|'Height'

               function = 'abs'|'sqrt'|'sin'|'cos'|'tan'|'atan'|'acos'|'asin'|'exp'|'log'

               basic_expression =
                                number |
                                identifier |
                                function '(' additive_expression ')' |
                                '(' additive_expression ')'

               unary_expression =
                                   '-' basic_expression |
                                basic_expression

               multiplicative_expression =
                                   unary_expression ( ( '*' unary_expression )* |
                                                   ( '/' unary_expression )* )

               additive_expression =
                                   multiplicative_expression ( ( '+' multiplicative_expression )* |
                                                               ( '-' multiplicative_expression )* )

                </code>

                @param rSmilFunction
                The string to parse

                @param rRelativeShapeBounds
                The bounds of the shape this SMIL value is to be
                evaluated for. The bounds must be <em>relative</em> to
                the page the shape is part of, i.e. within the [0,1]
                range. This is necessary, since the string might
                contain symbolic references to the shape bounding box.

                @throws ParseError if an invalid expression is given.

                @return the generated function object.
             */
            static ExpressionNodeSharedPtr parseSmilFunction( const OUString&            rSmilFunction,
                                                              const ::basegfx::B2DRectangle&    rRelativeShapeBounds ); // throw ParseError

        private:
            // disabled constructor/destructor, since this is
            // supposed to be a singleton
            SmilFunctionParser();
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_SMILFUNCTIONPARSER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
