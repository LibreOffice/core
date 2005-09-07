/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: smilfunctionparser.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:21:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SLIDESHOW_SMILFUNCTIONPARSER_HXX
#define _SLIDESHOW_SMILFUNCTIONPARSER_HXX

#include <expressionnode.hxx>
#include <slideshowexceptions.hxx>

#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif

namespace rtl
{
    class OUString;
}

/* Definition of SmilFunctionParser class */

namespace presentation
{
    namespace internal
    {
        class SmilFunctionParser
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

            // default: disabled copy/assignment
            SmilFunctionParser(const SmilFunctionParser&);
            SmilFunctionParser& operator=( const SmilFunctionParser& );
        };
    }
}

#endif /* _SLIDESHOW_SMILFUNCTIONPARSER_HXX */
