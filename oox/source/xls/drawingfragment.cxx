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

#include "oox/xls/drawingfragment.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include "properties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/drawingml/connectorshapecontext.hxx"
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/shapecontext.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"
#include "oox/xls/formulaparser.hxx"
#include "oox/xls/stylesbuffer.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/unitconverter.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::awt::Size;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::form::binding::XBindableValue;
using ::com::sun::star::form::binding::XListEntrySink;
using ::com::sun::star::form::binding::XListEntrySource;
using ::com::sun::star::form::binding::XValueBinding;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::oox::core::ContextHandlerRef;
using ::oox::drawingml::ConnectorShapeContext;
using ::oox::drawingml::EmuPoint;
using ::oox::drawingml::EmuRectangle;
using ::oox::drawingml::EmuSize;
using ::oox::drawingml::GraphicalObjectFrameContext;
using ::oox::drawingml::GraphicShapeContext;
using ::oox::drawingml::Shape;
using ::oox::drawingml::ShapePtr;
using ::oox::drawingml::ShapeContext;
using ::oox::drawingml::ShapeGroupContext;
// no using's for ::oox::vml, that may clash with ::oox::drawingml types

namespace oox {
namespace xls {

// ============================================================================

OoxDrawingFragment::OoxDrawingFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath ),
    mxDrawPage( rHelper.getDrawPage(), UNO_QUERY )
{
    OSL_ENSURE( mxDrawPage.is(), "OoxDrawingFragment::OoxDrawingFragment - missing drawing page" );
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef OoxDrawingFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XDR_TOKEN( wsDr ) ) return this;
        break;

        case XDR_TOKEN( wsDr ):
            switch( nElement )
            {
                case XDR_TOKEN( absoluteAnchor ):
                case XDR_TOKEN( oneCellAnchor ):
                case XDR_TOKEN( twoCellAnchor ):
                    mxAnchor.reset( new ShapeAnchor( *this ) );
                    mxAnchor->importAnchor( nElement, rAttribs );
                    return this;
            }
        break;

        case XDR_TOKEN( absoluteAnchor ):
        case XDR_TOKEN( oneCellAnchor ):
        case XDR_TOKEN( twoCellAnchor ):
            switch( nElement )
            {
                case XDR_TOKEN( sp ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.CustomShape" ) );
                    return new ShapeContext( *this, ShapePtr(), mxShape );
                case XDR_TOKEN( cxnSp ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.ConnectorShape" ) );
                    return new ConnectorShapeContext( *this, ShapePtr(), mxShape );
                case XDR_TOKEN( pic ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.GraphicObjectShape" ) );
                    return new GraphicShapeContext( *this, ShapePtr(), mxShape );
                case XDR_TOKEN( graphicFrame ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.OLE2Shape" ) );
                    return new GraphicalObjectFrameContext( *this, ShapePtr(), mxShape, getSheetType() != SHEETTYPE_CHARTSHEET );
                case XDR_TOKEN( grpSp ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.GroupShape" ) );
                    return new ShapeGroupContext( *this, ShapePtr(), mxShape );

                case XDR_TOKEN( from ):
                case XDR_TOKEN( to ):           return this;

                case XDR_TOKEN( pos ):          if( mxAnchor.get() ) mxAnchor->importPos( rAttribs );           break;
                case XDR_TOKEN( ext ):          if( mxAnchor.get() ) mxAnchor->importExt( rAttribs );           break;
                case XDR_TOKEN( clientData ):   if( mxAnchor.get() ) mxAnchor->importClientData( rAttribs );    break;
            }
        break;

        case XDR_TOKEN( from ):
        case XDR_TOKEN( to ):
            switch( nElement )
            {
                case XDR_TOKEN( col ):
                case XDR_TOKEN( row ):
                case XDR_TOKEN( colOff ):
                case XDR_TOKEN( rowOff ):       return this;    // collect index in onEndElement()
            }
        break;
    }
    return 0;
}

void OoxDrawingFragment::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XDR_TOKEN( col ):
        case XDR_TOKEN( row ):
        case XDR_TOKEN( colOff ):
        case XDR_TOKEN( rowOff ):
            if( mxAnchor.get() ) mxAnchor->setCellPos( getCurrentElement(), getPreviousElement(), rChars );
        break;
        case XDR_TOKEN( absoluteAnchor ):
        case XDR_TOKEN( oneCellAnchor ):
        case XDR_TOKEN( twoCellAnchor ):
            if( mxDrawPage.is() && mxShape.get() && mxAnchor.get() )
            {
                EmuRectangle aShapeRectEmu = mxAnchor->calcAnchorRectEmu( getDrawPageSize() );
                if( (aShapeRectEmu.X >= 0) && (aShapeRectEmu.Y >= 0) && (aShapeRectEmu.Width >= 0) && (aShapeRectEmu.Height >= 0) )
                {
                    // TODO: DrawingML implementation expects 32-bit coordinates for EMU rectangles (change that to EmuRectangle)
                    Rectangle aShapeRectEmu32(
                        getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.X, 0, SAL_MAX_INT32 ),
                        getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Y, 0, SAL_MAX_INT32 ),
                        getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Width, 0, SAL_MAX_INT32 ),
                        getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Height, 0, SAL_MAX_INT32 ) );
                    mxShape->addShape( getOoxFilter(), &getTheme(), mxDrawPage, &aShapeRectEmu32 );
                    // collect all shape positions in the WorksheetHelper base class
                    extendShapeBoundingBox( aShapeRectEmu32 );
                }
            }
            mxShape.reset();
            mxAnchor.reset();
        break;
    }
}

// ============================================================================

namespace {

class VmlFindNoteFunc
{
public:
    explicit            VmlFindNoteFunc( const CellAddress& rPos );
    bool                operator()( const ::oox::vml::ShapeBase& rShape ) const;

private:
    sal_Int32           mnCol;
    sal_Int32           mnRow;
};

VmlFindNoteFunc::VmlFindNoteFunc( const CellAddress& rPos ) :
    mnCol( rPos.Column ),
    mnRow( rPos.Row )
{
}

bool VmlFindNoteFunc::operator()( const ::oox::vml::ShapeBase& rShape ) const
{
    const ::oox::vml::ShapeModel::ShapeClientDataPtr& rxClientData = rShape.getShapeModel().mxClientData;
    return rxClientData.get() && (rxClientData->mnCol == mnCol) && (rxClientData->mnRow == mnRow);
}

} // namespace

// ----------------------------------------------------------------------------

VmlDrawing::VmlDrawing( const WorksheetHelper& rHelper ) :
    ::oox::vml::Drawing( rHelper.getOoxFilter(), rHelper.getDrawPage(), ::oox::vml::VMLDRAWING_EXCEL ),
    WorksheetHelper( rHelper )
{
}

const ::oox::vml::ShapeBase* VmlDrawing::getNoteShape( const CellAddress& rPos ) const
{
    return getShapes().findShape( VmlFindNoteFunc( rPos ) );
}

bool VmlDrawing::isShapeSupported( const ::oox::vml::ShapeBase& rShape ) const
{
    const ::oox::vml::ShapeModel::ShapeClientDataPtr& rxClientData = rShape.getShapeModel().mxClientData;
    return !rxClientData.get() || (rxClientData->mnObjType != XML_Note);
}

bool VmlDrawing::convertShapeClientAnchor( Rectangle& orShapeRect, const OUString& rShapeAnchor ) const
{
    if( rShapeAnchor.getLength() == 0 )
        return false;
    ShapeAnchor aAnchor( *this );
    aAnchor.importVmlAnchor( rShapeAnchor );
    orShapeRect = aAnchor.calcAnchorRectHmm( getDrawPageSize() );
    return (orShapeRect.Width >= 0) && (orShapeRect.Height >= 0);
}

void VmlDrawing::convertControlClientData( const Reference< XControlModel >& rxCtrlModel,
        const ::oox::vml::ShapeClientData& rClientData ) const
{
    if( rxCtrlModel.is() )
    {
        PropertySet aPropSet( rxCtrlModel );

        // printable
        aPropSet.setProperty( PROP_Printable, rClientData.mbPrintObject );

        // linked cell
        if( rClientData.maLinkedCell.getLength() > 0 ) try
        {
            Reference< XBindableValue > xBindable( rxCtrlModel, UNO_QUERY_THROW );

            // convert formula string to cell address
            FormulaParser& rParser = getFormulaParser();
            TokensFormulaContext aContext( true, false );
            aContext.setBaseAddress( CellAddress( getSheetIndex(), 0, 0 ) );
            rParser.importFormula( aContext, rClientData.maLinkedCell );
            CellAddress aAddress;
            if( rParser.extractCellAddress( aAddress, aContext.getTokens(), true ) )
            {
                // create argument sequence for createInstanceWithArguments()
                NamedValue aValue;
                aValue.Name = CREATE_OUSTRING( "BoundCell" );
                aValue.Value <<= aAddress;
                Sequence< Any > aArgs( 1 );
                aArgs[ 0 ] <<= aValue;

                // create the CellValueBinding instance and set at the control model
                Reference< XValueBinding > xBinding( getDocumentFactory()->createInstanceWithArguments(
                    CREATE_OUSTRING( "com.sun.star.table.CellValueBinding" ), aArgs ), UNO_QUERY_THROW );
                xBindable->setValueBinding( xBinding );
            }
        }
        catch( Exception& )
        {
        }

        // source range
        if( rClientData.maSourceRange.getLength() > 0 ) try
        {
            Reference< XListEntrySink > xEntrySink( rxCtrlModel, UNO_QUERY_THROW );

            // convert formula string to cell range
            FormulaParser& rParser = getFormulaParser();
            TokensFormulaContext aContext( true, false );
            aContext.setBaseAddress( CellAddress( getSheetIndex(), 0, 0 ) );
            rParser.importFormula( aContext, rClientData.maSourceRange );
            CellRangeAddress aRange;
            if( rParser.extractCellRange( aRange, aContext.getTokens(), true ) )
            {
                // create argument sequence for createInstanceWithArguments()
                NamedValue aValue;
                aValue.Name = CREATE_OUSTRING( "CellRange" );
                aValue.Value <<= aRange;
                Sequence< Any > aArgs( 1 );
                aArgs[ 0 ] <<= aValue;

                // create the EntrySource instance and set at the control model
                Reference< XListEntrySource > xEntrySource( getDocumentFactory()->createInstanceWithArguments(
                    CREATE_OUSTRING( "com.sun.star.table.CellRangeListSource"  ), aArgs ), UNO_QUERY_THROW );
                xEntrySink->setListEntrySource( xEntrySource );
            }
        }
        catch( Exception& )
        {
        }
    }
}

void VmlDrawing::notifyShapeInserted( const Reference< XShape >& /*rxShape*/, const Rectangle& rShapeRect )
{
    // collect all shape positions in the WorksheetHelper base class
    extendShapeBoundingBox( rShapeRect );
}

// ============================================================================

OoxVmlDrawingFragment::OoxVmlDrawingFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    ::oox::vml::DrawingFragment( rHelper.getOoxFilter(), rFragmentPath, rHelper.getVmlDrawing() ),
    WorksheetHelper( rHelper )
{
}

void OoxVmlDrawingFragment::finalizeImport()
{
    ::oox::vml::DrawingFragment::finalizeImport();
    getVmlDrawing().convertAndInsert();
}

// ============================================================================

} // namespace xls
} // namespace oox

