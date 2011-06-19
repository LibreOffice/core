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
#include "FrameOASISTContext.hxx"
#include "IgnoreTContext.hxx"
#include "MutableAttrList.hxx"
#include "xmloff/xmlnmspe.hxx"
#include "ActionMapTypesOASIS.hxx"
#include "ElemTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLFrameOASISTransformerContext, XMLTransformerContext );

sal_Bool XMLFrameOASISTransformerContext::IsLinkedEmbeddedObject(
            const OUString& rLocalName,
            const Reference< XAttributeList >& rAttrList )
{
    if( !(IsXMLToken( rLocalName, XML_OBJECT ) ||
          IsXMLToken( rLocalName, XML_OBJECT_OLE)  ) )
        return sal_False;

    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString aAttrName( rAttrList->getNameByIndex( i ) );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( aAttrName,
                                                                 &aLocalName );
        if( XML_NAMESPACE_XLINK == nPrefix &&
            IsXMLToken( aLocalName, XML_HREF ) )
        {
            OUString sHRef( rAttrList->getValueByIndex( i ) );
            if (sHRef.getLength() == 0)
            {
                // When the href is empty then the object is not linked but
                // a placeholder.
                return sal_False;
            }
            GetTransformer().ConvertURIToOOo( sHRef, sal_True );
            return !(sHRef.getLength() && '#'==sHRef[0]);
        }
    }

    return sal_False;
}


XMLFrameOASISTransformerContext::XMLFrameOASISTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName ),
    m_bIgnoreElement( false )
{
}

XMLFrameOASISTransformerContext::~XMLFrameOASISTransformerContext()
{
}

void XMLFrameOASISTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    m_xAttrList = new XMLMutableAttributeList( rAttrList, sal_True );

    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = rAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );

        if( (nPrefix == XML_NAMESPACE_PRESENTATION) && IsXMLToken( aLocalName, XML_CLASS ) )
        {
            const OUString& rAttrValue = rAttrList->getValueByIndex( i );
            if( IsXMLToken( rAttrValue, XML_HEADER ) || IsXMLToken( rAttrValue, XML_FOOTER ) ||
                IsXMLToken( rAttrValue, XML_PAGE_NUMBER ) || IsXMLToken( rAttrValue, XML_DATE_TIME ) )
            {
                m_bIgnoreElement = true;
                break;
            }
        }
    }
}

XMLTransformerContext *XMLFrameOASISTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    XMLTransformerContext *pContext = 0;

    if( m_bIgnoreElement )
    {
        // do not export the frame element and all of its children
        pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                                rQName,
                                                                sal_True, sal_True );
    }
    else
    {
        XMLTransformerActions *pActions =
            GetTransformer().GetUserDefinedActions( OASIS_FRAME_ELEM_ACTIONS );
        OSL_ENSURE( pActions, "go no actions" );
        XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
        XMLTransformerActions::const_iterator aIter = pActions->find( aKey );

        if( !(aIter == pActions->end()) )
        {
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ETACTION_COPY:
                if( !m_aElemQName.getLength() &&
                    !IsLinkedEmbeddedObject( rLocalName, rAttrList ) )
                {
                    pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                                rQName,
                                                                sal_False, sal_False );
                    m_aElemQName = rQName;
                    static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                        ->AppendAttributeList( rAttrList );
                    GetTransformer().ProcessAttrList( m_xAttrList,
                                                      OASIS_SHAPE_ACTIONS,
                                                      sal_False );
                    GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                                    m_xAttrList );
                }
                else
                {
                    pContext = new XMLIgnoreTransformerContext( GetTransformer(),
                                                                rQName,
                                                                sal_True, sal_True );
                }
                break;
            default:
                OSL_ENSURE( !this, "unknown action" );
                break;
            }
        }
    }

    // default is copying
    if( !pContext )
        pContext = XMLTransformerContext::CreateChildContext( nPrefix,
                                                              rLocalName,
                                                              rQName,
                                                              rAttrList );

    return pContext;
}

void XMLFrameOASISTransformerContext::EndElement()
{
    if( !m_bIgnoreElement )
        GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

void XMLFrameOASISTransformerContext::Characters( const OUString& rChars )
{
    // ignore
    if( m_aElemQName.getLength() && !m_bIgnoreElement )
        XMLTransformerContext::Characters( rChars );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
