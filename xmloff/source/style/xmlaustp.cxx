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
#include <tools/debug.hxx>
#include "impastpl.hxx"
#include <xmloff/xmlaustp.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlprhdl.hxx>
#include <xmloff/XMLTextListAutoStylePool.hxx>

#include <xmloff/PageMasterStyleMap.hxx>
#include "PageMasterExportPropMapper.hxx"
#include "XMLBackgroundImageExport.hxx"


using namespace ::std;

using namespace ::com::sun::star;
using namespace ::xmloff::token;


namespace
{
    static void lcl_exportDataStyle( SvXMLExport& _rExport, const rtl::Reference< XMLPropertySetMapper >& _rxMapper,
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
        SvXMLAttributeList&,
        sal_Int32 nFamily,
        const vector< XMLPropertyState >& rProperties,
        const SvXMLExportPropertyMapper& rPropExp,
        const SvXMLUnitConverter&,
        const SvXMLNamespaceMap&
        ) const
{
    if ( XML_STYLE_FAMILY_CONTROL_ID == nFamily )
    {   // it's a control-related style
        rtl::Reference< XMLPropertySetMapper > aPropertyMapper = rPropExp.getPropertySetMapper();

        for (   vector< XMLPropertyState >::const_iterator pProp = rProperties.begin();
                pProp != rProperties.end();
                ++pProp
            )
        {
            if  (   ( pProp->mnIndex > -1 )
                &&  ( CTF_FORMS_DATA_STYLE == aPropertyMapper->GetEntryContextId( pProp->mnIndex ) )
                )
            {   // it's the data-style for a grid column
                lcl_exportDataStyle( GetExport(), aPropertyMapper, *pProp );
            }
        }
    }

    if( (XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily) || (XML_STYLE_FAMILY_SD_PRESENTATION_ID == nFamily) )
    {   // it's a graphics style
        rtl::Reference< XMLPropertySetMapper > aPropertyMapper = rPropExp.getPropertySetMapper();
        assert(aPropertyMapper.is());

        bool bFoundControlShapeDataStyle = false;
        bool bFoundNumberingRulesName = false;

        for (   vector< XMLPropertyState >::const_iterator pProp = rProperties.begin();
                pProp != rProperties.end();
                ++pProp
            )
        {
            if (pProp->mnIndex > -1)
            {   // it's a valid property
                switch( aPropertyMapper->GetEntryContextId(pProp->mnIndex) )
                {
                case CTF_SD_CONTROL_SHAPE_DATA_STYLE:
                    {   // it's the control shape data style property

                        if (bFoundControlShapeDataStyle)
                        {
                            OSL_FAIL("SvXMLAutoStylePoolP::exportStyleAttributes: found two properties with the ControlShapeDataStyle context id!");
                            // already added the attribute for the first occurrence
                            break;
                        }

                        lcl_exportDataStyle( GetExport(), aPropertyMapper, *pProp );

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
                        pProp->maValue >>= xNumRule;
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

    if( nFamily == XML_STYLE_FAMILY_PAGE_MASTER )
    {
        for( vector< XMLPropertyState >::const_iterator pProp = rProperties.begin(); pProp != rProperties.end(); ++pProp )
        {
            if (pProp->mnIndex > -1)
            {
                rtl::Reference< XMLPropertySetMapper > aPropMapper = rPropExp.getPropertySetMapper();
                sal_Int32 nIndex = pProp->mnIndex;
                sal_Int16 nContextID = aPropMapper->GetEntryContextId( nIndex );
                switch( nContextID )
                {
                    case CTF_PM_PAGEUSAGE:
                    {
                        OUString sValue;
                        const XMLPropertyHandler* pPropHdl = aPropMapper->GetPropertyHandler( nIndex );
                        if( pPropHdl &&
                            pPropHdl->exportXML( sValue, pProp->maValue,
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
}

void SvXMLAutoStylePoolP::exportStyleContent(
        const css::uno::Reference< css::xml::sax::XDocumentHandler > &,
        sal_Int32 nFamily,
        const vector< XMLPropertyState >& rProperties,
        const SvXMLExportPropertyMapper& rPropExp,
        const SvXMLUnitConverter&,
        const SvXMLNamespaceMap&
        ) const
{
    if( nFamily == XML_STYLE_FAMILY_PAGE_MASTER )
    {
        sal_Int32       nHeaderStartIndex(-1);
        sal_Int32       nHeaderEndIndex(-1);
        sal_Int32       nFooterStartIndex(-1);
        sal_Int32       nFooterEndIndex(-1);
        bool        bHeaderStartIndex(false);
        bool        bHeaderEndIndex(false);
        bool        bFooterStartIndex(false);
        bool        bFooterEndIndex(false);

        rtl::Reference< XMLPropertySetMapper > aPropMapper = rPropExp.getPropertySetMapper();

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
}

SvXMLAutoStylePoolP::SvXMLAutoStylePoolP( SvXMLExport& rExport )
    : pImpl( new SvXMLAutoStylePoolP_Impl( rExport ) )
{
}

SvXMLAutoStylePoolP::~SvXMLAutoStylePoolP()
{
}

SvXMLExport& SvXMLAutoStylePoolP::GetExport() const
{
    return pImpl->GetExport();
}

// TODO: remove this
void SvXMLAutoStylePoolP::AddFamily(
        sal_Int32 nFamily,
        const OUString& rStrName,
        SvXMLExportPropertyMapper* pMapper,
        const OUString& aStrPrefix,
        bool bAsFamily )
{
    rtl::Reference <SvXMLExportPropertyMapper> xTmp = pMapper;
    AddFamily( nFamily, rStrName, xTmp, aStrPrefix, bAsFamily );
}

void SvXMLAutoStylePoolP::AddFamily(
        sal_Int32 nFamily,
        const OUString& rStrName,
        const rtl::Reference < SvXMLExportPropertyMapper > & rMapper,
        const OUString& rStrPrefix,
        bool bAsFamily )
{
    pImpl->AddFamily( nFamily, rStrName, rMapper, rStrPrefix, bAsFamily );
}

void  SvXMLAutoStylePoolP::SetFamilyPropSetMapper(
        sal_Int32 nFamily,
        const rtl::Reference < SvXMLExportPropertyMapper > & rMapper )
{
    pImpl->SetFamilyPropSetMapper( nFamily, rMapper );
}

void SvXMLAutoStylePoolP::RegisterName( sal_Int32 nFamily,
                                         const OUString& rName )
{
    pImpl->RegisterName( nFamily, rName );
}

void SvXMLAutoStylePoolP::GetRegisteredNames(
    uno::Sequence<sal_Int32>& rFamilies,
    uno::Sequence<OUString>& rNames )
{
    pImpl->GetRegisteredNames( rFamilies, rNames );
}

void SvXMLAutoStylePoolP::RegisterNames(
    uno::Sequence<sal_Int32>& aFamilies,
    uno::Sequence<OUString>& aNames )
{
    assert(aFamilies.getLength() == aNames.getLength());

    // iterate over sequence(s) and call RegisterName(..) for each pair
    const sal_Int32* pFamilies = aFamilies.getConstArray();
    const OUString* pNames = aNames.getConstArray();
    sal_Int32 nCount = min( aFamilies.getLength(), aNames.getLength() );
    for( sal_Int32 n = 0; n < nCount; n++ )
        RegisterName( pFamilies[n], pNames[n] );
}

OUString SvXMLAutoStylePoolP::Add( sal_Int32 nFamily,
                                  const vector< XMLPropertyState >& rProperties )
{
    OUString sName;
    pImpl->Add(sName, nFamily, "", rProperties );
    return sName;
}

OUString SvXMLAutoStylePoolP::Add( sal_Int32 nFamily,
                                  const OUString& rParent,
                                  const vector< XMLPropertyState >& rProperties, bool bDontSeek )
{
    OUString sName;
    pImpl->Add(sName, nFamily, rParent, rProperties, bDontSeek);
    return sName;
}

bool SvXMLAutoStylePoolP::Add(OUString& rName, sal_Int32 nFamily, const OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties )
{
    return pImpl->Add(rName, nFamily, rParent, rProperties);
}

bool SvXMLAutoStylePoolP::AddNamed( const OUString& rName, sal_Int32 nFamily, const OUString& rParent,
                                        const ::std::vector< XMLPropertyState >& rProperties )

{
    return pImpl->AddNamed(rName, nFamily, rParent, rProperties);
}

OUString SvXMLAutoStylePoolP::Find( sal_Int32 nFamily,
                                   const OUString& rParent,
                                   const vector< XMLPropertyState >& rProperties ) const
{
    return pImpl->Find( nFamily, rParent, rProperties );
}

void SvXMLAutoStylePoolP::exportXML( sal_Int32 nFamily,
    const uno::Reference< css::xml::sax::XDocumentHandler > &,
    const SvXMLUnitConverter&,
    const SvXMLNamespaceMap&
    ) const
{
    pImpl->exportXML( nFamily,
                      GetExport().GetDocHandler(),
                      GetExport().GetMM100UnitConverter(),
                      GetExport().GetNamespaceMap(),
                      this);
}

void SvXMLAutoStylePoolP::ClearEntries()
{
    pImpl->ClearEntries();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
