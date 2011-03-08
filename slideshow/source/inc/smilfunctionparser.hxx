/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
            static ExpressionNodeSharedPtr parseSmilValue( const ::rtl::OUString&           rSmilValue,
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
            static ExpressionNodeSharedPtr parseSmilFunction( const ::rtl::OUString&            rSmilFunction,
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
