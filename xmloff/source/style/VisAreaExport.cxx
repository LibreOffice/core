/*************************************************************************
 *
 *  $RCSfile: VisAreaExport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: cl $ $Date: 2001-02-21 18:05:31 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

XMLVisAreaExport::XMLVisAreaExport(SvXMLExport& rExport, const sal_Char *pName,
                            const Rectangle& aRect, const MapUnit aMapUnit)
{
    SvXMLUnitConverter& rUnitConv = rExport.GetMM100UnitConverter();

    // write VisArea Element and its Attributes
    rtl::OUStringBuffer sBuffer;
    rUnitConv.convertMeasure(sBuffer, aRect.getX(), aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_x, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.getY(), aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_y, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.getWidth(), aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_width, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.getHeight(), aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_height, sBuffer.makeStringAndClear());
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
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_x, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.Y, aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_y, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.Width, aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_width, sBuffer.makeStringAndClear());
    rUnitConv.convertMeasure(sBuffer, aRect.Height, aMapUnit);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_height, sBuffer.makeStringAndClear());
    SvXMLElementExport aVisAreaElem(rExport, XML_NAMESPACE_OFFICE, pName, sal_True, sal_True);
}

XMLVisAreaExport::~XMLVisAreaExport()
{
}


