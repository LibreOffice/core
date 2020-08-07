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

#ifndef INCLUDED_CHART2_SOURCE_VIEW_INC_VLINEPROPERTIES_HXX
#define INCLUDED_CHART2_SOURCE_VIEW_INC_VLINEPROPERTIES_HXX

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>

namespace com::sun::star::beans { class XPropertySet; }

namespace chart
{

struct VLineProperties
{
    css::uno::Any Color; //type sal_Int32 UNO_NAME_LINECOLOR
    css::uno::Any LineStyle; //type drawing::LineStyle for property UNO_NAME_LINESTYLE
    css::uno::Any Transparence;//type sal_Int16 for property UNO_NAME_LINETRANSPARENCE
    css::uno::Any Width;//type sal_Int32 for property UNO_NAME_LINEWIDTH
    css::uno::Any DashName;//type OUString for property "LineDashName"
    css::uno::Any LineCap; //type drawing::LineCap for property UNO_NAME_LINECAP

    VLineProperties();
    void initFromPropertySet( const css::uno::Reference< css::beans::XPropertySet >& xProp );

    bool isLineVisible() const;
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
