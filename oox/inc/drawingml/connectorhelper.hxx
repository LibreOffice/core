/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>

#include <basegfx/point/b2dpoint.hxx>
#include <drawingml/customshapeproperties.hxx>
#include <oox/drawingml/shape.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <vector>

namespace ConnectorHelper
{
/**
 * Some preset shapes use the default connector site but in order right, bottom, left, top.
 * The function detects this.

 * @param rShapeType The shape type identifier as it is used in the prst attribute in OOXML.
 * @return true if this shape type has the right, bottom, left, top order of connection site.
 */
bool hasClockwiseCxn(const OUString& rShapeType);

/**
 * Calculates the handle positions based on the definition in presetShapeDefinitions.xml and the
 * actual size of the connector shape. It transforms the handles positions to the actual used
 * connector layout. It transforms the handle coordinates to Hmm.
 * The vector rHandlePositions is cleared and then filled with the actual handle positions. It is
 * empty if the geometry does not use handles, e.g. "bentConnector2" and "curvedConnector2".
 * This method works both for bentConnector and curvedConnector.

 * @pre pConnector is not empty and points to a ooxml::drawing::Shape that represents a connector
 * shape.

 * @param [in] pConnector pointer to a connector shape
 * @param [in, out] rHandlePositions contains the calculated handle positions.
*/
void getOOXHandlePositionsHmm(const oox::drawingml::ShapePtr& pConnector,
                              std::vector<basegfx::B2DPoint>& rHandlePositions);

/**
  * OOXML defines the connector shapes so, that the start point is top-left and the leaving
  * direction is horizontal. Other layout is done by flipV, flipH and 90deg, 180deg or 270deg
  * rotation of the connector shape. This method collects these transformations into a
  * B2DHomMatrix.

  * @param [in] pConnector is pointer to a oox::drawing::Shape that represents a connector shape.
  * @return a newly created B2DHomMatrix. It might be the unit matrix.
*/
basegfx::B2DHomMatrix getConnectorTransformMatrix(const oox::drawingml::ShapePtr& pConnector);

/**
 * Calculates the handle positions of a connector of type ConnectorType_STANDARD. Such connector
 * corresponds to the OOXML bentConnector shapes, aka "ElbowConnector". The calculation is based on
 * the actual polygon of the connector. The coordinates are always returned in Hmm, even for shapes
 * on a text document draw page.
 * The vector rHandlePositions is cleaned and then filled with the actual handle positions. It
 * is empty if the geometry does not use handles.

 * @param [in] pConnector is pointer to a oox::drawing::Shape that represents a connector shape.
 * @param [in,out] rHandlePositions contains the calculated handle positions.
*/
void getLOBentHandlePositionsHmm(const oox::drawingml::ShapePtr& pConnector,
                                 std::vector<basegfx::B2DPoint>& rHandlePositions);

/**
 * Calculates the handle positions of a connector of type ConnectorType_CURVE for which OOXML
 * compatible routing is enabled. Such connector corresponds to the OOXML curvedConnector shapes. The
 * calculation is based on the actual polygon of the connector. The coordinates are always returned
 * in Hmm, even for shapes on a text document draw page.
 * The vector rHandlePositions is cleaned and then filled with the actual handle positions. It
 * is empty if the geometry does not use handles.

 * @param [in] pConnector is pointer to a oox::drawing::Shape that represents a connector shape.
 * @param [in,out] rHandlePositions contains the calculated handle positions.
*/
void getLOCurvedHandlePositionsHmm(const oox::drawingml::ShapePtr& pConnector,
                                   std::vector<basegfx::B2DPoint>& rHandlePositions);

/**
 * Sets the properties "StartShape", "EndShape", "StartGluePointIndex" and "EndGluePointIndex". Thus
 * it actually connects the shapes. Connecting generates the default connector path.

 * @param pConnector is pointer to a oox::drawing::Shape that represents a connector shape.
 * @param [in] A flat map of target shape candidates, indexed by their msId.
*/
void applyConnections(const oox::drawingml::ShapePtr& pConnector,
                      oox::drawingml::ShapeIdMap& rShapeMap);

/**
 * Calculates the difference between handle positions in OOXML and the default handle positions in
 * LibreOffice. The difference is written to "EdgeLine1Delta", "EdgeLine2Delta" and "EdgeLine3Delta"
 * properties. It uses the connector polygon.

 * @pre The referenced connector has type ConnectorType_STANDARD and has the default connector path.

 * @param pConnector refers to the shape whose handles are adapted.
*/
void applyBentHandleAdjustments(oox::drawingml::ShapePtr pConnector);

/**
 * Calculates the difference between handle positions in OOXML and the default handle positions in
 * LibreOffice. The difference is written to "EdgeLine1Delta", "EdgeLine2Delta" and "EdgeLine3Delta"
 * properties. It uses the connector polygon.

 * @pre The referenced connector has type ConnectorType_CURVE, OOXML compatible routing is enabled,
        and the connector has the default connector path.
 * @param pConnector refers to the shape whose handles are adapted.
*/
void applyCurvedHandleAdjustments(oox::drawingml::ShapePtr pConnector);

} // end namespace ConnectorHelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
