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
#include <xmloff/families.hxx>
#include <xmloff/xmlnumfi.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <xmloff/xmlprcon.hxx>
#include <xmloff/xmluconv.hxx>
#include <osl/diagnose.h>
#include "xmlfilter.hxx"
#include "xmlHelper.hxx"

namespace dbaxml
{

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace xmloff::token;


OTableStyleContext::OTableStyleContext( ODBFilter& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily )
    :XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily, false )
    ,pStyles(&rStyles)
    ,m_nNumberFormat(-1)
{

}

OTableStyleContext::~OTableStyleContext()
{

}

void OTableStyleContext::FillPropertySet(
            const Reference< XPropertySet > & rPropSet )
{
    if ( !IsDefaultStyle() )
    {
        if ( GetFamily() == XML_STYLE_FAMILY_TABLE_TABLE )
        {
            if ( !sPageStyle.isEmpty() )
            {
                AddProperty(CTF_DB_MASTERPAGENAME, Any(sPageStyle));
            }
        }
        else if ( GetFamily() == XML_STYLE_FAMILY_TABLE_COLUMN )
        {
            if ((m_nNumberFormat == -1) && !m_sDataStyleName.isEmpty())
            {
                SvXMLNumFormatContext* pStyle = const_cast<SvXMLNumFormatContext*>(dynamic_cast< const SvXMLNumFormatContext* >(pStyles->FindStyleChildContext(
                    XML_STYLE_FAMILY_DATA_STYLE, m_sDataStyleName, true)));
                if ( !pStyle )
                {
                    OTableStylesContext* pMyStyles = dynamic_cast<OTableStylesContext* >(GetOwnImport().GetAutoStyles());
                    if ( pMyStyles )
                        pStyle = const_cast<SvXMLNumFormatContext*>(dynamic_cast< const SvXMLNumFormatContext* >(pMyStyles->
                            FindStyleChildContext(XML_STYLE_FAMILY_DATA_STYLE, m_sDataStyleName, true)));
                    else {
                        OSL_FAIL("not possible to get style");
                    }
                }
                if ( pStyle )
                {
                    uno::Any aNumberFormat;
                    m_nNumberFormat = pStyle->GetKey();
                    aNumberFormat <<= m_nNumberFormat;
                    AddProperty(CTF_DB_NUMBERFORMAT, aNumberFormat);
                }
            }
        }
    }
    XMLPropStyleContext::FillPropertySet(rPropSet);
}

void OTableStyleContext::SetDefaults()
{
}

void OTableStyleContext::AddProperty(const sal_Int16 nContextID, const uno::Any& rValue)
{
    sal_Int32 nIndex(static_cast<OTableStylesContext *>(pStyles)->GetIndex(nContextID));
    OSL_ENSURE(nIndex != -1, "Property not found in Map");
    XMLPropertyState aPropState(nIndex, rValue);
    GetProperties().push_back(aPropState); // has to be inserted in a sort order later
}

void OTableStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
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

ODBFilter& OTableStyleContext::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}


OTableStylesContext::OTableStylesContext( SvXMLImport& rImport,
                                          sal_uInt16 nPrfx ,
                                          const OUString& rLName ,
                                          const Reference< XAttributeList > & xAttrList,
                                          const bool bTempAutoStyles )
    : SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList )
    , m_nNumberFormatIndex(-1)
    , m_nMasterPageNameIndex(-1)
    , bAutoStyles(bTempAutoStyles)
{

}

OTableStylesContext::~OTableStylesContext()
{

}

void OTableStylesContext::EndElement()
{
    SvXMLStylesContext::EndElement();
    if (bAutoStyles)
        GetImport().GetTextImport()->SetAutoStyles( this );
    else
        GetImport().GetStyles()->CopyStylesToDoc(true);
}

rtl::Reference < SvXMLImportPropertyMapper >
    OTableStylesContext::GetImportPropertyMapper(
                    sal_uInt16 nFamily ) const
{
    rtl::Reference < SvXMLImportPropertyMapper > xMapper = SvXMLStylesContext::GetImportPropertyMapper(nFamily);

    if (!xMapper.is())
    {
        switch( nFamily )
        {
            case XML_STYLE_FAMILY_TABLE_TABLE:
            {
                if ( !m_xTableImpPropMapper.is() )
                    m_xTableImpPropMapper = new SvXMLImportPropertyMapper( const_cast<OTableStylesContext*>(this)->GetOwnImport().GetTableStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
                xMapper = m_xTableImpPropMapper;
            }
             break;
            case XML_STYLE_FAMILY_TABLE_COLUMN:
            {
                if ( !m_xColumnImpPropMapper.is() )
                    m_xColumnImpPropMapper = new SvXMLImportPropertyMapper( const_cast<OTableStylesContext*>(this)->GetOwnImport().GetColumnStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
                xMapper = m_xColumnImpPropMapper;
            }
             break;
            case XML_STYLE_FAMILY_TABLE_CELL:
            {
                if ( !m_xCellImpPropMapper.is() )
                    m_xCellImpPropMapper = new SvXMLImportPropertyMapper( const_cast<OTableStylesContext*>(this)->GetOwnImport().GetCellStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
                xMapper = m_xCellImpPropMapper;
            }
             break;
        }
    }

    return xMapper;
}

SvXMLStyleContext *OTableStylesContext::CreateStyleStyleChildContext(
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
        case XML_STYLE_FAMILY_TABLE_CELL:
            pStyle = new OTableStyleContext( GetOwnImport(), nPrefix, rLocalName,
                                               xAttrList, *this, nFamily );
            break;
        }
    }

    return pStyle;
}

OUString OTableStylesContext::GetServiceName( sal_uInt16 nFamily ) const
{
    OUString sServiceName = SvXMLStylesContext::GetServiceName(nFamily);
    if (sServiceName.isEmpty())
    {
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TABLE_TABLE:
            sServiceName = XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME;
            break;
        case XML_STYLE_FAMILY_TABLE_COLUMN:
            sServiceName = XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME;
            break;
        case XML_STYLE_FAMILY_TABLE_CELL:
            sServiceName = XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME;
            break;

        }
    }
    return sServiceName;
}

sal_Int32 OTableStylesContext::GetIndex(const sal_Int16 nContextID)
{
    if ( nContextID == CTF_DB_NUMBERFORMAT )
    {
        if (m_nNumberFormatIndex == -1)
            m_nNumberFormatIndex =
                GetImportPropertyMapper(XML_STYLE_FAMILY_TABLE_COLUMN)->getPropertySetMapper()->FindEntryIndex(nContextID);
        return m_nNumberFormatIndex;
    }
    else if ( nContextID == CTF_DB_MASTERPAGENAME )
    {
        if (m_nMasterPageNameIndex == -1)
            m_nMasterPageNameIndex =
                GetImportPropertyMapper(XML_STYLE_FAMILY_TABLE_TABLE)->getPropertySetMapper()->FindEntryIndex(nContextID);
        return m_nMasterPageNameIndex;
    }
    else
        return -1;
}

ODBFilter& OTableStylesContext::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}

} // dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
