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



#include "oox/vml/vmlshapecontainer.hxx"

#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshape.hxx"

namespace oox {
namespace vml {

// ============================================================================

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

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

ShapeContainer::ShapeContainer( Drawing& rDrawing ) :
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
