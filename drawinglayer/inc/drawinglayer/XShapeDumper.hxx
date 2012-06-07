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

#include <libxml/xmlwriter.h>
#include <drawinglayer/drawinglayerdllapi.h>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/RectanglePoint.hpp>

#ifndef ChartViewDumper_hxx
#define ChartViewDumper_hxx

class DRAWINGLAYER_DLLPUBLIC XShapeDumper
{

public:
    XShapeDumper();
    rtl::OUString dump(com::sun::star::uno::Reference<com::sun::star::drawing::XShapes> xPageShapes);

private:
    void dumpFillStyleAsAttribute(com::sun::star::drawing::FillStyle eFillStyle, xmlTextWriterPtr xmlWriter);
    void dumpFillColorAsAttribute(sal_Int32 aColor, xmlTextWriterPtr xmlWriter);
    void dumpFillTransparenceAsAttribute(sal_Int32 aTransparence, xmlTextWriterPtr xmlWriter);
    void dumpFillTransparenceGradientNameAsAttribute(rtl::OUString sTranspGradName, xmlTextWriterPtr xmlWriter);
    void dumpGradientProperty(com::sun::star::awt::Gradient aGradient, xmlTextWriterPtr xmlWriter);
    void dumpFillTransparenceGradientAsElement(com::sun::star::awt::Gradient aTranspGrad, xmlTextWriterPtr xmlWriter);
    void dumpFillGradientNameAsAttribute(rtl::OUString sGradName, xmlTextWriterPtr xmlWriter);
    void dumpFillGradientAsElement(com::sun::star::awt::Gradient aGradient, xmlTextWriterPtr xmlWriter);
    void dumpFillHatchNameAsAttribute(rtl::OUString sHatchName, xmlTextWriterPtr xmlWriter);
    void dumpFillHatchAsElement(com::sun::star::drawing::Hatch aHatch, xmlTextWriterPtr xmlWriter);
    void dumpFillBackgroundAsAttribute(sal_Bool aBackground, xmlTextWriterPtr xmlWriter);
    void dumpFillBitmapNameAsAttribute(rtl::OUString sBitmapName, xmlTextWriterPtr xmlWriter);
    void dumpFillBitmapAsElement(com::sun::star::uno::Reference<com::sun::star::awt::XBitmap> xBitmap, xmlTextWriterPtr xmlWriter);
    void dumpFillBitmapURLAsAttribute(rtl::OUString sBitmapURL, xmlTextWriterPtr xmlWriter);
    void dumpFillBitmapPositionOffsetXAsAttribute(sal_Int32 aBitmapPositionOffsetX, xmlTextWriterPtr xmlWriter);
    void dumpFillBitmapPositionOffsetYAsAttribute(sal_Int32 aBitmapPositionOffsetY, xmlTextWriterPtr xmlWriter);
    void dumpFillBitmapOffsetXAsAttribute(sal_Int32 aBitmapOffsetX, xmlTextWriterPtr xmlWriter);
    void dumpFillBitmapOffsetYAsAttribute(sal_Int32 aBitmapOffsetY, xmlTextWriterPtr xmlWriter);
    void dumpFillBitmapRectanglePointAsAttribute(com::sun::star::drawing::RectanglePoint eBitmapRectanglePoint, xmlTextWriterPtr xmlWriter);

    void dumpPositionAsAttribute(const com::sun::star::awt::Point& rPoint, xmlTextWriterPtr xmlWriter);
    void dumpSizeAsAttribute(const com::sun::star::awt::Size& rSize, xmlTextWriterPtr xmlWriter);
    void dumpShapeDescriptorAsAttribute( com::sun::star::uno::Reference< com::sun::star::drawing::XShapeDescriptor > xDescr, xmlTextWriterPtr xmlWriter );
    void dumpXShape(com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape, xmlTextWriterPtr xmlWriter);
    void dumpXShapes( com::sun::star::uno::Reference< com::sun::star::drawing::XShapes > xShapes, xmlTextWriterPtr xmlWriter );

};
#endif
