/*************************************************************************
 *
 *  $RCSfile: XMLGraphicsDefaultStyle.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-04 23:07:53 $
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

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
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

#ifndef _XMLOFF_FAMILIES_HXX
#include "families.hxx"
#endif

#ifndef _XMLOFF_XMLSHAPEPROPERTYSETCONTEXT_HXX
#include "XMLShapePropertySetContext.hxx"
#endif

#ifndef _XMLOFF_XMLGRAPHICSDEFAULTSTYLE_HXX
#include "XMLGraphicsDefaultStyle.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;

// ---------------------------------------------------------------------

TYPEINIT1( XMLGraphicsDefaultStyle, XMLPropStyleContext );

XMLGraphicsDefaultStyle::XMLGraphicsDefaultStyle( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const Reference< XAttributeList >& xAttrList, SvXMLStylesContext& rStyles )
: XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, XML_STYLE_FAMILY_SD_GRAPHICS_ID, sal_True )
{
}

XMLGraphicsDefaultStyle::~XMLGraphicsDefaultStyle()
{
}

SvXMLImportContext *XMLGraphicsDefaultStyle::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix && rLocalName.compareToAscii( sXML_properties ) == 0 )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap = GetStyles()->GetImportPropertyMapper( GetFamily() );
        if( xImpPrMap.is() )
            pContext = new XMLShapePropertySetContext( GetImport(), nPrefix, rLocalName, xAttrList, GetProperties(), xImpPrMap );
    }

    if( !pContext )
        pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

// This method is called for every default style
void XMLGraphicsDefaultStyle::SetDefaults()
{
    Reference< XMultiServiceFactory > xFact( GetImport().GetModel(), UNO_QUERY );
    if( !xFact.is() )
        return;

    Reference< XPropertySet > xDefaults( xFact->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Defaults") ) ), UNO_QUERY );
    if( !xDefaults.is() )
        return;

    FillPropertySet( xDefaults );
}


