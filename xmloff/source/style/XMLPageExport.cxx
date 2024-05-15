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

#include <xmloff/XMLPageExport.hxx>
#include <o3tl/any.hxx>
#include <sal/log.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <xmloff/families.hxx>
#include <xmloff/xmlexp.hxx>
#include <PageMasterPropHdlFactory.hxx>
#include <PageMasterStyleMap.hxx>
#include <PageMasterPropMapper.hxx>
#include "PageMasterExportPropMapper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

constexpr OUString gsIsPhysical( u"IsPhysical"_ustr );
constexpr OUString gsFollowStyle( u"FollowStyle"_ustr );

namespace {

bool findPageMasterNameEntry(
        ::std::vector<XMLPageExportNameEntry> const& aNameVector,
        const OUString& rStyleName, XMLPageExportNameEntry & o_rEntry)
{
    auto pEntry = std::find_if(aNameVector.cbegin(), aNameVector.cend(),
        [&rStyleName](const XMLPageExportNameEntry& rEntry) { return rEntry.sStyleName == rStyleName; });

    if( pEntry != aNameVector.cend() )
    {
        o_rEntry = *pEntry;
        return true;
    }

    return false;
}

} // namespace

void XMLPageExport::collectPageMasterAutoStyle(
        const Reference < XPropertySet > & rPropSet,
        XMLPageExportNameEntry & rEntry)
{
    SAL_WARN_IF( !m_xPageMasterPropSetMapper.is(), "xmloff", "page master family/XMLPageMasterPropSetMapper not found" );
    if( m_xPageMasterPropSetMapper.is() )
    {
        ::std::vector<XMLPropertyState> aPropStates = m_xPageMasterExportPropMapper->Filter(m_rExport, rPropSet);
        if( !aPropStates.empty())
        {
            OUString sParent;
            rEntry.sPageMasterName = m_rExport.GetAutoStylePool()->Find( XmlStyleFamily::PAGE_MASTER, sParent, aPropStates );
            if (rEntry.sPageMasterName.isEmpty())
            {
                rEntry.sPageMasterName = m_rExport.GetAutoStylePool()->Add(XmlStyleFamily::PAGE_MASTER, sParent, std::move(aPropStates));
            }
        }
    }
    assert(m_xPageMasterDrawingPageExportPropMapper.is());
    ::std::vector<XMLPropertyState> aPropStates(
        m_xPageMasterDrawingPageExportPropMapper->Filter(m_rExport, rPropSet));
    if (!aPropStates.empty())
    {
        OUString sParent;
        rEntry.sDrawingPageStyleName = m_rExport.GetAutoStylePool()->Find(XmlStyleFamily::SD_DRAWINGPAGE_ID, sParent, aPropStates);
        if (rEntry.sDrawingPageStyleName.isEmpty())
        {
            rEntry.sDrawingPageStyleName = m_rExport.GetAutoStylePool()->Add(XmlStyleFamily::SD_DRAWINGPAGE_ID, sParent, std::move(aPropStates));
        }
    }
}

void XMLPageExport::exportMasterPageContent(
                const Reference < XPropertySet > &,
                bool /*bAutoStyles*/ )
{

}

bool XMLPageExport::exportStyle(
            const Reference< XStyle >& rStyle,
            bool bAutoStyles )
{
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    // Don't export styles that aren't existing really. This may be the
    // case for StarOffice Writer's pool styles.
    if( xPropSetInfo->hasPropertyByName( gsIsPhysical ) )
    {
        Any aAny = xPropSet->getPropertyValue( gsIsPhysical );
        if( !*o3tl::doAccess<bool>(aAny) )
            return false;
    }

    if( bAutoStyles )
    {
        XMLPageExportNameEntry aEntry;
        collectPageMasterAutoStyle(xPropSet, aEntry);
        aEntry.sStyleName = rStyle->getName();
        m_aNameVector.push_back( aEntry );

        exportMasterPageContent( xPropSet, true );
    }
    else
    {
        OUString sName( rStyle->getName() );
        bool bEncoded = false;
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
                          GetExport().EncodeStyleName( sName, &bEncoded ) );

        if ( xPropSetInfo->hasPropertyByName( u"Hidden"_ustr ) )
        {
            uno::Any aValue = xPropSet->getPropertyValue( u"Hidden"_ustr );
            bool bHidden = false;
            if ((aValue >>= bHidden) && bHidden
                && GetExport().getSaneDefaultVersion() & SvtSaveOptions::ODFSVER_EXTENDED)
            {
                GetExport().AddAttribute(XML_NAMESPACE_LO_EXT, XML_HIDDEN, u"true"_ustr);
                GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_HIDDEN, u"true"_ustr); // FIXME for compatibility
            }
        }

        if( bEncoded )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_DISPLAY_NAME,
                                   sName);

        XMLPageExportNameEntry entry;
        if (findPageMasterNameEntry(m_aNameVector, sName, entry))
        {
            GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT_NAME, GetExport().EncodeStyleName(entry.sPageMasterName));
            if (!entry.sDrawingPageStyleName.isEmpty())
            {
                GetExport().AddAttribute(XML_NAMESPACE_DRAW, XML_STYLE_NAME, GetExport().EncodeStyleName(entry.sDrawingPageStyleName));
            }
        }

        Reference<XPropertySetInfo> xInfo = xPropSet->getPropertySetInfo();
        if ( xInfo.is() && xInfo->hasPropertyByName(gsFollowStyle) )
        {
            OUString sNextName;
            xPropSet->getPropertyValue( gsFollowStyle ) >>= sNextName;

            if( sName != sNextName && !sNextName.isEmpty() )
            {
                GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NEXT_STYLE_NAME,
                    GetExport().EncodeStyleName( sNextName ) );
            }
        }

        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                  XML_MASTER_PAGE, true, true );

        exportMasterPageContent( xPropSet, false );
    }

    return true;
}

XMLPageExport::XMLPageExport(SvXMLExport & rExp)
    : m_rExport(rExp)
    , m_xPageMasterPropHdlFactory(new XMLPageMasterPropHdlFactory)
    , m_xPageMasterPropSetMapper(new XMLPageMasterPropSetMapper(
                                aXMLPageMasterStyleMap,
                                m_xPageMasterPropHdlFactory))
    , m_xPageMasterExportPropMapper(new XMLPageMasterExportPropMapper(
                                    m_xPageMasterPropSetMapper, rExp))
    , m_xPageMasterDrawingPagePropSetMapper(new XMLPageMasterPropSetMapper(
                                g_XMLPageMasterDrawingPageStyleMap,
                                m_xPageMasterPropHdlFactory))
      // use same class but with different map, need its ContextFilter()
    , m_xPageMasterDrawingPageExportPropMapper(new XMLPageMasterExportPropMapper(
                m_xPageMasterDrawingPagePropSetMapper, rExp))
{
    m_rExport.GetAutoStylePool()->AddFamily( XmlStyleFamily::PAGE_MASTER, XML_STYLE_FAMILY_PAGE_MASTER_NAME,
        m_xPageMasterExportPropMapper, XML_STYLE_FAMILY_PAGE_MASTER_PREFIX, false );
    m_rExport.GetAutoStylePool()->AddFamily(XmlStyleFamily::SD_DRAWINGPAGE_ID, XML_STYLE_FAMILY_SD_DRAWINGPAGE_NAME,
        m_xPageMasterDrawingPageExportPropMapper, XML_STYLE_FAMILY_SD_DRAWINGPAGE_PREFIX);

    Reference< XStyleFamiliesSupplier > xFamiliesSupp( GetExport().GetModel(),
                                                       UNO_QUERY );
    SAL_WARN_IF( !xFamiliesSupp.is(), "xmloff",
                "No XStyleFamiliesSupplier from XModel for export!" );
    if( !xFamiliesSupp.is() )
        return;

    Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
    SAL_WARN_IF( !xFamiliesSupp.is(), "xmloff",
                "getStyleFamilies() from XModel failed for export!" );
    if( xFamilies.is() )
    {
        static constexpr OUString aPageStyleName(u"PageStyles"_ustr);

        if( xFamilies->hasByName( aPageStyleName ) )
        {
            m_xPageStyles.set(xFamilies->getByName( aPageStyleName ),uno::UNO_QUERY);

            SAL_WARN_IF( !m_xPageStyles.is(), "xmloff",
                        "Page Styles not found for export!" );
        }
    }

    if (GetExport().GetModelType() != SvtModuleOptions::EFactory::WRITER)
        return;

    uno::Reference<lang::XMultiServiceFactory> xFac(GetExport().GetModel(), uno::UNO_QUERY);
    if (!xFac.is())
        return;

    uno::Reference<beans::XPropertySet> xProps(
        xFac->createInstance(u"com.sun.star.document.Settings"_ustr), uno::UNO_QUERY);
    if (!xProps.is())
        return;

    bool bGutterAtTop{};
    xProps->getPropertyValue(u"GutterAtTop"_ustr) >>= bGutterAtTop;
    if (bGutterAtTop)
    {
        m_xPageMasterExportPropMapper->SetGutterAtTop(true);
    }
}

XMLPageExport::~XMLPageExport()
{
}

void XMLPageExport::exportStyles( bool bUsed, bool bAutoStyles )
{
    if( m_xPageStyles.is() )
    {
        const uno::Sequence< OUString> aSeq = m_xPageStyles->getElementNames();
        for(const auto& rName : aSeq)
        {
            Reference< XStyle > xStyle(m_xPageStyles->getByName( rName ),uno::UNO_QUERY);
            if( !bUsed || xStyle->isInUse() )
                exportStyle( xStyle, bAutoStyles );
        }
    }
}

void XMLPageExport::exportAutoStyles()
{
    m_rExport.GetAutoStylePool()->exportXML(XmlStyleFamily::PAGE_MASTER);
    // tdf#103602 this is called by both Writer and Calc but Calc doesn't
    // have fill properties yet
    m_rExport.GetAutoStylePool()->exportXML(XmlStyleFamily::SD_DRAWINGPAGE_ID);
}

void XMLPageExport::exportDefaultStyle()
{
    Reference < lang::XMultiServiceFactory > xFactory (GetExport().GetModel(), UNO_QUERY);
    if (!xFactory.is())
        return;

    Reference < XPropertySet > xPropSet (xFactory->createInstance ( u"com.sun.star.text.Defaults"_ustr ), UNO_QUERY);
    if (!xPropSet.is())
        return;

    // <style:default-style ...>
    GetExport().CheckAttrList();

    ::std::vector< XMLPropertyState > aPropStates =
        m_xPageMasterExportPropMapper->FilterDefaults(m_rExport, xPropSet);

    bool bExport = false;
    rtl::Reference < XMLPropertySetMapper > aPropMapper(m_xPageMasterExportPropMapper->getPropertySetMapper());
    for( const auto& rProp : aPropStates )
    {
        sal_Int16 nContextId    = aPropMapper->GetEntryContextId( rProp.mnIndex );
        if( nContextId == CTF_PM_STANDARD_MODE )
        {
            bExport = true;
            break;
        }
    }

    if( !bExport )
        return;

    assert(GetExport().getSaneDefaultVersion()
            >= SvtSaveOptions::ODFSVER_012);

    //<style:default-page-layout>
    SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                              XML_DEFAULT_PAGE_LAYOUT,
                              true, true );

    m_xPageMasterExportPropMapper->exportXML( GetExport(), aPropStates,
                                 SvXmlExportFlags::IGN_WS );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
