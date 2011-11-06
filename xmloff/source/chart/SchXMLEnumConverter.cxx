/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
