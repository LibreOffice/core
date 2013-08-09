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
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include "IgnoreTContext.hxx"
#include "ActionMapTypesOOo.hxx"
#include "AttrTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"
#include "FormPropOOoTContext.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

class XMLFormPropValueTContext_Impl : public XMLTransformerContext
{
    OUString m_aAttrQName;
    OUString m_aCharacters;
    sal_Bool m_bPersistent;
    sal_Bool m_bIsVoid;

public:
    TYPEINFO();

    // element content persistence only
    XMLFormPropValueTContext_Impl( XMLTransformerBase& rTransformer,
                           const OUString& rQName );
    XMLFormPropValueTContext_Impl( XMLTransformerBase& rTransformer,
                           const OUString& rQName,
                              sal_uInt16 nAttrPrefix,
                           XMLTokenEnum eAttrToken );

    virtual ~XMLFormPropValueTContext_Impl();

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void EndElement();

    virtual void Characters( const OUString& rChars );

    virtual sal_Bool IsPersistent() const;

    sal_Bool IsVoid() const { return m_bIsVoid; }
    const OUString& GetTextContent() const { return m_aCharacters; }
};

TYPEINIT1( XMLFormPropValueTContext_Impl, XMLTransformerContext );

XMLFormPropValueTContext_Impl::XMLFormPropValueTContext_Impl(
        XMLTransformerBase& rTransformer,
        const OUString& rQName ) :
    XMLTransformerContext( rTransformer, rQName ),
    m_bPersistent( sal_True ),
    m_bIsVoid( sal_False )
{
}

XMLFormPropValueTContext_Impl::XMLFormPropValueTContext_Impl(
        XMLTransformerBase& rTransformer,
        const OUString& rQName,
        sal_uInt16 nAttrPrefix,
        XMLTokenEnum eAttrToken ) :
    XMLTransformerContext( rTransformer, rQName ),
    m_aAttrQName( rTransformer.GetNamespaceMap().GetQNameByKey(
                    nAttrPrefix, GetXMLToken(eAttrToken) ) ),
    m_bPersistent( sal_True ),
    m_bIsVoid( sal_False )
{
}

XMLFormPropValueTContext_Impl::~XMLFormPropValueTContext_Impl()
{
}

void XMLFormPropValueTContext_Impl::StartElement(
    const Reference< XAttributeList >& rAttrList )
{
    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = rAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                 &aLocalName );
        if( XML_NAMESPACE_FORM == nPrefix &&
            IsXMLToken( aLocalName, XML_PROPERTY_IS_VOID ) &&
             IsXMLToken( rAttrList->getValueByIndex( i ), XML_TRUE ) )
            m_bIsVoid = sal_True;
    }
}

void XMLFormPropValueTContext_Impl::EndElement()
{
    if( !m_bPersistent )
    {
        XMLMutableAttributeList *pMutableAttrList =
            new XMLMutableAttributeList;
        Reference< XAttributeList > xAttrList( pMutableAttrList );
        pMutableAttrList->AddAttribute( m_aAttrQName,
                                        m_aCharacters );

        OUString aElemQName( GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_FORM, GetXMLToken(XML_LIST_VALUE) ) );
        GetTransformer().GetDocHandler()->startElement( aElemQName,
                                                    xAttrList );
        GetTransformer().GetDocHandler()->endElement( aElemQName );
    }
}

void XMLFormPropValueTContext_Impl::Characters( const OUString& rChars )
{
    m_aCharacters += rChars;
}

sal_Bool XMLFormPropValueTContext_Impl::IsPersistent() const
{
    return m_bPersistent;
}

TYPEINIT1( XMLFormPropOOoTransformerContext, XMLTransformerContext );

XMLFormPropOOoTransformerContext::XMLFormPropOOoTransformerContext(
        XMLTransformerBase& rImp,
        const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rQName ),
    m_nValueTypeAttr( -1 ),
    m_eValueToken( XML_VALUE ),
    m_eValueTypeToken( XML_TOKEN_END ),
    m_bIsList( sal_False )
{
}

XMLFormPropOOoTransformerContext::~XMLFormPropOOoTransformerContext()
{
}

XMLTransformerContext *XMLFormPropOOoTransformerContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const OUString& rQName,
        const Reference< XAttributeList >& )
{
    XMLTransformerContext *pContext = 0;

    if( XML_NAMESPACE_FORM == nPrefix &&
        IsXMLToken( rLocalName, XML_PROPERTY_VALUE ) )
    {
        if( m_bIsList )
        {
            pContext = new XMLFormPropValueTContext_Impl( GetTransformer(),
                                                          rQName,
                                                          XML_NAMESPACE_OFFICE,
                                                          m_eValueToken );
        }
        else if( !m_xValueContext.is() )
        {
            m_xValueContext=
                new XMLFormPropValueTContext_Impl( GetTransformer(), rQName );
            pContext = m_xValueContext.get();
        }
    }

    // default is ignore
    if( !pContext )
        pContext = new XMLIgnoreTransformerContext( GetTransformer(), rQName,
                                             sal_True, sal_True );
    return pContext;
}

void XMLFormPropOOoTransformerContext::StartElement(
    const Reference< XAttributeList >& rAttrList )
{

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_FORM_PROP_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    XMLMutableAttributeList *pMutableAttrList =
        new XMLMutableAttributeList( rAttrList, sal_True );
    m_xAttrList = pMutableAttrList;

    OUString aValueType;
    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = rAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                 &aLocalName );
        XMLTransformerActions::key_type aKey( nPrefix, aLocalName );
        XMLTransformerActions::const_iterator aIter =
            pActions->find( aKey );
        if( !(aIter == pActions->end() ) )
        {
            const OUString& rAttrValue = rAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_RENAME:
                if( IsXMLToken( aLocalName, XML_PROPERTY_TYPE ) )
                {
                    aValueType = rAttrValue;
                    m_nValueTypeAttr = i;
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
                if( IsXMLToken( aLocalName, XML_PROPERTY_IS_LIST ) )
                {
                    m_aElemQName =
                        GetTransformer().GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_FORM, GetXMLToken( XML_LIST_PROPERTY ) );
                    m_bIsList = sal_True;
                }
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

    if( !aValueType.isEmpty() )
    {
        if( IsXMLToken( aValueType, XML_STRING ) )
            m_eValueToken = XML_STRING_VALUE;
        else if( IsXMLToken( aValueType, XML_BOOLEAN ) )
            m_eValueToken = XML_BOOLEAN_VALUE;
        else if( IsXMLToken( aValueType, XML_SHORT ) ||
                 IsXMLToken( aValueType, XML_INT ) ||
                 IsXMLToken( aValueType, XML_LONG ) ||
                 IsXMLToken( aValueType, XML_DOUBLE ) )
            m_eValueTypeToken = XML_FLOAT;
    }
}

void XMLFormPropOOoTransformerContext::EndElement()
{
    if( m_xValueContext.is() )
    {
        if( m_xValueContext->IsVoid() )
        {
            m_eValueTypeToken = XML_VOID;
        }
        else
        {
            OUString aAttrQName(
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_OFFICE, GetXMLToken(m_eValueToken) ) );
            static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                ->AddAttribute( aAttrQName, m_xValueContext->GetTextContent() );
        }
    }

    if( m_nValueTypeAttr != -1 && m_eValueTypeToken != XML_TOKEN_END )
    {
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
            ->SetValueByIndex( m_nValueTypeAttr ,
                                GetXMLToken( m_eValueTypeToken ) );
    }

    GetTransformer().GetDocHandler()->startElement( m_aElemQName,
                                                    m_xAttrList );
    GetTransformer().GetDocHandler()->endElement( m_aElemQName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
