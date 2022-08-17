/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <comphelper/diagnose_ex.hxx>

#include <shapesubset.hxx>
#include <utility>


using namespace ::com::sun::star;

namespace slideshow::internal
{
        ShapeSubset::ShapeSubset( AttributableShapeSharedPtr              xOriginalShape,
                                  const DocTreeNode&                      rTreeNode,
                                  SubsettableShapeManagerSharedPtr xShapeManager ) :
            mpOriginalShape(std::move( xOriginalShape )),
            mpSubsetShape(),
            maTreeNode( rTreeNode ),
            mpShapeManager(std::move( xShapeManager ))
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

        ShapeSubset::ShapeSubset( AttributableShapeSharedPtr        xOriginalShape,
                                  SubsettableShapeManagerSharedPtr  xShapeManager ) :
            mpOriginalShape(std::move( xOriginalShape )),
            mpSubsetShape(),
            maTreeNode(),
            mpShapeManager(std::move( xShapeManager ))
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
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("slideshow", "");
            }
        }

        AttributableShapeSharedPtr const & ShapeSubset::getSubsetShape() const
        {
            return mpSubsetShape ? mpSubsetShape : mpOriginalShape;
        }

        void ShapeSubset::enableSubsetShape()
        {
            if( !mpSubsetShape &&
                !maTreeNode.isEmpty() )
            {
                mpSubsetShape = mpShapeManager->getSubsetShape(
                    mpOriginalShape,
                    maTreeNode );
            }
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

        const DocTreeNode& ShapeSubset::getSubset() const
        {
            return maTreeNode;
        }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
