/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "LineProperties.hxx"

using namespace com::sun::star;

LineProperties::LineProperties():
    mnLineWidth(0),
    meLineStyle(drawing::LineStyle_SOLID),
    maLineColor(0),
    mnLineTransparence(0),
    meLineJoint(drawing::LineJoint_ROUND)
{
}

LineProperties::LineProperties(const LineProperties& r):
    maDashName(r.maDashName),
    maLineDash(r.maLineDash),
    mnLineWidth(r.mnLineWidth),
    meLineStyle(r.meLineStyle),
    maLineColor(r.maLineColor),
    mnLineTransparence(r.mnLineTransparence),
    meLineJoint(r.meLineJoint)
{
}

uno::Any LineProperties::getPropertyValue(const OUString& rName)
{
    uno::Any aRet;
    if(rName == "LineDashName")
    {
        aRet <<= maDashName;
    }
    else if(rName == "LineDash")
    {
        aRet <<= maLineDash;
    }
    else if(rName == "LineWidth")
    {
        aRet <<= mnLineWidth;
    }
    else if(rName == "LineStyle")
    {
        aRet = uno::makeAny(meLineStyle);
    }
    else if(rName == "LineColor")
    {
        aRet <<= maLineColor;
    }
    else if(rName == "LineTransparence")
    {
        aRet <<= mnLineTransparence;
    }
    else if(rName == "LineJoint")
    {
        aRet <<= meLineJoint;
    }
    return aRet;
}

void LineProperties::setPropertyValue(const OUString& rName, const uno::Any& rAny)
{
    if(rName == "LineDashName")
    {
        rAny >>= maDashName;
    }
    else if(rName == "LineDash")
    {
        rAny >>= maLineDash;
    }
    else if(rName == "LineWidth")
    {
        rAny >>= mnLineWidth;
    }
    else if(rName == "LineStyle")
    {
        rAny >>= meLineStyle;
    }
    else if(rName == "LineColor")
    {
        rAny >>= maLineColor;
    }
    else if(rName == "LineTransparence")
    {
        rAny >>= mnLineTransparence;
    }
    else if(rName == "LineJoint")
    {
        rAny >>= meLineJoint;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
