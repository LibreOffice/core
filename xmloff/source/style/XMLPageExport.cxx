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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include <xmloff/XMLPageExport.hxx>
#include <tools/debug.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <xmloff/families.hxx>
#include <xmloff/xmlexp.hxx>
#include "PageMasterPropHdlFactory.hxx"
#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include <xmloff/PageMasterStyleMap.hxx>
#endif
#ifndef _XMLOFF_PAGEMASTERPROPMAPPER_HXX
#include "PageMasterPropMapper.hxx"
#endif
#include "PageMasterExportPropMapper.hxx"
#include "PageMasterExportPropMapper.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;


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
        if( !xPropStates.empty())
        {
            OUString sParent;
            rPageMasterName = rExport.GetAutoStylePool()->Find( XML_STYLE_FAMILY_PAGE_MASTER, sParent, xPropStates );
            if (!rPageMasterName.getLength())
                rPageMasterName = rExport.GetAutoStylePool()->Add(XML_STYLE_FAMILY_PAGE_MASTER, sParent, xPropStates);
        }
    }
}

void XMLPageExport::exportMasterPageContent(
                const Reference < XPropertySet > &,
                sal_Bool /*bAutoStyles*/ )
{

}

sal_Bool XMLPageExport::exportStyle(
            const Reference< XStyle >& rStyle,
            sal_Bool bAutoStyles )
{
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    // Don't export styles that aren't existing really. This may be the
    // case for StarOffice Writer's pool styles.
    if( xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        Any aAny = xPropSet->getPropertyValue( sIsPhysical );
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
        sal_Bool bEncoded = sal_False;
        GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NAME,
                          GetExport().EncodeStyleName( sName, &bEncoded ) );

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

            if( sName != sNextName && sNextName.getLength() )
            {
                GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_NEXT_STYLE_NAME,
                    GetExport().EncodeStyleName( sNextName ) );
            }
        }
//      OUString sPageMaster = GetExport().GetAutoStylePool()->Find(
//                                          XML_STYLE_FAMILY_PAGE_MASTER,
//                                          xPropSet );
//      if( sPageMaster.getLength() )
//          GetExport().AddAttribute( XML_NAMESPACE_STYLE,
//                                    XML_PAGE_MASTER_NAME,
//                                    sPageMaster );

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

void XMLPageExport::exportStyles( sal_Bool bUsed, sal_Bool bAutoStyles )
{
    if( xPageStyles.is() )
    {
        uno::Sequence< ::rtl::OUString> aSeq = xPageStyles->getElementNames();
        const ::rtl::OUString* pIter = aSeq.getConstArray();
        const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
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
        OUString sTextDefaults ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.Defaults" ) );
        Reference < XPropertySet > xPropSet (xFactory->createInstance ( sTextDefaults ), UNO_QUERY);
        if (xPropSet.is())
        {
            // <style:default-style ...>
            GetExport().CheckAttrList();

            ::std::vector< XMLPropertyState > xPropStates =
                xPageMasterExportPropMapper->FilterDefaults( xPropSet );

            sal_Bool bExport = sal_False;
            UniReference < XMLPropertySetMapper > aPropMapper(xPageMasterExportPropMapper->getPropertySetMapper());
            for( ::std::vector< XMLPropertyState >::iterator aIter = xPropStates.begin(); aIter != xPropStates.end(); ++aIter )
            {
                XMLPropertyState *pProp = &(*aIter);
                sal_Int16 nContextId    = aPropMapper->GetEntryContextId( pProp->mnIndex );
                if( nContextId == CTF_PM_STANDARD_MODE )
                {
                    bExport = sal_True;
                    break;
                }
            }

        //  if ( xPropStates.size() != 0 &&
          //          ( xPropStates.size() != 1 || xPropStates[0].mnIndex != -1 ) )
            if( bExport )
            {
                //<style:default-page-layout>
                SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_STYLE,
                                          XML_DEFAULT_PAGE_LAYOUT,
                                          sal_True, sal_True );

                xPageMasterExportPropMapper->exportXML( GetExport(), xPropStates,
                                             XML_EXPORT_FLAG_IGN_WS );
            }
        }
    }
}
