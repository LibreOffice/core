/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: expressionnode.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:11:58 $
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

#ifndef _SLIDESHOW_EXPRESSIONNODE_HXX
#define _SLIDESHOW_EXPRESSIONNODE_HXX

#include <animationfunction.hxx>


/* Definition of ExpressionNode interface */

namespace presentation
{
    namespace internal
    {
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

        typedef ::boost::shared_ptr< ExpressionNode > ExpressionNodeSharedPtr;
    }
}

#endif /* _SLIDESHOW_EXPRESSIONNODE_HXX */
