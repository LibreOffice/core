/*************************************************************************
 *
 *  $RCSfile: MarkerStyle.cxx,v $
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

#ifndef _XMLOFF_MARKERSTYLE_HXX
#include "MarkerStyle.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include"attrlist.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include"xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include"xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include<rtl/ustrbuf.hxx>
#endif

#ifndef _RTL_USTRING_
#include<rtl/ustring>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;

XMLMarkerStyle::XMLMarkerStyle( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & _rHandler,
                                        const SvXMLNamespaceMap& _rNamespaceMap, const SvXMLUnitConverter& _rUnitConverter )
: rHandler      ( _rHandler ),
  mrNamespaceMap ( _rNamespaceMap ),
  rUnitConverter( _rUnitConverter ),
  pAttrList     ( NULL )
{
}

XMLMarkerStyle::~XMLMarkerStyle()
{
}

void XMLMarkerStyle::AddAttribute( sal_uInt16 nPrefix, const sal_Char *pName, const OUString& rStrValue )
{
    const OUString aStrName( OUString::createFromAscii( pName ) );
    const OUString aStrCDATA( OUString::createFromAscii( sXML_CDATA ) );

    pAttrList->AddAttribute( mrNamespaceMap.GetQNameByKey( nPrefix, aStrName ), aStrCDATA, rStrValue );
}

sal_Bool XMLMarkerStyle::exportXML( const OUString& rStrName, const ::com::sun::star::uno::Any& rValue )
{
    return ImpExportXML( rHandler, mrNamespaceMap, rUnitConverter, rStrName, rValue );
}

sal_Bool XMLMarkerStyle::importXML( const uno::Reference< xml::sax::XAttributeList >& xAttrList, uno::Any& rValue, OUString& rStrName )
{
    return ImpImportXML( rUnitConverter, xAttrList, rValue, rStrName );
}

sal_Bool XMLMarkerStyle::ImpExportXML( const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
                                       const SvXMLNamespaceMap& rNamespaceMap, const SvXMLUnitConverter& rUnitConverter,
                                       const OUString& rStrName, const uno::Any& rValue )
{
    sal_Bool bRet = sal_False;
    drawing::PointSequence aPointSequence;

    if( rStrName.getLength() )
    {
        if( rValue >>= aPointSequence )
        {
            pAttrList = new SvXMLAttributeList();   // Do NOT delete me !!
            ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > xAttrList( pAttrList );

            OUString aStrValue;
            OUStringBuffer aOut;

            /////////////////
            // Name
            OUString aStrName( rStrName );
            AddAttribute( XML_NAMESPACE_DRAW, sXML_name, aStrName );

            /////////////////
            // Viewbox (viewBox="0 0 1500 1000")
            const awt::Point *pPoints = aPointSequence.getConstArray();
            sal_Int32 nPointCount = aPointSequence.getLength();
            sal_Int32 nMinX = 0, nMaxX = 0, nMinY = 0, nMaxY = 0;

            for( sal_Int32 i = 0; i<nPointCount; i++ )
            {
                const awt::Point aPoint = pPoints[i];

                if( aPoint.X < nMinX )
                    nMinX = aPoint.X;
                if( aPoint.X > nMaxX )
                    nMaxX = aPoint.X;
                if( aPoint.Y < nMinY )
                    nMinY = aPoint.Y;
                if( aPoint.Y > nMaxY )
                    nMaxY = aPoint.Y;
            }

            sal_Int32 nDifX = nMaxX - nMinX;
            sal_Int32 nDifY = nMaxY - nMinY;
            nDifX = nDifX >= 0 ? nDifX : -nDifX;
            nDifY = nDifY >= 0 ? nDifY : -nDifY;

            SdXMLImExViewBox aViewBox( 0, 0, nDifX, nDifY );
            AddAttribute( XML_NAMESPACE_SVG, sXML_viewBox, aViewBox.GetExportString( rUnitConverter ) );

            /////////////////
            // Pathdata
            SdXMLImExPointsElement aPoints( &aPointSequence, aViewBox, awt::Point( 0, 0 ),
                                            awt::Size( aViewBox.GetWidth(), aViewBox.GetHeight() ),
                                            rUnitConverter );

            AddAttribute( XML_NAMESPACE_SVG, sXML_d, aPoints.GetExportString() );

            /////////////////
            // Do Write
            OUString aStrTmp( RTL_CONSTASCII_USTRINGPARAM( sXML_marker ) );
            rHandler->startElement( rNamespaceMap.GetQNameByKey( XML_NAMESPACE_DRAW, aStrTmp ),
                                    xAttrList );
            rHandler->endElement( OUString::createFromAscii( sXML_marker ) );
        }
    }

    return bRet;
}

sal_Bool XMLMarkerStyle::ImpImportXML( const SvXMLUnitConverter& rUnitConverter,
                                           const uno::Reference< xml::sax::XAttributeList >& xAttrList,
                                           uno::Any& rValue, OUString& rStrName )
{
    sal_Bool bRet           = sal_False;
    sal_Bool bHasViewBox    = sal_False;
    sal_Bool bHasPathData   = sal_False;

    SdXMLImExViewBox* pViewBox = NULL;

    drawing::PointSequence aPointSequence;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString aStrFullAttrName = xAttrList->getNameByIndex( i );
        OUString aStrAttrName;
        mrNamespaceMap.GetKeyByAttrName( aStrFullAttrName, &aStrAttrName );
        OUString aStrValue = xAttrList->getValueByIndex( i );

        if( aStrAttrName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( sXML_name ) ) == 0 )
        {
            rStrName = aStrValue;

        } else if( aStrAttrName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( sXML_viewBox ) ) == 0 )
        {
            pViewBox = new SdXMLImExViewBox( aStrValue, rUnitConverter );
            bHasViewBox = sal_True;

        } else if( bHasViewBox && aStrAttrName.compareToAscii( RTL_CONSTASCII_STRINGPARAM( sXML_d ) ) == 0 )
        {
            SdXMLImExPointsElement aPoints( aStrValue, *pViewBox, awt::Point( 0, 0 ),
                                            awt::Size( pViewBox->GetWidth(), pViewBox->GetHeight() ),
                                            rUnitConverter );
            const drawing::PointSequenceSequence& rSequence = aPoints.GetPointSequenceSequence();
            const drawing::PointSequence *pSequences = rSequence.getConstArray();

            aPointSequence = drawing::PointSequence( pSequences[0] );
        }
    }

    if( pViewBox )
        delete pViewBox;

    rValue <<= aPointSequence;

    bRet = bHasViewBox && bHasPathData;

    return bRet;
}
