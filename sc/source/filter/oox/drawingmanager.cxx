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

#include "drawingmanager.hxx"

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PolygonKind.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <oox/core/filterbase.hxx>
#include <oox/drawingml/fillproperties.hxx>
#include <oox/drawingml/lineproperties.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/token/tokens.hxx>
#include "biffinputstream.hxx"
#include "unitconverter.hxx"

namespace oox {
namespace xls {

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::oox::drawingml;

namespace {

// OBJ record -----------------------------------------------------------------

const sal_uInt16 BIFF_OBJTYPE_UNKNOWN       = 0xFFFF;   // for internal use only

// line formatting ------------------------------------------------------------

const sal_uInt8 BIFF_OBJ_LINE_AUTOCOLOR     = 64;

const sal_uInt8 BIFF_OBJ_LINE_SOLID         = 0;
const sal_uInt8 BIFF_OBJ_LINE_DASH          = 1;
const sal_uInt8 BIFF_OBJ_LINE_DOT           = 2;
const sal_uInt8 BIFF_OBJ_LINE_DASHDOT       = 3;
const sal_uInt8 BIFF_OBJ_LINE_DASHDOTDOT    = 4;
const sal_uInt8 BIFF_OBJ_LINE_MEDTRANS      = 5;
const sal_uInt8 BIFF_OBJ_LINE_DARKTRANS     = 6;
const sal_uInt8 BIFF_OBJ_LINE_LIGHTTRANS    = 7;
const sal_uInt8 BIFF_OBJ_LINE_NONE          = 255;

const sal_uInt8 BIFF_OBJ_LINE_HAIR          = 0;
const sal_uInt8 BIFF_OBJ_LINE_THIN          = 1;
const sal_uInt8 BIFF_OBJ_LINE_MEDIUM        = 2;
const sal_uInt8 BIFF_OBJ_LINE_THICK         = 3;

const sal_uInt8 BIFF_OBJ_ARROW_OPEN         = 1;
const sal_uInt8 BIFF_OBJ_ARROW_FILLED       = 2;
const sal_uInt8 BIFF_OBJ_ARROW_OPENBOTH     = 3;
const sal_uInt8 BIFF_OBJ_ARROW_FILLEDBOTH   = 4;

const sal_uInt8 BIFF_OBJ_ARROW_NARROW       = 0;
const sal_uInt8 BIFF_OBJ_ARROW_MEDIUM       = 1;
const sal_uInt8 BIFF_OBJ_ARROW_WIDE         = 2;

// fill formatting ------------------------------------------------------------

const sal_uInt8 BIFF_OBJ_FILL_AUTOCOLOR     = 65;

const sal_uInt8 BIFF_OBJ_PATT_NONE          = 0;
const sal_uInt8 BIFF_OBJ_PATT_SOLID         = 1;

} // namespace

// Model structures for BIFF OBJ record data

BiffObjLineModel::BiffObjLineModel() :
    mnColorIdx( BIFF_OBJ_LINE_AUTOCOLOR ),
    mnStyle( BIFF_OBJ_LINE_SOLID ),
    mnWidth( BIFF_OBJ_LINE_HAIR ),
    mbAuto( true )
{
}

BiffObjFillModel::BiffObjFillModel() :
    mnBackColorIdx( BIFF_OBJ_LINE_AUTOCOLOR ),
    mnPattColorIdx( BIFF_OBJ_FILL_AUTOCOLOR ),
    mnPattern( BIFF_OBJ_PATT_SOLID ),
    mbAuto( true )
{
}

// BIFF drawing page
BiffDrawingBase::BiffDrawingBase( const WorksheetHelper& rHelper, const Reference< XDrawPage >& rxDrawPage ) :
    WorksheetHelper( rHelper ),
    mxDrawPage( rxDrawPage )
{
}

void BiffDrawingBase::finalizeImport()
{
    Reference< XShapes > xShapes( mxDrawPage, UNO_QUERY );
    OSL_ENSURE( xShapes.is(), "BiffDrawingBase::finalizeImport - no shapes container" );
    if( !xShapes.is() )
        return;
}

BiffSheetDrawing::BiffSheetDrawing( const WorksheetHelper& rHelper ) :
    BiffDrawingBase( rHelper, rHelper.getDrawPage() )
{
}

void BiffSheetDrawing::notifyShapeInserted( const Reference< XShape >& /*rxShape*/, const css::awt::Rectangle& rShapeRect )
{
    // collect all shape positions in the WorksheetHelper base class
    extendShapeBoundingBox( rShapeRect );
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
