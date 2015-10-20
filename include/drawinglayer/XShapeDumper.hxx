/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DRAWINGLAYER_XSHAPEDUMPER_HXX
#define INCLUDED_DRAWINGLAYER_XSHAPEDUMPER_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>

#include <com/sun/star/drawing/PolygonKind.hpp>

#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextAnimationDirection.hpp>
#include <com/sun/star/drawing/TextAnimationKind.hpp>
#include <com/sun/star/text/WritingMode.hpp>

#include <com/sun/star/drawing/HomogenMatrixLine3.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>

#include <com/sun/star/beans/PropertyValue.hpp>

class DRAWINGLAYER_DLLPUBLIC XShapeDumper
{

public:
    XShapeDumper();
    static OUString dump(css::uno::Reference<css::drawing::XShapes> xPageShapes, bool bDumpInteropProperties=false);
    static OUString dump(css::uno::Reference<css::drawing::XShape> xPageShapes, bool bDumpInteropProperties=false);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
