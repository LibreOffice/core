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

#include "XMLTableShapeImportHelper.hxx"
#include "xmlimprt.hxx"
#include <drwlayer.hxx>
#include "xmlannoi.hxx"
#include <rangeutl.hxx>
#include <userdat.hxx>
#include <docuno.hxx>
#include <sheetdata.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <svx/unoshape.hxx>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XShapes.hpp>

#define SC_LAYERID "LayerID"

using namespace ::com::sun::star;
using namespace xmloff::token;

XMLTableShapeImportHelper::XMLTableShapeImportHelper( ScXMLImport& rImp ) :
    XMLShapeImportHelper(rImp, rImp.GetModel(), nullptr ),
    pAnnotationContext(nullptr),
    bOnTable(false)
{
}

XMLTableShapeImportHelper::~XMLTableShapeImportHelper()
{
}

void XMLTableShapeImportHelper::SetLayer(const uno::Reference<drawing::XShape>& rShape, SdrLayerID nLayerID, const OUString& sType)
{
    if ( sType == "com.sun.star.drawing.ControlShape" )
        nLayerID = SC_LAYER_CONTROLS;
    if (nLayerID != SDRLAYER_NOTFOUND)
    {
        uno::Reference< beans::XPropertySet > xShapeProp( rShape, uno::UNO_QUERY );
        if( xShapeProp.is() )
            xShapeProp->setPropertyValue( SC_LAYERID, uno::makeAny<sal_uInt16>(sal_uInt8(nLayerID)) );
    }
}

// Attempt to find the topmost parent of the group, this is the one we apply
// offsets to
static uno::Reference< drawing::XShape > lcl_getTopLevelParent( const uno::Reference< drawing::XShape >& rShape )
{
    uno::Reference< container::XChild > xChild( rShape, uno::UNO_QUERY );
    uno::Reference< drawing::XShape > xParent( xChild->getParent(), uno::UNO_QUERY );
    if ( xParent.is() )
        return lcl_getTopLevelParent( xParent );
    return rShape;
}

void XMLTableShapeImportHelper::finishShape(
    uno::Reference< drawing::XShape >& rShape,
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes )
{
    bool bNote = false;
    XMLShapeImportHelper::finishShape( rShape, xAttrList, rShapes );
    static_cast<ScXMLImport&>(mrImporter).LockSolarMutex();
    ScMyTables& rTables = static_cast<ScXMLImport&>(mrImporter).GetTables();
    if (rShapes == rTables.GetCurrentXShapes())
    {
        if (!pAnnotationContext)
        {
            ScDrawObjData aAnchor;
            aAnchor.maStart = aStartCell;
            awt::Point aStartPoint(rShape->getPosition());
            aAnchor.maStartOffset = Point(aStartPoint.X, aStartPoint.Y);
            aAnchor.mbResizeWithCell = false;

            sal_Int32 nEndX(-1);
            sal_Int32 nEndY(-1);
            std::optional<OUString> xRangeList;
            SdrLayerID nLayerID = SDRLAYER_NOTFOUND;
            for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
            {
                const OUString sValue = aIter.toString();

                switch(aIter.getToken())
                {
                    case XML_ELEMENT(TABLE, XML_END_CELL_ADDRESS):
                    {
                        sal_Int32 nOffset(0);
                        ScRangeStringConverter::GetAddressFromString(aAnchor.maEnd, sValue, static_cast<ScXMLImport&>(mrImporter).GetDocument(), ::formula::FormulaGrammar::CONV_OOO, nOffset);
                        // When the cell end address is set, we let the shape resize with the cell
                        aAnchor.mbResizeWithCell = true;
                        break;
                    }
                    case XML_ELEMENT(TABLE, XML_END_X):
                    {
                        static_cast<ScXMLImport&>(mrImporter).
                            GetMM100UnitConverter().convertMeasureToCore(
                                    nEndX, sValue);
                        aAnchor.maEndOffset.setX( nEndX );
                        break;
                    }
                    case XML_ELEMENT(TABLE, XML_END_Y):
                    {
                        static_cast<ScXMLImport&>(mrImporter).
                            GetMM100UnitConverter().convertMeasureToCore(
                                    nEndY, sValue);
                        aAnchor.maEndOffset.setY( nEndY );
                        break;
                    }
                    case XML_ELEMENT(TABLE, XML_TABLE_BACKGROUND):
                        if (IsXMLToken(sValue, XML_TRUE))
                            nLayerID = SC_LAYER_BACK;
                        break;
                    case XML_ELEMENT(DRAW, XML_NOTIFY_ON_UPDATE_OF_RANGES):
                        xRangeList = sValue;
                        break;
                    default:
                        SAL_WARN("sc", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
                }
            }
            SetLayer(rShape, nLayerID, rShape->getShapeType());

            if (SvxShape* pShapeImp = comphelper::getUnoTunnelImplementation<SvxShape>(rShape))
            {
                if (SdrObject *pSdrObj = pShapeImp->GetSdrObject())
                {
                    if (!bOnTable)
                        ScDrawLayer::SetCellAnchored(*pSdrObj, aAnchor);
                    else
                        ScDrawLayer::SetPageAnchored(*pSdrObj);
                }
            }

            if (xRangeList)
            {
                // #i78086# If there are notification ranges, the ChartListener must be created
                // also when anchored to the sheet
                // -> call AddOLE with invalid cell position (checked in ScMyShapeResizer::ResizeShapes)

                if (ScMyTables::IsOLE(rShape))
                    rTables.AddOLE(rShape, *xRangeList);
            }
        }
        else // shape is annotation
        {
            // get the style names for stream copying
            OUString aStyleName;
            OUString aTextStyle;
            for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
            {
                const OUString sValue = aIter.toString();
                switch (aIter.getToken())
                {
                    case XML_ELEMENT(DRAW, XML_STYLE_NAME):
                        aStyleName = sValue;
                        break;
                    case XML_ELEMENT(DRAW, XML_TEXT_STYLE_NAME):
                        aTextStyle = sValue;
                        break;
                    default:
                        SAL_WARN("sc", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
                }
            }

            pAnnotationContext->SetShape(rShape, rShapes, aStyleName, aTextStyle);
            bNote = true;
        }
    }
    else //this are grouped shapes which should also get the layerid
    {
        uno::Reference< drawing::XShapes > xGroup( rShape, uno::UNO_QUERY );
        // ignore the group ( within group ) object if it exists
        if ( !bOnTable && !xGroup.is() )
        {
            // For cell anchored grouped shape we need to set the start
            // position from the most top and left positioned shape(s) within
            // the group
            Point aStartPoint( rShape->getPosition().X,rShape->getPosition().Y );
            uno::Reference< drawing::XShape > xChild( rShapes, uno::UNO_QUERY );
            if (SvxShape* pGroupShapeImp = xChild.is() ? comphelper::getUnoTunnelImplementation<SvxShape>(lcl_getTopLevelParent(xChild)) : nullptr)
            {
                if (SdrObject *pSdrObj = pGroupShapeImp->GetSdrObject())
                {
                    if ( ScDrawObjData* pAnchor = ScDrawLayer::GetObjData( pSdrObj ) )
                    {
                       if ( pAnchor->maStartOffset.getX() == 0 && pAnchor->maStartOffset.getY() == 0 )
                            pAnchor->maStartOffset = aStartPoint;
                       if ( aStartPoint.getX() < pAnchor->maStartOffset.getX() )
                             pAnchor->maStartOffset.setX( aStartPoint.getX() );
                       if ( aStartPoint.getY() < pAnchor->maStartOffset.getY() )
                           pAnchor->maStartOffset.setY( aStartPoint.getY() );
                    }
                }
            }
        }
        SdrLayerID nLayerID = SDRLAYER_NOTFOUND;
        for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
        {
            const OUString sValue = aIter.toString();
            switch (aIter.getToken())
            {
                case XML_ELEMENT(TABLE, XML_TABLE_BACKGROUND):
                    if (IsXMLToken(sValue, XML_TRUE))
                        nLayerID = SC_LAYER_BACK;
                    break;
                default:
                    SAL_WARN("sc", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
            }
        }
        SetLayer(rShape, nLayerID, rShape->getShapeType());
    }

    if (!bNote)
    {
        // any shape other than a note prevents copying the sheet
        ScSheetSaveData* pSheetData = comphelper::getUnoTunnelImplementation<ScModelObj>(mrImporter.GetModel())->GetSheetSaveData();
        pSheetData->BlockSheet( rTables.GetCurrentSheet() );
    }

    static_cast<ScXMLImport&>(mrImporter).UnlockSolarMutex();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
