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
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/txtimppr.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <comphelper/extract.hxx>
#include <xmloff/xmlprcon.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/XMLGraphicsDefaultStyle.hxx>
#include <tools/debug.hxx>
#include "xmlfilter.hxx"
#include "xmlHelper.hxx"
#include <osl/diagnose.h>

namespace rptxml
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace xmloff::token;

// -----------------------------------------------------------------------------

class OSpecialHanldeXMLImportPropertyMapper : public SvXMLImportPropertyMapper
{
public:
    OSpecialHanldeXMLImportPropertyMapper(const UniReference< XMLPropertySetMapper >& rMapper,SvXMLImport& _rImport) : SvXMLImportPropertyMapper(rMapper ,_rImport)
    {
    }
    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
    virtual bool handleSpecialItem(
            XMLPropertyState& /*rProperty*/,
            ::std::vector< XMLPropertyState >& /*rProperties*/,
            const OUString& /*rValue*/,
            const SvXMLUnitConverter& /*rUnitConverter*/,
            const SvXMLNamespaceMap& /*rNamespaceMap*/ ) const
    {
        // nothing to do here
        return sal_True;
    }
};

TYPEINIT1( OControlStyleContext, XMLPropStyleContext );
TYPEINIT1( OReportStylesContext, SvXMLStylesContext );
DBG_NAME( rpt_OControlStyleContext )

OControlStyleContext::OControlStyleContext( ORptFilter& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily, sal_Bool bDefaultStyle ) :
    XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily, bDefaultStyle ),
    sNumberFormat(OUString("NumberFormat")),
    pStyles(&rStyles),
    m_nNumberFormat(-1),
    m_rImport(rImport),
    bConditionalFormatCreated(sal_False),
    bParentSet(sal_False)
{
    DBG_CTOR( rpt_OControlStyleContext,NULL);

}
// -----------------------------------------------------------------------------

OControlStyleContext::~OControlStyleContext()
{

    DBG_DTOR( rpt_OControlStyleContext,NULL);
}
// -----------------------------------------------------------------------------

void OControlStyleContext::FillPropertySet(const Reference< XPropertySet > & rPropSet )
{
    if ( !IsDefaultStyle() )
    {
        if ( GetFamily() == XML_STYLE_FAMILY_TABLE_CELL )
        {
            if ((m_nNumberFormat == -1) && !m_sDataStyleName.isEmpty())
            {
                SvXMLNumFormatContext* pStyle = const_cast< SvXMLNumFormatContext*>(dynamic_cast<const SvXMLNumFormatContext*>(pStyles->FindStyleChildContext(
                    XML_STYLE_FAMILY_DATA_STYLE, m_sDataStyleName, sal_False)));
                if ( !pStyle )
                {
                    OReportStylesContext* pMyStyles = PTR_CAST(OReportStylesContext,GetOwnImport().GetAutoStyles());
                    if ( pMyStyles )
                        pStyle = PTR_CAST(SvXMLNumFormatContext,pMyStyles->
                            FindStyleChildContext(XML_STYLE_FAMILY_DATA_STYLE, m_sDataStyleName, sal_True));
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
// -----------------------------------------------------------------------------
void OControlStyleContext::SetDefaults()
{
}
// -----------------------------------------------------------------------------

void OControlStyleContext::AddProperty(const sal_Int16 nContextID, const uno::Any& rValue)
{
    sal_Int32 nIndex(static_cast<OReportStylesContext *>(pStyles)->GetIndex(nContextID));
    OSL_ENSURE(nIndex != -1, "Property not found in Map");
    XMLPropertyState aPropState(nIndex, rValue);
    GetProperties().push_back(aPropState); // has to be insertes in a sort order later
}
// -----------------------------------------------------------------------------
void OControlStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    // TODO: use a map here
    if( IsXMLToken(rLocalName, XML_DATA_STYLE_NAME ) )
        m_sDataStyleName = rValue;
    else if ( IsXMLToken(rLocalName, XML_MASTER_PAGE_NAME ) )
        sPageStyle = rValue;
    else
        XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
}
// -----------------------------------------------------------------------------
ORptFilter& OControlStyleContext::GetOwnImport() const
{
    return m_rImport;
}
// -----------------------------------------------------------------------------
DBG_NAME( rpt_OReportStylesContext )

OReportStylesContext::OReportStylesContext( ORptFilter& rImport,
        sal_uInt16 nPrfx ,
        const OUString& rLName ,
        const Reference< XAttributeList > & xAttrList,
        const sal_Bool bTempAutoStyles ) :
    SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList ),
    m_sTableStyleFamilyName( OUString( XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME )),
    m_sColumnStyleFamilyName( OUString( XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME )),
    m_sRowStyleFamilyName( OUString( XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME )),
    m_sCellStyleFamilyName( OUString( XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME )),
    m_rImport(rImport),
    m_nNumberFormatIndex(-1),
    bAutoStyles(bTempAutoStyles)
{
    DBG_CTOR( rpt_OReportStylesContext,NULL);

}
// -----------------------------------------------------------------------------

OReportStylesContext::~OReportStylesContext()
{

    DBG_DTOR( rpt_OReportStylesContext,NULL);
}
// -----------------------------------------------------------------------------

void OReportStylesContext::EndElement()
{
    SvXMLStylesContext::EndElement();
    if (bAutoStyles)
        GetImport().GetTextImport()->SetAutoStyles( this );
    else
        GetImport().GetStyles()->CopyStylesToDoc(sal_True);
}
// -----------------------------------------------------------------------------

UniReference < SvXMLImportPropertyMapper >
    OReportStylesContext::GetImportPropertyMapper(
                    sal_uInt16 nFamily ) const
{
    UniReference < SvXMLImportPropertyMapper > xMapper(SvXMLStylesContext::GetImportPropertyMapper(nFamily));

    if (!xMapper.is())
    {
        ORptFilter& rImport = GetOwnImport();
        switch( nFamily )
        {
            case XML_STYLE_FAMILY_TABLE_CELL:
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
            case XML_STYLE_FAMILY_TABLE_COLUMN:
            {
                if( !m_xColumnImpPropMapper.is() )
                    m_xColumnImpPropMapper =
                        new SvXMLImportPropertyMapper( rImport.GetColumnStylesPropertySetMapper(), m_rImport );

                xMapper = m_xColumnImpPropMapper;
            }
             break;
            case XML_STYLE_FAMILY_TABLE_ROW:
            {
                if( !m_xRowImpPropMapper.is() )
                    m_xRowImpPropMapper =new OSpecialHanldeXMLImportPropertyMapper( rImport.GetRowStylesPropertySetMapper(), m_rImport );
                xMapper = m_xRowImpPropMapper;
            }
             break;
            case XML_STYLE_FAMILY_TABLE_TABLE:
            {
                if( !m_xTableImpPropMapper.is() )
                {
                    UniReference < XMLPropertyHandlerFactory> xFac = new ::xmloff::OControlPropertyHandlerFactory();
                    m_xTableImpPropMapper = new SvXMLImportPropertyMapper( new XMLPropertySetMapper(OXMLHelper::GetTableStyleProps(), xFac), m_rImport );
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
// -----------------------------------------------------------------------------
SvXMLStyleContext *OReportStylesContext::CreateDefaultStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = 0;

    switch( nFamily )
    {
        case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
            // There are no writer specific defaults for graphic styles!
            pStyle = new XMLGraphicsDefaultStyle( GetImport(), nPrefix,
                                rLocalName, xAttrList, *this );
            break;
        default:
            pStyle = SvXMLStylesContext::CreateDefaultStyleStyleChildContext( nFamily,
                                                                       nPrefix,
                                                                rLocalName,
                                                                xAttrList );
            break;
    }
    return pStyle;
}
// ----------------------------------------------------------------------------
SvXMLStyleContext *OReportStylesContext::CreateStyleStyleChildContext(
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
        case XML_STYLE_FAMILY_TABLE_TABLE:
        case XML_STYLE_FAMILY_TABLE_COLUMN:
        case XML_STYLE_FAMILY_TABLE_ROW:
        case XML_STYLE_FAMILY_TABLE_CELL:
            pStyle = new OControlStyleContext( GetOwnImport(), nPrefix, rLocalName,
                                               xAttrList, *this, nFamily );
            break;
        default:
            OSL_FAIL("OReportStylesContext::CreateStyleStyleChildContext: Unknown style family. PLease check.");
            break;
        }
    }

    return pStyle;
}
// -----------------------------------------------------------------------------
Reference < XNameContainer >
        OReportStylesContext::GetStylesContainer( sal_uInt16 nFamily ) const
{
    Reference < XNameContainer > xStyles(SvXMLStylesContext::GetStylesContainer(nFamily));
    if (!xStyles.is())
    {
     OUString sName;
        switch( nFamily )
        {
            case XML_STYLE_FAMILY_TABLE_TABLE:
            {
                if( m_xTableStyles.is() )
                    xStyles.set(m_xTableStyles);
                else
                    sName =
                     OUString( OUString( "TableStyles" ));
            }
            break;
            case XML_STYLE_FAMILY_TABLE_CELL:
            {
                if( m_xCellStyles.is() )
                    xStyles.set(m_xCellStyles);
                else
                    sName =
                     OUString( OUString( "CellStyles" ));
            }
            break;
            case XML_STYLE_FAMILY_TABLE_COLUMN:
            {
                if( m_xColumnStyles.is() )
                    xStyles.set(m_xColumnStyles);
                else
                    sName =
                     OUString( OUString( "ColumnStyles" ));
            }
            break;
            case XML_STYLE_FAMILY_TABLE_ROW:
            {
                if( m_xRowStyles.is() )
                    xStyles.set(m_xRowStyles);
                else
                    sName =
                     OUString( OUString( "RowStyles" ));
            }
            break;
            case XML_STYLE_FAMILY_SD_GRAPHICS_ID:
                xStyles = ((SvXMLImport *)&GetImport())->GetTextImport()->GetFrameStyles();
                break;
            default:
                OSL_FAIL("OReportStylesContext::CreateStyleStyleChildContext: Unknown style family. PLease check.");
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
                case XML_STYLE_FAMILY_TABLE_TABLE:
                    m_xTableStyles.set(xStyles);
                    break;
                case XML_STYLE_FAMILY_TABLE_CELL:
                    m_xCellStyles.set(xStyles);
                    break;
                case XML_STYLE_FAMILY_TABLE_COLUMN:
                    m_xColumnStyles.set(xStyles);
                    break;
                case XML_STYLE_FAMILY_TABLE_ROW:
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
// -----------------------------------------------------------------------------

OUString OReportStylesContext::GetServiceName( sal_uInt16 nFamily ) const
{
    OUString sServiceName = SvXMLStylesContext::GetServiceName(nFamily);
    if (sServiceName.isEmpty())
    {
        switch( nFamily )
        {
            case XML_STYLE_FAMILY_TABLE_TABLE:
                sServiceName = m_sTableStyleFamilyName;
                break;
            case XML_STYLE_FAMILY_TABLE_COLUMN:
                sServiceName = m_sColumnStyleFamilyName;
                break;
            case XML_STYLE_FAMILY_TABLE_ROW:
                sServiceName = m_sRowStyleFamilyName;
                break;
            case XML_STYLE_FAMILY_TABLE_CELL:
                sServiceName = m_sCellStyleFamilyName;
                break;
            default:
                break;
        }
    }
    return sServiceName;
}
// -----------------------------------------------------------------------------

sal_Int32 OReportStylesContext::GetIndex(const sal_Int16 nContextID)
{
    if ( nContextID == CTF_RPT_NUMBERFORMAT )
    {
        if (m_nNumberFormatIndex == -1)
            m_nNumberFormatIndex =
                GetImportPropertyMapper(XML_STYLE_FAMILY_TABLE_CELL)->getPropertySetMapper()->FindEntryIndex(nContextID);
        return m_nNumberFormatIndex;
    }
    return -1;
}
// -----------------------------------------------------------------------------
ORptFilter& OReportStylesContext::GetOwnImport() const
{
    return m_rImport;
}
// -----------------------------------------------------------------------------
sal_uInt16 OReportStylesContext::GetFamily( const OUString& rFamily ) const
{
    sal_uInt16 nFamily = SvXMLStylesContext::GetFamily(rFamily);
    return nFamily;
}
// -----------------------------------------------------------------------------
} // rptxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
