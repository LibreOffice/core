/*************************************************************************
 *
 *  $RCSfile: xmlstyli.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sab $ $Date: 2000-09-25 13:40:03 $
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

#ifndef _XMLSTYLI_HXX
#include "xmlstyli.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif
#ifndef _XMLOFF_XMLIMPPR_HXX
#include <xmloff/xmlimppr.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_XMLNUMFI_HXX
#include <xmloff/xmlnumfi.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSHEETCONDITIONALENTRIES_HPP_
#include <com/sun/star/sheet/XSheetConditionalEntries.hpp>
#endif
#ifndef _XMLOFF_XMLPROPERTYSETCONTEXT_HXX
#include <xmloff/xmlprcon.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define SC_NUMBERFORMAT "NumberFormat"
#define SC_CONDITIONALFORMAT "ConditionalFormat"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
//using namespace ::com::sun::star::text;

ScXMLImportPropertyMapper::ScXMLImportPropertyMapper(
        const UniReference< XMLPropertySetMapper >& rMapper ) :
    SvXMLImportPropertyMapper( rMapper )
{
}

ScXMLImportPropertyMapper::~ScXMLImportPropertyMapper()
{
}

/*sal_Bool ScXMLImportPropertyMapper::handleSpecialItem(
        XMLPropertyState& rProperty,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    return sal_True;
}*/

/*sal_Bool ScXMLImportPropertyMapper::handleNoItem(
        sal_Int32 nIndex,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    return sal_True;
}*/

void ScXMLImportPropertyMapper::finished(::std::vector< XMLPropertyState >& rProperties ) const
{
    SvXMLImportPropertyMapper::finished(rProperties);
    XMLPropertyState* pAllPaddingProperty = NULL;
    XMLPropertyState* pAllBorderProperty = NULL;
    XMLPropertyState* pAllBorderWidthProperty = NULL;
    XMLPropertyState* pParaIndent = NULL;
    ::std::vector< XMLPropertyState >::iterator i = rProperties.begin();
    ::std::vector< XMLPropertyState >::iterator aParaIndentItr = rProperties.begin();
    for (i; i != rProperties.end(); i++)
    {
        sal_Int16 nContextID = getPropertySetMapper()->GetEntryContextId(i->mnIndex);
        switch (nContextID)
        {
            case CTF_ALLPADDING :
            {
                pAllPaddingProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
            case CTF_ALLBORDER :
            {
                pAllBorderProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
            case CTF_ALLBORDERWIDTH :
            {
                pAllBorderWidthProperty = new XMLPropertyState(i->mnIndex, i->maValue);
            }
            break;
            case CTF_PARAINDENT :
            {
                pParaIndent = new XMLPropertyState(i->mnIndex, i->maValue);
                aParaIndentItr = i;
            }
        }
    }
    if (pAllPaddingProperty)
    {
        sal_Int32 nIndex = pAllPaddingProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllPaddingProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
    if (pAllBorderProperty)
    {
        sal_Int32 nIndex = pAllBorderProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllBorderProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
    if (pAllBorderWidthProperty)
    {
        sal_Int32 nIndex = pAllBorderWidthProperty->mnIndex + 2;
        XMLPropertyState aNewProperty (nIndex, pAllBorderWidthProperty->maValue);
        for (sal_Int16 j = 0; j < 3; j++)
        {
            aNewProperty.mnIndex = nIndex++;
            rProperties.push_back(aNewProperty);
        }
    }
    if (pParaIndent)
    {
        sal_Int32 nValue;
        if (pParaIndent->maValue >>= nValue)
        {
            sal_Int16 n16Value = nValue;
            pParaIndent->maValue <<= n16Value;
            aParaIndentItr->mnIndex = pParaIndent->mnIndex;
            aParaIndentItr->maValue = pParaIndent->maValue;
        }
    }
}

class ScXMLMapContext : public SvXMLImportContext
{
    rtl::OUString sApplyStyle;
    rtl::OUString sCondition;
    rtl::OUString sBaseCell;
public:

    ScXMLMapContext(
            SvXMLImport& rImport, sal_uInt16 nPrfx,
            const rtl::OUString& rLName,
            const Reference< xml::sax::XAttributeList > & xAttrList );
    virtual ~ScXMLMapContext();

    const rtl::OUString& GetApplyStyle() const { return sApplyStyle; }
    const rtl::OUString& GetCondition() const { return sCondition; }
    const rtl::OUString& GetBaseCell() const { return sBaseCell; }
};

ScXMLMapContext::ScXMLMapContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName, const Reference< xml::sax::XAttributeList > & xAttrList )
    : SvXMLImportContext( rImport, nPrfx, rLName ),
    sCondition(),
    sApplyStyle(),
    sBaseCell()
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                            &aLocalName );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        // TODO: use a map here
        if( XML_NAMESPACE_STYLE == nPrefix )
        {
            if( aLocalName.compareToAscii( sXML_condition ) == 0 )
                sCondition = rValue;
            else if( aLocalName.compareToAscii( sXML_apply_style_name ) == 0 )
                sApplyStyle = rValue;
            else if (aLocalName.compareToAscii( sXML_base_cell_address ) == 0 )
                sBaseCell = rValue;
        }
    }
}

ScXMLMapContext::~ScXMLMapContext()
{
}

uno::Any XMLTableStyleContext::GetConditionalFormat(const uno::Any aAny,
        const rtl::OUString& sCondition,
        const rtl::OUString& sApplyStyle, const rtl::OUString& sBaseCell) const
{
    if (sCondition.getLength() && sApplyStyle.getLength())
    {
        uno::Reference<sheet::XSheetConditionalEntries> xConditionalEntries;
        if (aAny >>= xConditionalEntries)
        {
            uno::Sequence<beans::PropertyValue> aProps;
            if (sBaseCell.getLength())
            {
                /*aProps.realloc(aProps.getLength() + 1);
                beans::PropertyValue aProp;
                ScAddress aBase;
                aBase.Parse(sBaseCell, pDoc);
                table::CellAddress aBaseAddress;
                aBaseAddress.Column = aBase.Col();
                aBaseAddress.Row = aBase.Row();
                aBaseAddress.Sheet = aBase.Tab();
                uno::Any aAnyBase;
                aAnyBase <<= aBaseAddress;
                aProp.Value = aAnyBase;
                aProp.Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_UNONAME_SOURCEPOS));
                aProps[aProps.getLength() - 1] = aProp;*/
            }
        }
    }
    return aAny;
}

void XMLTableStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    // TODO: use a map here
    if( rLocalName.compareToAscii( sXML_data_style_name ) == 0 )
    {
        sDataStyleName = rValue;
    }
    else
    {
        XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}

TYPEINIT1( XMLTableStyleContext, XMLPropStyleContext );

XMLTableStyleContext::XMLTableStyleContext( ScXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles ) :
    XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles ),
    sNumberFormat(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat"))),
    sDataStyleName(),
    pStyles(&rStyles)
{
}

XMLTableStyleContext::~XMLTableStyleContext()
{
}

SvXMLImportContext *XMLTableStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                           xAttrList );

    if (!pContext)
    {
        if( XML_NAMESPACE_STYLE == nPrefix &&
            rLocalName.compareToAscii( sXML_map ) == 0 )
        {
            pContext = new ScXMLMapContext(GetImport(), nPrefix, rLocalName, xAttrList);
            aMaps.push_back((ScXMLMapContext*)pContext);
        }
    }

    return pContext;
}

void XMLTableStyleContext::FillPropertySet(
            const Reference< XPropertySet > & rPropSet )
{
    XMLPropStyleContext::FillPropertySet(rPropSet);
    if (sDataStyleName.len())
    {
        SvXMLNumFormatContext* pStyle = (SvXMLNumFormatContext *)pStyles->FindStyleChildContext(
            XML_STYLE_FAMILY_DATA_STYLE, sDataStyleName, sal_True);
        if (!pStyle)
        {
            XMLTableStylesContext* pMyStyles = (XMLTableStylesContext *)&GetScImport().GetStyles();
            pStyle = (SvXMLNumFormatContext *)pMyStyles->
                FindStyleChildContext(XML_STYLE_FAMILY_DATA_STYLE, sDataStyleName, sal_True);
        }
        if (pStyle)
        {
            uno::Any aNumberFormat;
            sal_Int32 nNumberFormat = pStyle->GetKey();
            aNumberFormat <<= nNumberFormat;
            rPropSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NUMBERFORMAT)), aNumberFormat);
        }
    }
    if (aMaps.size() > 0)
    {
        std::vector<ScXMLMapContext*>::iterator aItr = aMaps.begin();
        while(aItr != aMaps.end())
        {
            rtl::OUString sApplyStyle = (*aItr)->GetApplyStyle();
            rtl::OUString sCondition = (*aItr)->GetCondition();
            rtl::OUString sBaseCell = (*aItr)->GetBaseCell();
            uno::Any aConditionalFormat = rPropSet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_CONDITIONALFORMAT)));
            rPropSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_CONDITIONALFORMAT)),
                GetConditionalFormat(aConditionalFormat, sCondition, sApplyStyle, sBaseCell));
            aItr++;
        }
    }
}

void XMLTableStyleContext::CreateAndInsert( sal_Bool bOverwrite )
{
    XMLPropStyleContext::CreateAndInsert( bOverwrite );
}

void XMLTableStyleContext::CreateAndInsertLate( sal_Bool bOverwrite )
{
    XMLPropStyleContext::CreateAndInsertLate( bOverwrite );
}

void XMLTableStyleContext::Finish( sal_Bool bOverwrite )
{
    XMLPropStyleContext::Finish( bOverwrite );
    Reference < XStyle > xStyle = GetStyle();
    if( !xStyle.is() )
        return;

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();
    if( (xPropSetInfo->hasPropertyByName( sNumberFormat )) && sDataStyleName.len() )
    {
        Any aAny;
        SvXMLNumFormatContext* pStyle = (SvXMLNumFormatContext *)pStyles->FindStyleChildContext(
            XML_STYLE_FAMILY_DATA_STYLE, sDataStyleName, sal_False);
        if (pStyle)
        {
            sal_Int32 nNumberFormat = pStyle->GetKey();
            aAny <<= nNumberFormat;
            xPropSet->setPropertyValue( sNumberFormat, aAny );
        }
    }
}

// ----------------------------------------------------------------------------

SvXMLStyleContext *XMLTableStylesContext::CreateStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = SvXMLStylesContext::CreateStyleStyleChildContext( nFamily, nPrefix,
                                                            rLocalName,
                                                            xAttrList );
    if (!pStyle)
    {
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TABLE_CELL:
        case XML_STYLE_FAMILY_TABLE_PAGE_STYLES:
        case XML_STYLE_FAMILY_TABLE_COLUMN:
        case XML_STYLE_FAMILY_TABLE_ROW:
        case XML_STYLE_FAMILY_TABLE_TABLE:
            pStyle = new XMLTableStyleContext( GetScImport(), nPrefix, rLocalName,
                                               xAttrList, *this );
            break;
        }
    }

    return pStyle;
}

XMLTableStylesContext::XMLTableStylesContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx ,
        const OUString& rLName ,
        const Reference< XAttributeList > & xAttrList ) :
    SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList ),
    sCellStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.style.CellStyle" ) )),
    sPageStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.style.PageStyle" ) )),
    sColumnStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME ))),
    sRowStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME ))),
    sTableStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME )))
{
}

XMLTableStylesContext::~XMLTableStylesContext()
{
}

void XMLTableStylesContext::EndElement()
{
    SvXMLStylesContext::EndElement();
    GetImport().GetTextImport()->SetAutoStyles( this );
}

UniReference < SvXMLImportPropertyMapper >
    XMLTableStylesContext::GetImportPropertyMapper(
                    sal_uInt16 nFamily ) const
{
    UniReference < SvXMLImportPropertyMapper > xMapper =
        SvXMLStylesContext::GetImportPropertyMapper(nFamily);

    if (!xMapper.is())
    {
        switch( nFamily )
        {
            case XML_STYLE_FAMILY_TABLE_CELL:
            {
                if( !xCellImpPropMapper.is() )
                {
                    XMLCellStylesPropertySetMapper *pPropMapper = GetScImport().GetCellStylesPropertySetMapper();
                    ((XMLTableStylesContext *)this)->xCellImpPropMapper =
                        new ScXMLImportPropertyMapper( pPropMapper );
                }
                xMapper = xCellImpPropMapper;
            }
            break;
            case XML_STYLE_FAMILY_TABLE_PAGE_STYLES:
            {
                if( !xPageImpPropMapper.is() )
                {
                    XMLPageStylesPropertySetMapper *pPropMapper = GetScImport().GetPageStylesPropertySetMapper();
                    ((XMLTableStylesContext *)this)->xPageImpPropMapper =
                        new ScXMLImportPropertyMapper( pPropMapper );
                }
                xMapper = xPageImpPropMapper;
            }
             break;
            case XML_STYLE_FAMILY_TABLE_COLUMN:
            {
                if( !xColumnImpPropMapper.is() )
                {
                    XMLColumnStylesPropertySetMapper *pPropMapper = GetScImport().GetColumnStylesPropertySetMapper();
                    ((XMLTableStylesContext *)this)->xColumnImpPropMapper =
                        new ScXMLImportPropertyMapper( pPropMapper );
                }
                xMapper = xColumnImpPropMapper;
            }
             break;
            case XML_STYLE_FAMILY_TABLE_ROW:
            {
                if( !xRowImpPropMapper.is() )
                {
                    XMLRowStylesPropertySetMapper *pPropMapper = GetScImport().GetRowStylesPropertySetMapper();
                    ((XMLTableStylesContext *)this)->xRowImpPropMapper =
                        new ScXMLImportPropertyMapper( pPropMapper );
                }
                xMapper = xRowImpPropMapper;
            }
             break;
            case XML_STYLE_FAMILY_TABLE_TABLE:
            {
                if( !xTableImpPropMapper.is() )
                {
                    XMLTableStylesPropertySetMapper *pPropMapper = GetScImport().GetTableStylesPropertySetMapper();
                    ((XMLTableStylesContext *)this)->xTableImpPropMapper =
                        new ScXMLImportPropertyMapper( pPropMapper );
                }
                xMapper = xTableImpPropMapper;
            }
             break;
        }
    }

    return xMapper;
}

Reference < XNameContainer >
        XMLTableStylesContext::GetStylesContainer( sal_uInt16 nFamily ) const
{
    Reference < XNameContainer > xStyles = SvXMLStylesContext::GetStylesContainer(nFamily);
    if (!xStyles.is())
    {
        OUString sName;
        switch( nFamily )
        {
            case XML_STYLE_FAMILY_TABLE_TABLE:
            {
                if( xTableStyles.is() )
                    xStyles = xTableStyles;
                else
                    sName =
                        OUString( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TableStyles" ) ));
            }
            break;
            case XML_STYLE_FAMILY_TABLE_CELL:
            {
                if( xCellStyles.is() )
                    xStyles = xCellStyles;
                else
                    sName =
                        OUString( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "CellStyles" ) ));
            }
            break;
            case XML_STYLE_FAMILY_TABLE_PAGE_STYLES:
            {
                if( xPageStyles.is() )
                    xStyles = xPageStyles;
                else
                    sName =
                        OUString( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "PageStyles" ) ));
            }
            break;
            case XML_STYLE_FAMILY_TABLE_COLUMN:
            {
                if( xColumnStyles.is() )
                    xStyles = xColumnStyles;
                else
                    sName =
                        OUString( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ColumnStyles" ) ));
            }
            break;
            case XML_STYLE_FAMILY_TABLE_ROW:
            {
                if( xRowStyles.is() )
                    xStyles = xRowStyles;
                else
                    sName =
                        OUString( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "RowStyles" ) ));
            }
            break;
        }
        if( !xStyles.is() && sName.getLength() )
        {
            Reference< XStyleFamiliesSupplier > xFamiliesSupp(
                                            GetScImport().GetModel(), UNO_QUERY );
            if (xFamiliesSupp.is())
            {
                Reference< XNameAccess > xFamilies = xFamiliesSupp->getStyleFamilies();
                Any aAny = xFamilies->getByName( sName );

                xStyles = *(Reference<XNameContainer>*)aAny.getValue();
                switch( nFamily )
                {
                case XML_STYLE_FAMILY_TABLE_TABLE:
                    ((XMLTableStylesContext *)this)->xTableStyles = xStyles;
                    break;
                case XML_STYLE_FAMILY_TABLE_CELL:
                    ((XMLTableStylesContext *)this)->xCellStyles = xStyles;
                    break;
                case XML_STYLE_FAMILY_TABLE_PAGE_STYLES:
                    ((XMLTableStylesContext *)this)->xPageStyles = xStyles;
                    break;
                case XML_STYLE_FAMILY_TABLE_COLUMN:
                    ((XMLTableStylesContext *)this)->xColumnStyles = xStyles;
                case XML_STYLE_FAMILY_TABLE_ROW:
                    ((XMLTableStylesContext *)this)->xRowStyles = xStyles;
                }
            }
        }
    }

    return xStyles;
}

OUString XMLTableStylesContext::GetServiceName( sal_uInt16 nFamily ) const
{
    rtl::OUString sServiceName = SvXMLStylesContext::GetServiceName(nFamily);
    if (!sServiceName.len())
    {
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TABLE_PAGE_STYLES:
            sServiceName = sPageStyleServiceName;
            break;
        case XML_STYLE_FAMILY_TABLE_COLUMN:
            sServiceName = sColumnStyleServiceName;
            break;
        case XML_STYLE_FAMILY_TABLE_ROW:
            sServiceName = sRowStyleServiceName;
            break;
        case XML_STYLE_FAMILY_TABLE_CELL:
            sServiceName = sCellStyleServiceName;
            break;
        case XML_STYLE_FAMILY_TABLE_TABLE:
            sServiceName = sTableStyleServiceName;
            break;
        }
    }
    return sServiceName;
}



