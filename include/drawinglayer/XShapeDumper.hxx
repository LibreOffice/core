/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Artur Dorda <artur.dorda+libo@gmail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

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

#ifndef XShapeDumper_hxx
#define XShapeDumper_hxx

class DRAWINGLAYER_DLLPUBLIC XShapeDumper
{

public:
    XShapeDumper();
    OUString dump(com::sun::star::uno::Reference<com::sun::star::drawing::XShapes> xPageShapes);

};
#endif
