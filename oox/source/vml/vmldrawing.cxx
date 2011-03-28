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

#include "oox/vml/vmldrawing.hxx"

#include <algorithm>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/ole/axcontrol.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"

namespace oox {
namespace vml {

// ============================================================================

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::oox::core::XmlFilterBase;
using ::rtl::OUString;

// ============================================================================

namespace {

/** Returns the textual representation of a numeric VML shape identifier. */
OUString lclGetShapeId( sal_Int32 nShapeId )
{
    // identifier consists of a literal NUL character, a lowercase 's', and the id
    return CREATE_OUSTRING( "\0s" ) + OUString::valueOf( nShapeId );
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
    OSL_ENSURE( rOleObject.maShapeId.getLength() > 0, "Drawing::registerOleObject - missing OLE object shape id" );
    OSL_ENSURE( maOleObjects.count( rOleObject.maShapeId ) == 0, "Drawing::registerOleObject - OLE object already registered" );
    maOleObjects.insert( OleObjectInfoMap::value_type( rOleObject.maShapeId, rOleObject ) );
}

void Drawing::registerControl( const ControlInfo& rControl )
{
    OSL_ENSURE( rControl.maShapeId.getLength() > 0, "Drawing::registerControl - missing form control shape id" );
    OSL_ENSURE( rControl.maName.getLength() > 0, "Drawing::registerControl - missing form control name" );
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
        const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    OSL_ENSURE( rService.getLength() > 0, "Drawing::createAndInsertXShape - missing UNO shape service name" );
    OSL_ENSURE( rxShapes.is(), "Drawing::createAndInsertXShape - missing XShapes container" );
    Reference< XShape > xShape;
    if( (rService.getLength() > 0) && rxShapes.is() ) try
    {
        Reference< XMultiServiceFactory > xModelFactory( mrFilter.getModelFactory(), UNO_SET_THROW );
        xShape.set( xModelFactory->createInstance( rService ), UNO_QUERY_THROW );
        // insert shape into passed shape collection (maybe drawpage or group shape)
        rxShapes->add( xShape );
        xShape->setPosition( Point( rShapeRect.X, rShapeRect.Y ) );
        xShape->setSize( Size( rShapeRect.Width, rShapeRect.Height ) );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xShape.is(), "Drawing::createAndInsertXShape - cannot instanciate shape object" );
    return xShape;
}

Reference< XShape > Drawing::createAndInsertXControlShape( const ::oox::ole::EmbeddedControl& rControl,
        const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect, sal_Int32& rnCtrlIndex ) const
{
    Reference< XShape > xShape;
    try
    {
        // create control model and insert it into the form of the draw page
        Reference< XControlModel > xCtrlModel( getControlForm().convertAndInsert( rControl, rnCtrlIndex ), UNO_SET_THROW );

        // create the control shape
        xShape = createAndInsertXShape( CREATE_OUSTRING( "com.sun.star.drawing.ControlShape" ), rxShapes, rShapeRect );

        // set the control model at the shape
        Reference< XControlShape >( xShape, UNO_QUERY_THROW )->setControl( xCtrlModel );
    }
    catch( Exception& )
    {
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

bool Drawing::convertClientAnchor( Rectangle& /*orShapeRect*/, const OUString& /*rShapeAnchor*/ ) const
{
    return false;
}

Reference< XShape > Drawing::createAndInsertClientXShape( const ShapeBase& /*rShape*/,
        const Reference< XShapes >& /*rxShapes*/, const Rectangle& /*rShapeRect*/ ) const
{
    return Reference< XShape >();
}

void Drawing::notifyXShapeInserted( const Reference< XShape >& /*rxShape*/,
        const Rectangle& /*rShapeRect*/, const ShapeBase& /*rShape*/, bool /*bGroupChild*/ )
{
}

// ============================================================================

} // namespace vml
} // namespave oox
