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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/Direction3D.hpp>

#ifndef EnhancedShapeDumper_hxx
#define EnhancedShapeDumper_hxx

class DRAWINGLAYER_DLLPUBLIC EnhancedShapeDumper
{
public:
    EnhancedShapeDumper(xmlTextWriterPtr writer)
    :
    xmlWriter(writer)
    {

    }

    // auxiliary functions
    void dumpEnhancedCustomShapeParameterPair(com::sun::star::drawing::EnhancedCustomShapeParameterPair aParameterPair);
    void dumpDirection3D(com::sun::star::drawing::Direction3D aDirection3D);

    // EnhancedCustomShapeExtrusion.idl
    void dumpEnhancedCustomShapeExtrusionService(com::sun::star::uno::Reference< com::sun::star::beans::XPropertySet > xPropSet);
    void dumpExtrusionAsAttribute(sal_Bool bExtrusion);
    void dumpBrightnessAsAttribute(double aBrightness);
    void dumpDepthAsElement(com::sun::star::drawing::EnhancedCustomShapeParameterPair aDepth);
    void dumpDiffusionAsAttribute(double aDiffusion);
    void dumpNumberOfLineSegmentsAsAttribute(sal_Int32 aNumberOfLineSegments);
    void dumpLightFaceAsAttribute(sal_Bool bLightFace);
    void dumpFirstLightHarshAsAttribute(sal_Bool bFirstLightHarsh);
    void dumpSecondLightHarshAsAttribute(sal_Bool bSecondLightHarsh);
    void dumpFirstLightLevelAsAttribute(double aFirstLightLevel);
    void dumpSecondLightLevelAsAttribute(double aSecondLightLevel);

private:
    xmlTextWriterPtr xmlWriter;
};
#endif
