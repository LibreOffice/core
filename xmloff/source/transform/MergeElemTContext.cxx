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

    virtual ~XMLParagraphTransformerContext();

    // Create a children element context. By default, the import's
    // CreateContext method is called to create a new default context.
    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const OUString& rQName,
                                   const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    // StartElement is called after a context has been constructed and
    // before a elements context is parsed. It may be used for actions that
    // require virtual methods. The default is to do nothing.
    virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    // EndElement is called before a context will be destructed, but
    // after a elements context has been parsed. It may be used for actions
    // that require virtual methods. The default is to do nothing.
    virtual void EndElement() override;

    // This method is called for all characters that are contained in the
    // current element. The default is to ignore them.
    virtual void Characters( const OUString& rChars ) override;
};

XMLParagraphTransformerContext::XMLParagraphTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLParagraphTransformerContext::~XMLParagraphTransformerContext()
{
}

XMLTransformerContext *XMLParagraphTransformerContext::CreateChildContext(
        sal_uInt16 /*nPrefix*/,
        const OUString& /*rLocalName*/,
        const OUString& rQName,
        const Reference< XAttributeList >& )
{
    XMLTransformerContext *pContext = nullptr;

    pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                rQName, true );

    return pContext;
}

void XMLParagraphTransformerContext::StartElement( const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext::StartElement( rAttrList );
}

void XMLParagraphTransformerContext::EndElement()
{
    XMLTransformerContext::EndElement();
}

void XMLParagraphTransformerContext::Characters( const OUString& rChars )
{
    XMLTransformerContext::Characters( rChars );
}

class XMLPersTextContentRNGTransformTContext : public XMLPersTextContentTContext
{
public:
    XMLPersTextContentRNGTransformTContext(
        XMLTransformerBase& rTransformer,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken );
    virtual ~XMLPersTextContentRNGTransformTContext();

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

XMLPersTextContentRNGTransformTContext::~XMLPersTextContentRNGTransformTContext()
{}

void XMLPersTextContentRNGTransformTContext::Characters( const OUString& rChars )
{
    OUString aConvChars( rChars );
    XMLTransformerBase::ConvertRNGDateTimeToISO( aConvChars );
    XMLPersTextContentTContext::Characters( aConvChars );
}


void XMLMergeElemTransformerContext::ExportStartElement()
{
    XMLPersTextContentTContextVector::iterator aIter = m_aChildContexts.begin();

    for( ; aIter != m_aChildContexts.end(); ++aIter )
    {
        XMLPersTextContentTContext *pContext = (*aIter).get();
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

XMLMergeElemTransformerContext::~XMLMergeElemTransformerContext()
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

XMLTransformerContext *XMLMergeElemTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = nullptr;

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

            if( !(aIter == pActions->end()) )
            {
                switch( (*aIter).second.m_nActionType )
                {
                case XML_ETACTION_MOVE_TO_ATTR_RNG2ISO_DATETIME:
                    {
                        XMLPersTextContentTContext *pTC =
                            new XMLPersTextContentRNGTransformTContext(
                                    GetTransformer(), rQName,
                                    (*aIter).second.GetQNamePrefixFromParam1(),
                                    (*aIter).second.GetQNameTokenFromParam1() );
                        XMLPersTextContentTContextVector::value_type aVal(pTC);
                        m_aChildContexts.push_back( aVal );
                        pContext = pTC;
                    }
                    break;
                case XML_ETACTION_MOVE_TO_ATTR:
                    {
                        XMLPersTextContentTContext *pTC =
                            new XMLPersTextContentTContext(
                                    GetTransformer(), rQName,
                                    (*aIter).second.GetQNamePrefixFromParam1(),
                                    (*aIter).second.GetQNameTokenFromParam1() );
                        XMLPersTextContentTContextVector::value_type aVal(pTC);
                        m_aChildContexts.push_back( aVal );
                        pContext = pTC;
                    }
                    break;
                case XML_ETACTION_EXTRACT_CHARACTERS:
                    {
                        if( !m_bStartElementExported )
                            ExportStartElement();
                        XMLParagraphTransformerContext* pPTC =
                            new XMLParagraphTransformerContext( GetTransformer(),
                            rQName);
                        pContext = pPTC;
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

            if( !(aIter == pActions->end()) )
            {
                switch( (*aIter).second.m_nActionType )
                {
                case XML_ETACTION_EXTRACT_CHARACTERS:
                    {
                        if( !m_bStartElementExported )
                            ExportStartElement();
                        XMLParagraphTransformerContext* pPTC =
                            new XMLParagraphTransformerContext( GetTransformer(),
                            rQName);
                        pContext = pPTC;
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
    if( !pContext )
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
