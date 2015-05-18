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
#include <osl/diagnose.h>
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

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
