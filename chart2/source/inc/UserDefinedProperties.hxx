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

#include "FastPropertyIdRanges.hxx"
#include "charttoolsdllapi.hxx"

#include <vector>

namespace com::sun::star::beans { struct Property; }

namespace chart
{

// implements service com.sun.star.xml.UserDefinedAttributesSupplier
namespace UserDefinedProperties
{
    // FastProperty Ids for properties
    enum
    {
        // UserDefined attributes in the chart-properties
        PROP_XML_USERDEF_CHART = FAST_PROPERTY_ID_START_USERDEF_PROP,
        PROP_XML_USERDEF_TEXT,
        PROP_XML_USERDEF_PARA,
        // com.sun.star.xml.UserDefinedAttributesSupplier
        // UserDefined attributes in the other properties-sections. (as long as
        // there is no reliable mechanism, properties may move between the
        // sections)
        PROP_XML_USERDEF
    };

    OOO_DLLPUBLIC_CHARTTOOLS void AddPropertiesToVector(
        std::vector< css::beans::Property > & rOutProperties );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
