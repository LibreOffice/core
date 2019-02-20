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

#include "FrameOOoTContext.hxx"
#include "IgnoreTContext.hxx"
#include "MutableAttrList.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "ActionMapTypesOOo.hxx"
#include "AttrTransformerAction.hxx"
#include "ElemTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

XMLFrameOOoTransformerContext::XMLFrameOOoTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLPersElemContentTContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_DRAW,
                            ::xmloff::token::GetXMLToken( XML_FRAME ) ) )
{
}

void XMLFrameOOoTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_FRAME_ATTR_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList =
        GetTransformer().ProcessAttrList( xAttrList, OOO_SHAPE_ACTIONS,
                                          true );
    if( !pMutableAttrList )
        pMutableAttrList = new XMLMutableAttributeList( rAttrList );
    xAttrList = pMutableAttrList;

    XMLMutableAttributeList *pFrameMutableAttrList =
        new XMLMutableAttributeList;
    Reference< XAttributeList > xFrameAttrList( pFrameMutableAttrList );

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
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_MOVE_TO_ELEM:
                pFrameMutableAttrList->AddAttribute( rAttrName, rAttrValue );
                pMutableAttrList->RemoveAttributeByIndex( i );
                --i;
                --nAttrCount;
                break;
            default:
                OSL_ENSURE( false, "unknown action" );
                break;
            }
        }
    }

    GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                    xFrameAttrList );
    XMLTransformerContext::StartElement( xAttrList );
}

rtl::Reference<XMLTransformerContext> XMLFrameOOoTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    rtl::Reference<XMLTransformerContext> pContext;

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_FRAME_ELEM_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );
    XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
    XMLTransformerActions::const_iterator aIter = pActions->find( aKey );

    if( aIter != pActions->end() )
    {
        switch( (*aIter).second.m_nActionType )
        {
        case XML_ETACTION_COPY:
        case XML_ETACTION_COPY_TEXT:
        case XML_ETACTION_RENAME_ELEM:
            // the ones in the list have to be persistent

            pContext = XMLPersElemContentTContext::CreateChildContext(
                           nPrefix, rLocalName, rQName, rAttrList );
            break;
        default:
            OSL_ENSURE( false, "unknown action" );
            break;
        }
    }

    // default is copying
    if( !pContext.is() )
        pContext = XMLTransformerContext::CreateChildContext(
                    nPrefix, rLocalName, rQName, rAttrList );

    return pContext;
}

void XMLFrameOOoTransformerContext::EndElement()
{
    XMLTransformerContext::EndElement();
    ExportContent();
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

void XMLFrameOOoTransformerContext::Characters( const OUString& rChars )
{
    XMLTransformerContext::Characters( rChars );
}

bool XMLFrameOOoTransformerContext::IsPersistent() const
{
    // this context stores some of its child elements, but is not persistent
    // itself.
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
