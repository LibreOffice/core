/*************************************************************************
 *
 *  $RCSfile: ximpstyl.cxx,v $
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

#pragma hdrstop

#ifndef _XIMPSTYLE_HXX
#include "ximpstyl.hxx"
#endif

#ifndef _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
#include "XMLShapeStyleContext.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XIMPNOTES_HXX
#include "ximpnote.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XDRAWPAGES_HPP_
#include <com/sun/star/drawing/XDrawPages.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _XMLOFF_XMLPROPERTYSETCONTEXT_HXX
#include "xmlprcon.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPageMasterContext, SvXMLStyleContext );

SdXMLPageMasterContext::SdXMLPageMasterContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList),
    mnBorderBottom( 0L ),
    mnBorderLeft( 0L ),
    mnBorderRight( 0L ),
    mnBorderTop( 0L ),
    mnWidth( 0L ),
    mnHeight( 0L ),
    meOrientation(GetSdImport().IsDraw() ? view::PaperOrientation_PORTRAIT : view::PaperOrientation_LANDSCAPE)
{
    // set family to somethiong special at SvXMLStyleContext
    // for differences in search-methods
    SetFamily(XML_STYLE_FAMILY_SD_PAGEMASTERCONEXT_ID);

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetPageMasterAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_PAGEMASTER_NAME:
            {
                msName = sValue;
                break;
            }
            case XML_TOK_PAGEMASTER_MARGIN_TOP:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnBorderTop, sValue);
                break;
            }
            case XML_TOK_PAGEMASTER_MARGIN_BOTTOM:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnBorderBottom, sValue);
                break;
            }
            case XML_TOK_PAGEMASTER_MARGIN_LEFT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnBorderLeft, sValue);
                break;
            }
            case XML_TOK_PAGEMASTER_MARGIN_RIGHT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnBorderRight, sValue);
                break;
            }
            case XML_TOK_PAGEMASTER_PAGE_WIDTH:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_PAGEMASTER_PAGE_HEIGHT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
            case XML_TOK_PAGEMASTER_PAGE_ORIENTATION:
            {
                if(sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_orientation_portrait))))
                    meOrientation = view::PaperOrientation_PORTRAIT;
                else
                    meOrientation = view::PaperOrientation_LANDSCAPE;
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPageMasterContext::~SdXMLPageMasterContext()
{
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPresentationPageLayoutContext, SvXMLStyleContext );

SdXMLPresentationPageLayoutContext::SdXMLPresentationPageLayoutContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList)
:   SvXMLStyleContext(rImport, nPrfx, rLName, xAttrList),
    mnTypeId( 20 ) // AUTOLAYOUT_NONE
{
    // set family to somethiong special at SvXMLStyleContext
    // for differences in search-methods
    SetFamily(XML_STYLE_FAMILY_SD_PRESENTATIONPAGELAYOUT_ID);

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );

        if(nPrefix == XML_NAMESPACE_STYLE && aLocalName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_name))))
        {
            msName = xAttrList->getValueByIndex( i );
        }
    }
}

SdXMLPresentationPageLayoutContext::~SdXMLPresentationPageLayoutContext()
{
}

SvXMLImportContext *SdXMLPresentationPageLayoutContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    if(nPrefix == XML_NAMESPACE_PRESENTATION && rLocalName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_placeholder))))
    {
        // presentation:placeholder inside style:presentation-page-layout context
        pContext = new SdXMLPresentationPlaceholderContext(
            GetSdImport(), nPrefix, rLocalName, xAttrList);

        // remember SdXMLPresentationPlaceholderContext for later evaluation
        if(pContext)
        {
            pContext->AddRef();
            maList.Insert((SdXMLPresentationPlaceholderContext*)pContext, LIST_APPEND);
        }
    }

    // call base class
    if(!pContext)
        pContext = SvXMLStyleContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

void SdXMLPresentationPageLayoutContext::EndElement()
{
    // build presentation page layout type here
    // calc mnTpeId due to content of maList
    // at the moment only use number of types used there
    if(maList.Count())
    {
        SdXMLPresentationPlaceholderContext* pObj0 = maList.GetObject(0);
        if(pObj0->GetName().equals(OUString(RTL_CONSTASCII_USTRINGPARAM("handout"))))
        {
            if(maList.Count() == 1)
                mnTypeId = 22; // AUTOLAYOUT_HANDOUT1
            if(maList.Count() == 2)
                mnTypeId = 23; // AUTOLAYOUT_HANDOUT2
            if(maList.Count() == 3)
                mnTypeId = 24; // AUTOLAYOUT_HANDOUT3
            if(maList.Count() == 4)
                mnTypeId = 25; // AUTOLAYOUT_HANDOUT4
            else
                mnTypeId = 26; // AUTOLAYOUT_HANDOUT6
        }
        else
        {
            switch(maList.Count())
            {
                case 1:
                {
                    mnTypeId = 19; // AUTOLAYOUT_ONLY_TITLE
                    break;
                }
                case 2:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList.GetObject(1);

                    if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("subtitle"))))
                    {
                        mnTypeId = 0; // AUTOLAYOUT_TITLE
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("outline"))))
                    {
                        mnTypeId = 1; // AUTOLAYOUT_ENUM
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("chart"))))
                    {
                        mnTypeId = 2; // AUTOLAYOUT_CHART
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("table"))))
                    {
                        mnTypeId = 8; // AUTOLAYOUT_TAB
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("object"))))
                    {
                        mnTypeId = 11; // AUTOLAYOUT_OBJ
                    }
                    else
                    {
                        mnTypeId = 21; // AUTOLAYOUT_NOTES
                    }
                    break;
                }
                case 3:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList.GetObject(1);
                    SdXMLPresentationPlaceholderContext* pObj2 = maList.GetObject(2);

                    if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("outline"))))
                    {
                        if(pObj2->GetName().equals(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("outline"))))
                        {
                            mnTypeId = 3; // AUTOLAYOUT_2TEXT
                        }
                        else if(pObj2->GetName().equals(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("chart"))))
                        {
                            mnTypeId = 4; // AUTOLAYOUT_TEXTCHART
                        }
                        else if(pObj2->GetName().equals(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("chart"))))
                        {
                            mnTypeId = 6; // AUTOLAYOUT_TEXTCLIP
                        }
                        else
                        {
                            if(pObj1->GetX() < pObj2->GetX())
                            {
                                mnTypeId = 10; // AUTOLAYOUT_TEXTOBJ -> outline left, object right
                            }
                            else
                            {
                                mnTypeId = 17; // AUTOLAYOUT_TEXTOVEROBJ -> outline top, object right
                            }
                        }
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("chart"))))
                    {
                        mnTypeId = 7; // AUTOLAYOUT_CHARTTEXT
                    }
                    else if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("graphic"))))
                    {
                        mnTypeId = 9; // AUTOLAYOUT_CLIPTEXT
                    }
                    else
                    {
                        if(pObj1->GetX() < pObj2->GetX())
                        {
                            mnTypeId = 13; // AUTOLAYOUT_OBJTEXT -> left, right
                        }
                        else
                        {
                            mnTypeId = 14; // AUTOLAYOUT_OBJOVERTEXT -> top, bottom
                        }
                    }
                    break;
                }
                case 4:
                {
                    SdXMLPresentationPlaceholderContext* pObj1 = maList.GetObject(1);
                    SdXMLPresentationPlaceholderContext* pObj2 = maList.GetObject(2);

                    if(pObj1->GetName().equals(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("object"))))
                    {
                        if(pObj1->GetX() < pObj2->GetX())
                        {
                            mnTypeId = 16; // AUTOLAYOUT_2OBJOVERTEXT
                        }
                        else
                        {
                            mnTypeId = 15; // AUTOLAYOUT_2OBJTEXT
                        }
                    }
                    else
                    {
                        mnTypeId = 12; // AUTOLAYOUT_TEXT2OBJ
                    }
                    break;
                }
                default: // 5 items
                {
                    mnTypeId = 18; // AUTOLAYOUT_4OBJ
                    break;
                }
            }
        }

        // release remembered contexts, they are no longer needed
        while(maList.Count())
            maList.Remove(maList.Count() - 1)->ReleaseRef();
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

SdXMLPresentationPlaceholderContext::SdXMLPresentationPlaceholderContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx, const
    OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList)
:   SvXMLImportContext( rImport, nPrfx, rLName),
    mnX(0L),
    mnY(0L),
    mnWidth(1L),
    mnHeight(1L)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName(sAttrName, &aLocalName);
        OUString sValue = xAttrList->getValueByIndex(i);
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetPresentationPlaceholderAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_PRESENTATIONPLACEHOLDER_OBJECTNAME:
            {
                msName = sValue;
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_X:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnX, sValue);
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_Y:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnY, sValue);
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_WIDTH:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_PRESENTATIONPLACEHOLDER_HEIGHT:
            {
                GetSdImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPresentationPlaceholderContext::~SdXMLPresentationPlaceholderContext()
{
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLMasterPageContext, SdXMLGroupShapeContext );

SdXMLMasterPageContext::SdXMLMasterPageContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLGroupShapeContext( rImport, nPrfx, rLName, rShapes )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetSdImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetSdImport().GetMasterPageAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_MASTERPAGE_NAME:
            {
                msName = sValue;
                break;
            }
            case XML_TOK_MASTERPAGE_PAGE_MASTER_NAME:
            {
                msPageMasterName = sValue;
                break;
            }
        }
    }

    // set page name?
    if(msName.getLength() && GetLocalShapesContext().is())
    {
        uno::Reference < container::XNamed > xNamed(GetLocalShapesContext(), uno::UNO_QUERY);
        if(xNamed.is())
            xNamed->setName(msName);
    }

    // set page-master?
    if(msPageMasterName.getLength() && GetSdImport().GetShapeImport()->GetStylesContext())
    {
        // look for PageMaster with this name
        const SvXMLStyleContext* pStyle =
            GetSdImport().GetShapeImport()->GetStylesContext()->FindStyleChildContext(
            XML_STYLE_FAMILY_SD_PAGEMASTERCONEXT_ID, msPageMasterName);

        if(pStyle && pStyle->ISA(SdXMLPageMasterContext))
        {
            const SdXMLPageMasterContext* pPageMaster = (SdXMLPageMasterContext*)pStyle;

            uno::Reference< drawing::XDrawPage > xMasterPage(
                GetLocalShapesContext(), uno::UNO_QUERY);
            if(xMasterPage.is())
            {
                // set sizes for this masterpage
                uno::Reference <beans::XPropertySet> xPropSet(xMasterPage, uno::UNO_QUERY);
                if(xPropSet.is())
                {
                    uno::Any aAny;

                    aAny <<= pPageMaster->GetBorderBottom();
                    xPropSet->setPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("BorderBottom")), aAny);

                    aAny <<= pPageMaster->GetBorderLeft();
                    xPropSet->setPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("BorderLeft")), aAny);

                    aAny <<= pPageMaster->GetBorderRight();
                    xPropSet->setPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("BorderRight")), aAny);

                    aAny <<= pPageMaster->GetBorderTop();
                    xPropSet->setPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("BorderTop")), aAny);

                    aAny <<= pPageMaster->GetWidth();
                    xPropSet->setPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("Width")), aAny);

                    aAny <<= pPageMaster->GetHeight();
                    xPropSet->setPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("Height")), aAny);

                    aAny <<= pPageMaster->GetOrientation();
                    xPropSet->setPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("Orientation")), aAny);
                }
            }
        }
    }

    // now delete all up-to-now contained shapes.
    while(rShapes->getCount())
    {
        uno::Reference< drawing::XShape > xShape;
        uno::Any aAny(rShapes->getByIndex(0L));

        aAny >>= xShape;

        if(xShape.is())
        {
            rShapes->remove(xShape);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLMasterPageContext::~SdXMLMasterPageContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLMasterPageContext::EndElement()
{
    // set styles on master-page
    if(msName.getLength() && GetSdImport().GetShapeImport()->GetStylesContext())
    {
        SvXMLImportContext* pContext = GetSdImport().GetShapeImport()->GetStylesContext();
        if( pContext && pContext->ISA( SvXMLStyleContext ) )
            ((SdXMLStylesContext*)pContext)->SetMasterPageStyles(*this);
    }
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXMLMasterPageContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = GetSdImport().GetMasterPageElemTokenMap();

    // some special objects inside style:masterpage context
    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_MASTERPAGE_STYLE:
        {
            if(GetSdImport().GetShapeImport()->GetStylesContext())
            {
                // style:style inside master-page context -> presentation style
                XMLShapeStyleContext* pNew = new XMLShapeStyleContext(
                    GetSdImport(), nPrefix, rLocalName, xAttrList,
                    *GetSdImport().GetShapeImport()->GetStylesContext(),
                    XML_STYLE_FAMILY_SD_PRESENTATION_ID);

                // add this style to the outer StylesContext class for later processing
                if(pNew)
                {
                    pContext = pNew;
                    GetSdImport().GetShapeImport()->GetStylesContext()->AddStyle(*pNew);
                }
            }
            break;
        }
        case XML_TOK_MASTERPAGE_NOTES:
        {
            // presentation:notes inside master-page context
            pContext = new SdXMLNotesContext( GetSdImport(), nPrefix, rLocalName,
                GetLocalShapesContext());
            break;
        }
    }

    // call base class
    if(!pContext)
        pContext = SdXMLGroupShapeContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLStylesContext, SvXMLStyleContext );

SdXMLStylesContext::SdXMLStylesContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    sal_Bool bIsAutoStyle)
:   SvXMLStylesContext(rImport, nPrfx, rLName, xAttrList),
    mbIsAutoStyle(bIsAutoStyle)
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLStylesContext::~SdXMLStylesContext()
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLStyleContext* SdXMLStylesContext::CreateStyleChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList)
{
    SvXMLStyleContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = GetSdImport().GetStylesElemTokenMap();

    switch(rTokenMap.Get(nPrefix, rLocalName))
    {
        case XML_TOK_STYLES_PAGE_MASTER:
        {
            // style:page-master inside office:styles context
            pContext = new SdXMLPageMasterContext(GetSdImport(), nPrefix, rLocalName, xAttrList);
            break;
        }
        case XML_TOK_STYLES_PRESENTATION_PAGE_LAYOUT:
        {
            // style:presentation-page-layout inside office:styles context
            pContext = new SdXMLPresentationPageLayoutContext(GetSdImport(), nPrefix, rLocalName, xAttrList);
            break;
        }
    }

    // call base class
    if(!pContext)
        pContext = SvXMLStylesContext::CreateStyleChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

SvXMLStyleContext* SdXMLStylesContext::CreateStyleStyleChildContext(
    sal_uInt16 nFamily,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList)
{
    SvXMLStyleContext* pContext = 0;

//  if(XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily
//      || XML_STYLE_FAMILY_SD_PRESENTATION_ID == nFamily
//      || XML_STYLE_FAMILY_SD_POOL_ID == nFamily
//      || XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID == nFamily)
//  {
//      // style:style inside office:automatic-styles context
//      pContext = new XMLShapeStyleContext(
//          GetSdImport(), nPrefix, rLocalName, xAttrList, *this, nFamily);
//  }

    // call base class
    if(!pContext)
        pContext = SvXMLStylesContext::CreateStyleStyleChildContext(nFamily, nPrefix, rLocalName, xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

sal_uInt16 SdXMLStylesContext::GetFamily( const OUString& rFamily ) const
{
//  if(rFamily.getLength())
//  {
//      if(rFamily.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME))))
//          return XML_STYLE_FAMILY_SD_GRAPHICS_ID;
//
//      if(rFamily.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_PRESENTATION_NAME))))
//          return XML_STYLE_FAMILY_SD_PRESENTATION_ID;
//
//      if(rFamily.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_POOL_NAME))))
//          return XML_STYLE_FAMILY_SD_POOL_ID;
//
//      if(rFamily.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME))))
//          return XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID;
//  }

    // call base class
    return SvXMLStylesContext::GetFamily(rFamily);
}

//////////////////////////////////////////////////////////////////////////////

UniReference< SvXMLImportPropertyMapper > SdXMLStylesContext::GetImportPropertyMapper(
    sal_uInt16 nFamily) const
{
    UniReference < SvXMLImportPropertyMapper > xMapper;

//  if(XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily
//      || XML_STYLE_FAMILY_SD_PRESENTATION_ID == nFamily
//      || XML_STYLE_FAMILY_SD_POOL_ID == nFamily)
//  {
//      if(!xPropImpPropMapper.is())
//      {
//          UniReference< XMLShapeImportHelper > aImpHelper = ((SvXMLImport&)GetImport()).GetShapeImport();
//          ((SdXMLStylesContext*)this)->xPropImpPropMapper =
//              new SvXMLImportPropertyMapper(aImpHelper->GetPropertySetMapper());
//      }
//      xMapper = xPropImpPropMapper;
//      return xMapper;
//  }

    if(XML_STYLE_FAMILY_SD_DRAWINGPAGE_ID == nFamily)
    {
        if(!xPresImpPropMapper.is())
        {
            UniReference< XMLShapeImportHelper > aImpHelper = ((SvXMLImport&)GetImport()).GetShapeImport();
            ((SdXMLStylesContext*)this)->xPresImpPropMapper =
                new SvXMLImportPropertyMapper(aImpHelper->GetPresPagePropsMapper());
        }
        xMapper = xPresImpPropMapper;
        return xMapper;
    }

    // call base class
    return SvXMLStylesContext::GetImportPropertyMapper(nFamily);
}

//////////////////////////////////////////////////////////////////////////////
// Process all style and object info
//
void SdXMLStylesContext::EndElement()
{
    if(mbIsAutoStyle)
    {
        // associate AutoStyles with styles in preparation to setting Styles on shapes
        for(sal_uInt32 a(0L); a < GetStyleCount(); a++)
        {
            const SvXMLStyleContext* pStyle = GetStyle(a);
            if(pStyle && pStyle->ISA(XMLShapeStyleContext))
            {
                XMLShapeStyleContext* pDocStyle = (XMLShapeStyleContext*)pStyle;
                pStyle = GetSdImport().GetShapeImport()->GetStylesContext()->FindStyleChildContext(
                    pStyle->GetFamily(), pStyle->GetParent());

                if(pStyle && pStyle->ISA(XMLShapeStyleContext))
                {
                    XMLShapeStyleContext* pParentStyle = (XMLShapeStyleContext*)pStyle;
                    if(pParentStyle->GetStyle().is())
                    {
                        pDocStyle->SetStyle(pParentStyle->GetStyle());
                    }
                }
            }
        }

        // AutoStyles for text import
        GetImport().GetTextImport()->SetAutoStyles( this );

        // AutoStyles for chart
        GetImport().GetChartImport()->SetAutoStylesContext( this );
    }
    else
    {
        // find pool defaults and set on model
        const SvXMLStyleContext* pStyle = FindStyleChildContext(XML_STYLE_FAMILY_SD_POOL_ID,
            OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_drawpool)));
        if(pStyle)
            GetSdImport().ImportPoolDefaults((const XMLPropStyleContext*)pStyle);

        // Process styles list
        ImpSetGraphicStyles();
    }
}

//////////////////////////////////////////////////////////////////////////////
// set master-page styles (all with family="presentation" and a special
// prefix) on given master-page.
//
void SdXMLStylesContext::SetMasterPageStyles(SdXMLMasterPageContext& rMaster) const
{
    UniString sPrefix(rMaster.GetName(), rMaster.GetName().getLength());
    sPrefix += sal_Unicode('-');

    if(GetSdImport().GetLocalDocStyleFamilies()->hasByName(rMaster.GetName()))
    {
        uno::Any aAny(GetSdImport().GetLocalDocStyleFamilies()->getByName(rMaster.GetName()));
        uno::Reference< container::XNameAccess > xMasterPageStyles;
        aAny >>= xMasterPageStyles;

        if(xMasterPageStyles.is())
        {
            ImpSetGraphicStyles(xMasterPageStyles, XML_STYLE_FAMILY_SD_PRESENTATION_ID, sPrefix);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// Process styles list:
// set graphic styles (all with family="graphics"). Remember xStyle at list element.
//
void SdXMLStylesContext::ImpSetGraphicStyles() const
{
    if(GetSdImport().GetLocalDocStyleFamilies().is())
    {
        const OUString sGraphicStyleName(OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME)));
        uno::Any aAny(GetSdImport().GetLocalDocStyleFamilies()->getByName(sGraphicStyleName));
        uno::Reference< container::XNameAccess > xGraphicPageStyles;
        aAny >>= xGraphicPageStyles;

        if(xGraphicPageStyles.is())
        {
            UniString aPrefix;
            ImpSetGraphicStyles(xGraphicPageStyles, XML_STYLE_FAMILY_SD_GRAPHICS_ID, aPrefix);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// help function used by ImpSetGraphicStyles() and ImpSetMasterPageStyles()
//
void SdXMLStylesContext::ImpSetGraphicStyles(
    uno::Reference< container::XNameAccess >& xPageStyles,
    sal_uInt16 nFamily,
    const UniString& rPrefix) const
{
    xub_StrLen nPrefLen(rPrefix.Len());
    uno::Any aAny;

    // create all styles and set properties
    for(sal_uInt32 a(0L); a < GetStyleCount(); a++)
    {
        const SvXMLStyleContext* pStyle = GetStyle(a);

        if(nFamily == pStyle->GetFamily())
        {
            const UniString aStyleName(pStyle->GetName(), pStyle->GetName().getLength());
            if(!nPrefLen || aStyleName.Equals(rPrefix, 0, nPrefLen))
            {
                uno::Reference< style::XStyle > xStyle;
                const OUString aPureStyleName = nPrefLen ?
                    pStyle->GetName().copy((sal_Int32)nPrefLen) : pStyle->GetName();

                if(xPageStyles->hasByName(aPureStyleName))
                {
                    aAny = xPageStyles->getByName(aPureStyleName);
                    aAny >>= xStyle;
                }
                else
                {
                    // graphics style does not exist, create and add it
                    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetSdImport().GetModel(), uno::UNO_QUERY);
                    if(xServiceFact.is())
                    {
                        uno::Reference< style::XStyle > xNewStyle(
                            xServiceFact->createInstance(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.Style"))),
                            uno::UNO_QUERY);

                        if(xNewStyle.is())
                        {
                            // remember style
                            xStyle = xNewStyle;

                            // add new style to graphics style pool
                            uno::Reference< container::XNameContainer > xInsertContainer(xPageStyles, uno::UNO_QUERY);
                            if(xInsertContainer.is())
                            {
                                aAny <<= xStyle;
                                xInsertContainer->insertByName(aPureStyleName, aAny);
                            }
                        }
                    }
                }

                if(xStyle.is())
                {
                    // set properties at style
                    XMLShapeStyleContext* pPropStyle =
                        (pStyle->ISA(XMLShapeStyleContext)) ? (XMLShapeStyleContext*)pStyle : 0L;
                    uno::Reference< beans::XPropertySet > xPropSet(xStyle, uno::UNO_QUERY);

                    if(xPropSet.is() && pPropStyle)
                    {
                        pPropStyle->FillPropertySet(xPropSet);
                        pPropStyle->SetStyle(xStyle);
                    }
                }
            }
        }
    }

    // now set parents for all styles (when necessary)
    for(a = 0L; a < GetStyleCount(); a++)
    {
        const SvXMLStyleContext* pStyle = GetStyle(a);

        if(pStyle && nFamily == pStyle->GetFamily())
        {
            const UniString aStyleName(pStyle->GetName(), pStyle->GetName().getLength());
            if(!nPrefLen || aStyleName.Equals(rPrefix, 0, nPrefLen))
            {
                OUString aPureParentName;

                if(pStyle->GetParent().getLength())
                {
                    // no "drawpool" parents (!)
                    if(!pStyle->GetParent().equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_drawpool))))
                    {
                        aPureParentName = nPrefLen ?
                            pStyle->GetParent().copy((sal_Int32)nPrefLen) : pStyle->GetParent();
                    }
                }

                const OUString aPureStyleName = nPrefLen ?
                    pStyle->GetName().copy((sal_Int32)nPrefLen) : pStyle->GetName();
                uno::Reference< style::XStyle > xStyle;
                aAny = xPageStyles->getByName(aPureStyleName);
                aAny >>= xStyle;

                if(xStyle.is())
                {
                    // set parent style name
                    xStyle->setParentStyle(aPureParentName);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
TYPEINIT1( SdXMLMasterStylesContext, SvXMLImportContext );

SdXMLMasterStylesContext::SdXMLMasterStylesContext(
    SdXMLImport& rImport,
    sal_uInt16 nPrfx,
    const rtl::OUString& rLName)
:   SvXMLImportContext( rImport, nPrfx, rLName)
{
}

SdXMLMasterStylesContext::~SdXMLMasterStylesContext()
{
    while(maMasterPageList.Count())
        maMasterPageList.Remove(maMasterPageList.Count() - 1)->ReleaseRef();
}

SvXMLImportContext* SdXMLMasterStylesContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList)
{
    SvXMLImportContext* pContext = 0;

    if(nPrefix == XML_NAMESPACE_STYLE
        && rLocalName.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_master_page))))
    {
        // style:masterpage inside office:styles context
        uno::Reference< drawing::XDrawPage > xNewMasterPage;
        uno::Reference< drawing::XDrawPages > xMasterPages(GetSdImport().GetLocalMasterPages(), uno::UNO_QUERY);

        if(GetSdImport().GetNewMasterPageCount() + 1 > xMasterPages->getCount())
        {
            // new page, create and insert
            xNewMasterPage = xMasterPages->insertNewByIndex(xMasterPages->getCount());
        }
        else
        {
            // existing page, use it
            uno::Any aAny(xMasterPages->getByIndex(GetSdImport().GetNewMasterPageCount()));
            aAny >>= xNewMasterPage;
        }

        // increment global import page counter
        GetSdImport().IncrementNewMasterPageCount();

        if(xNewMasterPage.is())
        {
            uno::Reference< drawing::XShapes > xNewShapes(xNewMasterPage, uno::UNO_QUERY);
            if(xNewShapes.is() && GetSdImport().GetShapeImport()->GetStylesContext())
            {
                pContext = new SdXMLMasterPageContext(GetSdImport(),
                    nPrefix, rLocalName, xAttrList, xNewShapes);

                if(pContext)
                {
                    pContext->AddRef();
                    maMasterPageList.Insert((SdXMLMasterPageContext*)pContext, LIST_APPEND);
                }
            }
        }
    }

    // call base class
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}


