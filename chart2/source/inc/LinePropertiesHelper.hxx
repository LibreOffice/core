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
#pragma once

#include "PropertyHelper.hxx"
#include "FastPropertyIdRanges.hxx"
#include "charttoolsdllapi.hxx"

#include <vector>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::beans { struct Property; }
namespace com::sun::star::uno { template <class interface_type> class Reference; }

namespace chart
{

// implements service LineProperties
namespace LinePropertiesHelper
{
    // FastProperty Ids for properties
    enum
    {
        // com.sun.star.drawing.LineProperties
        PROP_LINE_STYLE = FAST_PROPERTY_ID_START_LINE_PROP,
        PROP_LINE_DASH,
        PROP_LINE_DASH_NAME, //not in service description
        PROP_LINE_COLOR,
        PROP_LINE_TRANSPARENCE,
        PROP_LINE_WIDTH,
        PROP_LINE_JOINT,
        PROP_LINE_CAP
    };

    OOO_DLLPUBLIC_CHARTTOOLS void AddPropertiesToVector(
        std::vector< css::beans::Property > & rOutProperties );

    OOO_DLLPUBLIC_CHARTTOOLS void AddDefaultsToMap( ::chart::tPropertyValueMap & rOutMap );

    bool IsLineVisible( const css::uno::Reference<
        css::beans::XPropertySet >& xLineProperties );
    void SetLineVisible( const css::uno::Reference<
        css::beans::XPropertySet >& xLineProperties );
    void SetLineInvisible( const css::uno::Reference<
        css::beans::XPropertySet >& xLineProperties );
    void SetLineColor( const css::uno::Reference<
        css::beans::XPropertySet >& xGridProperties, sal_Int32 nColor  );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
