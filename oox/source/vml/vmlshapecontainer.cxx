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

#include <oox/vml/vmlshapecontainer.hxx>

#include <oox/vml/vmldrawing.hxx>
#include <oox/vml/vmlshape.hxx>

#include <osl/diagnose.h>

namespace oox {
namespace vml {

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;

namespace {

template< typename ShapeType >
void lclMapShapesById( RefMap< OUString, ShapeType >& orMap, const RefVector< ShapeType >& rVector )
{
    for (auto const& elem : rVector)
    {
        const OUString& rShapeId = elem->getShapeId();
        OSL_ENSURE( !rShapeId.isEmpty(), "lclMapShapesById - missing shape identifier" );
        if( !rShapeId.isEmpty() )
        {
            OSL_ENSURE( orMap.find( rShapeId ) == orMap.end(), "lclMapShapesById - shape identifier already used " );
            orMap[ rShapeId ] = elem;
        }
    }
}

} // namespace

ShapeContainer::ShapeContainer( Drawing& rDrawing ) :
    mrDrawing( rDrawing )
{
}

ShapeContainer::~ShapeContainer()
{
}

std::shared_ptr<ShapeType> ShapeContainer::createShapeType()
{
    std::shared_ptr< ShapeType > xShape( new ShapeType( mrDrawing ) );
    maTypes.push_back( xShape );
    return xShape;
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

const ShapeType* ShapeContainer::getShapeTypeById( const OUString& rShapeId ) const
{
    // search in own shape template list
    if( const ShapeType* pType = maTypesById.get( rShapeId ).get() )
        return pType;
    // search deep in child shapes
    for (auto const& shape : maShapes)
        if( const ShapeType* pType = shape->getChildTypeById( rShapeId ) )
            return pType;
    return nullptr;
}

const ShapeBase* ShapeContainer::getShapeById( const OUString& rShapeId ) const
{
    // search in own shape list
    if( const ShapeBase* pShape = maShapesById.get( rShapeId ).get() )
        return pShape;
    // search deep in child shapes
    for (auto const& shape : maShapes)
        if( const ShapeBase* pShape = shape->getChildById( rShapeId ) )
            return pShape;
    return nullptr;
}

std::shared_ptr< ShapeBase > ShapeContainer::takeLastShape()
{
    OSL_ENSURE( mrDrawing.getType() == VMLDRAWING_WORD, "ShapeContainer::takeLastShape - illegal call, Word filter only" );
    assert( !markStack.empty());
    if( markStack.top() >= maShapes.size())
        return std::shared_ptr< ShapeBase >();
    std::shared_ptr< ShapeBase > ret = maShapes.back();
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
    for (auto const& shape : maShapes)
        shape->convertAndInsert( rxShapes, pParentAnchor );
}

} // namespace vml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
