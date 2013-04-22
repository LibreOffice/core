/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHART2_LINE_PROPERTIES_HXX
#define CHART2_LINE_PROPERTIES_HXX

#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/uno/Any.hxx>

class LineProperties
{
private:
    OUString maDashName;
    com::sun::star::drawing::LineDash maLineDash;
    sal_Int32 mnLineWidth;
    com::sun::star::drawing::LineStyle meLineStyle;
    com::sun::star::util::Color maLineColor;
    sal_Int16 mnLineTransparence;
    com::sun::star::drawing::LineJoint meLineJoint;

public:
    LineProperties();
    LineProperties(const LineProperties&);

    com::sun::star::uno::Any getPropertyValue(const OUString& rName);
    void setPropertyValue(const OUString& rName, const com::sun::star::uno::Any& rAny);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
