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

#include <sal/config.h>

#include <vector>

#include "CreateElemTContext.hxx"
#include "MutableAttrList.hxx"
#include "TransformerBase.hxx"
#include "TransformerActions.hxx"
#include "FlatTContext.hxx"
#include "AttrTransformerAction.hxx"
#include <xmloff/nmspmap.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

XMLCreateElemTransformerContext::XMLCreateElemTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
       sal_uInt16 nActionMap ) :
    XMLTransformerContext( rImp, rQName ),
    m_nActionMap( nActionMap )
{
}

void XMLCreateElemTransformerContext::StartElement(
        const Reference< XAttributeList >& rAttrList )
{
    Reference< XAttributeList > xAttrList( rAttrList );

    std::vector<rtl::Reference<XMLTransformerContext>> aChildContexts;

    XMLMutableAttributeList *pMutableAttrList = nullptr;
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
            if( aIter != pActions->end() )
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
                        rtl::Reference<XMLTransformerContext> pContext(
                            new XMLPersTextContentTContext( GetTransformer(),
                                                       aElemQName ));
                        pContext->Characters( rAttrValue );
                        aChildContexts.push_back(pContext);
                        pMutableAttrList->RemoveAttributeByIndex( i );
                        --i;
                        --nAttrCount;
                    }
                    break;
                default:
                    OSL_ENSURE( false, "unknown action" );
                    break;
                }
            }
        }
    }
    XMLTransformerContext::StartElement( xAttrList );

    for (auto const & i: aChildContexts)
    {
        i->Export();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
