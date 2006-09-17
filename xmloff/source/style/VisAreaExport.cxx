/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VisAreaExport.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:47:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _XMLOFF_VISAREAEXPORT_HXX
#include "VisAreaExport.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

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


