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
#include <com/sun/star/text/XTextContent.hpp>
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif

#ifndef _XMLOFF_XMLTIMP_HXX_
#include <xmloff/xmlimp.hxx>
#endif
#include <xmloff/txtimp.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/nmspmap.hxx>
#include "XMLAnchorTypePropHdl.hxx"
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include "xmloff/XMLTextShapeImportHelper.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml::sax;

XMLTextShapeImportHelper::XMLTextShapeImportHelper(
        SvXMLImport& rImp ) :
    XMLShapeImportHelper( rImp, rImp.GetModel(),
                          XMLTextImportHelper::CreateShapeExtPropMapper(rImp) ),
    rImport( rImp ),
    sAnchorType(RTL_CONSTASCII_USTRINGPARAM("AnchorType")),
    sAnchorPageNo(RTL_CONSTASCII_USTRINGPARAM("AnchorPageNo")),
    sVertOrientPosition(RTL_CONSTASCII_USTRINGPARAM("VertOrientPosition"))
{
    Reference < XDrawPageSupplier > xDPS( rImp.GetModel(), UNO_QUERY );
    if( xDPS.is() )
    {
         Reference < XShapes > xShapes( xDPS->getDrawPage(), UNO_QUERY );
        pushGroupForSorting( xShapes );
    }

}

XMLTextShapeImportHelper::~XMLTextShapeImportHelper()
{
    popGroupAndSort();
}

void XMLTextShapeImportHelper::addShape(
    Reference< XShape >& rShape,
    const Reference< XAttributeList >& xAttrList,
    Reference< XShapes >& rShapes )
{
    if( rShapes.is() )
    {
        // It's a group shape or 3DScene , so we have to call the base class method.
        XMLShapeImportHelper::addShape( rShape, xAttrList, rShapes );
        return;
    }

    TextContentAnchorType eAnchorType = TextContentAnchorType_AT_PARAGRAPH;
    sal_Int16   nPage = 0;
    sal_Int32   nY = 0;

    UniReference < XMLTextImportHelper > xTxtImport =
        rImport.GetTextImport();
    const SvXMLTokenMap& rTokenMap =
        xTxtImport->GetTextFrameAttrTokenMap();

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        OUString aLocalName;
        sal_uInt16 nPrefix =
            rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        switch( rTokenMap.Get( nPrefix, aLocalName ) )
        {
        case XML_TOK_TEXT_FRAME_ANCHOR_TYPE:
            {
                TextContentAnchorType eNew;
                // OD 2004-06-01 #i26791# - allow all anchor types
                if ( XMLAnchorTypePropHdl::convert( rValue, eNew ) )
                {
                    eAnchorType = eNew;
                }
            }
            break;
        case XML_TOK_TEXT_FRAME_ANCHOR_PAGE_NUMBER:
            {
                sal_Int32 nTmp;
                   if( rImport.GetMM100UnitConverter().
                                convertNumber( nTmp, rValue, 1, SHRT_MAX ) )
                    nPage = (sal_Int16)nTmp;
            }
            break;
        case XML_TOK_TEXT_FRAME_Y:
            rImport.GetMM100UnitConverter().convertMeasure( nY, rValue );
            break;
        }
    }

    Reference < XPropertySet > xPropSet( rShape, UNO_QUERY );
    Any aAny;

    // anchor type
    aAny <<= eAnchorType;
    xPropSet->setPropertyValue( sAnchorType, aAny );

    Reference < XTextContent > xTxtCntnt( rShape, UNO_QUERY );
    xTxtImport->InsertTextContent( xTxtCntnt );

    // page number (must be set after the frame is inserted, because it
    // will be overwritten then inserting the frame.
    switch( eAnchorType )
    {
    case TextContentAnchorType_AT_PAGE:
        // only set positive page numbers
        if ( nPage > 0 )
        {
            aAny <<= nPage;
            xPropSet->setPropertyValue( sAnchorPageNo, aAny );
        }
        break;
    case TextContentAnchorType_AS_CHARACTER:
        aAny <<= nY;
        xPropSet->setPropertyValue( sVertOrientPosition, aAny );
        break;
    default:
        break;
    }
}
