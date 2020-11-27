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
#include "ElemTransformerAction.hxx"
#include "IgnoreTContext.hxx"
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

namespace {

class XMLParagraphTransformerContext : public XMLTransformerContext
{
public:
    XMLParagraphTransformerContext( XMLTransformerBase& rTransformer,
                           sal_Int32 rQName );

    // Create a children element context. By default, the import's
    // CreateContext method is called to create a new default context.
    virtual rtl::Reference<XMLTransformerContext> createFastChildContext(
                                   sal_Int32 nElement,
                                   const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;
};

}

XMLParagraphTransformerContext::XMLParagraphTransformerContext(
        XMLTransformerBase& rImp,
        sal_Int32 rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

rtl::Reference<XMLTransformerContext> XMLParagraphTransformerContext::createFastChildContext(
        sal_Int32 nElement,
        const Reference< XFastAttributeList >& )
{
    return new XMLIgnoreTransformerContext( GetTransformer(),
                                                nElement, true );
}

namespace {

class XMLPersTextContentRNGTransformTContext : public XMLPersTextContentTContext
{
public:
    XMLPersTextContentRNGTransformTContext(
        XMLTransformerBase& rTransformer,
        sal_Int32 rQName,
        sal_Int32 rQName2 );

    virtual void Characters( const OUString& rChars ) override;
};

}

XMLPersTextContentRNGTransformTContext::XMLPersTextContentRNGTransformTContext(
    XMLTransformerBase& rTransformer,
    sal_Int32 rQName,
    sal_Int32 rQName2 ) :
        XMLPersTextContentTContext(
            rTransformer, rQName, rQName2 )
{}

void XMLPersTextContentRNGTransformTContext::Characters( const OUString& rChars )
{
    OUString aConvChars( rChars );
    XMLTransformerBase::ConvertRNGDateTimeToISO( aConvChars );
    XMLPersTextContentTContext::Characters( aConvChars );
}


void XMLMergeElemTransformerContext::ExportStartElement(sal_Int32 nElement)
{
    for( const auto& rChildContext : m_aChildContexts )
    {
        XMLPersTextContentTContext *pContext = rChildContext.get();
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
            ->AddAttribute( pContext->GetExportQName(),
                            pContext->GetTextContent() );
    }
    XMLTransformerContext::startFastElement( nElement, m_xAttrList );

    m_bStartElementExported = true;
}

XMLMergeElemTransformerContext::XMLMergeElemTransformerContext(
        XMLTransformerBase& rImp,
        sal_Int32 rQName,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_nActionMap( nActionMap ),
    m_bStartElementExported( false )
{
}

void XMLMergeElemTransformerContext::startFastElement(sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{
    XMLMutableAttributeList *pMutableAttrList =
        new XMLMutableAttributeList( rAttrList, true );
    m_xAttrList = pMutableAttrList;

    sal_Int16 nAttrCount = m_xAttrList.is() ? m_xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        sal_Int32 rAttrName = m_xAttrList->getTokenByIndex( i );
        bool bRemove = true;
        switch (rAttrName)
        {
            case XML_ELEMENT(OFFICE, XML_DISPLAY):
            case XML_ELEMENT(OFFICE, XML_AUTHOR):
            case XML_ELEMENT(OFFICE, XML_CREATE_DATE):
            case XML_ELEMENT(OFFICE, XML_CREATE_DATE_STRING):
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

rtl::Reference<XMLTransformerContext> XMLMergeElemTransformerContext::createFastChildContext(
        sal_Int32 nElement,
        const Reference< XFastAttributeList >& rAttrList )
{
    rtl::Reference<XMLTransformerContext> pContext;

    if( !m_bStartElementExported )
    {
        XMLTransformerActions *pActions =
            GetTransformer().GetUserDefinedActions( m_nActionMap );
        OSL_ENSURE( pActions, "go no actions" );
        if( pActions )
        {
            XMLTransformerActions::key_type aKey( nElement );
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
                                    GetTransformer(), nElement,
                                    (*aIter).second.GetTokenFromParam1() ));
                        m_aChildContexts.push_back(pTC);
                        pContext.set(pTC.get());
                    }
                    break;
                case XML_ETACTION_MOVE_TO_ATTR:
                    {
                        rtl::Reference<XMLPersTextContentTContext> pTC(
                            new XMLPersTextContentTContext(
                                    GetTransformer(), nElement,
                                    (*aIter).second.GetTokenFromParam1() ));
                        m_aChildContexts.push_back(pTC);
                        pContext.set(pTC.get());
                    }
                    break;
                case XML_ETACTION_EXTRACT_CHARACTERS:
                    {
                        if( !m_bStartElementExported )
                            ExportStartElement(nElement);
                        pContext.set(
                            new XMLParagraphTransformerContext( GetTransformer(),
                            nElement));
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
            XMLTransformerActions::key_type aKey( nElement );
            XMLTransformerActions::const_iterator aIter =
                pActions->find( aKey );

            if( aIter != pActions->end() )
            {
                switch( (*aIter).second.m_nActionType )
                {
                case XML_ETACTION_EXTRACT_CHARACTERS:
                    {
                        if( !m_bStartElementExported )
                            ExportStartElement(nElement);
                        pContext.set(
                            new XMLParagraphTransformerContext( GetTransformer(),
                            nElement));
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
            ExportStartElement(nElement);
        pContext = XMLTransformerContext::createFastChildContext( nElement,
                                                              rAttrList );
    }

    return pContext;
}

void XMLMergeElemTransformerContext::endFastElement(sal_Int32 nElement)
{
    if( !m_bStartElementExported )
        ExportStartElement(nElement);
    XMLTransformerContext::endFastElement(nElement);
}

void XMLMergeElemTransformerContext::Characters( const OUString& )
{
    // ignore
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
