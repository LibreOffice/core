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

#include "SchXMLEnumConverter.hxx"

#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/uno/Any.hxx>

// header for rtl::Static
#include <rtl/instance.hxx>

using namespace ::xmloff::token;
using namespace ::com::sun::star;

namespace
{

SvXMLEnumMapEntry aXMLLegendPositionEnumMap[] =
{
    { XML_START,    chart::ChartLegendPosition_LEFT     },
    { XML_TOP,      chart::ChartLegendPosition_TOP      },
    { XML_END,      chart::ChartLegendPosition_RIGHT    },
    { XML_BOTTOM,   chart::ChartLegendPosition_BOTTOM   },
    { XML_TOKEN_INVALID, 0 }
};

class XMLLegendPositionPropertyHdl : public XMLEnumPropertyHdl
{
public:
    XMLLegendPositionPropertyHdl()
        : XMLEnumPropertyHdl( aXMLLegendPositionEnumMap, ::getCppuType((const chart::ChartLegendPosition*)0) ) {}
    virtual ~XMLLegendPositionPropertyHdl() {};
};

struct TheLegendPositionPropertyHdl : public rtl::Static< XMLLegendPositionPropertyHdl, TheLegendPositionPropertyHdl >
{
};

SvXMLEnumMapEntry aXMLLegendExpansionEnumMap[] =
{
    { XML_WIDE,        chart::ChartLegendExpansion_WIDE },
    { XML_HIGH,        chart::ChartLegendExpansion_HIGH },
    { XML_BALANCED,    chart::ChartLegendExpansion_BALANCED },
    { XML_CUSTOM,      chart::ChartLegendExpansion_CUSTOM },
    { XML_TOKEN_INVALID, 0 }
};

class XMLLegendExpansionPropertyHdl : public XMLEnumPropertyHdl
{
public:
    XMLLegendExpansionPropertyHdl()
        : XMLEnumPropertyHdl( aXMLLegendExpansionEnumMap, ::getCppuType((const chart::ChartLegendExpansion*)0) ) {}
    virtual ~XMLLegendExpansionPropertyHdl() {};
};

struct TheLegendExpansionPropertyHdl : public rtl::Static< XMLLegendExpansionPropertyHdl, TheLegendExpansionPropertyHdl >
{
};

}//end anonymous namespace

XMLEnumPropertyHdl& SchXMLEnumConverter::getLegendPositionConverter()
{
    return TheLegendPositionPropertyHdl::get();
}
XMLEnumPropertyHdl& SchXMLEnumConverter::getLegendExpansionConverter()
{
    return TheLegendExpansionPropertyHdl::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
