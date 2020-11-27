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
#include "IgnoreTContext.hxx"
#include "ActionMapTypesOOo.hxx"
#include "AttrTransformerAction.hxx"
#include "TransformerActions.hxx"
#include "TransformerBase.hxx"
#include "FormPropOOoTContext.hxx"
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;

class XMLFormPropValueTContext_Impl : public XMLTransformerContext
{
    OUString m_aCharacters;
    bool m_bIsVoid;

public:
    // element content persistence only
    XMLFormPropValueTContext_Impl( XMLTransformerBase& rTransformer,
                           sal_Int32 rQName );

    virtual void startFastElement(sal_Int32 nElement,
                    const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttribs) override;

    virtual void endFastElement(sal_Int32 nElement) override;

    virtual void Characters( const OUString& rChars ) override;

    virtual bool IsPersistent() const override;

    bool IsVoid() const { return m_bIsVoid; }
    const OUString& GetTextContent() const { return m_aCharacters; }
};

XMLFormPropValueTContext_Impl::XMLFormPropValueTContext_Impl(
        XMLTransformerBase& rTransformer,
        sal_Int32 rQName ) :
    XMLTransformerContext( rTransformer, rQName ),
    m_bIsVoid( false )
{
}

void XMLFormPropValueTContext_Impl::startFastElement(sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{
    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        sal_Int32 rAttrName = rAttrList->getTokenByIndex( i );
        if( rAttrName == XML_ELEMENT(FORM, XML_PROPERTY_IS_VOID) &&
             IsXMLToken( rAttrList->getValueByIndex( i ), XML_TRUE ) )
            m_bIsVoid = true;
    }
}

void XMLFormPropValueTContext_Impl::endFastElement(sal_Int32 )
{
}

void XMLFormPropValueTContext_Impl::Characters( const OUString& rChars )
{
    m_aCharacters += rChars;
}

bool XMLFormPropValueTContext_Impl::IsPersistent() const
{
    return true;
}

XMLFormPropOOoTransformerContext::XMLFormPropOOoTransformerContext(
        XMLTransformerBase& rImp,
        sal_Int32 rQName ) :
    XMLTransformerContext( rImp, rQName ),
    m_aElemQName( rQName ),
    m_nValueTypeAttr( -1 ),
    m_eValueToken( XML_VALUE ),
    m_eValueTypeToken( XML_TOKEN_END ),
    m_bIsList( false )
{
}

XMLFormPropOOoTransformerContext::~XMLFormPropOOoTransformerContext()
{
}

rtl::Reference<XMLTransformerContext> XMLFormPropOOoTransformerContext::createFastChildContext(
        sal_Int32 nElement,
        const Reference< XFastAttributeList >& )
{
    rtl::Reference<XMLTransformerContext> pContext;

    if( nElement == XML_ELEMENT(FORM, XML_PROPERTY_VALUE) )
    {
        if( m_bIsList )
        {
            pContext.set(new XMLFormPropValueTContext_Impl( GetTransformer(),
                                                          nElement ));
        }
        else if( !m_xValueContext.is() )
        {
            m_xValueContext=
                new XMLFormPropValueTContext_Impl( GetTransformer(), nElement );
            pContext.set(m_xValueContext.get());
        }
    }

    // default is ignore
    if( !pContext.is() )
        pContext.set(new XMLIgnoreTransformerContext( GetTransformer(), nElement,
                                             true, true ));
    return pContext;
}

void XMLFormPropOOoTransformerContext::startFastElement(sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList > & rAttrList)
{

    XMLTransformerActions *pActions =
        GetTransformer().GetUserDefinedActions( OOO_FORM_PROP_ACTIONS );
    OSL_ENSURE( pActions, "go no actions" );

    XMLMutableAttributeList *pMutableAttrList =
        new XMLMutableAttributeList( rAttrList, true );
    m_xAttrList = pMutableAttrList;

    OUString aValueType;
    sal_Int16 nAttrCount = rAttrList.is() ? rAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        sal_Int32 rAttrName = rAttrList->getTokenByIndex( i );
        XMLTransformerActions::key_type aKey( rAttrName );
        XMLTransformerActions::const_iterator aIter =
            pActions->find( aKey );
        if( aIter != pActions->end() )
        {
            const OUString& rAttrValue = rAttrList->getValueByIndex( i );
            switch( (*aIter).second.m_nActionType )
            {
            case XML_ATACTION_RENAME:
                if( (rAttrName & TOKEN_MASK) == XML_PROPERTY_TYPE )
                {
                    aValueType = rAttrValue;
                    m_nValueTypeAttr = i;
                }
                {
                    pMutableAttrList->RenameAttributeByIndex( i, (*aIter).second.GetTokenFromParam1() );
                }
                break;
            case XML_ATACTION_REMOVE:
                if( (rAttrName & TOKEN_MASK) == XML_PROPERTY_IS_LIST )
                {
                    m_aElemQName = XML_ELEMENT(FORM, XML_LIST_PROPERTY);
                    m_bIsList = true;
                }
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

void XMLFormPropOOoTransformerContext::endFastElement(sal_Int32 )
{
    if( m_xValueContext.is() )
    {
        if( m_xValueContext->IsVoid() )
        {
            m_eValueTypeToken = XML_VOID;
        }
        else
        {
            static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
                ->AddAttribute( XML_ELEMENT(OFFICE,m_eValueToken) , m_xValueContext->GetTextContent() );
        }
    }

    if( m_nValueTypeAttr != -1 && m_eValueTypeToken != XML_TOKEN_END )
    {
        static_cast< XMLMutableAttributeList * >( m_xAttrList.get() )
            ->SetValueByIndex( m_nValueTypeAttr ,
                                GetXMLToken( m_eValueTypeToken ) );
    }

    GetTransformer().GetDocHandler()->startFastElement( m_aElemQName, m_xAttrList );
    GetTransformer().GetDocHandler()->endFastElement( m_aElemQName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
