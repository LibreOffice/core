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
#include "MutableAttrList.hxx"
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include "ActionMapTypesOASIS.hxx"
#include "AttrTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"
#include "FormPropOASISTContext.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

TYPEINIT1( XMLFormPropOASISTransformerContext,
                XMLRenameElemTransformerContext );

XMLTokenEnum XMLFormPropOASISTransformerContext::GetValueType(
                const OUString& rValue )
{
    XMLTokenEnum eRet = XML_DOUBLE;
    sal_Bool bNeg = sal_False;
    sal_uInt32 nVal = 0;

    sal_Int32 nPos = 0;
    sal_Int32 nLen = rValue.getLength();

    // skip white space
    while( nPos < nLen && sal_Unicode(' ') == rValue[nPos] )
        nPos++;

    if( nPos < nLen && sal_Unicode('-') == rValue[nPos] )
    {
        bNeg = sal_True;
        nPos++;
    }

    // get number
    sal_Bool bOverflow = sal_False;
    while( nPos < nLen &&
           sal_Unicode('0') <= rValue[nPos] &&
           sal_Unicode('9') >= rValue[nPos] )
    {
        nVal *= 10;
        nVal += (rValue[nPos] - sal_Unicode('0'));
        bOverflow |= (nVal > (bNeg ? 2147483648UL : 2147483647UL));
        nPos++;
    }

    // skip white space
    while( nPos < nLen && sal_Unicode(' ') == rValue[nPos] )
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
    sal_Bool bIsVoid = sal_False;
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
                        bIsVoid = sal_True;
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
                OSL_ENSURE( !this, "unknown action" );
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
