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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#ifndef _XMLOFF_VISAREACONTEXT_HXX
#include "VisAreaContext.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

//------------------------------------------------------------------

XMLVisAreaContext::XMLVisAreaContext( SvXMLImport& rImport,
                                         USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                            ::com::sun::star::awt::Rectangle& rRect, const sal_Int16 nMeasureUnit ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    process( xAttrList, rRect, nMeasureUnit );
}

XMLVisAreaContext::~XMLVisAreaContext()
{
}

void XMLVisAreaContext::process( const uno::Reference< xml::sax::XAttributeList>& xAttrList, awt::Rectangle& rRect, const sal_Int16 nMeasureUnit )
{
    MapUnit aMapUnit = (MapUnit)nMeasureUnit;

    sal_Int32 nX(0);
    sal_Int32 nY(0);
    sal_Int32 nWidth(0);
    sal_Int32 nHeight(0);
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_OFFICE)
        {
            if (IsXMLToken( aLocalName, XML_X ))
            {
                SvXMLUnitConverter::convertMeasure(nX, sValue, aMapUnit);
                rRect.X = nX;
            }
            else if (IsXMLToken( aLocalName, XML_Y ))
            {
                SvXMLUnitConverter::convertMeasure(nY, sValue, aMapUnit);
                rRect.Y = nY;
            }
            else if (IsXMLToken( aLocalName, XML_WIDTH ))
            {
                SvXMLUnitConverter::convertMeasure(nWidth, sValue, aMapUnit);
                rRect.Width = nWidth;
            }
            else if (IsXMLToken( aLocalName, XML_HEIGHT ))
            {
                SvXMLUnitConverter::convertMeasure(nHeight, sValue, aMapUnit);
                rRect.Height = nHeight;
            }
        }
    }
}

SvXMLImportContext *XMLVisAreaContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    // here is no context
    SvXMLImportContext *pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void XMLVisAreaContext::EndElement()
{
}
}//end of namespace binfilter
