/*************************************************************************
 *
 *  $RCSfile: FillStyleContext.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:04 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _XMLOFF_FILLSTYLECONTEXTS_HXX_
#include "FillStyleContext.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_GRADIENTSTYLE_HXX
#include "GradientStyle.hxx"
#endif
#ifndef _XMLOFF_HATCHSTYLE_HXX
#include "HatchStyle.hxx"
#endif
#ifndef _XMLOFF_IMAGESTYLE_HXX
#include "ImageStyle.hxx"
#endif
#ifndef _XMLOFF_TRANSGRADIENTSTYLE_HXX
#include "TransGradientStyle.hxx"
#endif
#ifndef _XMLOFF_MARKERSTYLE_HXX
#include "MarkerStyle.hxx"
#endif
#ifndef _XMLOFF_DASHSTYLE_HXX
#include "DashStyle.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( XMLGradientStyleContext, SvXMLStyleContext );

XMLGradientStyleContext::XMLGradientStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                              const OUString& rLName,
                                              const uno::Reference< xml::sax::XAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList)
{
    // set Family
//  SetFamily( XML_STYLE_FAMILY_FILLSTYLE_GRADIENT_ID );

    // start import
    XMLGradientStyle aGradientStyle( NULL, GetImport().GetNamespaceMap(), GetImport().GetMM100UnitConverter() );
    aGradientStyle.importXML( xAttrList, maAny, maStrName );
}

XMLGradientStyleContext::~XMLGradientStyleContext()
{
}

void XMLGradientStyleContext::EndElement()
{
    uno::Reference< container::XNameContainer > xGradient( GetImport().GetGradientHelper() );

    try
    {
        if(xGradient.is())
            xGradient->insertByName( maStrName, maAny );
    }
    catch( container::ElementExistException& )
    {}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( XMLHatchStyleContext, SvXMLStyleContext );

XMLHatchStyleContext::XMLHatchStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                              const OUString& rLName,
                                              const uno::Reference< xml::sax::XAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList)
{
    // start import
    XMLHatchStyle aHatchStyle( NULL, GetImport().GetNamespaceMap(), GetImport().GetMM100UnitConverter() );
    aHatchStyle.importXML( xAttrList, maAny, maStrName );
}

XMLHatchStyleContext::~XMLHatchStyleContext()
{
}

void XMLHatchStyleContext::EndElement()
{
    uno::Reference< container::XNameContainer > xHatch( GetImport().GetHatchHelper() );

    try
    {
        if(xHatch.is())
            xHatch->insertByName( maStrName, maAny );
    }
    catch( container::ElementExistException& )
    {}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( XMLBitmapStyleContext, SvXMLStyleContext );

XMLBitmapStyleContext::XMLBitmapStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                              const OUString& rLName,
                                              const uno::Reference< xml::sax::XAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList)
{
    // start import
    XMLImageStyle aBitmapStyle( NULL, GetImport().GetNamespaceMap(), GetImport().GetMM100UnitConverter() );
    aBitmapStyle.importXML( xAttrList, maAny, maStrName );
}

XMLBitmapStyleContext::~XMLBitmapStyleContext()
{
}

void XMLBitmapStyleContext::EndElement()
{
    uno::Reference< container::XNameContainer > xBitmap( GetImport().GetBitmapHelper() );

    try
    {
        if(xBitmap.is())
            xBitmap->insertByName( maStrName, maAny );
    }
    catch( container::ElementExistException& )
    {}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( XMLTransGradientStyleContext, SvXMLStyleContext );

XMLTransGradientStyleContext::XMLTransGradientStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                              const OUString& rLName,
                                              const uno::Reference< xml::sax::XAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList)
{
    // start import
    XMLTransGradientStyle aTransGradientStyle( NULL, GetImport().GetNamespaceMap(), GetImport().GetMM100UnitConverter() );
    aTransGradientStyle.importXML( xAttrList, maAny, maStrName );
}

XMLTransGradientStyleContext::~XMLTransGradientStyleContext()
{
}

void XMLTransGradientStyleContext::EndElement()
{
    uno::Reference< container::XNameContainer > xTransGradient( GetImport().GetTransGradientHelper() );

    try
    {
        if(xTransGradient.is())
            xTransGradient->insertByName( maStrName, maAny );
    }
    catch( container::ElementExistException& )
    {}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( XMLMarkerStyleContext, SvXMLStyleContext );

XMLMarkerStyleContext::XMLMarkerStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                              const OUString& rLName,
                                              const uno::Reference< xml::sax::XAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList)
{
    // start import
    XMLMarkerStyle aMarkerStyle( NULL, GetImport().GetNamespaceMap(), GetImport().GetMM100UnitConverter() );
    aMarkerStyle.importXML( xAttrList, maAny, maStrName );
}

XMLMarkerStyleContext::~XMLMarkerStyleContext()
{
}

void XMLMarkerStyleContext::EndElement()
{
    uno::Reference< container::XNameContainer > xMarker( GetImport().GetMarkerHelper() );

    try
    {
        if(xMarker.is())
            xMarker->insertByName( maStrName, maAny );
    }
    catch( container::ElementExistException& )
    {}
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( XMLDashStyleContext, SvXMLStyleContext );

XMLDashStyleContext::XMLDashStyleContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                                          const OUString& rLName,
                                          const uno::Reference< xml::sax::XAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList)
{
    // start import
    XMLDashStyle aDashStyle( NULL, GetImport().GetNamespaceMap(), GetImport().GetMM100UnitConverter() );
    aDashStyle.importXML( xAttrList, maAny, maStrName );
}

XMLDashStyleContext::~XMLDashStyleContext()
{
}

void XMLDashStyleContext::EndElement()
{
    uno::Reference< container::XNameContainer > xDashes( GetImport().GetDashHelper() );

    try
    {
        if(xDashes.is())
            xDashes->insertByName( maStrName, maAny );
    }
    catch( container::ElementExistException& )
    {}
}
