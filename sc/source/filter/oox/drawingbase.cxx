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

#include "drawingbase.hxx"

#include <com/sun/star/awt/Rectangle.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "unitconverter.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/token/properties.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::table;
using namespace ::oox::drawingml;


// ============================================================================

namespace {

/** Converts the passed 32-bit integer value from 1/100 mm to EMUs. */
inline sal_Int64 lclHmmToEmu( sal_Int32 nValue )
{
    return (nValue < 0) ? -1 : convertHmmToEmu( nValue );
}

/** Converts the passed 64-bit integer value from EMUs to 1/100 mm. */
inline sal_Int32 lclEmuToHmm( sal_Int64 nValue )
{
    return (nValue < 0) ? -1 : convertEmuToHmm( nValue );
}

} // namespace

// ============================================================================

CellAnchorModel::CellAnchorModel() :
    mnCol( -1 ),
    mnRow( -1 ),
    mnColOffset( 0 ),
    mnRowOffset( 0 )
{
}

// ----------------------------------------------------------------------------

AnchorClientDataModel::AnchorClientDataModel() :
    mbLocksWithSheet( true ),
    mbPrintsWithSheet( true )
{
}

// ============================================================================

ShapeAnchor::ShapeAnchor( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper ),
    meAnchorType( ANCHOR_INVALID ),
    meCellAnchorType( CELLANCHOR_EMU ),
    meEditAs( ANCHOR_TWOCELL )
{
}

void ShapeAnchor::importAnchor( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case XDR_TOKEN( absoluteAnchor ):
            meAnchorType = ANCHOR_ABSOLUTE;
        break;
        case XDR_TOKEN( oneCellAnchor ):
            meAnchorType = ANCHOR_ONECELL;
        break;
        case XDR_TOKEN( twoCellAnchor ):
        {
            meAnchorType = ANCHOR_TWOCELL;
            OUString sEditAs = rAttribs.getXString( XML_editAs, OUString() );
            if ( !sEditAs.isEmpty() )
            {
                if ( sEditAs.equalsIgnoreAsciiCase("absolute" ) )
                    meEditAs = ANCHOR_ABSOLUTE;
                else if ( sEditAs.equalsIgnoreAsciiCase("oneCell") )
                    meEditAs = ANCHOR_ONECELL;
            }
        }
        break;
        default:
            OSL_ENSURE( false, "ShapeAnchor::importAnchor - unexpected element" );
    }
    meCellAnchorType = CELLANCHOR_EMU;
}

void ShapeAnchor::importPos( const AttributeList& rAttribs )
{
    OSL_ENSURE( meAnchorType == ANCHOR_ABSOLUTE, "ShapeAnchor::importPos - unexpected 'xdr:pos' element" );
    maPos.X = rAttribs.getHyper( XML_x, 0 );
    maPos.Y = rAttribs.getHyper( XML_y, 0 );
}

void ShapeAnchor::importExt( const AttributeList& rAttribs )
{
    OSL_ENSURE( (meAnchorType == ANCHOR_ABSOLUTE) || (meAnchorType == ANCHOR_ONECELL), "ShapeAnchor::importExt - unexpected 'xdr:ext' element" );
    maSize.Width = rAttribs.getHyper( XML_cx, 0 );
    maSize.Height = rAttribs.getHyper( XML_cy, 0 );
}

void ShapeAnchor::importClientData( const AttributeList& rAttribs )
{
    maClientData.mbLocksWithSheet  = rAttribs.getBool( XML_fLocksWithSheet, true );
    maClientData.mbPrintsWithSheet = rAttribs.getBool( XML_fPrintsWithSheet, true );
}

void ShapeAnchor::setCellPos( sal_Int32 nElement, sal_Int32 nParentContext, const OUString& rValue )
{
    CellAnchorModel* pCellAnchor = 0;
    switch( nParentContext )
    {
        case XDR_TOKEN( from ):
            OSL_ENSURE( (meAnchorType == ANCHOR_ONECELL) || (meAnchorType == ANCHOR_TWOCELL), "ShapeAnchor::setCellPos - unexpected 'xdr:from' element" );
            pCellAnchor = &maFrom;
        break;
        case XDR_TOKEN( to ):
            OSL_ENSURE( meAnchorType == ANCHOR_TWOCELL, "ShapeAnchor::setCellPos - unexpected 'xdr:to' element" );
            pCellAnchor = &maTo;
        break;
        default:
            OSL_ENSURE( false, "ShapeAnchor::setCellPos - unexpected parent element" );
    }
    if( pCellAnchor ) switch( nElement )
    {
        case XDR_TOKEN( col ):      pCellAnchor->mnCol = rValue.toInt32();          break;
        case XDR_TOKEN( row ):      pCellAnchor->mnRow = rValue.toInt32();          break;
        case XDR_TOKEN( colOff ):   pCellAnchor->mnColOffset = rValue.toInt64();    break;
        case XDR_TOKEN( rowOff ):   pCellAnchor->mnRowOffset = rValue.toInt64();    break;
        default:    OSL_ENSURE( false, "ShapeAnchor::setCellPos - unexpected element" );
    }
}

void ShapeAnchor::importVmlAnchor( const OUString& rAnchor )
{
    meAnchorType = ANCHOR_VML;
    meCellAnchorType = CELLANCHOR_PIXEL;

    ::std::vector< OUString > aTokens;
    sal_Int32 nIndex = 0;
    while( nIndex >= 0 )
        aTokens.push_back( rAnchor.getToken( 0, ',', nIndex ).trim() );

    OSL_ENSURE( aTokens.size() >= 8, "ShapeAnchor::importVmlAnchor - missing anchor tokens" );
    if( aTokens.size() >= 8 )
    {
        maFrom.mnCol       = aTokens[ 0 ].toInt32();
        maFrom.mnColOffset = aTokens[ 1 ].toInt32();
        maFrom.mnRow       = aTokens[ 2 ].toInt32();
        maFrom.mnRowOffset = aTokens[ 3 ].toInt32();
        maTo.mnCol         = aTokens[ 4 ].toInt32();
        maTo.mnColOffset   = aTokens[ 5 ].toInt32();
        maTo.mnRow         = aTokens[ 6 ].toInt32();
        maTo.mnRowOffset   = aTokens[ 7 ].toInt32();
    }
}

EmuRectangle ShapeAnchor::calcAnchorRectEmu( const css::awt::Size& rPageSizeHmm ) const
{
    AddressConverter& rAddrConv = getAddressConverter();
    EmuSize aPageSize( lclHmmToEmu( rPageSizeHmm.Width ), lclHmmToEmu( rPageSizeHmm.Height ) );
    EmuRectangle aAnchorRect( -1, -1, -1, -1 );

    // calculate shape position
    switch( meAnchorType )
    {
        case ANCHOR_ABSOLUTE:
            OSL_ENSURE( maPos.isValid(), "ShapeAnchor::calcAnchorRectEmu - invalid position" );
            if( maPos.isValid() && (maPos.X < aPageSize.Width) && (maPos.Y < aPageSize.Height) )
                aAnchorRect.setPos( maPos );
        break;
        case ANCHOR_ONECELL:
        case ANCHOR_TWOCELL:
        case ANCHOR_VML:
            OSL_ENSURE( maFrom.isValid(), "ShapeAnchor::calcAnchorRectEmu - invalid position" );
            if( maFrom.isValid() && rAddrConv.checkCol( maFrom.mnCol, true ) && rAddrConv.checkRow( maFrom.mnRow, true ) )
            {
                EmuPoint aPoint = calcCellAnchorEmu( maFrom );
                if( (aPoint.X < aPageSize.Width) && (aPoint.Y < aPageSize.Height) )
                    aAnchorRect.setPos( aPoint );
            }
        break;
        case ANCHOR_INVALID:
            OSL_ENSURE( false, "ShapeAnchor::calcAnchorRectEmu - invalid anchor" );
        break;
    }

    // calculate shape size
    if( (aAnchorRect.X >= 0) && (aAnchorRect.Y >= 0) ) switch( meAnchorType )
    {
        case ANCHOR_ABSOLUTE:
        case ANCHOR_ONECELL:
            OSL_ENSURE( maSize.isValid(), "ShapeAnchor::calcAnchorRectEmu - invalid size" );
            if( maSize.isValid() )
            {
                aAnchorRect.Width = ::std::min< sal_Int64 >( maSize.Width, aPageSize.Width - aAnchorRect.X );
                aAnchorRect.Height = ::std::min< sal_Int64 >( maSize.Height, aPageSize.Height - aAnchorRect.Y );
            }
        break;
        case ANCHOR_TWOCELL:
        case ANCHOR_VML:
            OSL_ENSURE( maTo.isValid(), "ShapeAnchor::calcAnchorRectEmu - invalid position" );
            if( maTo.isValid() )
            {
                /*  Pass a valid cell address to calcCellAnchorEmu(), otherwise
                    nothing useful is returned, even if either row or column is valid. */
                CellAddress aToCell = rAddrConv.createValidCellAddress( BinAddress( maTo.mnCol, maTo.mnRow ), getSheetIndex(), true );
                CellAnchorModel aValidTo = maTo;
                aValidTo.mnCol = aToCell.Column;
                aValidTo.mnRow = aToCell.Row;
                EmuPoint aPoint = calcCellAnchorEmu( aValidTo );
                // width (if column index is valid, use the calculated offset, otherwise stretch to maximum available X position)
                aAnchorRect.Width = aPageSize.Width - aAnchorRect.X;
                if( aToCell.Column == maTo.mnCol )
                    aAnchorRect.Width = ::std::min< sal_Int64 >( aPoint.X - aAnchorRect.X + 1, aAnchorRect.Width );
                // height (if row index is valid, use the calculated offset, otherwise stretch to maximum available Y position)
                aAnchorRect.Height = aPageSize.Height - aAnchorRect.Y;
                if( aToCell.Row == maTo.mnRow )
                    aAnchorRect.Height = ::std::min< sal_Int64 >( aPoint.Y - aAnchorRect.Y + 1, aAnchorRect.Height );
            }
        break;
        case ANCHOR_INVALID:
        break;
    }

    // add 0.75 mm (27,000 EMUs) in X direction to correct display error
    if( aAnchorRect.X >= 0 )
        aAnchorRect.X += 27000;
    // remove 0.25 mm (9,000 EMUs) in Y direction to correct display error
    if( aAnchorRect.Y >= 9000 )
        aAnchorRect.Y -= 9000;

    return aAnchorRect;
}

css::awt::Rectangle ShapeAnchor::calcAnchorRectHmm( const css::awt::Size& rPageSizeHmm ) const
{
    EmuRectangle aAnchorRect = calcAnchorRectEmu( rPageSizeHmm );
    return css::awt::Rectangle( lclEmuToHmm( aAnchorRect.X ), lclEmuToHmm( aAnchorRect.Y ), lclEmuToHmm( aAnchorRect.Width ), lclEmuToHmm( aAnchorRect.Height ) );
}

EmuPoint ShapeAnchor::calcCellAnchorEmu( const CellAnchorModel& rModel ) const
{
    // calculate position of top-left edge of the cell
    css::awt::Point aPoint = getCellPosition( rModel.mnCol, rModel.mnRow );
    EmuPoint aEmuPoint( lclHmmToEmu( aPoint.X ), lclHmmToEmu( aPoint.Y ) );

    // add the offset inside the cell
    switch( meCellAnchorType )
    {
        case CELLANCHOR_EMU:
            aEmuPoint.X += rModel.mnColOffset;
            aEmuPoint.Y += rModel.mnRowOffset;
        break;

        case CELLANCHOR_PIXEL:
        {
            const UnitConverter& rUnitConv = getUnitConverter();
            aEmuPoint.X += static_cast< sal_Int64 >( rUnitConv.scaleValue( static_cast< double >( rModel.mnColOffset ), UNIT_SCREENX, UNIT_EMU ) );
            aEmuPoint.Y += static_cast< sal_Int64 >( rUnitConv.scaleValue( static_cast< double >( rModel.mnRowOffset ), UNIT_SCREENY, UNIT_EMU ) );
        }
        break;

        case CELLANCHOR_COLROW:
        {
            css::awt::Size aCellSize = getCellSize( rModel.mnCol, rModel.mnRow );
            EmuSize aEmuSize( lclHmmToEmu( aCellSize.Width ), lclHmmToEmu( aCellSize.Height ) );
            // X offset is given in 1/1024 of column width
            aEmuPoint.X += static_cast< sal_Int64 >( aEmuSize.Width * getLimitedValue< double >( static_cast< double >( rModel.mnColOffset ) / 1024.0, 0.0, 1.0 ) + 0.5 );
            // Y offset is given in 1/256 of row height
            aEmuPoint.Y += static_cast< sal_Int64 >( aEmuSize.Height * getLimitedValue< double >( static_cast< double >( rModel.mnRowOffset ) / 256.0, 0.0, 1.0 ) + 0.5 );
        }
        break;
    }

    return aEmuPoint;
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
