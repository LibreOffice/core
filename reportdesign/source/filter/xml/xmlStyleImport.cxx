/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "xmlStyleImport.hxx"
#include <xmloff/maptype.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/txtimppr.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <xmloff/xmluconv.hxx>
#include <xmloff/XMLGraphicsDefaultStyle.hxx>
#include "xmlfilter.hxx"
#include "xmlHelper.hxx"
#include <osl/diagnose.h>

namespace rptxml
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace xmloff::token;

namespace {

class OSpecialHanldeXMLImportPropertyMapper : public SvXMLImportPropertyMapper
{
public:
    OSpecialHanldeXMLImportPropertyMapper(const rtl::Reference< XMLPropertySetMapper >& rMapper,SvXMLImport& _rImport) : SvXMLImportPropertyMapper(rMapper ,_rImport)
    {
    }
    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
    virtual bool handleSpecialItem(
            XMLPropertyState& /*rProperty*/,
            ::std::vector< XMLPropertyState >& /*rProperties*/,
            const OUString& /*rValue*/,
            const SvXMLUnitConverter& /*rUnitConverter*/,
            const SvXMLNamespaceMap& /*rNamespaceMap*/ ) const override
    {
        // nothing to do here
        return true;
    }
};

}

OControlStyleContext::OControlStyleContext( ORptFilter& rImport,
        sal_Int32 nElement,
        const Reference< XFastAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, XmlStyleFamily nFamily ) :
    XMLPropStyleContext( rImport, nElement, xAttrList, rStyles, nFamily, false/*bDefaultStyle*/ ),
    pStyles(&rStyles),
    m_nNumberFormat(-1),
    m_rImport(rImport)
{

}


OControlStyleContext::~OControlStyleContext()
{

}


void OControlStyleContext::FillPropertySet(const Reference< XPropertySet > & rPropSet )
{
    if ( !IsDefaultStyle() )
    {
        if ( GetFamily() == XmlStyleFamily::TABLE_CELL )
        {
            if ((m_nNumberFormat == -1) && !m_sDataStyleName.isEmpty())
            {
                SvXMLNumFormatContext* pStyle = const_cast< SvXMLNumFormatContext*>(dynamic_cast<const SvXMLNumFormatContext*>(pStyles->FindStyleChildContext(
                    XmlStyleFamily::DATA_STYLE, m_sDataStyleName)));
                if ( !pStyle )
                {
                    OReportStylesContext* pMyStyles = dynamic_cast< OReportStylesContext *>(m_rImport.GetAutoStyles());
                    if ( pMyStyles )
                        pStyle = const_cast<SvXMLNumFormatContext*>(dynamic_cast< const SvXMLNumFormatContext *>(pMyStyles->
                            FindStyleChildContext(XmlStyleFamily::DATA_STYLE, m_sDataStyleName, true)));
                    else {
                        OSL_FAIL("not possible to get style");
                    }
                }
                if ( pStyle )
                {
                    m_nNumberFormat = pStyle->GetKey();
                    AddProperty(CTF_RPT_NUMBERFORMAT, uno::makeAny(m_nNumberFormat));
                }
            }
        }
    }
    XMLPropStyleContext::FillPropertySet(rPropSet);
}

void OControlStyleContext::SetDefaults()
{
}


void OControlStyleContext::AddProperty(const sal_Int16 nContextID, const uno::Any& rValue)
{
    sal_Int32 nIndex(static_cast<OReportStylesContext *>(pStyles)->GetIndex(nContextID));
    OSL_ENSURE(nIndex != -1, "Property not found in Map");
    XMLPropertyState aPropState(nIndex, rValue);
    GetProperties().push_back(aPropState); // has to be inserted in a sort order later
}

void OControlStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    // TODO: use a map here
    if( IsXMLToken(rLocalName, XML_DATA_STYLE_NAME ) )
        m_sDataStyleName = rValue;
    else if ( IsXMLToken(rLocalName, XML_MASTER_PAGE_NAME ) )
        ;
    else
        XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
}


const OUStringLiteral g_sTableStyleFamilyName( u"" XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME );
const OUStringLiteral g_sColumnStyleFamilyName( u"" XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME );
const OUStringLiteral g_sRowStyleFamilyName( u"" XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME );
const OUStringLiteral g_sCellStyleFamilyName( u"" XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME );

OReportStylesContext::OReportStylesContext( ORptFilter& rImport,
        const bool bTempAutoStyles ) :
    SvXMLStylesContext( rImport ),
    m_rImport(rImport),
    m_nNumberFormatIndex(-1),
    bAutoStyles(bTempAutoStyles)
{

}


OReportStylesContext::~OReportStylesContext()
{

}


void OReportStylesContext::endFastElement(sal_Int32 nElement)
{
    SvXMLStylesContext::endFastElement(nElement);
    if (bAutoStyles)
        GetImport().GetTextImport()->SetAutoStyles( this );
    else
        GetImport().GetStyles()->CopyStylesToDoc(true);
}


rtl::Reference < SvXMLImportPropertyMapper >
    OReportStylesContext::GetImportPropertyMapper(
                    XmlStyleFamily nFamily ) const
{
    rtl::Reference < SvXMLImportPropertyMapper > xMapper(SvXMLStylesContext::GetImportPropertyMapper(nFamily));

    if (!xMapper.is())
    {
        ORptFilter& rImport = GetOwnImport();
        switch( nFamily )
        {
            case XmlStyleFamily::TABLE_CELL:
            {
                if( !m_xCellImpPropMapper.is() )
                {
                    m_xCellImpPropMapper =
                        new XMLTextImportPropertyMapper/*OSpecialHanldeXMLImportPropertyMapper*/( rImport.GetCellStylesPropertySetMapper(), m_rImport );

                    m_xCellImpPropMapper->ChainImportMapper(XMLTextImportHelper::CreateParaExtPropMapper(m_rImport));
                }
                xMapper = m_xCellImpPropMapper;
            }
            break;
            case XmlStyleFamily::TABLE_COLUMN:
            {
                if( !m_xColumnImpPropMapper.is() )
                    m_xColumnImpPropMapper =
                        new SvXMLImportPropertyMapper( rImport.GetColumnStylesPropertySetMapper(), m_rImport );

                xMapper = m_xColumnImpPropMapper;
            }
             break;
            case XmlStyleFamily::TABLE_ROW:
            {
                if( !m_xRowImpPropMapper.is() )
                    m_xRowImpPropMapper =new OSpecialHanldeXMLImportPropertyMapper( rImport.GetRowStylesPropertySetMapper(), m_rImport );
                xMapper = m_xRowImpPropMapper;
            }
             break;
            case XmlStyleFamily::TABLE_TABLE:
            {
                if( !m_xTableImpPropMapper.is() )
                {
                    rtl::Reference < XMLPropertyHandlerFactory> xFac = new ::xmloff::OControlPropertyHandlerFactory();
                    m_xTableImpPropMapper = new SvXMLImportPropertyMapper( new XMLPropertySetMapper(OXMLHelper::GetTableStyleProps(), xFac, false), m_rImport );
                }
                xMapper = m_xTableImpPropMapper;
            }
             break;
            default:
                break;
        }
    }

    return xMapper;
}

SvXMLStyleContext *OReportStylesContext::CreateDefaultStyleStyleChildContext(
        XmlStyleFamily nFamily, sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    switch( nFamily )
    {
        case XmlStyleFamily::SD_GRAPHICS_ID:
            // There are no writer specific defaults for graphic styles!
            return new XMLGraphicsDefaultStyle( GetImport(), nElement, xAttrList, *this );
        default:
            return nullptr;
    }
}

SvXMLStyleContext *OReportStylesContext::CreateStyleStyleChildContext(
        XmlStyleFamily nFamily, sal_Int32 nElement,
        const Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = SvXMLStylesContext::CreateStyleStyleChildContext( nFamily, nElement,
                                                            xAttrList );
    if (!pStyle)
    {
        switch( nFamily )
        {
        case XmlStyleFamily::TABLE_TABLE:
        case XmlStyleFamily::TABLE_COLUMN:
        case XmlStyleFamily::TABLE_ROW:
        case XmlStyleFamily::TABLE_CELL:
            pStyle = new OControlStyleContext( GetOwnImport(), nElement,
                                               xAttrList, *this, nFamily );
            break;
        default:
            OSL_FAIL("OReportStylesContext::CreateStyleStyleChildContext: Unknown style family. Please check.");
            break;
        }
    }

    return pStyle;
}

Reference < XNameContainer >
        OReportStylesContext::GetStylesContainer( XmlStyleFamily nFamily ) const
{
    Reference < XNameContainer > xStyles(SvXMLStylesContext::GetStylesContainer(nFamily));
    if (!xStyles.is())
    {
        OUString sName;
        switch( nFamily )
        {
            case XmlStyleFamily::TABLE_TABLE:
            {
                if( m_xTableStyles.is() )
                    xStyles.set(m_xTableStyles);
                else
                    sName = "TableStyles";
            }
            break;
            case XmlStyleFamily::TABLE_CELL:
            {
                if( m_xCellStyles.is() )
                    xStyles.set(m_xCellStyles);
                else
                    sName = "CellStyles";
            }
            break;
            case XmlStyleFamily::TABLE_COLUMN:
            {
                if( m_xColumnStyles.is() )
                    xStyles.set(m_xColumnStyles);
                else
                    sName = "ColumnStyles";
            }
            break;
            case XmlStyleFamily::TABLE_ROW:
            {
                if( m_xRowStyles.is() )
                    xStyles.set(m_xRowStyles);
                else
                    sName = "RowStyles";
            }
            break;
            case XmlStyleFamily::SD_GRAPHICS_ID:
                xStyles = const_cast<SvXMLImport *>(&GetImport())->GetTextImport()->GetFrameStyles();
                break;
            default:
                OSL_FAIL("OReportStylesContext::CreateStyleStyleChildContext: Unknown style family. Please check.");
                break;
        }
        if( !xStyles.is() && !sName.isEmpty() && GetOwnImport().GetModel().is() )
        {
            Reference< XStyleFamiliesSupplier > xFamiliesSupp(
                                            GetOwnImport().GetModel(), UNO_QUERY );
            if (xFamiliesSupp.is())
            {
                Reference< XNameAccess > xFamilies(xFamiliesSupp->getStyleFamilies());

                xStyles.set(xFamilies->getByName( sName ), uno::UNO_QUERY);
                switch( nFamily )
                {
                case XmlStyleFamily::TABLE_TABLE:
                    m_xTableStyles.set(xStyles);
                    break;
                case XmlStyleFamily::TABLE_CELL:
                    m_xCellStyles.set(xStyles);
                    break;
                case XmlStyleFamily::TABLE_COLUMN:
                    m_xColumnStyles.set(xStyles);
                    break;
                case XmlStyleFamily::TABLE_ROW:
                    m_xRowStyles.set(xStyles);
                    break;
                    default:
                        break;
                }
            }
        }
    }

    return xStyles;
}


OUString OReportStylesContext::GetServiceName( XmlStyleFamily nFamily ) const
{
    OUString sServiceName = SvXMLStylesContext::GetServiceName(nFamily);
    if (sServiceName.isEmpty())
    {
        switch( nFamily )
        {
            case XmlStyleFamily::TABLE_TABLE:
                sServiceName = g_sTableStyleFamilyName;
                break;
            case XmlStyleFamily::TABLE_COLUMN:
                sServiceName = g_sColumnStyleFamilyName;
                break;
            case XmlStyleFamily::TABLE_ROW:
                sServiceName = g_sRowStyleFamilyName;
                break;
            case XmlStyleFamily::TABLE_CELL:
                sServiceName = g_sCellStyleFamilyName;
                break;
            default:
                break;
        }
    }
    return sServiceName;
}


sal_Int32 OReportStylesContext::GetIndex(const sal_Int16 nContextID)
{
    if ( nContextID == CTF_RPT_NUMBERFORMAT )
    {
        if (m_nNumberFormatIndex == -1)
            m_nNumberFormatIndex =
                GetImportPropertyMapper(XmlStyleFamily::TABLE_CELL)->getPropertySetMapper()->FindEntryIndex(nContextID);
        return m_nNumberFormatIndex;
    }
    return -1;
}


} // rptxml


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
