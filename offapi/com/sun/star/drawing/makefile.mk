#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.30 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=cssdrawing
PACKAGE=com$/sun$/star$/drawing

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AccessibleShape.idl      		\
    AccessibleGraphicShape.idl      \
    AccessibleOLEShape.idl      	\
    AccessibleDrawDocumentView.idl	\
    AccessibleSlideView.idl			\
    AccessibleSlideViewObject.idl	\
    AccessibleGraphControl.idl\
    Alignment.idl\
    AppletShape.idl\
    Arrangement.idl\
    Background.idl\
    BezierPoint.idl\
    BitmapMode.idl\
    BitmapTable.idl\
    BoundVolume.idl\
    CameraGeometry.idl\
    CaptionEscapeDirection.idl\
    CaptionShape.idl\
    CaptionType.idl\
    CircleKind.idl\
    ClosedBezierShape.idl\
    ColorMode.idl\
    ConnectionType.idl\
    ConnectorProperties.idl\
    ConnectorShape.idl\
    ConnectorType.idl\
    ControlShape.idl\
    CoordinateSequence.idl\
    CoordinateSequenceSequence.idl\
    CustomShape.idl\
    CustomShapeEngine.idl\
    DashStyle.idl\
    DashTable.idl\
    Defaults.idl\
    Direction3D.idl\
    DocumentSettings.idl\
    DoubleSequence.idl\
    DoubleSequenceSequence.idl\
    DrawingDocument.idl\
    DrawingDocumentDrawView.idl\
    DrawingDocumentFactory.idl\
    DrawPage.idl\
    DrawPages.idl\
    DrawViewMode.idl\
    EnhancedCustomShapeAdjustmentValue.idl\
    EnhancedCustomShapeExtrusion.idl\
    EnhancedCustomShapeGeometry.idl\
    EnhancedCustomShapeGluePointType.idl\
    EnhancedCustomShapeHandle.idl\
    EnhancedCustomShapeParameter.idl\
    EnhancedCustomShapeParameterPair.idl\
    EnhancedCustomShapeParameterType.idl\
    EnhancedCustomShapePath.idl\
    EnhancedCustomShapeSegment.idl\
    EnhancedCustomShapeSegmentCommand.idl\
    EnhancedCustomShapeTextFrame.idl\
    EnhancedCustomShapeTextPath.idl\
    EnhancedCustomShapeTextPathMode.idl\
    EllipseShape.idl\
    EscapeDirection.idl\
    FillProperties.idl\
    FillStyle.idl\
    FlagSequence.idl\
    FlagSequenceSequence.idl\
    GenericDrawingDocument.idl\
    GenericDrawPage.idl\
    GluePoint.idl\
    GluePoint2.idl\
    GradientTable.idl\
    GraphicExportFilter.idl\
    GraphicFilterRequest.idl\
    GraphicObjectShape.idl\
    GroupShape.idl\
    Hatch.idl\
    HatchStyle.idl\
    HatchTable.idl\
    HomogenMatrix.idl\
    HomogenMatrix3.idl\
    HomogenMatrix4.idl\
    HomogenMatrixLine.idl\
    HomogenMatrixLine3.idl\
    HomogenMatrixLine4.idl\
    HorizontalDimensioning.idl\
    Layer.idl\
    LayerManager.idl\
    LayerType.idl\
    LineDash.idl\
    LineEndType.idl\
    LineJoint.idl\
    LineProperties.idl\
    LineShape.idl\
    LineStyle.idl\
    MarkerTable.idl\
    MasterPage.idl\
    MasterPages.idl\
    MeasureKind.idl\
    MeasureProperties.idl\
    MeasureShape.idl\
    MeasureTextHorzPos.idl\
    MeasureTextVertPos.idl\
    MirrorAxis.idl\
    NormalsKind.idl\
    OLE2Shape.idl\
    OpenBezierShape.idl\
    PageShape.idl\
    PluginShape.idl\
    PointSequence.idl\
    PointSequenceSequence.idl\
    PolygonFlags.idl\
    PolygonKind.idl\
    PolyLineShape.idl\
    PolyPolygonBezierCoords.idl\
    PolyPolygonBezierDescriptor.idl\
    PolyPolygonBezierShape.idl\
    PolyPolygonDescriptor.idl\
    PolyPolygonShape.idl\
    PolyPolygonShape3D.idl\
    Position3D.idl\
    ProjectionMode.idl\
    RectanglePoint.idl\
    RectangleShape.idl\
    RotationDescriptor.idl\
    ShadeMode.idl\
    ShadowProperties.idl\
    Shape.idl\
    ShapeCollection.idl\
    Shapes.idl\
    SlideRenderer.idl\
    SlideSorter.idl\
    SnapObjectType.idl\
    Text.idl\
    TextAdjust.idl\
    TextAnimationDirection.idl\
    TextAnimationKind.idl\
    TextFitToSizeType.idl\
    TextHorizontalAdjust.idl\
    TextProperties.idl\
    TextShape.idl\
    TextureKind.idl\
    TextureKind2.idl\
    TextureMode.idl\
    TextureProjectionMode.idl\
    TextVerticalAdjust.idl\
    TransparencyGradientTable.idl\
    VerticalDimensioning.idl\
    XConnectableShape.idl\
    XConnectorShape.idl\
    XControlShape.idl\
    XCustomShapeEngine.idl\
    XCustomShapeHandle.idl\
    XDrawPage.idl\
    XDrawPageDuplicator.idl\
    XDrawPageExpander.idl\
    XDrawPages.idl\
    XDrawPagesSupplier.idl\
    XDrawPageSummarizer.idl\
    XDrawPageSupplier.idl\
    XDrawSubController.idl\
    XDrawView.idl\
    XEnhancedCustomShapeDefaulter.idl\
    XGluePointsSupplier.idl\
    XLayer.idl\
    XLayerManager.idl\
    XLayerSupplier.idl\
    XMasterPagesSupplier.idl\
    XMasterPageTarget.idl\
    XPresenterHelper.idl\
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
    XSlidePreviewCache.idl\
    XSlideRenderer.idl\
    XUniversalShapeDescriptor.idl\
    XSelectionFunction.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk

