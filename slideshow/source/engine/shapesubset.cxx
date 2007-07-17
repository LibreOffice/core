/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shapesubset.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 14:39:43 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>

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
            ENSURE_AND_THROW( mpShapeManager,
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
            ENSURE_AND_THROW( mpShapeManager,
                              "ShapeSubset::ShapeSubset(): Invalid shape manager" );
            ENSURE_AND_THROW( rOriginalSubset->maTreeNode.isEmpty() ||
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
            ENSURE_AND_THROW( mpShapeManager,
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

            return mpSubsetShape;
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
