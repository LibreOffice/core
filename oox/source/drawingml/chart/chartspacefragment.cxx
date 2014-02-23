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

#include "oox/drawingml/chart/chartspacefragment.hxx"

#include "oox/drawingml/shapepropertiescontext.hxx"
#include "oox/drawingml/textbodycontext.hxx"
#include "oox/drawingml/chart/chartspacemodel.hxx"
#include "oox/drawingml/chart/plotareacontext.hxx"
#include "oox/drawingml/chart/titlecontext.hxx"

namespace oox {
namespace drawingml {
namespace chart {



using namespace ::oox::core;



ChartSpaceFragment::ChartSpaceFragment( XmlFilterBase& rFilter, const OUString& rFragmentPath, ChartSpaceModel& rModel ) :
    FragmentBase< ChartSpaceModel >( rFilter, rFragmentPath, rModel ),
    mpFilterBase(rFilter)
{
}

ChartSpaceFragment::~ChartSpaceFragment()
{
}

ContextHandlerRef ChartSpaceFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            switch( nElement )
            {
                case C_TOKEN( chartSpace ):
                    return this;
            }
        break;

        case C_TOKEN( chartSpace ):
            switch( nElement )
            {
                case C_TOKEN( chart ):
                    return this;
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
                case C_TOKEN( style ):
                    mrModel.mnStyle = rAttribs.getInteger( XML_val, 2 );
                    return 0;
                case C_TOKEN( txPr ):
                    return new TextBodyContext( *this, mrModel.mxTextProp.create() );
                case C_TOKEN( userShapes ):
                    mrModel.maDrawingPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
                    return 0;
                case C_TOKEN( pivotSource ):
                    mrModel.mbPivotChart = true;
                    return 0;
                case C_TOKEN (externalData):
                    mrModel.maSheetPath = getFragmentPathFromRelId(rAttribs.getString(R_TOKEN(id),OUString()));
                    return 0;
            }
        break;

        case C_TOKEN( chart ):
            switch( nElement )
            {
                case C_TOKEN( autoTitleDeleted ):
                    // default is 'false', not 'true' as specified
                    mrModel.mbAutoTitleDel = rAttribs.getBool( XML_val, false );
                    return 0;
                case C_TOKEN( backWall ):
                    return new WallFloorContext( *this, mrModel.mxBackWall.create() );
                case C_TOKEN( dispBlanksAs ):
                    mrModel.mnDispBlanksAs = rAttribs.getToken( XML_val, XML_zero );
                    return 0;
                case C_TOKEN( floor ):
                    return new WallFloorContext( *this, mrModel.mxFloor.create() );
                case C_TOKEN( legend ):
                    return new LegendContext( *this, mrModel.mxLegend.create() );
                case C_TOKEN( plotArea ):
                    return new PlotAreaContext( *this, mrModel.mxPlotArea.create() );
                case C_TOKEN( plotVisOnly ):
                    // default is 'false', not 'true' as specified
                    mrModel.mbPlotVisOnly = rAttribs.getBool( XML_val, false );
                    return 0;
                case C_TOKEN( showDLblsOverMax ):
                    // default is 'false', not 'true' as specified
                    mrModel.mbShowLabelsOverMax = rAttribs.getBool( XML_val, false );
                    return 0;
                case C_TOKEN( sideWall ):
                    return new WallFloorContext( *this, mrModel.mxSideWall.create() );
                case C_TOKEN( title ):
                    return new TitleContext( *this, mrModel.mxTitle.create() );
                case C_TOKEN( view3D ):
                    return new View3DContext( *this, mrModel.mxView3D.create() );
            }
        break;
    }
    return 0;
}



} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
