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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include "shapesubset.hxx"


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        ShapeSubset::ShapeSubset( const AttributableShapeSharedPtr&       rOriginalShape,
                                  const DocTreeNode&                      rTreeNode,
                                  const SubsettableShapeManagerSharedPtr& rShapeManager ) :
            mpOriginalShape( rOriginalShape ),
            mpSubsetShape(),
            maTreeNode( rTreeNode ),
            mpShapeManager( rShapeManager )
        {
            ENSURE_OR_THROW( mpShapeManager,
                              "ShapeSubset::ShapeSubset(): Invalid shape manager" );
        }

        ShapeSubset::ShapeSubset( const ShapeSubsetSharedPtr&   rOriginalSubset,
                                  const DocTreeNode&            rTreeNode ) :
            mpOriginalShape( rOriginalSubset->mpSubsetShape ?
                             rOriginalSubset->mpSubsetShape :
                             rOriginalSubset->mpOriginalShape ),
            mpSubsetShape(),
            maTreeNode( rTreeNode ),
            mpShapeManager( rOriginalSubset->mpShapeManager )
        {
            ENSURE_OR_THROW( mpShapeManager,
                              "ShapeSubset::ShapeSubset(): Invalid shape manager" );
            ENSURE_OR_THROW( rOriginalSubset->maTreeNode.isEmpty() ||
                              (rTreeNode.getStartIndex() >= rOriginalSubset->maTreeNode.getStartIndex() &&
                               rTreeNode.getEndIndex() <= rOriginalSubset->maTreeNode.getEndIndex()),
                              "ShapeSubset::ShapeSubset(): Subset is bigger than parent" );
        }

        ShapeSubset::ShapeSubset( const AttributableShapeSharedPtr&       rOriginalShape,
                                  const SubsettableShapeManagerSharedPtr& rShapeManager ) :
            mpOriginalShape( rOriginalShape ),
            mpSubsetShape(),
            maTreeNode(),
            mpShapeManager( rShapeManager )
        {
            ENSURE_OR_THROW( mpShapeManager,
                              "ShapeSubset::ShapeSubset(): Invalid shape manager" );
        }

        ShapeSubset::~ShapeSubset()
        {
            try
            {
                // if not done yet: revoke subset from original
                disableSubsetShape();
            }
            catch (uno::Exception &)
            {
                OSL_ENSURE( false, rtl::OUStringToOString(
                                comphelper::anyToString(
                                    cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
            }
        }

        AttributableShapeSharedPtr ShapeSubset::getSubsetShape() const
        {
            return mpSubsetShape ? mpSubsetShape : mpOriginalShape;
        }

        bool ShapeSubset::enableSubsetShape()
        {
            if( !mpSubsetShape &&
                !maTreeNode.isEmpty() )
            {
                mpSubsetShape = mpShapeManager->getSubsetShape(
                    mpOriginalShape,
                    maTreeNode );
            }

            return (mpSubsetShape.get() != NULL);
        }

        void ShapeSubset::disableSubsetShape()
        {
            if( mpSubsetShape )
            {
                mpShapeManager->revokeSubset( mpOriginalShape,
                                              mpSubsetShape );
                mpSubsetShape.reset();
            }
        }

        bool ShapeSubset::isFullSet() const
        {
            return maTreeNode.isEmpty();
        }

        DocTreeNode ShapeSubset::getSubset() const
        {
            return maTreeNode;
        }

    }
}
