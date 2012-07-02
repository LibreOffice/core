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

#include <drawinglayer/EnhancedShapeDumper.hxx>
#include <rtl/strbuf.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star;

// ------------------------------------------------------
// ---------- EnhancedCustomShapeExtrusion.idl ----------
// ------------------------------------------------------

void EnhancedShapeDumper::dumpEnhancedCustomShapeExtrusionService(uno::Reference< beans::XPropertySet > xPropSet)
{
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Extrusion");
        sal_Bool bExtrusion;
        if(anotherAny >>= bExtrusion)
            dumpExtrusionAsAttribute(bExtrusion);
    }
    {
        uno::Any anotherAny = xPropSet->getPropertyValue("Brightness");
        double aBrightness;
        if(anotherAny >>= aBrightness)
            dumpBrightnessAsAttribute(aBrightness);
    }
}
void EnhancedShapeDumper::dumpExtrusionAsAttribute(sal_Bool bExtrusion)
{
    if(bExtrusion)
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusion"), "%s", "true");
    else
        xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("extrusion"), "%s", "false");
}

void EnhancedShapeDumper::dumpBrightnessAsAttribute(double aBrightness)
{
    xmlTextWriterWriteFormatAttribute( xmlWriter, BAD_CAST("brightness"), "%f", aBrightness);
}

