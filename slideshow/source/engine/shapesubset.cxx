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
                OSL_FAIL( rtl::OUStringToOString(
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
