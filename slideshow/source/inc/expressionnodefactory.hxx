/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: expressionnodefactory.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:57:15 $
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

#ifndef _SLIDESHOW_EXPRESSIONNODEFACTORY_HXX
#define _SLIDESHOW_EXPRESSIONNODEFACTORY_HXX

#include <expressionnode.hxx>

#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/range/b2drectangle.hxx>


/* Definition of ExpressionNodeFactory class */

namespace slideshow
{
    namespace internal
    {
        /** ExpressionNode factory

            This class can be used to generate a wide variety of
            ExpressionNode objects, e.g. when parsing SMIL function
            expressions.
        */
        class ExpressionNodeFactory
        {
        public:
            static ExpressionNodeSharedPtr createConstantValueExpression( double rConstantValue );

            static ExpressionNodeSharedPtr createValueTExpression    ();

            static ExpressionNodeSharedPtr createPlusExpression      ( const ExpressionNodeSharedPtr&   rLHS,
                                                                       const ExpressionNodeSharedPtr&   rRHS );
            static ExpressionNodeSharedPtr createMinusExpression     ( const ExpressionNodeSharedPtr&   rLHS,
                                                                       const ExpressionNodeSharedPtr&   rRHS );
            static ExpressionNodeSharedPtr createMultipliesExpression( const ExpressionNodeSharedPtr&   rLHS,
                                                                       const ExpressionNodeSharedPtr&   rRHS );
            static ExpressionNodeSharedPtr createDividesExpression   ( const ExpressionNodeSharedPtr&   rLHS,
                                                                       const ExpressionNodeSharedPtr&   rRHS );

            /** Composes two ExpressionNode function.

                The resulting expression will calculate
                rOuterFunction( rInnerFunction(t) ).
             */
            static ExpressionNodeSharedPtr createComposedExpression  ( const ExpressionNodeSharedPtr&   rOuterFunction,
                                                                       const ExpressionNodeSharedPtr&   rInnerFunction );

            static ExpressionNodeSharedPtr createMinExpression      ( const ExpressionNodeSharedPtr&    rOuterFunction,
                                                                      const ExpressionNodeSharedPtr&    rInnerFunction );

            static ExpressionNodeSharedPtr createMaxExpression      ( const ExpressionNodeSharedPtr&    rOuterFunction,
                                                                      const ExpressionNodeSharedPtr&    rInnerFunction );
        };
    }
}

#endif /* _SLIDESHOW_EXPRESSIONNODEFACTORY_HXX */
