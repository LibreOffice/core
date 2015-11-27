/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CHART2_SOURCE_INC_LINEPROPERTIES_HXX
#define INCLUDED_CHART2_SOURCE_INC_LINEPROPERTIES_HXX

#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/uno/Any.hxx>

class LineProperties
{
private:
    OUString maDashName;
    css::drawing::LineDash maLineDash;
    sal_Int32 mnLineWidth;
    css::drawing::LineStyle meLineStyle;
    css::util::Color maLineColor;
    sal_Int16 mnLineTransparence;
    css::drawing::LineJoint meLineJoint;

public:
    LineProperties();
    LineProperties(const LineProperties&);

    css::uno::Any getPropertyValue(const OUString& rName);
    void setPropertyValue(const OUString& rName, const css::uno::Any& rAny);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
