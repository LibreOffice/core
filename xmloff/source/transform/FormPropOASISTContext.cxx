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
#include "FormPropOASISTContext.hxx"
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

XMLTokenEnum XMLFormPropOASISTransformerContext::GetValueType(
                const OUString& rValue )
{
    XMLTokenEnum eRet = XML_DOUBLE;
    bool bNeg = false;
    sal_uInt32 nVal = 0;

    sal_Int32 nPos = 0;
    sal_Int32 nLen = rValue.getLength();

    // skip white space
    while( nPos < nLen && ' ' == rValue[nPos] )
        nPos++;

    if( nPos < nLen && '-' == rValue[nPos] )
    {
        bNeg = true;
        nPos++;
    }

    // get number
    bool bOverflow = false;
    while( nPos < nLen &&
           '0' <= rValue[nPos] &&
           '9' >= rValue[nPos] )
    {
        nVal *= 10;
        nVal += (rValue[nPos] - '0');
        bOverflow |= (nVal > (bNeg ? 2147483648UL : 2147483647UL));
        nPos++;
    }

    // skip white space
    while( nPos < nLen && ' ' == rValue[nPos] )
        nPos++;

    if( nPos == nLen )
    {
        // It's a integer number
        if( bOverflow )
            eRet = XML_LONG;
        else if( nVal > (bNeg ? 32768UL : 32767UL) )
            eRet = XML_INT;
        else
            eRet = XML_SHORT;
    }

    return eRet;
}

XMLFormPropOASISTransformerContext::XMLFormPropOASISTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName,
        XMLTokenEnum eLocalName ) :
    XMLRenameElemTransformerContext( rImp, rQName, XML_NAMESPACE_FORM,
                                       XML_PROPERTY ),
    m_bIsList( XML_LIST_PROPERTY == eLocalName),
    m_bIsListValue( XML_LIST_VALUE == eLocalName)
{
}

XMLFormPropOASISTransformerContext::~XMLFormPropOASISTransformerContext()
{
}

void XMLFormPropOASISTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_FORM_PROP_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    XMLMutableAttributeList *pMutableAttrList =
        new XMLMutableAttributeList( rAttrList );
    Reference< XAttributeList > xAttrList( pMutableAttrList );

    sal_Int16 nValueTypeAttr = -1;
    OUString aValue;
    bool bIsVoid = false;
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
            case XML_ATACTION_RENAME:
                if( IsXMLToken( aLocalName, XML_VALUE_TYPE ) )
                {
                    if( IsXMLToken( rAttrValue, XML_FLOAT ) )
                    {
                        nValueTypeAttr = i;
                    }
                    else if( IsXMLToken( rAttrValue, XML_VOID ) )
                    {
                        pMutableAttrList->SetValueByIndex( i,
                                GetXMLToken( XML_SHORT ) );
                        bIsVoid = true;
                    }
                }
                {
                    OUString aNewAttrQName(
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                                (*aIter).second.GetQNamePrefixFromParam1(),
                                ::xmloff::token::GetXMLToken(
                                    (*aIter).second.GetQNameTokenFromParam1()) ) );
                    pMutableAttrList->RenameAttributeByIndex( i, aNewAttrQName );
                }
                break;
            case XML_ATACTION_REMOVE:
                if( !IsXMLToken( aLocalName, XML_CURRENCY ) )
                    aValue = rAttrValue;
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
    if( m_bIsList )
    {
        OUString aNewAttrQName(
                GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_FORM,
                    GetXMLToken( XML_PROPERTY_IS_LIST ) ) );
        pMutableAttrList->AddAttribute( aNewAttrQName,
                                        GetXMLToken( XML_TRUE ) );
    }

    if( nValueTypeAttr != -1 )
        pMutableAttrList->SetValueByIndex( nValueTypeAttr,
                                GetXMLToken( GetValueType( aValue ) ) );

    if( !m_bIsListValue )
        XMLRenameElemTransformerContext::StartElement( xAttrList );
    if( !m_bIsList )
    {
        pMutableAttrList =
            new XMLMutableAttributeList;
        xAttrList = pMutableAttrList;
        if( bIsVoid )
        {
            OUString aNewAttrQName(
                GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_FORM, GetXMLToken( XML_PROPERTY_IS_VOID ) ) );
            pMutableAttrList->AddAttribute( aNewAttrQName,
                                        GetXMLToken( XML_TRUE ) );
        }

        OUString aValueElemQName(
            GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_FORM, GetXMLToken( XML_PROPERTY_VALUE ) ) );
        GetTransformer().GetDocHandler()->startElement( aValueElemQName,
                                                        xAttrList );
        GetTransformer().GetDocHandler()->characters( aValue );
        GetTransformer().GetDocHandler()->endElement( aValueElemQName );
    }
}

void XMLFormPropOASISTransformerContext::EndElement()
{
    if( !m_bIsListValue )
        XMLRenameElemTransformerContext::EndElement();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
