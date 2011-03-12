/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
