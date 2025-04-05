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

class OSpecialHandleXMLImportPropertyMapper : public SvXMLImportPropertyMapper
{
public:
    OSpecialHandleXMLImportPropertyMapper(const rtl::Reference< XMLPropertySetMapper >& rMapper,SvXMLImport& _rImport) : SvXMLImportPropertyMapper(rMapper ,_rImport)
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
        OReportStylesContext& rStyles, XmlStyleFamily nFamily ) :
    XMLPropStyleContext( rImport, rStyles, nFamily, false/*bDefaultStyle*/ ),
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
                    AddProperty(CTF_RPT_NUMBERFORMAT, uno::Any(m_nNumberFormat));
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
    sal_Int32 nIndex(pStyles->GetIndex(nContextID));
    OSL_ENSURE(nIndex != -1, "Property not found in Map");
    XMLPropertyState aPropState(nIndex, rValue);
    GetProperties().push_back(aPropState); // has to be inserted in a sort order later
}

void OControlStyleContext::SetAttribute( sal_Int32 nElement,
                                        const OUString& rValue )
{
    switch(nElement & TOKEN_MASK)
    {
        case XML_DATA_STYLE_NAME:
            m_sDataStyleName = rValue;
            break;
        case XML_MASTER_PAGE_NAME:
            break;
        default:
        XMLPropStyleContext::SetAttribute( nElement, rValue );
    }
}


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


void OReportStylesContext::endFastElement(sal_Int32 )
{
    if (bAutoStyles)
        GetImport().GetTextImport()->SetAutoStyles( this );
    else
        GetImport().GetStyles()->CopyStylesToDoc(true);
}


SvXMLImportPropertyMapper*
    OReportStylesContext::GetImportPropertyMapper(
                    XmlStyleFamily nFamily ) const
{
    SvXMLImportPropertyMapper* pMapper(SvXMLStylesContext::GetImportPropertyMapper(nFamily));

    if (!pMapper)
    {
        ORptFilter& rImport = GetOwnImport();
        switch( nFamily )
        {
            case XmlStyleFamily::TABLE_CELL:
            {
                if( !m_xCellImpPropMapper )
                {
                    m_xCellImpPropMapper =
                        std::make_unique<XMLTextImportPropertyMapper>/*OSpecialHandleXMLImportPropertyMapper*/( rImport.GetCellStylesPropertySetMapper(), m_rImport );

                    m_xCellImpPropMapper->ChainImportMapper(XMLTextImportHelper::CreateParaExtPropMapper(m_rImport));
                }
                pMapper = m_xCellImpPropMapper.get();
            }
            break;
            case XmlStyleFamily::TABLE_COLUMN:
            {
                if( !m_xColumnImpPropMapper )
                    m_xColumnImpPropMapper =
                        std::make_unique<SvXMLImportPropertyMapper>( rImport.GetColumnStylesPropertySetMapper(), m_rImport );

                pMapper = m_xColumnImpPropMapper.get();
            }
             break;
            case XmlStyleFamily::TABLE_ROW:
            {
                if( !m_xRowImpPropMapper )
                    m_xRowImpPropMapper = std::make_unique<OSpecialHandleXMLImportPropertyMapper>( rImport.GetRowStylesPropertySetMapper(), m_rImport );
                pMapper = m_xRowImpPropMapper.get();
            }
             break;
            case XmlStyleFamily::TABLE_TABLE:
            {
                if( !m_xTableImpPropMapper )
                {
                    rtl::Reference < XMLPropertyHandlerFactory> xFac = new ::xmloff::OControlPropertyHandlerFactory();
                    m_xTableImpPropMapper = std::make_unique<SvXMLImportPropertyMapper>( new XMLPropertySetMapper(OXMLHelper::GetTableStyleProps(), xFac, false), m_rImport );
                }
                pMapper = m_xTableImpPropMapper.get();
            }
             break;
            default:
                break;
        }
    }

    return pMapper;
}

SvXMLStyleContext *OReportStylesContext::CreateDefaultStyleStyleChildContext(
        XmlStyleFamily nFamily, sal_Int32 /*nElement*/,
        const uno::Reference< xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    switch( nFamily )
    {
        case XmlStyleFamily::SD_GRAPHICS_ID:
            // There are no writer specific defaults for graphic styles!
            return new XMLGraphicsDefaultStyle( GetImport(), *this );
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
            pStyle = new OControlStyleContext( GetOwnImport(), *this, nFamily );
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
                sServiceName = XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME;
                break;
            case XmlStyleFamily::TABLE_COLUMN:
                sServiceName = XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME;
                break;
            case XmlStyleFamily::TABLE_ROW:
                sServiceName = XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME;
                break;
            case XmlStyleFamily::TABLE_CELL:
                sServiceName = XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME;
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
