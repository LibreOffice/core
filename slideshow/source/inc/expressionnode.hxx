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

#ifndef INCLUDED_SLIDESHOW_EXPRESSIONNODE_HXX
#define INCLUDED_SLIDESHOW_EXPRESSIONNODE_HXX

#include "animationfunction.hxx"


/* Definition of ExpressionNode interface */

namespace slideshow
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

#endif /* INCLUDED_SLIDESHOW_EXPRESSIONNODE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
