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
#include <tools/debug.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
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

bool XMLPageExport::findPageMasterName( const OUString& rStyleName, OUString& rPMName ) const
{
    for( ::std::vector< XMLPageExportNameEntry >::const_iterator pEntry = aNameVector.begin();
            pEntry != aNameVector.end(); ++pEntry )
    {
        if( pEntry->sStyleName == rStyleName )
        {
            rPMName = pEntry->sPageMasterName;
            return true;
        }
    }
    return false;
}

void XMLPageExport::collectPageMasterAutoStyle(
        const Reference < XPropertySet > & rPropSet,
        OUString& rPageMasterName )
{
    DBG_ASSERT( xPageMasterPropSetMapper.is(), "page master family/XMLPageMasterPropSetMapper not found" );
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
    if( xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        Any aAny = xPropSet->getPropertyValue( sIsPhysical );
        if( !*static_cast<sal_Bool const *>(aAny.getValue()) )
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
        if ( xInfo.is() && xInfo->hasPropertyByName(sFollowStyle) )
        {
            OUString sNextName;
            xPropSet->getPropertyValue( sFollowStyle ) >>= sNextName;

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
    rExport( rExp ),
    sIsPhysical( "IsPhysical" ),
    sFollowStyle( "FollowStyle" )
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
    DBG_ASSERT( xFamiliesSupp.is(),
                "No XStyleFamiliesSupplier from XModel for export!" );
    if( xFamiliesSupp.is() )
    {
        Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
        DBG_ASSERT( xFamiliesSupp.is(),
                    "getStyleFamilies() from XModel failed for export!" );
        if( xFamilies.is() )
        {
            const OUString aPageStyleName("PageStyles");

            if( xFamilies->hasByName( aPageStyleName ) )
            {
                xPageStyles.set(xFamilies->getByName( aPageStyleName ),uno::UNO_QUERY);

                DBG_ASSERT( xPageStyles.is(),
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
        uno::Sequence< OUString> aSeq = xPageStyles->getElementNames();
        const OUString* pIter = aSeq.getConstArray();
        const OUString* pEnd   = pIter + aSeq.getLength();
        for(;pIter != pEnd;++pIter)
        {
            Reference< XStyle > xStyle(xPageStyles->getByName( *pIter ),uno::UNO_QUERY);
            if( !bUsed || xStyle->isInUse() )
                exportStyle( xStyle, bAutoStyles );
        }
    }
}

void XMLPageExport::exportAutoStyles()
{
    rExport.GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_PAGE_MASTER
        , rExport.GetDocHandler(), rExport.GetMM100UnitConverter(),
        rExport.GetNamespaceMap()
        );
}

void XMLPageExport::exportDefaultStyle()
{
    Reference < lang::XMultiServiceFactory > xFactory (GetExport().GetModel(), UNO_QUERY);
    if (xFactory.is())
    {
        OUString sTextDefaults ( "com.sun.star.text.Defaults" );
        Reference < XPropertySet > xPropSet (xFactory->createInstance ( sTextDefaults ), UNO_QUERY);
        if (xPropSet.is())
        {
            // <style:default-style ...>
            GetExport().CheckAttrList();

            ::std::vector< XMLPropertyState > aPropStates =
                xPageMasterExportPropMapper->FilterDefaults( xPropSet );

            bool bExport = false;
            rtl::Reference < XMLPropertySetMapper > aPropMapper(xPageMasterExportPropMapper->getPropertySetMapper());
            for( ::std::vector< XMLPropertyState >::iterator aIter = aPropStates.begin(); aIter != aPropStates.end(); ++aIter )
            {
                XMLPropertyState *pProp = &(*aIter);
                sal_Int16 nContextId    = aPropMapper->GetEntryContextId( pProp->mnIndex );
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
