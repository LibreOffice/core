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

#include "MergeElemTContext.hxx"
#include "MutableAttrList.hxx"
#include "TransformerBase.hxx"
#include "TransformerActions.hxx"
#include "AttrTransformerAction.hxx"
#include "ElemTransformerAction.hxx"
#include "IgnoreTContext.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

class XMLParagraphTransformerContext : public XMLTransformerContext
{
public:
    XMLParagraphTransformerContext( XMLTransformerBase& rTransformer,
                           const OUString& rQName );

    // Create a children element context. By default, the import's
    // CreateContext method is called to create a new default context.
    virtual rtl::Reference<XMLTransformerContext> CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const OUString& rQName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;
};

XMLParagraphTransformerContext::XMLParagraphTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

rtl::Reference<XMLTransformerContext> XMLParagraphTransformerContext::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const OUString& rQName,
        const Reference< XAttributeList >& )
{
    return new XMLIgnoreTransformerContext( GetTransformer(),
                                                rQName, true );
}

class XMLPersTextContentRNGTransformTContext : public XMLPersTextContentTContext
{
public:
    XMLPersTextContentRNGTransformTContext(
        XMLTransformerBase& rTransformer,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken );

    virtual void Characters( const OUString& rChars ) override;
};

XMLPersTextContentRNGTransformTContext::XMLPersTextContentRNGTransformTContext(
    XMLTransformerBase& rTransformer,
    const OUString& rQName,
    sal_uInt16 nPrefix,
    ::xmloff::token::XMLTokenEnum eToken ) :
        XMLPersTextContentTContext(
            rTransformer, rQName, nPrefix, eToken )
{}

void XMLPersTextContentRNGTransformTContext::Characters( const OUString& rChars )
{
    OUString aConvChars( rChars );
    XMLTransformerBase::ConvertRNGDateTimeToISO( aConvChars );
    XMLPersTextContentTContext::Characters( aConvChars );
}


void XMLMergeElemTransformerContext::ExportStartElement()
{
    for( const auto& rChildContext : m_aChildContexts )
    {
        XMLPersTextContentTContext *pContext = rChildContext.get();
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
            ->AddAttribute( pContext->GetExportQName(),
                            pContext->GetTextContent() );
    }
    XMLTransformerContext::StartElement( m_xAttrList );

    m_bStartElementExported = true;
}

XMLMergeElemTransformerContext::XMLMergeElemTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_nActionMap( nActionMap ),
    m_bStartElementExported( false )
{
}

void XMLMergeElemTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    XMLMutableAttributeList *pMutableAttrList =
        new XMLMutableAttributeList( rAttrList, true );
    m_xAttrList = pMutableAttrList;

    sal_Int16 nAttrCount = m_xAttrList.is() ? m_xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = m_xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                &aLocalName );
        bool bRemove = true;
        if( XML_NAMESPACE_OFFICE == nPrefix)
        {
            if (IsXMLToken( aLocalName, XML_DISPLAY ) )
                bRemove = false;
            else if (IsXMLToken( aLocalName, XML_AUTHOR ) )
                bRemove = false;
            else if (IsXMLToken( aLocalName, XML_CREATE_DATE ) )
                bRemove = false;
            else if (IsXMLToken( aLocalName, XML_CREATE_DATE_STRING ) )
                bRemove = false;
        }
        if (bRemove)
        {
            pMutableAttrList->RemoveAttributeByIndex( i );
            --i;
            --nAttrCount;
        }
    }
}

rtl::Reference<XMLTransformerContext> XMLMergeElemTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    rtl::Reference<XMLTransformerContext> pContext;

    if( !m_bStartElementExported )
    {
        XMLTransformerActions *pActions =
            GetTransformer().GetUserDefinedActions( m_nActionMap );
        OSL_ENSURE( pActions, "go no actions" );
        if( pActions )
        {
            XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
            XMLTransformerActions::const_iterator aIter =
                pActions->find( aKey );

            if( aIter != pActions->end() )
            {
                switch( (*aIter).second.m_nActionType )
                {
                case XML_ETACTION_MOVE_TO_ATTR_RNG2ISO_DATETIME:
                    {
                        rtl::Reference<XMLPersTextContentTContext> pTC(
                            new XMLPersTextContentRNGTransformTContext(
                                    GetTransformer(), rQName,
                                    (*aIter).second.GetQNamePrefixFromParam1(),
                                    (*aIter).second.GetQNameTokenFromParam1() ));
                        m_aChildContexts.push_back(pTC);
                        pContext.set(pTC.get());
                    }
                    break;
                case XML_ETACTION_MOVE_TO_ATTR:
                    {
                        rtl::Reference<XMLPersTextContentTContext> pTC(
                            new XMLPersTextContentTContext(
                                    GetTransformer(), rQName,
                                    (*aIter).second.GetQNamePrefixFromParam1(),
                                    (*aIter).second.GetQNameTokenFromParam1() ));
                        m_aChildContexts.push_back(pTC);
                        pContext.set(pTC.get());
                    }
                    break;
                case XML_ETACTION_EXTRACT_CHARACTERS:
                    {
                        if( !m_bStartElementExported )
                            ExportStartElement();
                        pContext.set(
                            new XMLParagraphTransformerContext( GetTransformer(),
                            rQName));
                    }
                    break;
                default:
                    OSL_ENSURE( false, "unknown action" );
                    break;
                }
            }
        }
    }
    else
    {
        XMLTransformerActions *pActions =
            GetTransformer().GetUserDefinedActions( m_nActionMap );
        OSL_ENSURE( pActions, "go no actions" );
        if( pActions )
        {
            XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
            XMLTransformerActions::const_iterator aIter =
                pActions->find( aKey );

            if( aIter != pActions->end() )
            {
                switch( (*aIter).second.m_nActionType )
                {
                case XML_ETACTION_EXTRACT_CHARACTERS:
                    {
                        if( !m_bStartElementExported )
                            ExportStartElement();
                        pContext.set(
                            new XMLParagraphTransformerContext( GetTransformer(),
                            rQName));
                    }
                    break;
                default:
                    OSL_ENSURE( false, "unknown action" );
                    break;
                }
            }
        }
    }

    // default is copying
    if( !pContext.is() )
    {
        if( !m_bStartElementExported )
            ExportStartElement();
        pContext = XMLTransformerContext::CreateChildContext( nPrefix,
                                                              rLocalName,
                                                              rQName,
                                                              rAttrList );
    }

    return pContext;
}

void XMLMergeElemTransformerContext::EndElement()
{
    if( !m_bStartElementExported )
        ExportStartElement();
    XMLTransformerContext::EndElement();
}

void XMLMergeElemTransformerContext::Characters( const OUString& )
{
    // ignore
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
