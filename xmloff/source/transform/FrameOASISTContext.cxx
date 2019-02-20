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

#include "FrameOASISTContext.hxx"
#include "IgnoreTContext.hxx"
#include "MutableAttrList.hxx"
#include <xmloff/xmlnmspe.hxx>
#include "ActionMapTypesOASIS.hxx"
#include "ElemTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"
#include <osl/diagnose.h>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

bool XMLFrameOASISTransformerContext::IsLinkedEmbeddedObject(
            const OUString& rLocalName,
            const Reference< XAttributeList >& rAttrList )
{
    if( !(IsXMLToken( rLocalName, XML_OBJECT ) ||
          IsXMLToken( rLocalName, XML_OBJECT_OLE)  ) )
        return false;

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
            if (sHRef.isEmpty())
            {
                // When the href is empty then the object is not linked but
                // a placeholder.
                return false;
            }
            GetTransformer().ConvertURIToOOo( sHRef, true );
            return !(!sHRef.isEmpty() && '#'==sHRef[0]);
        }
    }

    return false;
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
    m_xAttrList = new XMLMutableAttributeList( rAttrList, true );

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

rtl::Reference<XMLTransformerContext> XMLFrameOASISTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& rAttrList )
{
    rtl::Reference<XMLTransformerContext> pContext;

    if( m_bIgnoreElement )
    {
        // do not export the frame element and all of its children
        pContext.set(new XMLIgnoreTransformerContext( GetTransformer(),
                                                                rQName,
                                                                true, true ));
    }
    else
    {
        XMLTransformerActions *pActions =
            GetTransformer().GetUserDefinedActions( OASIS_FRAME_ELEM_ACTIONS );
        OSL_ENSURE( pActions, "go no actions" );
        XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
        XMLTransformerActions::const_iterator aIter = pActions->find( aKey );

        if( aIter != pActions->end() )
        {
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ETACTION_COPY:
                if( m_aElemQName.isEmpty() &&
                    !IsLinkedEmbeddedObject( rLocalName, rAttrList ) )
                {
                    pContext.set(new XMLIgnoreTransformerContext( GetTransformer(),
                                                                rQName,
                                                                false, false ));
                    m_aElemQName = rQName;
                    static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                        ->AppendAttributeList( rAttrList );
                    GetTransformer().ProcessAttrList( m_xAttrList,
                                                      OASIS_SHAPE_ACTIONS,
                                                      false );
                    GetTransformer().GetDocHandler()->startElement( m_aElemQName, m_xAttrList );
                }
                else
                {
                    pContext.set(new XMLIgnoreTransformerContext( GetTransformer(),
                                                                rQName,
                                                                true, true ));
                }
                break;
            default:
                OSL_ENSURE( false, "unknown action" );
                break;
            }
        }
    }

    // default is copying
    if( !pContext.is() )
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
    if( !m_aElemQName.isEmpty() && !m_bIgnoreElement )
        XMLTransformerContext::Characters( rChars );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
