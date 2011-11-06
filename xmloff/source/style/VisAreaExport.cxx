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
#include <com/sun/star/awt/Rectangle.hpp>
#include "xmloff/VisAreaExport.hxx"
#include <xmloff/xmlexp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <tools/gen.hxx>

using namespace ::xmloff::token;


XMLVisAreaExport::XMLVisAreaExport(SvXMLExport& rExport, const sal_Char *pName,
                            const Rectangle& aRect, const MapUnit aMapUnit)
{
    SvXMLUnitConverter& rUnitConv = rExport.GetMM100UnitConverter();

    // write VisArea Element and its Attributes
    rtl::OUStringBuffer sBuffer;
    rUnitConv.convertMeasure(sBuffer, aRect.getX(), aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_X, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.getY(), aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_Y, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.getWidth(), aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_WIDTH, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.getHeight(), aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_HEIGHT, sBuffer.makeStringAndClear());
    SvXMLElementExport aVisAreaElem(rExport, XML_NAMESPACE_OFFICE, pName, sal_True, sal_True);
}

XMLVisAreaExport::XMLVisAreaExport(SvXMLExport& rExport, const sal_Char *pName,
                                   const com::sun::star::awt::Rectangle& aRect, const sal_Int16 nMeasureUnit )
{
    MapUnit aMapUnit = (MapUnit)nMeasureUnit;

    SvXMLUnitConverter& rUnitConv = rExport.GetMM100UnitConverter();

    // write VisArea Element and its Attributes
    rtl::OUStringBuffer sBuffer;
    rUnitConv.convertMeasure(sBuffer, aRect.X, aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_X, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.Y, aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_Y, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.Width, aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_WIDTH, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.Height, aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, XML_HEIGHT, sBuffer.makeStringAndClear());
    SvXMLElementExport aVisAreaElem(rExport, XML_NAMESPACE_OFFICE, pName, sal_True, sal_True);
}

XMLVisAreaExport::~XMLVisAreaExport()
{
}


