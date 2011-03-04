/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "SchXMLEnumConverter.hxx"

#include <com/sun/star/chart/ChartLegendPosition.hpp>
#include <com/sun/star/chart/ChartLegendExpansion.hpp>
#include <com/sun/star/uno/Any.hxx>

// header for rtl::Static
#include <rtl/instance.hxx>

using ::rtl::OUString;
using namespace ::xmloff::token;
using namespace ::com::sun::star;

namespace
{
//-----------------------------------------------------------------------

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

//-----------------------------------------------------------------------

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

//-----------------------------------------------------------------------

}//end anonymous namespace

//-----------------------------------------------------------------------

XMLEnumPropertyHdl& SchXMLEnumConverter::getLegendPositionConverter()
{
    return TheLegendPositionPropertyHdl::get();
}
XMLEnumPropertyHdl& SchXMLEnumConverter::getLegendExpansionConverter()
{
    return TheLegendExpansionPropertyHdl::get();
}
