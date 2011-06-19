/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "FrameOOoTContext.hxx"
#include "IgnoreTContext.hxx"
#include "MutableAttrList.hxx"
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include "ActionMapTypesOOo.hxx"
#include "AttrTransformerAction.hxx"
#include "ElemTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLFrameOOoTransformerContext, XMLPersElemContentTContext );

XMLFrameOOoTransformerContext::XMLFrameOOoTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLPersElemContentTContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_DRAW,
                            ::xmloff::token::GetXMLToken( XML_FRAME ) ) )
{
}

XMLFrameOOoTransformerContext::~XMLFrameOOoTransformerContext()
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
                                          sal_True );
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
        if( !(aIter == pActions->end() ) )
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
                OSL_ENSURE( !this, "unknown action" );
                break;
            }
        }
    }

    GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                    xFrameAttrList );
    XMLTransformerContext::StartElement( xAttrList );
}

XMLTransformerContext *XMLFrameOOoTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = 0;

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_FRAME_ELEM_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );
    XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
    XMLTransformerActions::const_iterator aIter = pActions->find( aKey );

    if( !(aIter == pActions->end()) )
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
            OSL_ENSURE( !this, "unknown action" );
            break;
        }
    }

    // default is copying
    if( !pContext )
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

sal_Bool XMLFrameOOoTransformerContext::IsPersistent() const
{
    // this context stores some of its child elements, but is not persistent
    // itself.
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
