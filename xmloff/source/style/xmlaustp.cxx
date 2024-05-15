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

#include <com/sun/star/container/XIndexReplace.hpp>
#include "impastpl.hxx"
#include <xmloff/xmlaustp.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlprhdl.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/XMLTextListAutoStylePool.hxx>

#include <PageMasterStyleMap.hxx>
#include <osl/diagnose.h>


using namespace ::com::sun::star;
using namespace ::xmloff::token;


namespace
{
    void lcl_exportDataStyle( SvXMLExport& _rExport, const rtl::Reference< XMLPropertySetMapper >& _rxMapper,
        const XMLPropertyState& _rProperty )
    {
        assert(_rxMapper.is());
        // obtain the data style name
        OUString sDataStyleName;
        _rProperty.maValue >>= sDataStyleName;
        assert(!sDataStyleName.isEmpty() && "xmloff::lcl_exportDataStyle: invalid property value for the data style name!");

        // add the attribute
        _rExport.AddAttribute(
            _rxMapper->GetEntryNameSpace( _rProperty.mnIndex ),
            _rxMapper->GetEntryXMLName( _rProperty.mnIndex ),
            sDataStyleName );
    }
}

void SvXMLAutoStylePoolP::exportStyleAttributes(
        comphelper::AttributeList&,
        XmlStyleFamily nFamily,
        const std::vector< XMLPropertyState >& rProperties,
        const SvXMLExportPropertyMapper& rPropExp,
        const SvXMLUnitConverter&,
        const SvXMLNamespaceMap&
        ) const
{
    if ( XmlStyleFamily::CONTROL_ID == nFamily )
    {   // it's a control-related style
        const rtl::Reference< XMLPropertySetMapper >& aPropertyMapper = rPropExp.getPropertySetMapper();

        for (const auto& rProp : rProperties)
        {
            if  (   ( rProp.mnIndex > -1 )
                &&  ( CTF_FORMS_DATA_STYLE == aPropertyMapper->GetEntryContextId( rProp.mnIndex ) )
                )
            {   // it's the data-style for a grid column
                lcl_exportDataStyle( GetExport(), aPropertyMapper, rProp );
            }
        }
    }

    if( (XmlStyleFamily::SD_GRAPHICS_ID == nFamily) || (XmlStyleFamily::SD_PRESENTATION_ID == nFamily) )
    {   // it's a graphics style
        const rtl::Reference< XMLPropertySetMapper >& aPropertyMapper = rPropExp.getPropertySetMapper();
        assert(aPropertyMapper.is());

        bool bFoundControlShapeDataStyle = false;
        bool bFoundNumberingRulesName = false;

        for (const auto& rProp : rProperties)
        {
            if (rProp.mnIndex > -1)
            {   // it's a valid property
                switch( aPropertyMapper->GetEntryContextId(rProp.mnIndex) )
                {
                case CTF_SD_CONTROL_SHAPE_DATA_STYLE:
                    {   // it's the control shape data style property

                        if (bFoundControlShapeDataStyle)
                        {
                            OSL_FAIL("SvXMLAutoStylePoolP::exportStyleAttributes: found two properties with the ControlShapeDataStyle context id!");
                            // already added the attribute for the first occurrence
                            break;
                        }

                        lcl_exportDataStyle( GetExport(), aPropertyMapper, rProp );

                        // check if there is another property with the special context id we're handling here
                        bFoundControlShapeDataStyle = true;
                        break;
                    }
                case CTF_SD_NUMBERINGRULES_NAME:
                    {
                        if (bFoundNumberingRulesName)
                        {
                            OSL_FAIL("SvXMLAutoStylePoolP::exportStyleAttributes: found two properties with the numbering rules name context id!");
                            // already added the attribute for the first occurrence
                            break;
                        }

                        uno::Reference< container::XIndexReplace > xNumRule;
                        rProp.maValue >>= xNumRule;
                        if( xNumRule.is() && (xNumRule->getCount() > 0 ) )
                        {
                            const OUString sName(const_cast<XMLTextListAutoStylePool*>(&GetExport().GetTextParagraphExport()->GetListAutoStylePool())->Add( xNumRule ));

                            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_LIST_STYLE_NAME, GetExport().EncodeStyleName( sName ) );
                        }

                        bFoundNumberingRulesName = true;
                        break;
                    }
                }
            }
        }
    }

    if( nFamily != XmlStyleFamily::PAGE_MASTER )
        return;

    for( const auto& rProp : rProperties )
    {
        if (rProp.mnIndex > -1)
        {
            const rtl::Reference< XMLPropertySetMapper >& aPropMapper = rPropExp.getPropertySetMapper();
            sal_Int32 nIndex = rProp.mnIndex;
            sal_Int16 nContextID = aPropMapper->GetEntryContextId( nIndex );
            switch( nContextID )
            {
                case CTF_PM_PAGEUSAGE:
                {
                    OUString sValue;
                    const XMLPropertyHandler* pPropHdl = aPropMapper->GetPropertyHandler( nIndex );
                    if( pPropHdl &&
                        pPropHdl->exportXML( sValue, rProp.maValue,
                                             GetExport().GetMM100UnitConverter() ) &&
                        ( ! IsXMLToken( sValue, XML_ALL ) ) )
                    {
                        GetExport().AddAttribute( aPropMapper->GetEntryNameSpace( nIndex ), aPropMapper->GetEntryXMLName( nIndex ), sValue );
                    }
                }
                break;
            }
        }
    }
}

void SvXMLAutoStylePoolP::exportStyleContent(
        const css::uno::Reference< css::xml::sax::XDocumentHandler > &,
        XmlStyleFamily nFamily,
        const std::vector< XMLPropertyState >& rProperties,
        const SvXMLExportPropertyMapper& rPropExp,
        const SvXMLUnitConverter&,
        const SvXMLNamespaceMap&
        ) const
{
    if( nFamily != XmlStyleFamily::PAGE_MASTER )
        return;

    sal_Int32       nHeaderStartIndex(-1);
    sal_Int32       nHeaderEndIndex(-1);
    sal_Int32       nFooterStartIndex(-1);
    sal_Int32       nFooterEndIndex(-1);
    bool        bHeaderStartIndex(false);
    bool        bHeaderEndIndex(false);
    bool        bFooterStartIndex(false);
    bool        bFooterEndIndex(false);

    const rtl::Reference< XMLPropertySetMapper >& aPropMapper = rPropExp.getPropertySetMapper();

    sal_Int32 nIndex(0);
    while(nIndex < aPropMapper->GetEntryCount())
    {
        switch( aPropMapper->GetEntryContextId( nIndex ) & CTF_PM_FLAGMASK )
        {
            case CTF_PM_HEADERFLAG:
            {
                if (!bHeaderStartIndex)
                {
                    nHeaderStartIndex = nIndex;
                    bHeaderStartIndex = true;
                }
                if (bFooterStartIndex && !bFooterEndIndex)
                {
                    nFooterEndIndex = nIndex;
                    bFooterEndIndex = true;
                }
            }
            break;
            case CTF_PM_FOOTERFLAG:
            {
                if (!bFooterStartIndex)
                {
                    nFooterStartIndex = nIndex;
                    bFooterStartIndex = true;
                }
                if (bHeaderStartIndex && !bHeaderEndIndex)
                {
                    nHeaderEndIndex = nIndex;
                    bHeaderEndIndex = true;
                }
            }
            break;
        }
        nIndex++;
    }
    if (!bHeaderEndIndex)
        nHeaderEndIndex = nIndex;
    if (!bFooterEndIndex)
        nFooterEndIndex = nIndex;

    // export header style element
    {
        SvXMLElementExport aElem(
            GetExport(), XML_NAMESPACE_STYLE, XML_HEADER_STYLE,
            true, true );

        rPropExp.exportXML(
            GetExport(), rProperties,
            nHeaderStartIndex, nHeaderEndIndex, SvXmlExportFlags::IGN_WS);
    }

    // export footer style
    {
        SvXMLElementExport aElem(
            GetExport(), XML_NAMESPACE_STYLE, XML_FOOTER_STYLE,
            true, true );

        rPropExp.exportXML(
            GetExport(), rProperties,
            nFooterStartIndex, nFooterEndIndex, SvXmlExportFlags::IGN_WS);
    }

}

SvXMLAutoStylePoolP::SvXMLAutoStylePoolP( SvXMLExport& rExport )
    : m_pImpl( new SvXMLAutoStylePoolP_Impl( rExport ) )
{
}

SvXMLAutoStylePoolP::~SvXMLAutoStylePoolP()
{
}

SvXMLExport& SvXMLAutoStylePoolP::GetExport() const
{
    return m_pImpl->GetExport();
}

// TODO: remove this
void SvXMLAutoStylePoolP::AddFamily(
        XmlStyleFamily nFamily,
        const OUString& rStrName,
        SvXMLExportPropertyMapper* pMapper,
        const OUString& aStrPrefix )
{
    rtl::Reference <SvXMLExportPropertyMapper> xTmp = pMapper;
    AddFamily( nFamily, rStrName, xTmp, aStrPrefix );
}

void SvXMLAutoStylePoolP::AddFamily(
        XmlStyleFamily nFamily,
        const OUString& rStrName,
        const rtl::Reference < SvXMLExportPropertyMapper > & rMapper,
        const OUString& rStrPrefix,
        bool bAsFamily )
{
    m_pImpl->AddFamily( nFamily, rStrName, rMapper, rStrPrefix, bAsFamily );
}

void  SvXMLAutoStylePoolP::SetFamilyPropSetMapper(
        XmlStyleFamily nFamily,
        const rtl::Reference < SvXMLExportPropertyMapper > & rMapper )
{
    m_pImpl->SetFamilyPropSetMapper( nFamily, rMapper );
}

void SvXMLAutoStylePoolP::RegisterName( XmlStyleFamily nFamily,
                                         const OUString& rName )
{
    m_pImpl->RegisterName( nFamily, rName );
}

void SvXMLAutoStylePoolP::RegisterDefinedName( XmlStyleFamily nFamily,
                                         const OUString& rName )
{
    m_pImpl->RegisterDefinedName( nFamily, rName );
}

void SvXMLAutoStylePoolP::GetRegisteredNames(
    uno::Sequence<sal_Int32>& rFamilies,
    uno::Sequence<OUString>& rNames )
{
    m_pImpl->GetRegisteredNames( rFamilies, rNames );
}

void SvXMLAutoStylePoolP::RegisterNames(
    uno::Sequence<sal_Int32> const & aFamilies,
    uno::Sequence<OUString> const & aNames )
{
    assert(aFamilies.getLength() == aNames.getLength());

    // iterate over sequence(s) and call RegisterName(..) for each pair
    const sal_Int32* pFamilies = aFamilies.getConstArray();
    const OUString* pNames = aNames.getConstArray();
    sal_Int32 nCount = std::min( aFamilies.getLength(), aNames.getLength() );
    for( sal_Int32 n = 0; n < nCount; n++ )
        RegisterName( static_cast<XmlStyleFamily>(pFamilies[n]), pNames[n] );
}

OUString SvXMLAutoStylePoolP::Add( XmlStyleFamily nFamily,
                                   std::vector< XMLPropertyState >&& rProperties )
{
    OUString sName;
    m_pImpl->Add(sName, nFamily, u""_ustr, std::move(rProperties) );
    return sName;
}

OUString SvXMLAutoStylePoolP::Add( XmlStyleFamily nFamily,
                                  const OUString& rParent,
                                  std::vector< XMLPropertyState >&& rProperties, bool bDontSeek )
{
    OUString sName;
    m_pImpl->Add(sName, nFamily, rParent, std::move(rProperties), bDontSeek);
    return sName;
}

bool SvXMLAutoStylePoolP::Add(OUString& rName, XmlStyleFamily nFamily, const OUString& rParent, ::std::vector< XMLPropertyState >&& rProperties )
{
    return m_pImpl->Add(rName, nFamily, rParent, std::move(rProperties));
}

bool SvXMLAutoStylePoolP::AddNamed( const OUString& rName, XmlStyleFamily nFamily, const OUString& rParent,
                                    std::vector< XMLPropertyState > rProperties )

{
    return m_pImpl->AddNamed(rName, nFamily, rParent, std::move(rProperties));
}

OUString SvXMLAutoStylePoolP::Find( XmlStyleFamily nFamily,
                                   const OUString& rParent,
                                   const std::vector< XMLPropertyState >& rProperties ) const
{
    return m_pImpl->Find( nFamily, rParent, rProperties );
}

void SvXMLAutoStylePoolP::exportXML( XmlStyleFamily nFamily ) const
{
    m_pImpl->exportXML( nFamily, this );
}

void SvXMLAutoStylePoolP::ClearEntries()
{
    m_pImpl->ClearEntries();
}

std::vector<xmloff::AutoStyleEntry> SvXMLAutoStylePoolP::GetAutoStyleEntries() const
{
    return m_pImpl->GetAutoStyleEntries();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
