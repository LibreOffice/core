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
#include "MutableAttrList.hxx"
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlimp.hxx>
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
        sal_Int32 rQName ) :
    XMLPersElemContentTContext( rImp, rQName ),
    m_aElemQName( XML_ELEMENT(DRAW, XML_FRAME ) )
{
}

void XMLFrameOOoTransformerContext::startFastElement(sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_FRAME_ATTR_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    Reference< XFastAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList =
        GetTransformer().ProcessAttrList( xAttrList, OOO_SHAPE_ACTIONS,
                                          true );
    if( !pMutableAttrList )
        pMutableAttrList = new XMLMutableAttributeList( rAttrList );
    xAttrList = pMutableAttrList;

    XMLMutableAttributeList *pFrameMutableAttrList =
        new XMLMutableAttributeList;
    Reference< XFastAttributeList > xFrameAttrList( pFrameMutableAttrList );

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        sal_Int32 rAttrName = xAttrList->getTokenByIndex( i );
        XMLTransformerActions::key_type aKey( rAttrName );
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

    GetTransformer().GetDocHandler()->startFastElement( m_aElemQName,
                                                    xFrameAttrList );
    XMLTransformerContext::startFastElement( nElement, xAttrList );
}

rtl::Reference<XMLTransformerContext> XMLFrameOOoTransformerContext::createFastChildContext(
        sal_Int32 nElement,
        const Reference< XFastAttributeList >& rAttrList )
{
    rtl::Reference<XMLTransformerContext> pContext;

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_FRAME_ELEM_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );
    XMLTransformerActions::key_type aKey( nElement );
    XMLTransformerActions::const_iterator aIter = pActions->find( aKey );

    if( aIter != pActions->end() )
    {
        switch( (*aIter).second.m_nActionType )
        {
        case XML_ETACTION_COPY:
        case XML_ETACTION_COPY_TEXT:
        case XML_ETACTION_RENAME_ELEM:
            // the ones in the list have to be persistent

            pContext = XMLPersElemContentTContext::createFastChildContext(
                           nElement, rAttrList );
            break;
        default:
            OSL_ENSURE( false, "unknown action" );
            break;
        }
    }

    // default is copying
    if( !pContext.is() )
        pContext = XMLTransformerContext::createFastChildContext(
                    nElement, rAttrList );

    return pContext;
}

void XMLFrameOOoTransformerContext::endFastElement(sal_Int32 nElement)
{
    XMLTransformerContext::endFastElement(nElement);
    ExportContent();
    GetTransformer().GetDocHandler()->endFastElement( m_aElemQName );
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
