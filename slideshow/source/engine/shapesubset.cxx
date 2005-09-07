/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shapesubset.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:29:41 $
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

#include <canvas/debug.hxx>
#include <shapesubset.hxx>


namespace presentation
{
    namespace internal
    {
        ShapeSubset::ShapeSubset( const AttributableShapeSharedPtr& rOriginalShape,
                                  const DocTreeNode&                rTreeNode,
                                  const LayerManagerSharedPtr&      rLayerManager ) :
            mpOriginalShape( rOriginalShape ),
            mpSubsetShape(),
            maTreeNode( rTreeNode ),
            mpLayerManager( rLayerManager )
        {
            ENSURE_AND_THROW( mpLayerManager.get(),
                              "ShapeSubset::ShapeSubset(): Invalid layer manager" );
        }

        ShapeSubset::ShapeSubset( const ShapeSubsetSharedPtr&   rOriginalSubset,
                                  const DocTreeNode&            rTreeNode ) :
            mpOriginalShape( rOriginalSubset->mpSubsetShape.get() ?
                             rOriginalSubset->mpSubsetShape :
                             rOriginalSubset->mpOriginalShape ),
            mpSubsetShape(),
            maTreeNode( rTreeNode ),
            mpLayerManager( rOriginalSubset->mpLayerManager )
        {
            ENSURE_AND_THROW( mpLayerManager.get(),
                              "ShapeSubset::ShapeSubset(): Invalid layer manager" );
            ENSURE_AND_THROW( rOriginalSubset->maTreeNode.isEmpty() ||
                              (rTreeNode.getStartIndex() >= rOriginalSubset->maTreeNode.getStartIndex() &&
                               rTreeNode.getEndIndex() <= rOriginalSubset->maTreeNode.getEndIndex()),
                              "ShapeSubset::ShapeSubset(): Subset is bigger than parent" );
        }

        ShapeSubset::ShapeSubset( const AttributableShapeSharedPtr& rOriginalShape,
                                  const LayerManagerSharedPtr&      rLayerManager ) :
            mpOriginalShape( rOriginalShape ),
            mpSubsetShape(),
            maTreeNode(),
            mpLayerManager( rLayerManager )
        {
            ENSURE_AND_THROW( mpLayerManager.get(),
                              "ShapeSubset::ShapeSubset(): Invalid layer manager" );
        }

        ShapeSubset::~ShapeSubset()
        {
            // if not done yet: revoke subset from original
            disableSubsetShape();
        }

        AttributableShapeSharedPtr ShapeSubset::getSubsetShape() const
        {
            return mpSubsetShape.get() ? mpSubsetShape : mpOriginalShape;
        }

        bool ShapeSubset::enableSubsetShape()
        {
            if( !mpSubsetShape.get() &&
                !maTreeNode.isEmpty() )
            {
                mpSubsetShape = mpLayerManager->getSubsetShape(
                    mpOriginalShape,
                    maTreeNode );
            }

            return mpSubsetShape.get();
        }

        void ShapeSubset::disableSubsetShape()
        {
            if( mpSubsetShape.get() )
            {
                mpLayerManager->revokeSubset( mpOriginalShape,
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
