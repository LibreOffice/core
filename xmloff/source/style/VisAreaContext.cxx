/*************************************************************************
 *
 *  $RCSfile: VisAreaContext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-16 14:20:19 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifndef _XMLOFF_VISAREACONTEXT_HXX
#include "VisAreaContext.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
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

using namespace com::sun::star;

//------------------------------------------------------------------

XMLVisAreaContext::XMLVisAreaContext( SvXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                                              Rectangle& rRect, const MapUnit aMapUnit ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    awt::Rectangle rAwtRect( rRect.getX(), rRect.getY(), rRect.getWidth(), rRect.getHeight() );
    process( xAttrList, rAwtRect, (sal_Int16)aMapUnit );

    rRect.setX( rAwtRect.X );
    rRect.setY( rAwtRect.Y );
    rRect.setWidth( rAwtRect.Width );
    rRect.setHeight( rAwtRect.Height );
}

XMLVisAreaContext::XMLVisAreaContext( SvXMLImport& rImport,
                                         USHORT nPrfx,
                                                   const rtl::OUString& rLName,
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
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_OFFICE)
        {
            if (aLocalName.compareToAscii(sXML_x) == 0)
            {
                SvXMLUnitConverter::convertMeasure(nX, sValue, aMapUnit);
                rRect.X = nX;
            }
            else if (aLocalName.compareToAscii(sXML_y) == 0)
            {
                SvXMLUnitConverter::convertMeasure(nY, sValue, aMapUnit);
                rRect.Y = nY;
            }
            else if (aLocalName.compareToAscii(sXML_width) == 0)
            {
                SvXMLUnitConverter::convertMeasure(nWidth, sValue, aMapUnit);
                rRect.Width = nWidth;
            }
            else if (aLocalName.compareToAscii(sXML_height) == 0)
            {
                SvXMLUnitConverter::convertMeasure(nHeight, sValue, aMapUnit);
                rRect.Height = nHeight;
            }
        }
    }
}

SvXMLImportContext *XMLVisAreaContext::CreateChildContext( USHORT nPrefix,
                                     const rtl::OUString& rLocalName,
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
