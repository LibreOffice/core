/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLTextShapeImportHelper.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:06:33 $
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

#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif

#ifndef _XMLOFF_XMLTIMP_HXX_
#include <xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_TEXTIMP_HXX_
#include <xmloff/txtimp.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLANCHORTYPEPROPHDL_HXX
#include "XMLAnchorTypePropHdl.hxx"
#endif
#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGESUPPLIER_HPP_
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

#ifndef _XMLTEXTSHAPEIMPORTHELPER_HXX
#include "XMLTextShapeImportHelper.hxx"
#endif

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
