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

#include "EventOASISTContext.hxx"
#include "EventMap.hxx"
#include "MutableAttrList.hxx"
#include <xmloff/xmlnmspe.hxx>
#include "ActionMapTypesOASIS.hxx"
#include "AttrTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"
#include <osl/diagnose.h>
#include <sal/log.hxx>

// Used to parse Scripting Framework URLs
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <comphelper/processfactory.hxx>

#include <unordered_map>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

class XMLTransformerOASISEventMap_Impl:
    public std::unordered_map< NameKey_Impl, OUString,
                            NameHash_Impl, NameHash_Impl >
{
public:
    explicit XMLTransformerOASISEventMap_Impl( XMLTransformerEventMapEntry const *pInit );
};

XMLTransformerOASISEventMap_Impl::XMLTransformerOASISEventMap_Impl( XMLTransformerEventMapEntry const *pInit )
{
    if( pInit )
    {
        XMLTransformerOASISEventMap_Impl::key_type aKey;
        XMLTransformerOASISEventMap_Impl::mapped_type aData;
        while( pInit->m_pOASISName )
        {
            aKey.m_nPrefix = pInit->m_nOASISPrefix;
            aKey.m_aLocalName = OUString::createFromAscii(pInit->m_pOASISName);

            OSL_ENSURE( find( aKey ) == end(), "duplicate event map entry" );

            aData = OUString::createFromAscii(pInit->m_pOOoName);

            XMLTransformerOASISEventMap_Impl::value_type aVal( aKey, aData );

            insert( aVal );
            ++pInit;
        }
    }
}

XMLEventOASISTransformerContext::XMLEventOASISTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLRenameElemTransformerContext( rImp, rQName,
         rImp.GetNamespaceMap().GetKeyByAttrName( rQName ), XML_EVENT )
{
}

XMLEventOASISTransformerContext::~XMLEventOASISTransformerContext()
{
}

XMLTransformerOASISEventMap_Impl
    *XMLEventOASISTransformerContext::CreateEventMap()
{
    return new XMLTransformerOASISEventMap_Impl( aTransformerEventMap );
}

XMLTransformerOASISEventMap_Impl
    *XMLEventOASISTransformerContext::CreateFormEventMap()
{
    return new XMLTransformerOASISEventMap_Impl( aFormTransformerEventMap );
}

void XMLEventOASISTransformerContext::FlushEventMap(
        XMLTransformerOASISEventMap_Impl *p )
{
    delete p;
}

OUString XMLEventOASISTransformerContext::GetEventName(
        sal_uInt16 nPrefix,
        const OUString& rName,
           XMLTransformerOASISEventMap_Impl& rMap,
           XMLTransformerOASISEventMap_Impl *pMap2)
{
    XMLTransformerOASISEventMap_Impl::key_type aKey( nPrefix, rName );
    if( pMap2 )
    {
        XMLTransformerOASISEventMap_Impl::const_iterator aIter =
            pMap2->find( aKey );
        if( aIter != pMap2->end() )
            return (*aIter).second;
    }

    XMLTransformerOASISEventMap_Impl::const_iterator aIter = rMap.find( aKey );
    if( aIter == rMap.end() )
        return rName;
    else
        return (*aIter).second;
}

static bool ParseURL(
    const OUString& rAttrValue,
    OUString* pName, OUString* pLocation )
{
    Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    Reference< css::uri::XUriReferenceFactory > xFactory = css::uri::UriReferenceFactory::create(xContext);

    Reference< css::uri::XVndSunStarScriptUrl > xUrl ( xFactory->parse( rAttrValue ), UNO_QUERY );

    if ( xUrl.is() )
    {
        const OUString& aLanguageKey = GetXMLToken( XML_LANGUAGE );
        if ( xUrl.is() && xUrl->hasParameter( aLanguageKey ) )
        {
            OUString aLanguage = xUrl->getParameter( aLanguageKey );

            if ( aLanguage.equalsIgnoreAsciiCase("basic") )
            {
                *pName = xUrl->getName();

                OUString tmp =
                    xUrl->getParameter( GetXMLToken( XML_LOCATION ) );

                const OUString& doc = GetXMLToken( XML_DOCUMENT );

                if ( tmp.equalsIgnoreAsciiCase( doc ) )
                {
                    *pLocation = doc;
                }
                else
                {
                    *pLocation = GetXMLToken( XML_APPLICATION );
                }
                return true;
            }
        }
    }
    return false;
}

void XMLEventOASISTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    SAL_INFO("xmloff.transform", "XMLEventOASISTransformerContext::StartElement");

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_EVENT_ACTIONS );
    SAL_WARN_IF( pActions == nullptr, "xmloff.transform", "got no actions" );

    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = nullptr;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                 &aLocalName );
        XMLTransformerActions::key_type aKey( nPrefix, aLocalName );
        XMLTransformerActions::const_iterator aIter =
            pActions->find( aKey );
        if( aIter != pActions->end() )
        {
            if( !pMutableAttrList )
            {
                pMutableAttrList =
                        new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_HREF:
                {
                    OUString aName, aLocation;

                    bool bNeedsTransform =
                        ParseURL( rAttrValue, &aName, &aLocation );

                    if ( bNeedsTransform )
                    {
                        pMutableAttrList->RemoveAttributeByIndex( i );

                        OUString aAttrQName(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_SCRIPT,
                            ::xmloff::token::GetXMLToken( XML_MACRO_NAME ) ) );

                        pMutableAttrList->AddAttribute( aAttrQName, aName );

                        sal_Int16 idx = pMutableAttrList->GetIndexByName(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_SCRIPT,
                            GetXMLToken( XML_LANGUAGE ) ) );

                        pMutableAttrList->SetValueByIndex( idx,
                            "StarBasic" );

                        OUString aLocQName(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_SCRIPT,
                                GetXMLToken( XML_LOCATION ) ) );

                        pMutableAttrList->AddAttribute( aLocQName, aLocation );
                    }
                }
                break;
            case XML_ATACTION_EVENT_NAME:
                {
                    // Check if the event belongs to a form or control by
                    // checking the 2nd ancestor element, f.i.:
                    // <form:button><form:event-listeners><form:event-listener>
                    const XMLTransformerContext *pObjContext =
                        GetTransformer().GetAncestorContext( 1 );
                    bool bForm = pObjContext &&

                        pObjContext->HasNamespace(XML_NAMESPACE_FORM );
                    pMutableAttrList->SetValueByIndex( i,
                                   GetTransformer().GetEventName( rAttrValue,
                                                                    bForm ) );
                }
                break;
            case XML_ATACTION_REMOVE_NAMESPACE_PREFIX:
                {
                    OUString aAttrValue( rAttrValue );
                    sal_uInt16 nValPrefix =
                        static_cast<sal_uInt16>((*aIter).second.m_nParam1);
                    if( GetTransformer().RemoveNamespacePrefix(
                                aAttrValue, nValPrefix ) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            case XML_ATACTION_MACRO_NAME:
            {
                OUString aName, aLocation;
                bool bNeedsTransform =
                ParseURL( rAttrValue, &aName, &aLocation );

                if ( bNeedsTransform )
                {
                    pMutableAttrList->SetValueByIndex( i, aName );

                    sal_Int16 idx = pMutableAttrList->GetIndexByName(
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_SCRIPT,
                    GetXMLToken( XML_LANGUAGE ) ) );

                    pMutableAttrList->SetValueByIndex( idx,
                    "StarBasic" );

                    OUString aLocQName(
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_SCRIPT,
                    GetXMLToken( XML_LOCATION ) ) );

                    pMutableAttrList->AddAttribute( aLocQName, aLocation );
                }
                else
                {
                    const OUString& rApp = GetXMLToken( XML_APPLICATION );
                    const OUString& rDoc = GetXMLToken( XML_DOCUMENT );
                    OUString aAttrValue;
                    if( rAttrValue.getLength() > rApp.getLength()+1 &&
                        rAttrValue.copy(0,rApp.getLength()).
                            equalsIgnoreAsciiCase( rApp ) &&
                        ':' == rAttrValue[rApp.getLength()] )
                    {
                        aLocation = rApp;
                        aAttrValue = rAttrValue.copy( rApp.getLength()+1 );
                    }
                    else if( rAttrValue.getLength() > rDoc.getLength()+1 &&
                             rAttrValue.copy(0,rDoc.getLength()).
                                equalsIgnoreAsciiCase( rDoc ) &&
                             ':' == rAttrValue[rDoc.getLength()] )
                    {
                        aLocation= rDoc;
                        aAttrValue = rAttrValue.copy( rDoc.getLength()+1 );
                    }
                    if( !aAttrValue.isEmpty() )
                        pMutableAttrList->SetValueByIndex( i,
                    aAttrValue );
                    if( !aLocation.isEmpty() )
                    {
                        OUString aAttrQName( GetTransformer().GetNamespaceMap().
                        GetQNameByKey( XML_NAMESPACE_SCRIPT,
                        ::xmloff::token::GetXMLToken( XML_LOCATION ) ) );
                        pMutableAttrList->AddAttribute( aAttrQName, aLocation );
                        // draw bug
                        aAttrQName = GetTransformer().GetNamespaceMap().
                        GetQNameByKey( XML_NAMESPACE_SCRIPT,
                        ::xmloff::token::GetXMLToken( XML_LIBRARY ) );
                        pMutableAttrList->AddAttribute( aAttrQName, aLocation );
                    }
                }
            }
            break;
            case XML_ATACTION_COPY:
                break;
            default:
                SAL_WARN( "xmloff.transform", "unknown action" );
                break;
            }
        }
    }

    XMLRenameElemTransformerContext::StartElement( xAttrList );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
