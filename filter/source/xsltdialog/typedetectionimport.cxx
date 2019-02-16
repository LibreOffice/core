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

#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <osl/diagnose.h>

#include "typedetectionimport.hxx"
#include "xmlfiltersettingsdialog.hxx"

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::beans;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star;
using namespace std;

TypeDetectionImporter::TypeDetectionImporter()
{
}

TypeDetectionImporter::~TypeDetectionImporter()
{
}

void TypeDetectionImporter::doImport( const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& xIS,
                                      std::vector< std::unique_ptr<filter_info_impl> >& rFilters )
{
    try
    {
        Reference< XParser > xParser = xml::sax::Parser::create( rxContext );

        TypeDetectionImporter* pImporter = new TypeDetectionImporter;
        Reference < XDocumentHandler > xDocHandler( pImporter );
        xParser->setDocumentHandler( xDocHandler );

        InputSource source;
        source.aInputStream = xIS;

        // start parsing
        xParser->parseStream( source );

        pImporter->fillFilterVector( rFilters );
    }
    catch( const Exception& /* e */ )
    {
        OSL_FAIL( "TypeDetectionImporter::doImport exception caught!" );
    }
}

void TypeDetectionImporter::fillFilterVector(  std::vector< std::unique_ptr<filter_info_impl> >& rFilters )
{
    // create filter infos from imported filter nodes
    for (auto const& filterNode : maFilterNodes)
    {
        std::unique_ptr<filter_info_impl> pFilter = createFilterForNode(filterNode.get());
        if( pFilter )
            rFilters.push_back( std::move(pFilter) );
    }
    maFilterNodes.clear();

    // now delete type nodes
    maTypeNodes.clear();
}

static OUString getSubdata( int index, sal_Unicode delimiter, const OUString& rData )
{
    sal_Int32 nLastIndex = 0;

    sal_Int32 nNextIndex = rData.indexOf( delimiter );

    OUString aSubdata;

    while( index )
    {
        nLastIndex = nNextIndex + 1;
        nNextIndex = rData.indexOf( delimiter, nLastIndex );

        index--;

        if( (index > 0) && (nLastIndex == 0) )
            return aSubdata;
    }

    if( nNextIndex == -1 )
    {
        aSubdata = rData.copy( nLastIndex );
    }
    else
    {
        aSubdata = rData.copy( nLastIndex, nNextIndex - nLastIndex );
    }

    return aSubdata;
}

Node* TypeDetectionImporter::findTypeNode( const OUString& rType )
{
    auto aIter = std::find_if(maTypeNodes.begin(), maTypeNodes.end(),
        [&rType](const std::unique_ptr<Node>& rxNode) { return rxNode->maName == rType; });
    if (aIter != maTypeNodes.end())
        return aIter->get();

    return nullptr;
}

std::unique_ptr<filter_info_impl> TypeDetectionImporter::createFilterForNode( Node * pNode )
{
    std::unique_ptr<filter_info_impl> pFilter(new filter_info_impl);

    pFilter->maFilterName = pNode->maName;
    pFilter->maInterfaceName = pNode->maPropertyMap["UIName"];

    OUString aData = pNode->maPropertyMap["Data"];

    sal_Unicode aComma(',');

    pFilter->maType = getSubdata( 1, aComma, aData  );
    pFilter->maDocumentService = getSubdata( 2, aComma, aData );

    OUString aFilterService( getSubdata( 3, aComma, aData ) );
    pFilter->maFlags = getSubdata( 4, aComma, aData ).toInt32();

    // parse filter user data
    sal_Unicode aDelim(';');
    OUString aFilterUserData( getSubdata( 5, aComma, aData ) );

    OUString aAdapterService( getSubdata( 0, aDelim, aFilterUserData ) );
    //Import/ExportService
    pFilter->mbNeedsXSLT2 = getSubdata( 1, aDelim, aFilterUserData ).toBoolean();
    pFilter->maImportService = getSubdata( 2, aDelim, aFilterUserData );
    pFilter->maExportService = getSubdata( 3, aDelim, aFilterUserData );
    pFilter->maImportXSLT = getSubdata( 4, aDelim, aFilterUserData );
    pFilter->maExportXSLT = getSubdata( 5, aDelim, aFilterUserData );
    pFilter->maComment = getSubdata( 7, aDelim, aFilterUserData );


    pFilter->maImportTemplate = getSubdata( 7, aComma, aData );

    Node* pTypeNode = findTypeNode( pFilter->maType );
    if( pTypeNode )
    {
        OUString aTypeUserData( pTypeNode->maPropertyMap["Data"] );

        pFilter->maDocType = getSubdata( 2, aComma, aTypeUserData );
        pFilter->maExtension = getSubdata( 4, aComma, aTypeUserData );
        pFilter->mnDocumentIconID = getSubdata( 5, aComma, aTypeUserData ).toInt32();
    }

    bool bOk = true;

    if( pTypeNode == nullptr )
        bOk = false;

    if( pFilter->maFilterName.isEmpty() )
        bOk = false;

    if( pFilter->maInterfaceName.isEmpty() )
        bOk = false;

    if( pFilter->maType.isEmpty() )
        bOk = false;

    if( pFilter->maFlags == 0 )
        bOk = false;

    if( aFilterService != "com.sun.star.comp.Writer.XmlFilterAdaptor" )
        bOk = false;

    if( aAdapterService != "com.sun.star.documentconversion.XSLTFilter" )
        bOk = false;

    if( pFilter->maExtension.isEmpty() )
        bOk = false;

    if( !bOk )
        return nullptr;

    return pFilter;
}

void SAL_CALL TypeDetectionImporter::startDocument(  )
{
}

void SAL_CALL TypeDetectionImporter::endDocument(  )
{
}

void SAL_CALL TypeDetectionImporter::startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs )
{
    ImportState eNewState = e_Unknown;

    if( maStack.empty() )
    {
        // #109668# support legacy name as well on import
        if( aName == "oor:component-data" || aName == "oor:node" )
        {
            eNewState = e_Root;
        }
    }
    else if( maStack.top() == e_Root )
    {
        if( aName == "node" )
        {
            OUString aNodeName( xAttribs->getValueByName( "oor:name" ) );

            if( aNodeName == "Filters" )
            {
                eNewState = e_Filters;
            }
            else if( aNodeName == "Types" )
            {
                eNewState = e_Types;
            }
        }
    }
    else if( (maStack.top() == e_Filters) || (maStack.top() == e_Types) )
    {
        if( aName == "node" )
        {
            maNodeName = xAttribs->getValueByName( "oor:name" );

            eNewState = (maStack.top() == e_Filters) ? e_Filter : e_Type;
        }
    }
    else if( (maStack.top() == e_Filter) || (maStack.top() == e_Type))
    {
        if( aName == "prop" )
        {
            maPropertyName = xAttribs->getValueByName( "oor:name" );
            eNewState = e_Property;
        }
    }
    else if( maStack.top() == e_Property )
    {
        if( aName == "value" )
        {
            eNewState = e_Value;
            maValue.clear();
        }
    }

    maStack.push( eNewState );
}
void SAL_CALL TypeDetectionImporter::endElement( const OUString& /* aName */ )
{
    if( !maStack.empty()  )
    {
        ImportState eCurrentState = maStack.top();
        switch( eCurrentState )
        {
        case e_Filter:
        case e_Type:
            {
                std::unique_ptr<Node> pNode(new Node);
                pNode->maName = maNodeName;
                pNode->maPropertyMap = maPropertyMap;
                maPropertyMap.clear();

                if( eCurrentState == e_Filter )
                {
                    maFilterNodes.push_back( std::move(pNode) );
                }
                else
                {
                    maTypeNodes.push_back( std::move(pNode) );
                }
            }
            break;

        case e_Property:
            maPropertyMap[ maPropertyName ] = maValue;
            break;
        default: break;
        }

        maStack.pop();
    }
}
void SAL_CALL TypeDetectionImporter::characters( const OUString& aChars )
{
    if( !maStack.empty() && maStack.top() == e_Value )
    {
        maValue += aChars;
    }
}
void SAL_CALL TypeDetectionImporter::ignorableWhitespace( const OUString& /* aWhitespaces */ )
{
}
void SAL_CALL TypeDetectionImporter::processingInstruction( const OUString& /* aTarget */, const OUString& /* aData */ )
{
}
void SAL_CALL TypeDetectionImporter::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& /* xLocator */ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
