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

#include <drawingml/chart/chartspacefragment.hxx>

#include <drawingml/shapepropertiescontext.hxx>
#include <drawingml/textbodycontext.hxx>
#include <drawingml/chart/chartspacemodel.hxx>
#include <drawingml/chart/plotareacontext.hxx>
#include <drawingml/chart/titlecontext.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>

namespace oox::drawingml::chart {

using namespace ::oox::core;

ChartSpaceFragment::ChartSpaceFragment( XmlFilterBase& rFilter, const OUString& rFragmentPath, ChartSpaceModel& rModel ) :
    FragmentBase< ChartSpaceModel >( rFilter, rFragmentPath, rModel )
{
}

ChartSpaceFragment::~ChartSpaceFragment()
{
}

ContextHandlerRef ChartSpaceFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Document = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            switch( nElement )
            {
                case C_TOKEN( chartSpace ):
                    return this;
                case CX_TOKEN(chartSpace) :
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
                    return nullptr;
                case C_TOKEN( txPr ):
                    return new TextBodyContext( *this, mrModel.mxTextProp.create() );
                case C_TOKEN( userShapes ):
                    mrModel.maDrawingPath = getFragmentPathFromRelId( rAttribs.getStringDefaulted( R_TOKEN( id )) );
                    return nullptr;
                case C_TOKEN( pivotSource ):
                    mrModel.mbPivotChart = true;
                    return nullptr;
                case C_TOKEN (externalData):
                    mrModel.maSheetPath = getFragmentPathFromRelId(rAttribs.getStringDefaulted(R_TOKEN(id)));
                    return nullptr;
                case C_TOKEN(clrMapOvr):
                    if (mrModel.mpClrMap)
                        for (auto nClrToken : {
                                 XML_bg1,
                                 XML_tx1,
                                 XML_bg2,
                                 XML_tx2,
                                 XML_accent1,
                                 XML_accent2,
                                 XML_accent3,
                                 XML_accent4,
                                 XML_accent5,
                                 XML_accent6,
                                 XML_hlink,
                                 XML_folHlink,
                             })
                            if (auto oMappedToken = rAttribs.getToken(nClrToken))
                                mrModel.mpClrMap->setColorMap(nClrToken, *oMappedToken);
                    return nullptr;
            }
        break;

        case C_TOKEN( chart ):
            switch( nElement )
            {
                case C_TOKEN( autoTitleDeleted ):
                {

                    mrModel.mbAutoTitleDel = rAttribs.getBool( XML_val, !bMSO2007Document );
                    return nullptr;
                }
                case C_TOKEN( backWall ):
                    return new WallFloorContext( *this, mrModel.mxBackWall.create() );
                case C_TOKEN( dispBlanksAs ):
                {
                    // default value is XML_gap for MSO 2007 and XML_zero in OOXML
                    mrModel.mnDispBlanksAs = rAttribs.getToken( XML_val, bMSO2007Document ? XML_gap : XML_zero );
                    return nullptr;
                }
                case C_TOKEN( floor ):
                    return new WallFloorContext( *this, mrModel.mxFloor.create() );
                case C_TOKEN( legend ):
                    return new LegendContext( *this, mrModel.mxLegend.create() );
                case C_TOKEN( plotArea ):
                    return new PlotAreaContext( *this, mrModel.mxPlotArea.create() );
                case C_TOKEN( plotVisOnly ):
                    mrModel.mbPlotVisOnly = rAttribs.getBool( XML_val, !bMSO2007Document );
                    return nullptr;
                case C_TOKEN( showDLblsOverMax ):
                    mrModel.mbShowLabelsOverMax = rAttribs.getBool( XML_val, !bMSO2007Document );
                    return nullptr;
                case C_TOKEN( sideWall ):
                    return new WallFloorContext( *this, mrModel.mxSideWall.create() );
                case C_TOKEN( title ):
                    return new TitleContext( *this, mrModel.mxTitle.create() );
                case C_TOKEN( view3D ):
                    return new View3DContext( *this, mrModel.mxView3D.create(bMSO2007Document) );
            }
        break;

        // chartex handling
        case CX_TOKEN(chartSpace) :
            switch (nElement) {
                case CX_TOKEN(chartData):
                    // TODO
                    return nullptr;
                case CX_TOKEN(chart):
                    return this;
                case CX_TOKEN(spPr):
                    return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
                case CX_TOKEN(txPr):
                    return new TextBodyContext( *this, mrModel.mxTextProp.create() );
                case CX_TOKEN(clrMapOvr):
                    // TODO
                    return nullptr;
                case CX_TOKEN(fmtOvrs):
                    // TODO
                    return nullptr;
                case CX_TOKEN(printSettings):
                    // TODO
                    return nullptr;
                case CX_TOKEN(extLst):
                    // TODO
                    return nullptr;
                default:
                    // shouldn't happen
                    assert(false);

            }
            break;
        case CX_TOKEN(chart) :
            switch (nElement) {
                case CX_TOKEN(title):
                    return new TitleContext( *this, mrModel.mxTitle.create() );
                case CX_TOKEN(plotArea):
                    return new PlotAreaContext( *this, mrModel.mxPlotArea.create() );
                case CX_TOKEN(legend):
                    return new LegendContext( *this, mrModel.mxLegend.create() );
                case CX_TOKEN(extLst):
                    // TODO
                    return nullptr;
                default:
                    // shouldn't happen
                    assert(false);
            }
            break;
    }
    return nullptr;
}

} // namespace oox::drawingml::chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
