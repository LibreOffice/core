/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "oox/vml/vmlshapecontainer.hxx"

#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshape.hxx"

namespace oox {
namespace vml {



using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;



namespace {

template< typename ShapeType >
void lclMapShapesById( RefMap< OUString, ShapeType >& orMap, const RefVector< ShapeType >& rVector )
{
    for( typename RefVector< ShapeType >::const_iterator aIt = rVector.begin(), aEnd = rVector.end(); aIt != aEnd; ++aIt )
    {
        const OUString& rShapeId = (*aIt)->getShapeId();
        OSL_ENSURE( !rShapeId.isEmpty(), "lclMapShapesById - missing shape identifier" );
        if( !rShapeId.isEmpty() )
        {
            OSL_ENSURE( orMap.find( rShapeId ) == orMap.end(), "lclMapShapesById - shape identifier already used " );
            orMap[ rShapeId ] = *aIt;
        }
    }
}

} 



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
    
    lclMapShapesById( maTypesById, maTypes );
    
    lclMapShapesById( maShapesById, maShapes );
    /*  process all shapes (map all children templates/shapes in group shapes,
        resolve template references in all shapes) */
    maShapes.forEachMem( &ShapeBase::finalizeFragmentImport );
}

const ShapeType* ShapeContainer::getShapeTypeById( const OUString& rShapeId, bool bDeep ) const
{
    
    if( const ShapeType* pType = maTypesById.get( rShapeId ).get() )
        return pType;
    
    if( bDeep )
        for( ShapeVector::const_iterator aVIt = maShapes.begin(), aVEnd = maShapes.end(); aVIt != aVEnd; ++aVIt )
            if( const ShapeType* pType = (*aVIt)->getChildTypeById( rShapeId ) )
                return pType;
   return 0;
}

const ShapeBase* ShapeContainer::getShapeById( const OUString& rShapeId, bool bDeep ) const
{
    
    if( const ShapeBase* pShape = maShapesById.get( rShapeId ).get() )
        return pShape;
    
    if( bDeep )
        for( ShapeVector::const_iterator aVIt = maShapes.begin(), aVEnd = maShapes.end(); aVIt != aVEnd; ++aVIt )
            if( const ShapeBase* pShape = (*aVIt)->getChildById( rShapeId ) )
                return pShape;
   return 0;
}

boost::shared_ptr< ShapeBase > ShapeContainer::takeLastShape()
{
    OSL_ENSURE( mrDrawing.getType() == VMLDRAWING_WORD, "ShapeContainer::takeLastShape - illegal call, Word filter only" );
    assert( !markStack.empty());
    if( markStack.top() >= maShapes.size())
        return boost::shared_ptr< ShapeBase >();
    boost::shared_ptr< ShapeBase > ret = maShapes.back();
    maShapes.pop_back();
    return ret;
}

void ShapeContainer::pushMark()
{
    markStack.push( maShapes.size());
}

void ShapeContainer::popMark()
{
    assert( !markStack.empty());
    markStack.pop();
}

void ShapeContainer::convertAndInsert( const Reference< XShapes >& rxShapes, const ShapeParentAnchor* pParentAnchor ) const
{
    for( ShapeVector::const_iterator aIt = maShapes.begin(), aEnd = maShapes.end(); aIt != aEnd; ++aIt )
        (*aIt)->convertAndInsert( rxShapes, pParentAnchor );
}



} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
