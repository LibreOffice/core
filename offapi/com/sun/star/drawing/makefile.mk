#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: cl $ $Date: 2000-11-20 14:22:33 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=api

TARGET=cssdrawing
PACKAGE=com$/sun$/star$/drawing

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    BoundVolume.idl\
    Alignment.idl\
    AreaShape.idl\
    AreaShapeDescriptor.idl\
    Arrangement.idl\
    BezierPoint.idl\
    CircleKind.idl\
    ClosedBezierShape.idl\
    ClosedFreeHandShape.idl\
    color.idl\
    ColorMode.idl\
    ConnectionType.idl\
    ConnectorShape.idl\
    ConnectorShapeDescriptor.idl\
    ConnectorType.idl\
    ControlShape.idl\
    CoordinateSequence.idl\
    CoordinateSequenceSequence.idl\
    DashStyle.idl\
    DimensioningShape.idl\
    DimensioningShapeDescriptor.idl\
    Direction3D.idl\
    DoubleSequence.idl\
    DoubleSequenceSequence.idl\
    DrawingDocument.idl\
    DrawingDocumentDrawView.idl\
    DrawPage.idl\
    DrawPages.idl\
    DrawViewMode.idl\
    EllipseShape.idl\
    EllipseShapeDescriptor.idl\
    FillStyle.idl\
    FlagSequence.idl\
    FlagSequenceSequence.idl\
    GenericDrawPage.idl\
    GluePoint.idl\
    GraphicObjectShape.idl\
    GraphicObjectShapeDescriptor.idl\
    GroupShape.idl\
    Hatch.idl\
    HatchStyle.idl\
    HomogenMatrix.idl\
    HomogenMatrixLine.idl\
    HorizontalDimensioning.idl\
    Layer.idl\
    LayerManager.idl\
    LayerType.idl\
    LineDash.idl\
    LineEndType.idl\
    LineShape.idl\
    LineShapeDescriptor.idl\
    LineStyle.idl\
    LineJoint.idl\
    MasterPage.idl\
    MasterPages.idl\
    MeasureKind.idl\
    MeasureTextHorzPos.idl\
    MeasureTextVertPos.idl\
    MirrorAxis.idl\
    NormalsKind.idl\
    OLE2Shape.idl\
    OpenBezierShape.idl\
    OpenFreeHandShape.idl\
    PageShape.idl\
    PointSequence.idl\
    PointSequenceSequence.idl\
    PolygonFlags.idl\
    PolygonKind.idl\
    PolyLinePathShape.idl\
    PolyLineShape.idl\
    PolyPolygonBezierCoords.idl\
    PolyPolygonBezierDescriptor.idl\
    PolyPolygonBezierShape.idl\
    PolyPolygonBezierShapeDescriptor.idl\
    PolyPolygonDescriptor.idl\
    PolyPolygonPathShape.idl\
    PolyPolygonShape.idl\
    PolyPolygonShape3D.idl\
    PolyPolygonShapeDescriptor.idl\
    Position3D.idl\
    ProjectionMode.idl\
    RectanglePoint.idl\
    RectangleShape.idl\
    RectangleShapeDescriptor.idl\
    RotationDescriptor.idl\
    ShadeMode.idl\
    ShadowDescriptor.idl\
    Shape.idl\
    ShapeDescriptor.idl\
    Shapes.idl\
    SnapObjectType.idl\
    Text.idl\
    TextAdjust.idl\
    TextAnimationDirection.idl\
    TextAnimationKind.idl\
    TextFitToSizeType.idl\
    TextShape.idl\
    TextShapeDescriptor.idl\
    TextureKind.idl\
    TextureMode.idl\
    TextureProjectionMode.idl\
    VerticalDimensioning.idl\
    XConnectableShape.idl\
    XConnectorShape.idl\
    XControlShape.idl\
    XDrawPage.idl\
    XDrawPageDuplicator.idl\
    XDrawPageExpander.idl\
    XDrawPages.idl\
    XDrawPagesSupplier.idl\
    XDrawPageSummarizer.idl\
    XDrawPageSupplier.idl\
    XDrawView.idl\
    XLayer.idl\
    XLayerManager.idl\
    XLayerSupplier.idl\
    XMasterPagesSupplier.idl\
    XMasterPageTarget.idl\
    XShape.idl\
    XShapeAligner.idl\
    XShapeArranger.idl\
    XShapeBinder.idl\
    XShapeCombiner.idl\
    XShapeDescriptor.idl\
    XShapeGroup.idl\
    XShapeGrouper.idl\
    XShapeMirror.idl\
    XShapes.idl\
    XUniversalShapeDescriptor.idl\
    ColorMode.idl\
    GluePoint2.idl\
    XGluePointsSupplier.idl\
    EscapeDirection.idl\
    TextVerticalAdjust.idl\
    TextHorizontalAdjust.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
