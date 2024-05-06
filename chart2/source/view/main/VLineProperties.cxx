/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <VLineProperties.hxx>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/diagnose_ex.hxx>

namespace chart
{
using namespace ::com::sun::star;

//  get line properties from a propertyset

VLineProperties::VLineProperties()
{
    Color <<= sal_Int32(0x000000); //type sal_Int32 UNO_NAME_LINECOLOR
    LineStyle
        <<= drawing::LineStyle_SOLID; //type drawing::LineStyle for property UNO_NAME_LINESTYLE
    Transparence <<= sal_Int16(0); //type sal_Int16 for property UNO_NAME_LINETRANSPARENCE
    Width <<= sal_Int32(0); //type sal_Int32 for property UNO_NAME_LINEWIDTH
    LineCap <<= drawing::LineCap_BUTT; //type drawing::LineCap for property UNO_NAME_LINECAP
}

void VLineProperties::initFromPropertySet(const uno::Reference<beans::XPropertySet>& xProp)
{
    if (xProp.is())
    {
        try
        {
            Color = xProp->getPropertyValue(u"LineColor"_ustr);
            LineStyle = xProp->getPropertyValue(u"LineStyle"_ustr);
            Transparence = xProp->getPropertyValue(u"LineTransparence"_ustr);
            Width = xProp->getPropertyValue(u"LineWidth"_ustr);
            DashName = xProp->getPropertyValue(u"LineDashName"_ustr);
            LineCap = xProp->getPropertyValue(u"LineCap"_ustr);
        }
        catch (const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("chart2", "");
        }
    }
    else
        LineStyle <<= drawing::LineStyle_NONE;
}

bool VLineProperties::isLineVisible() const
{
    bool bRet = false;

    drawing::LineStyle aLineStyle(drawing::LineStyle_SOLID);
    LineStyle >>= aLineStyle;
    if (aLineStyle != drawing::LineStyle_NONE)
    {
        sal_Int16 nLineTransparence = 0;
        Transparence >>= nLineTransparence;
        if (nLineTransparence != 100)
        {
            bRet = true;
        }
    }

    return bRet;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
