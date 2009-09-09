/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vmlshapecontainer.cxx,v $
 * $Revision: 1.1 $
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

#include "oox/vml/vmlshapecontainer.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshape.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::drawing::XShapes;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace vml {

// ============================================================================

namespace {

template< typename ShapeType >
void lclMapShapesById( RefMap< OUString, ShapeType >& orMap, const RefVector< ShapeType >& rVector )
{
    for( typename RefVector< ShapeType >::const_iterator aIt = rVector.begin(), aEnd = rVector.end(); aIt != aEnd; ++aIt )
    {
        const OUString& rShapeId = (*aIt)->getShapeId();
        OSL_ENSURE( rShapeId.getLength() > 0, "lclMapShapesById - missing shape identifier" );
        if( rShapeId.getLength() > 0 )
        {
            OSL_ENSURE( orMap.find( rShapeId ) == orMap.end(), "lclMapShapesById - shape identifier already used" );
            orMap[ rShapeId ] = *aIt;
        }
    }
}

} // namespace

// ============================================================================

ShapeContainer::ShapeContainer( const Drawing& rDrawing ) :
    mrDrawing( rDrawing )
{
}

ShapeContainer::~ShapeContainer()
{
}

ShapeType& ShapeContainer::createShapeType()
{
    ::boost::shared_ptr< ShapeType > xShape( new ShapeType( mrDrawing ) );
    maTypes.push_back( xShape );
    return *xShape;
}

void ShapeContainer::finalizeFragmentImport()
{
    // map all shape templates by shape identifier
    lclMapShapesById( maTypesById, maTypes );
    // map all shapes by shape identifier
    lclMapShapesById( maShapesById, maShapes );
    /*  process all shapes (map all children templates/shapes in group shapes,
        resolve template references in all shapes) */
    maShapes.forEachMem( &ShapeBase::finalizeFragmentImport );
}

const ShapeType* ShapeContainer::getShapeTypeById( const OUString& rShapeId, bool bDeep ) const
{
    // search in own shape template list
    if( const ShapeType* pType = maTypesById.get( rShapeId ).get() )
        return pType;
    // search deep in child shapes
    if( bDeep )
        for( ShapeVector::const_iterator aVIt = maShapes.begin(), aVEnd = maShapes.end(); aVIt != aVEnd; ++aVIt )
            if( const ShapeType* pType = (*aVIt)->getChildTypeById( rShapeId ) )
                return pType;
   return 0;
}

const ShapeBase* ShapeContainer::getShapeById( const OUString& rShapeId, bool bDeep ) const
{
    // search in own shape list
    if( const ShapeBase* pShape = maShapesById.get( rShapeId ).get() )
        return pShape;
    // search deep in child shapes
    if( bDeep )
        for( ShapeVector::const_iterator aVIt = maShapes.begin(), aVEnd = maShapes.end(); aVIt != aVEnd; ++aVIt )
            if( const ShapeBase* pShape = (*aVIt)->getChildById( rShapeId ) )
                return pShape;
   return 0;
}

const ShapeBase* ShapeContainer::getFirstShape() const
{
    OSL_ENSURE( mrDrawing.getType() == VMLDRAWING_WORD, "ShapeContainer::getFirstShape - illegal call, Word filter only" );
    OSL_ENSURE( maShapes.size() == 1, "ShapeContainer::getFirstShape - single shape expected" );
    return maShapes.get( 0 ).get();
}

void ShapeContainer::convertAndInsert( const Reference< XShapes >& rxShapes, const ShapeParentAnchor* pParentAnchor ) const
{
    for( ShapeVector::const_iterator aIt = maShapes.begin(), aEnd = maShapes.end(); aIt != aEnd; ++aIt )
        (*aIt)->convertAndInsert( rxShapes, pParentAnchor );
}

// ============================================================================

} // namespace vml
} // namespace oox

