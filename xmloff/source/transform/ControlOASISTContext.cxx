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

#include "MutableAttrList.hxx"
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "ActionMapTypesOASIS.hxx"
#include "AttrTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"
#include "ControlOASISTContext.hxx"
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

XMLControlOASISTransformerContext::XMLControlOASISTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        bool bCreateControl ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rImp.GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_FORM,
                            ::xmloff::token::GetXMLToken( XML_CONTROL ) ) ),
    m_bCreateControl( bCreateControl )
{
}

void XMLControlOASISTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_FORM_CONTROL_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    Reference< XAttributeList > xAttrList( rAttrList );
    XMLMutableAttributeList *pMutableAttrList = nullptr;
//      GetTransformer().ProcessAttrList( xAttrList, OOO_SHAPE_ACTIONS,
//                                        sal_True );

    XMLMutableAttributeList *pControlMutableAttrList =
        m_bCreateControl ? new XMLMutableAttributeList : nullptr;
    Reference< XAttributeList > xControlAttrList( pControlMutableAttrList );

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
            if( !pMutableAttrList )
            {
                pMutableAttrList =
                    new XMLMutableAttributeList( rAttrList );
                xAttrList = pMutableAttrList;
            }
            const OUString& rAttrValue = xAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_MOVE_TO_ELEM:
                if( m_bCreateControl )
                {
                    pControlMutableAttrList->AddAttribute( rAttrName,
                                                           rAttrValue );
                    pMutableAttrList->RemoveAttributeByIndex( i );
                    --i;
                    --nAttrCount;
                }
                break;
            case XML_ATACTION_RENAME_REMOVE_NAMESPACE_PREFIX:
                {
                    OUString aAttrValue( rAttrValue );
                    sal_uInt16 nValPrefix =
                        static_cast<sal_uInt16>( (*aIter).second.m_nParam2 );
                    GetTransformer().RemoveNamespacePrefix( aAttrValue,
                                                            nValPrefix );
                    OUString aNewAttrQName(
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                            (*aIter).second.GetQNamePrefixFromParam1(),
                            ::xmloff::token::GetXMLToken(
                                (*aIter).second.GetQNameTokenFromParam1()) ) );
                    if( m_bCreateControl )
                    {
                        pControlMutableAttrList->AddAttribute( aNewAttrQName,
                                                               aAttrValue );
                        pMutableAttrList->RemoveAttributeByIndex( i );
                        --i;
                        --nAttrCount;
                    }
                    else
                    {
                        pMutableAttrList->RenameAttributeByIndex( i,
                                                              aNewAttrQName );
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                    }
                }
                break;
            case XML_ATACTION_URI_OASIS:
                {
                    OUString aAttrValue( rAttrValue );
                    if( GetTransformer().ConvertURIToOOo( aAttrValue,
                           static_cast< bool >((*aIter).second.m_nParam1)) )
                        pMutableAttrList->SetValueByIndex( i, aAttrValue );
                }
                break;
            default:
                OSL_ENSURE( false, "unknown action" );
                break;
            }
        }
    }

    if( m_bCreateControl )
        GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                        xControlAttrList );
    XMLTransformerContext::StartElement( xAttrList );
}

void XMLControlOASISTransformerContext::EndElement()
{
    XMLTransformerContext::EndElement();
    if( m_bCreateControl )
        GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
