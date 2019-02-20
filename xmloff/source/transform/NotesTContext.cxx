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

#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <osl/diagnose.h>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>

#include "TransformerBase.hxx"
#include "TransformerActions.hxx"
#include "AttrTransformerAction.hxx"
#include "ActionMapTypesOASIS.hxx"
#include "MutableAttrList.hxx"
#include "RenameElemTContext.hxx"
#include "FlatTContext.hxx"

#include "NotesTContext.hxx"

using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

XMLNotesTransformerContext::XMLNotesTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        XMLTokenEnum eToken, bool bPersistent ) :
    XMLPersElemContentTContext( rImp, rQName ),
    m_bEndNote( false ),
    m_bPersistent( bPersistent ),
    m_eTypeToken( eToken )
{
}

XMLNotesTransformerContext::~XMLNotesTransformerContext()
{
}

void XMLNotesTransformerContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_NOTES_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

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
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );

            if( !pMutableAttrList )
            {
                pMutableAttrList =
                    new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_STYLE_FAMILY:
                {
                    if( IsXMLToken( rAttrValue, XML_FOOTNOTE ) )
                    {
                    }
                    else if( IsXMLToken( rAttrValue, XML_ENDNOTE ) )
                    {
                        m_bEndNote = true;
                    }
                    pMutableAttrList->RemoveAttributeByIndex( i );
                    --i;
                    --nAttrCount;
                }
                break;
            case XML_ATACTION_DECODE_STYLE_NAME:
            case XML_ATACTION_DECODE_STYLE_NAME_REF:
                {
                    OUString aAttrValue( rAttrValue );
                    if( XMLTransformerBase::DecodeStyleName(aAttrValue) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            }
        }
    }

    XMLTokenEnum eToken = XML_FOOTNOTE;
    switch( m_eTypeToken )
    {
    case XML_NOTE:
        eToken = (m_bEndNote ? XML_ENDNOTE : XML_FOOTNOTE);
        break;
    case XML_NOTES_CONFIGURATION:
        eToken = (m_bEndNote ? XML_ENDNOTES_CONFIGURATION
                             : XML_FOOTNOTES_CONFIGURATION);
        break;
    case XML_NOTE_REF:
        eToken = (m_bEndNote ? XML_ENDNOTE_REF : XML_FOOTNOTE_REF);
        break;
    default:
        OSL_ENSURE( XML_NOTE==m_eTypeToken, "invalid note type" );
        break;
    }

    SetExportQName( GetTransformer().GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_TEXT,
                            ::xmloff::token::GetXMLToken( eToken ) ) );
    if( m_bPersistent )
        XMLPersElemContentTContext::StartElement( xAttrList );
    else
        GetTransformer().GetDocHandler()->startElement( GetExportQName(),
                                                        xAttrList );
}

void XMLNotesTransformerContext::EndElement()
{
    if( m_bPersistent )
    {
        XMLPersElemContentTContext::EndElement();
    }
    else
    {
        GetTransformer().GetDocHandler()->endElement( GetExportQName() );
    }
}

rtl::Reference<XMLTransformerContext> XMLNotesTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    rtl::Reference<XMLTransformerContext> pContext;
    if( XML_NOTE == m_eTypeToken )
    {
        if( XML_NAMESPACE_TEXT == nPrefix )
        {
            XMLTokenEnum eToken ( XML_TOKEN_INVALID );
            if( IsXMLToken( rLocalName, XML_NOTE_CITATION ) )
            {
                eToken = m_bEndNote ? XML_ENDNOTE_CITATION
                                  : XML_FOOTNOTE_CITATION;
            }
            else if( IsXMLToken( rLocalName, XML_NOTE_BODY ) )
            {
                eToken = m_bEndNote ? XML_ENDNOTE_BODY
                                  : XML_FOOTNOTE_BODY;
            }

            if( XML_TOKEN_INVALID != eToken )
            {
                if( m_bPersistent  )
                {
                    pContext.set(new XMLPersTextContentTContext(
                                    GetTransformer(), rQName,
                                    XML_NAMESPACE_TEXT,
                                    eToken ));
                    AddContent( pContext );

                }
                else
                {
                    pContext.set(new XMLRenameElemTransformerContext(
                                    GetTransformer(), rQName,
                                    XML_NAMESPACE_TEXT,
                                    eToken ));
                }
            }
        }
    }

    if( !pContext.is() )
    {
        pContext = m_bPersistent
                        ? XMLPersElemContentTContext::CreateChildContext(
                                nPrefix, rLocalName, rQName, rAttrList )
                        : XMLTransformerContext::CreateChildContext(
                                nPrefix, rLocalName, rQName, rAttrList );
    }

    return pContext;
}

bool XMLNotesTransformerContext::IsPersistent() const
{
    return m_bPersistent;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
