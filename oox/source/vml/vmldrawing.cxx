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

#include "oox/vml/vmldrawing.hxx"

#include <algorithm>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <rtl/ustring.hxx>
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/ole/axcontrol.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"

namespace oox {
namespace vml {

// ============================================================================

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;

using ::oox::core::XmlFilterBase;

// ============================================================================

namespace {

/** Returns the textual representation of a numeric VML shape identifier. */
OUString lclGetShapeId( sal_Int32 nShapeId )
{
    // identifier consists of a literal NUL character, a lowercase 's', and the id
    sal_Unicode aStr[2] = { '\0', 's' };
    return OUString( aStr, 2 ) + OUString::number( nShapeId );
}

/** Returns the numeric VML shape identifier from its textual representation. */
sal_Int32 lclGetShapeId( const OUString& rShapeId )
{
    // identifier consists of a literal NUL character, a lowercase 's', and the id
    return ((rShapeId.getLength() >= 3) && (rShapeId[ 0 ] == '\0') && (rShapeId[ 1 ] == 's')) ? rShapeId.copy( 2 ).toInt32() : -1;
}

} // namespace

// ============================================================================

OleObjectInfo::OleObjectInfo( bool bDmlShape ) :
    mbAutoLoad( false ),
    mbDmlShape( bDmlShape )
{
}

void OleObjectInfo::setShapeId( sal_Int32 nShapeId )
{
    maShapeId = lclGetShapeId( nShapeId );
}

// ============================================================================

ControlInfo::ControlInfo()
{
}

void ControlInfo::setShapeId( sal_Int32 nShapeId )
{
    maShapeId = lclGetShapeId( nShapeId );
}

// ============================================================================

Drawing::Drawing( XmlFilterBase& rFilter, const Reference< XDrawPage >& rxDrawPage, DrawingType eType ) :
    mrFilter( rFilter ),
    mxDrawPage( rxDrawPage ),
    mxShapes( new ShapeContainer( *this ) ),
    meType( eType )
{
    OSL_ENSURE( mxDrawPage.is(), "Drawing::Drawing - missing UNO draw page" );
}

Drawing::~Drawing()
{
}

::oox::ole::EmbeddedForm& Drawing::getControlForm() const
{
    if( !mxCtrlForm.get() )
        mxCtrlForm.reset( new ::oox::ole::EmbeddedForm(
            mrFilter.getModel(), mxDrawPage, mrFilter.getGraphicHelper() ) );
    return *mxCtrlForm;
}

void Drawing::registerBlockId( sal_Int32 nBlockId )
{
    OSL_ENSURE( nBlockId > 0, "Drawing::registerBlockId - invalid block index" );
    if( nBlockId > 0 )
    {
        // lower_bound() returns iterator pointing to element equal to nBlockId, if existing
        BlockIdVector::iterator aIt = ::std::lower_bound( maBlockIds.begin(), maBlockIds.end(), nBlockId );
        if( (aIt == maBlockIds.end()) || (nBlockId != *aIt) )
            maBlockIds.insert( aIt, nBlockId );
    }
}

void Drawing::registerOleObject( const OleObjectInfo& rOleObject )
{
    OSL_ENSURE( !rOleObject.maShapeId.isEmpty(), "Drawing::registerOleObject - missing OLE object shape id" );
    OSL_ENSURE( maOleObjects.count( rOleObject.maShapeId ) == 0, "Drawing::registerOleObject - OLE object already registered" );
    maOleObjects.insert( OleObjectInfoMap::value_type( rOleObject.maShapeId, rOleObject ) );
}

void Drawing::registerControl( const ControlInfo& rControl )
{
    OSL_ENSURE( !rControl.maShapeId.isEmpty(), "Drawing::registerControl - missing form control shape id" );
    OSL_ENSURE( !rControl.maName.isEmpty(), "Drawing::registerControl - missing form control name" );
    OSL_ENSURE( maControls.count( rControl.maShapeId ) == 0, "Drawing::registerControl - form control already registered" );
    maControls.insert( ControlInfoMap::value_type( rControl.maShapeId, rControl ) );
}

void Drawing::finalizeFragmentImport()
{
    mxShapes->finalizeFragmentImport();
}

void Drawing::convertAndInsert() const
{
    Reference< XShapes > xShapes( mxDrawPage, UNO_QUERY );
    mxShapes->convertAndInsert( xShapes );
}

sal_Int32 Drawing::getLocalShapeIndex( const OUString& rShapeId ) const
{
    sal_Int32 nShapeId = lclGetShapeId( rShapeId );
    if( nShapeId <= 0 ) return -1;

    /*  Shapes in a drawing are counted per registered shape identifier blocks
        as stored in the o:idmap element. The contents of this element have
        been stored in our member maBlockIds. Each block represents 1024 shape
        identifiers, starting with identifier 1 for the block #0. This means,
        block #0 represents the identifiers 1-1024, block #1 represents the
        identifiers 1025-2048, and so on. The local shape index has to be
        calculated according to all blocks registered for this drawing.

        Example:
            Registered for this drawing are blocks #1 and #3 (shape identifiers
            1025-2048 and 3073-4096).
            Shape identifier 1025 -> local shape index 1.
            Shape identifier 1026 -> local shape index 2.
            ...
            Shape identifier 2048 -> local shape index 1024.
            Shape identifier 3073 -> local shape index 1025.
            ...
            Shape identifier 4096 -> local shape index 2048.
     */

    // get block id from shape id and find its index in the list of used blocks
    sal_Int32 nBlockId = (nShapeId - 1) / 1024;
    BlockIdVector::iterator aIt = ::std::lower_bound( maBlockIds.begin(), maBlockIds.end(), nBlockId );
    sal_Int32 nIndex = static_cast< sal_Int32 >( aIt - maBlockIds.begin() );

    // block id not found in set -> register it now (value of nIndex remains valid)
    if( (aIt == maBlockIds.end()) || (*aIt != nBlockId) )
        maBlockIds.insert( aIt, nBlockId );

    // get one-based offset of shape id in its block
    sal_Int32 nBlockOffset = (nShapeId - 1) % 1024 + 1;

    // calculate the local shape index
    return 1024 * nIndex + nBlockOffset;
}

const OleObjectInfo* Drawing::getOleObjectInfo( const OUString& rShapeId ) const
{
    return ContainerHelper::getMapElement( maOleObjects, rShapeId );
}

const ControlInfo* Drawing::getControlInfo( const OUString& rShapeId ) const
{
    return ContainerHelper::getMapElement( maControls, rShapeId );
}

Reference< XShape > Drawing::createAndInsertXShape( const OUString& rService,
        const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect ) const
{
    OSL_ENSURE( !rService.isEmpty(), "Drawing::createAndInsertXShape - missing UNO shape service name" );
    OSL_ENSURE( rxShapes.is(), "Drawing::createAndInsertXShape - missing XShapes container" );
    Reference< XShape > xShape;
    if( !rService.isEmpty() && rxShapes.is() ) try
    {
        Reference< XMultiServiceFactory > xModelFactory( mrFilter.getModelFactory(), UNO_SET_THROW );
        xShape.set( xModelFactory->createInstance( rService ), UNO_QUERY_THROW );
        if ( !rService.equalsAscii( "com.sun.star.text.TextFrame" ) )
        {
            // insert shape into passed shape collection (maybe drawpage or group shape)
            rxShapes->add( xShape );
            xShape->setPosition( awt::Point( rShapeRect.X, rShapeRect.Y ) );
        }
        else
        {
            Reference< XPropertySet > xPropSet( xShape, UNO_QUERY_THROW );
            xPropSet->setPropertyValue( "HoriOrient", makeAny( HoriOrientation::NONE ) );
            xPropSet->setPropertyValue( "VertOrient", makeAny( VertOrientation::NONE ) );
            xPropSet->setPropertyValue( "HoriOrientPosition", makeAny( rShapeRect.X ) );
            xPropSet->setPropertyValue( "VertOrientPosition", makeAny( rShapeRect.Y ) );
            xPropSet->setPropertyValue( "HoriOrientRelation", makeAny( RelOrientation::FRAME ) );
            xPropSet->setPropertyValue( "VertOrientRelation", makeAny( RelOrientation::FRAME ) );
        }
        xShape->setSize( awt::Size( rShapeRect.Width, rShapeRect.Height ) );
    }
    catch( Exception& e )
    {
        SAL_WARN( "oox", "Drawing::createAndInsertXShape - error during shape object creation: " << e.Message );
    }
    OSL_ENSURE( xShape.is(), "Drawing::createAndInsertXShape - cannot instanciate shape object" );
    return xShape;
}

Reference< XShape > Drawing::createAndInsertXControlShape( const ::oox::ole::EmbeddedControl& rControl,
        const Reference< XShapes >& rxShapes, const awt::Rectangle& rShapeRect, sal_Int32& rnCtrlIndex ) const
{
    Reference< XShape > xShape;
    try
    {
        // create control model and insert it into the form of the draw page
        Reference< XControlModel > xCtrlModel( getControlForm().convertAndInsert( rControl, rnCtrlIndex ), UNO_SET_THROW );

        // create the control shape
        xShape = createAndInsertXShape( "com.sun.star.drawing.ControlShape", rxShapes, rShapeRect );

        // set the control model at the shape
        Reference< XControlShape >( xShape, UNO_QUERY_THROW )->setControl( xCtrlModel );
    }
    catch (Exception const& e)
    {
        SAL_WARN("oox", "exception inserting Shape: " << e.Message);
    }
    return xShape;
}

bool Drawing::isShapeSupported( const ShapeBase& /*rShape*/ ) const
{
    return true;
}

OUString Drawing::getShapeBaseName( const ShapeBase& /*rShape*/ ) const
{
    return OUString();
}

bool Drawing::convertClientAnchor( awt::Rectangle& /*orShapeRect*/, const OUString& /*rShapeAnchor*/ ) const
{
    return false;
}

Reference< XShape > Drawing::createAndInsertClientXShape( const ShapeBase& /*rShape*/,
        const Reference< XShapes >& /*rxShapes*/, const awt::Rectangle& /*rShapeRect*/ ) const
{
    return Reference< XShape >();
}

void Drawing::notifyXShapeInserted( const Reference< XShape >& /*rxShape*/,
        const awt::Rectangle& /*rShapeRect*/, const ShapeBase& /*rShape*/, bool /*bGroupChild*/ )
{
}

// ============================================================================

} // namespace vml
} // namespave oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
