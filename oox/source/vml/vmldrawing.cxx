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
#include <com/sun/star/drawing/XShapes.hpp>
#include "tokens.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/ole/axcontrol.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XShapes;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace vml {

// ============================================================================

namespace {

/** Returns the textual representation of a numeric VML shape identifier. */
OUString lclGetShapeId( sal_Int32 nShapeId )
{
    // identifier consists of a literal NUL character, a lowercase 's', and the id
    return CREATE_OUSTRING( "\0s" ) + OUString::valueOf( nShapeId );
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
            mrFilter.getModelFactory(), mxDrawPage, mrFilter.getGraphicHelper() ) );
    return *mxCtrlForm;
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

const OleObjectInfo* Drawing::getOleObjectInfo( const OUString& rShapeId ) const
{
    return ContainerHelper::getMapElement( maOleObjects, rShapeId );
}

const ControlInfo* Drawing::getControlInfo( const OUString& rShapeId ) const
{
    return ContainerHelper::getMapElement( maControls, rShapeId );
}

bool Drawing::isShapeSupported( const ShapeBase& /*rShape*/ ) const
{
    return true;
}

bool Drawing::convertShapeClientAnchor( Rectangle& /*orShapeRect*/, const OUString& /*rShapeAnchor*/ ) const
{
    return false;
}

void Drawing::convertControlClientData( const Reference< XControlModel >& /*rxCtrlModel*/, const ShapeClientData& /*rClientData*/ ) const
{
}

void Drawing::notifyShapeInserted( const Reference< XShape >& /*rxShape*/, const Rectangle& /*rShapeRect*/ )
{
}

// ============================================================================

} // namespace vml
} // namespave oox

