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
#include "CreateElemTContext.hxx"
#include "MutableAttrList.hxx"
#include "TransformerBase.hxx"
#include "TransformerActions.hxx"
#include "TContextVector.hxx"
#include "FlatTContext.hxx"
#include "AttrTransformerAction.hxx"
#include <xmloff/nmspmap.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLCreateElemTransformerContext, XMLTransformerContext );

XMLCreateElemTransformerContext::XMLCreateElemTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_nActionMap( nActionMap )
{
}

XMLCreateElemTransformerContext::~XMLCreateElemTransformerContext()
{
}

void XMLCreateElemTransformerContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    Reference< XAttributeList > xAttrList( rAttrList );

    XMLTransformerContextVector aChildContexts;

    XMLMutableAttributeList *pMutableAttrList = 0;
    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( m_nActionMap );
    OSL_ENSURE( pActions, "go no actions" );
    if( pActions )
    {
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for( sal_Int16 i=0; i < nAttrCount; ++i )
        {
            const OUString& rAttrName = xAttrList->getNameByIndex( i );
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix =
                GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                           &aLocalName );

            XMLTransformerActions::key_type aKey( nPrefix, aLocalName );
            XMLTransformerActions::const_iterator aIter =
                    pActions->find( aKey );
            if( !(aIter == pActions->end() ) )
            {
                if( !pMutableAttrList )
                {
                    pMutableAttrList = new XMLMutableAttributeList( xAttrList );
                    xAttrList = pMutableAttrList;
                }
                sal_uInt32 nAction = (*aIter).second.m_nActionType;
                switch( nAction )
                {
                case XML_ATACTION_MOVE_TO_ELEM:
                    {
                        OUString aElemQName(
                            GetTransformer().GetNamespaceMap().GetQNameByKey(
                                (*aIter).second.GetQNamePrefixFromParam1(),
                                ::xmloff::token::GetXMLToken(
                                (*aIter).second.GetQNameTokenFromParam1()) ) );
                        XMLTransformerContext *pContext =
                            new XMLPersTextContentTContext( GetTransformer(),
                                                       aElemQName );
                        pContext->Characters( rAttrValue );
                        XMLTransformerContextVector::value_type aVal(
                                pContext );
                        aChildContexts.push_back( aVal );
                        pMutableAttrList->RemoveAttributeByIndex( i );
                        --i;
                        --nAttrCount;
                    }
                    break;
                default:
                    OSL_ENSURE( !this, "unknown action" );
                    break;
                }
            }
        }
    }
    XMLTransformerContext::StartElement( xAttrList );

    XMLTransformerContextVector::iterator aIter = aChildContexts.begin();

    for( ; aIter != aChildContexts.end(); ++aIter )
    {
        (*aIter)->Export();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
