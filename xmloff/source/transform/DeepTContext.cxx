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

#include "DeepTContext.hxx"
#include "FlatTContext.hxx"
#include "EventOOoTContext.hxx"
#include "TransformerActions.hxx"
#include "ElemTransformerAction.hxx"
#include "PersMixedContentTContext.hxx"
#include "TransformerBase.hxx"
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

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
    XMLTransformerContext *pContext = nullptr;

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
                            (*aIter).second, rQName, true );
            OSL_ENSURE( pContext && pContext->IsPersistent(),
                        "unknown or not persistent action" );
            if( pContext && !pContext->IsPersistent() )
            {
                delete pContext;
                pContext = nullptr;
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
