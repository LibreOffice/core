/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "MergeElemTContext.hxx"
#include "MutableAttrList.hxx"
#include "TransformerBase.hxx"
#include "TransformerActions.hxx"
#include "AttrTransformerAction.hxx"
#include "ElemTransformerAction.hxx"
#include "IgnoreTContext.hxx"
#include <xmloff/xmlnmspe.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

class XMLParagraphTransformerContext : public XMLTransformerContext
{
public:
    TYPEINFO();

    XMLParagraphTransformerContext( XMLTransformerBase& rTransformer,
                           const OUString& rQName );

    virtual ~XMLParagraphTransformerContext();

    
    
    virtual XMLTransformerContext *CreateChildContext( sal_uInt16 nPrefix,
                                   const OUString& rLocalName,
                                   const OUString& rQName,
                                   const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    
    
    
    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    
    
    
    virtual void EndElement();

    
    
    virtual void Characters( const OUString& rChars );
};

TYPEINIT1( XMLParagraphTransformerContext, XMLTransformerContext );

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
    XMLTransformerContext *pContext = 0;

    pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                rQName, sal_True );

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
    TYPEINFO();

    XMLPersTextContentRNGTransformTContext(
        XMLTransformerBase& rTransformer,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken );
    virtual ~XMLPersTextContentRNGTransformTContext();

    virtual void Characters( const OUString& rChars );
};

TYPEINIT1( XMLPersTextContentRNGTransformTContext, XMLPersAttrListTContext );

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
    GetTransformer().ConvertRNGDateTimeToISO( aConvChars );
    XMLPersTextContentTContext::Characters( aConvChars );
}


TYPEINIT1( XMLMergeElemTransformerContext, XMLTransformerContext );

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

    m_bStartElementExported = sal_True;
}

XMLMergeElemTransformerContext::XMLMergeElemTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_nActionMap( nActionMap ),
    m_bStartElementExported( sal_False )
{
}

XMLMergeElemTransformerContext::~XMLMergeElemTransformerContext()
{
}

void XMLMergeElemTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    XMLMutableAttributeList *pMutableAttrList =
        new XMLMutableAttributeList( rAttrList, sal_True );
    m_xAttrList = pMutableAttrList;

    sal_Int16 nAttrCount = m_xAttrList.is() ? m_xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = m_xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                &aLocalName );
        sal_Bool bRemove = sal_True;
        if( XML_NAMESPACE_OFFICE == nPrefix)
        {
            if (IsXMLToken( aLocalName, XML_DISPLAY ) )
                bRemove = sal_False;
            else if (IsXMLToken( aLocalName, XML_AUTHOR ) )
                bRemove = sal_False;
            else if (IsXMLToken( aLocalName, XML_CREATE_DATE ) )
                bRemove = sal_False;
            else if (IsXMLToken( aLocalName, XML_CREATE_DATE_STRING ) )
                bRemove = sal_False;
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
    XMLTransformerContext *pContext = 0;

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
                case XML_ATACTION_MOVE_FROM_ELEM_RNG2ISO_DATETIME:
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
                case XML_ATACTION_MOVE_FROM_ELEM:
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
                    OSL_ENSURE( !this, "unknown action" );
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
                    OSL_ENSURE( !this, "unknown action" );
                    break;
                }
            }
        }
    }

    
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
    
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
