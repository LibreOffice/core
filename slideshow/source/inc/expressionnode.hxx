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
