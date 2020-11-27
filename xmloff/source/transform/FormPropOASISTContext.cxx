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
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlimp.hxx>
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
        // It's an integer number
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
        sal_Int32 rQName,
        XMLTokenEnum eLocalName ) :
    XMLRenameElemTransformerContext( rImp, rQName, XML_ELEMENT(FORM,XML_PROPERTY) ),
    m_bIsList( XML_LIST_PROPERTY == eLocalName),
    m_bIsListValue( XML_LIST_VALUE == eLocalName)
{
}

XMLFormPropOASISTransformerContext::~XMLFormPropOASISTransformerContext()
{
}

void XMLFormPropOASISTransformerContext::startFastElement(sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OASIS_FORM_PROP_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    XMLMutableAttributeList *pMutableAttrList =
        new XMLMutableAttributeList( rAttrList );
    Reference< XFastAttributeList > xAttrList( pMutableAttrList );

    sal_Int16 nValueTypeAttr = -1;
    OUString aValue;
    bool bIsVoid = false;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        sal_Int32 rAttrName = xAttrList->getTokenByIndex( i );
        XMLTransformerActions::key_type aKey( rAttrName );
        XMLTransformerActions::const_iterator aIter =
            pActions->find( aKey );
        if( aIter != pActions->end() )
        {
            OUString rAttrValue = xAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_RENAME:
                if( (rAttrName & TOKEN_MASK) == XML_VALUE_TYPE )
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
                    pMutableAttrList->RenameAttributeByIndex( i, (*aIter).second.GetTokenFromParam1() );
                }
                break;
            case XML_ATACTION_REMOVE:
                if( (rAttrName & TOKEN_MASK) != XML_CURRENCY )
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
        pMutableAttrList->AddAttribute( XML_ELEMENT(FORM, XML_PROPERTY_IS_LIST),
                                        GetXMLToken( XML_TRUE ) );
    }

    if( nValueTypeAttr != -1 )
        pMutableAttrList->SetValueByIndex( nValueTypeAttr,
                                GetXMLToken( GetValueType( aValue ) ) );

    if( !m_bIsListValue )
        XMLRenameElemTransformerContext::startFastElement( nElement, xAttrList );
    if( m_bIsList )
        return;

    pMutableAttrList = new XMLMutableAttributeList;
    xAttrList = pMutableAttrList;
    if( bIsVoid )
    {
        pMutableAttrList->AddAttribute( XML_ELEMENT(FORM, XML_PROPERTY_IS_VOID) ,
                                    GetXMLToken( XML_TRUE ) );
    }

    GetTransformer().GetDocHandler()->startFastElement( XML_ELEMENT(FORM, XML_PROPERTY_VALUE),
                                                    xAttrList );
    GetTransformer().GetDocHandler()->characters( aValue );
    GetTransformer().GetDocHandler()->endFastElement( XML_ELEMENT(FORM, XML_PROPERTY_VALUE) );
}

void XMLFormPropOASISTransformerContext::endFastElement(sal_Int32 nElement)
{
    if( !m_bIsListValue )
        XMLRenameElemTransformerContext::endFastElement(nElement);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
