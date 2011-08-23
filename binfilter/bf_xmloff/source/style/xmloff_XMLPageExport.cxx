/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <tools/debug.hxx>

#include "xmlnmspe.hxx"

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>

#include <com/sun/star/container/XNameContainer.hpp>


#include "xmlexp.hxx"
#include "PageMasterPropHdlFactory.hxx"
#include "PageMasterStyleMap.hxx"
#include "PageMasterPropMapper.hxx"
#include "PageMasterExportPropMapper.hxx"
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;


//______________________________________________________________________________

sal_Bool XMLPageExport::findPageMasterName( const OUString& rStyleName, OUString& rPMName ) const
{
    for( ::std::vector< XMLPageExportNameEntry >::const_iterator pEntry = aNameVector.begin();
            pEntry != aNameVector.end(); pEntry++ )
    {
        if( pEntry->sStyleName == rStyleName )
        {
            rPMName = pEntry->sPageMasterName;
            return sal_True;
        }
    }
    return sal_False;
}

void XMLPageExport::collectPageMasterAutoStyle(
        const Reference < XPropertySet > & rPropSet,
        OUString& rPageMasterName )
{
    DBG_ASSERT( xPageMasterPropSetMapper.is(), "page master family/XMLPageMasterPropSetMapper not found" );
    if( xPageMasterPropSetMapper.is() )
    {
        ::std::vector<XMLPropertyState> xPropStates = xPageMasterExportPropMapper->Filter( rPropSet );
        if(xPropStates.size())
        {
            OUString sParent;
            rPageMasterName = rExport.GetAutoStylePool()->Find( XML_STYLE_FAMILY_PAGE_MASTER, sParent, xPropStates );
            if (!rPageMasterName.getLength())
                rPageMasterName = rExport.GetAutoStylePool()->Add(XML_STYLE_FAMILY_PAGE_MASTER, sParent, xPropStates);
        }
    }
}

void XMLPageExport::exportMasterPageContent(
                const Reference < XPropertySet > & rPropSet,
                sal_Bool bAutoStyles )
{

}

sal_Bool XMLPageExport::exportStyle(
            const Reference< XStyle >& rStyle,
            sal_Bool bAutoStyles )
{
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    Any aAny;

    // Don't export styles that aren't existing really. This may be the
    // case for StarOffice Writer's pool styles.
    if( xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        aAny = xPropSet->getPropertyValue( sIsPhysical );
        if( !*(sal_Bool *)aAny.getValue() )
            return sal_False;
    }

    if( bAutoStyles )
    {
        XMLPageExportNameEntry aEntry;
        collectPageMasterAutoStyle( xPropSet, aEntry.sPageMasterName );
        aEntry.sStyleName = rStyle->getName();
        aNameVector.push_back( aEntry );

        exportMasterPageContent( xPropSet, sal_True );
    }
    else
    {
        OUString sName( rStyle->getName() );
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME, sName );

        OUString sPMName;
        if( findPageMasterName( sName, sPMName ) )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_PAGE_MASTER_NAME, sPMName );

        aAny = xPropSet->getPropertyValue( sFollowStyle );
        OUString sNextName;
        aAny >>= sNextName;
        if( sName != sNextName && sNextName.getLength() )
        {
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NEXT_STYLE_NAME,
                          sNextName );
        }
//		OUString sPageMaster = GetExport().GetAutoStylePool()->Find(
//											XML_STYLE_FAMILY_PAGE_MASTER,
//											xPropSet );
//		if( sPageMaster.getLength() )
//			GetExport().AddAttribute( XML_NAMESPACE_STYLE,
//									  XML_PAGE_MASTER_NAME,
//									  sPageMaster );

        SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                  XML_MASTER_PAGE, sal_True, sal_True );

        exportMasterPageContent( xPropSet, sal_False );
    }

    return sal_True;
}

XMLPageExport::XMLPageExport( SvXMLExport& rExp ) :
    rExport( rExp ),
    sIsPhysical( RTL_CONSTASCII_USTRINGPARAM( "IsPhysical" ) ),
    sFollowStyle( RTL_CONSTASCII_USTRINGPARAM( "FollowStyle" ) )
{
    xPageMasterPropHdlFactory = new XMLPageMasterPropHdlFactory;
    xPageMasterPropSetMapper = new XMLPageMasterPropSetMapper(
                                (XMLPropertyMapEntry*) aXMLPageMasterStyleMap,
                                xPageMasterPropHdlFactory  );
    xPageMasterExportPropMapper = new XMLPageMasterExportPropMapper(
                                    xPageMasterPropSetMapper, rExp);

    rExport.GetAutoStylePool()->AddFamily( XML_STYLE_FAMILY_PAGE_MASTER, OUString( RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_PAGE_MASTER_NAME ) ),
        xPageMasterExportPropMapper, OUString( RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_PAGE_MASTER_PREFIX ) ), sal_False );

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
            const OUString aPageStyleName(
                        RTL_CONSTASCII_USTRINGPARAM( "PageStyles" ));

            if( xFamilies->hasByName( aPageStyleName ) )
            {
                Reference < XNameContainer > xStyleCont;
                xFamilies->getByName( aPageStyleName ) >>= xStyleCont;

                xPageStyles = Reference< XIndexAccess >( xStyleCont,
                                                          UNO_QUERY );

                DBG_ASSERT( xPageStyles.is(),
                            "Page Styles not found for export!" );
            }
        }
    }
}

XMLPageExport::~XMLPageExport()
{
}

void XMLPageExport::exportStyles( sal_Bool bUsed, sal_Bool bAutoStyles )
{
    if( xPageStyles.is() )
    {
        const sal_Int32 nStyles = xPageStyles->getCount();

        for( sal_Int32 i=0; i < nStyles; i++ )
        {
            Reference< XStyle > xStyle;
            xPageStyles->getByIndex( i ) >>= xStyle;

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
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
