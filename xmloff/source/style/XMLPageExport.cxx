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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <xmloff/families.hxx>
#include <xmloff/xmlexp.hxx>
#include "PageMasterPropHdlFactory.hxx"
#include <xmloff/PageMasterStyleMap.hxx>
#include "PageMasterPropMapper.hxx"
#include "PageMasterExportPropMapper.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

static const OUStringLiteral gsIsPhysical( "IsPhysical" );
static const OUStringLiteral gsFollowStyle( "FollowStyle" );

bool XMLPageExport::findPageMasterName( const OUString& rStyleName, OUString& rPMName ) const
{
    auto pEntry = std::find_if(aNameVector.cbegin(), aNameVector.cend(),
        [&rStyleName](const XMLPageExportNameEntry& rEntry) { return rEntry.sStyleName == rStyleName; });

    if( pEntry != aNameVector.cend() )
    {
        rPMName = pEntry->sPageMasterName;
        return true;
    }

    return false;
}

void XMLPageExport::collectPageMasterAutoStyle(
        const Reference < XPropertySet > & rPropSet,
        OUString& rPageMasterName )
{
    SAL_WARN_IF( !xPageMasterPropSetMapper.is(), "xmloff", "page master family/XMLPageMasterPropSetMapper not found" );
    if( xPageMasterPropSetMapper.is() )
    {
        ::std::vector<XMLPropertyState> aPropStates = xPageMasterExportPropMapper->Filter( rPropSet );
        if( !aPropStates.empty())
        {
            OUString sParent;
            rPageMasterName = rExport.GetAutoStylePool()->Find( XML_STYLE_FAMILY_PAGE_MASTER, sParent, aPropStates );
            if (rPageMasterName.isEmpty())
                rPageMasterName = rExport.GetAutoStylePool()->Add(XML_STYLE_FAMILY_PAGE_MASTER, sParent, aPropStates);
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
        collectPageMasterAutoStyle( xPropSet, aEntry.sPageMasterName );
        aEntry.sStyleName = rStyle->getName();
        aNameVector.push_back( aEntry );

        exportMasterPageContent( xPropSet, true );
    }
    else
    {
        OUString sName( rStyle->getName() );
        bool bEncoded = false;
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
                          GetExport().EncodeStyleName( sName, &bEncoded ) );

        if ( xPropSetInfo->hasPropertyByName( "Hidden" ) )
        {
            uno::Any aValue = xPropSet->getPropertyValue( "Hidden" );
            bool bHidden = false;
            if ( ( aValue >>= bHidden ) && bHidden && GetExport( ).getDefaultVersion( ) == SvtSaveOptions::ODFVER_LATEST )
                GetExport( ).AddAttribute( XML_NAMESPACE_STYLE, XML_HIDDEN, "true" );
        }

        if( bEncoded )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_DISPLAY_NAME,
                                   sName);

        OUString sPMName;
        if( findPageMasterName( sName, sPMName ) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_PAGE_LAYOUT_NAME, GetExport().EncodeStyleName( sPMName ) );

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

XMLPageExport::XMLPageExport( SvXMLExport& rExp ) :
    rExport( rExp )
{
    xPageMasterPropHdlFactory = new XMLPageMasterPropHdlFactory;
    xPageMasterPropSetMapper = new XMLPageMasterPropSetMapper(
                                aXMLPageMasterStyleMap,
                                xPageMasterPropHdlFactory );
    xPageMasterExportPropMapper = new XMLPageMasterExportPropMapper(
                                    xPageMasterPropSetMapper, rExp);

    rExport.GetAutoStylePool()->AddFamily( XML_STYLE_FAMILY_PAGE_MASTER, XML_STYLE_FAMILY_PAGE_MASTER_NAME,
        xPageMasterExportPropMapper, XML_STYLE_FAMILY_PAGE_MASTER_PREFIX, false );

    Reference< XStyleFamiliesSupplier > xFamiliesSupp( GetExport().GetModel(),
                                                       UNO_QUERY );
    SAL_WARN_IF( !xFamiliesSupp.is(), "xmloff",
                "No XStyleFamiliesSupplier from XModel for export!" );
    if( xFamiliesSupp.is() )
    {
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
        SAL_WARN_IF( !xFamiliesSupp.is(), "xmloff",
                    "getStyleFamilies() from XModel failed for export!" );
        if( xFamilies.is() )
        {
            const OUString aPageStyleName("PageStyles");

            if( xFamilies->hasByName( aPageStyleName ) )
            {
                xPageStyles.set(xFamilies->getByName( aPageStyleName ),uno::UNO_QUERY);

                SAL_WARN_IF( !xPageStyles.is(), "xmloff",
                            "Page Styles not found for export!" );
            }
        }
    }
}

XMLPageExport::~XMLPageExport()
{
}

void XMLPageExport::exportStyles( bool bUsed, bool bAutoStyles )
{
    if( xPageStyles.is() )
    {
        const uno::Sequence< OUString> aSeq = xPageStyles->getElementNames();
        for(const auto& rName : aSeq)
        {
            Reference< XStyle > xStyle(xPageStyles->getByName( rName ),uno::UNO_QUERY);
            if( !bUsed || xStyle->isInUse() )
                exportStyle( xStyle, bAutoStyles );
        }
    }
}

void XMLPageExport::exportAutoStyles()
{
    rExport.GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_PAGE_MASTER);
}

void XMLPageExport::exportDefaultStyle()
{
    Reference < lang::XMultiServiceFactory > xFactory (GetExport().GetModel(), UNO_QUERY);
    if (xFactory.is())
    {
        Reference < XPropertySet > xPropSet (xFactory->createInstance ( "com.sun.star.text.Defaults" ), UNO_QUERY);
        if (xPropSet.is())
        {
            // <style:default-style ...>
            GetExport().CheckAttrList();

            ::std::vector< XMLPropertyState > aPropStates =
                xPageMasterExportPropMapper->FilterDefaults( xPropSet );

            bool bExport = false;
            rtl::Reference < XMLPropertySetMapper > aPropMapper(xPageMasterExportPropMapper->getPropertySetMapper());
            for( const auto& rProp : aPropStates )
            {
                sal_Int16 nContextId    = aPropMapper->GetEntryContextId( rProp.mnIndex );
                if( nContextId == CTF_PM_STANDARD_MODE )
                {
                    bExport = true;
                    break;
                }
            }

            if( bExport )
            {
                assert(GetExport().getDefaultVersion()
                        >= SvtSaveOptions::ODFVER_012);

                //<style:default-page-layout>
                SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                          XML_DEFAULT_PAGE_LAYOUT,
                                          true, true );

                xPageMasterExportPropMapper->exportXML( GetExport(), aPropStates,
                                             SvXmlExportFlags::IGN_WS );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
