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
#include "DeepTContext.hxx"
#include "FlatTContext.hxx"
#include "EventOOoTContext.hxx"
#include "TransformerActions.hxx"
#include "ElemTransformerAction.hxx"
#include "PersMixedContentTContext.hxx"
#include "TransformerBase.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

TYPEINIT1( XMLPersElemContentTContext, XMLPersAttrListTContext );

void XMLPersElemContentTContext::AddContent( XMLTransformerContext *pContext )
{
    OSL_ENSURE( pContext && pContext->IsPersistent(),
                "non-persistent context" );
    XMLTransformerContextVector::value_type aVal( pContext );
    m_aChildContexts.push_back( aVal );
}

XMLPersElemContentTContext::XMLPersElemContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLPersAttrListTContext( rImp, rQName )
{
}

XMLPersElemContentTContext::XMLPersElemContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLPersAttrListTContext( rImp, rQName, nActionMap )
{
}

XMLPersElemContentTContext::XMLPersElemContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken ) :
    XMLPersAttrListTContext( rImp, rQName, nPrefix, eToken )
{
}

XMLPersElemContentTContext::XMLPersElemContentTContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        sal_uInt16 nPrefix,
        ::xmloff::token::XMLTokenEnum eToken,
       sal_uInt16 nActionMap ) :
    XMLPersAttrListTContext( rImp, rQName, nPrefix, eToken, nActionMap )
{
}

XMLPersElemContentTContext::~XMLPersElemContentTContext()
{
}

XMLTransformerContext *XMLPersElemContentTContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& )
{
    XMLTransformerContext *pContext = 0;

    XMLTransformerActions::key_type aKey( nPrefix, rLocalName );
    XMLTransformerActions::const_iterator aIter =
        GetTransformer().GetElemActions().find( aKey );

    if( !(aIter == GetTransformer().GetElemActions().end()) )
    {
        switch( (*aIter).second.m_nActionType )
        {
        case XML_ETACTION_COPY:
            pContext = new XMLPersMixedContentTContext( GetTransformer(),
                                                       rQName );
            break;
        case XML_ETACTION_COPY_TEXT:
            pContext = new XMLPersMixedContentTContext( GetTransformer(),
                                                       rQName );
            break;
        case XML_ETACTION_RENAME_ELEM:
            pContext = new XMLPersMixedContentTContext( GetTransformer(), rQName,
                    (*aIter).second.GetQNamePrefixFromParam1(),
                    (*aIter).second.GetQNameTokenFromParam1() );
            break;
        case XML_ETACTION_RENAME_ELEM_PROC_ATTRS:
            pContext = new XMLPersMixedContentTContext( GetTransformer(), rQName,
                    (*aIter).second.GetQNamePrefixFromParam1(),
                    (*aIter).second.GetQNameTokenFromParam1(),
                       static_cast< sal_uInt16 >( (*aIter).second.m_nParam2 ) );
            break;
        case XML_ETACTION_RENAME_ELEM_ADD_PROC_ATTR:
            {
                XMLPersMixedContentTContext *pMC =
                    new XMLPersMixedContentTContext( GetTransformer(), rQName,
                        (*aIter).second.GetQNamePrefixFromParam1(),
                        (*aIter).second.GetQNameTokenFromParam1(),
                        static_cast< sal_uInt16 >(
                            (*aIter).second.m_nParam3  >> 16 ) );
                pMC->AddAttribute(
                    (*aIter).second.GetQNamePrefixFromParam2(),
                    (*aIter).second.GetQNameTokenFromParam2(),
                       static_cast< ::xmloff::token::XMLTokenEnum >(
                        (*aIter).second.m_nParam3 & 0xffff ) );
                pContext = pMC;
            }
            break;
        case XML_ETACTION_PROC_ATTRS:
            pContext = new XMLPersMixedContentTContext( GetTransformer(), rQName,
                       static_cast< sal_uInt16 >( (*aIter).second.m_nParam1 ) );
            break;
        default:
            pContext = GetTransformer().CreateUserDefinedContext(
                            (*aIter).second, rQName, sal_True );
            OSL_ENSURE( pContext && pContext->IsPersistent(),
                        "unknown or not persistent action" );
            if( pContext && !pContext->IsPersistent() )
            {
                delete pContext;
                pContext = 0;
            }
            break;
        }
    }

    // default is copying
    if( !pContext )
        pContext = new XMLPersMixedContentTContext( GetTransformer(), rQName );
    XMLTransformerContextVector::value_type aVal( pContext );
    m_aChildContexts.push_back( aVal );

    return pContext;
}

void XMLPersElemContentTContext::ExportContent()
{
    XMLTransformerContextVector::iterator aIter = m_aChildContexts.begin();

    for( ; aIter != m_aChildContexts.end(); ++aIter )
    {
        (*aIter)->Export();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
