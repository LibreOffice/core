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

#include <SwXMLBlockImport.hxx>
#include <SwXMLTextBlocks.hxx>
#include <xmloff/xmlictxt.hxx>
#include <unotools/charclass.hxx>
#include <swtypes.hxx>

#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-register"
#endif
#endif
#include <tokens.cxx>
#if defined __clang__
#if __has_warning("-Wdeprecated-register")
#pragma GCC diagnostic pop
#endif
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace css::xml::sax;

class SwXMLBlockListImport;
class SwXMLTextBlockImport;

namespace {

class SwXMLBlockListContext : public SvXMLImportContext
{
private:
    SwXMLBlockListImport& m_rLocalRef;

public:
    SwXMLBlockListContext( SwXMLBlockListImport& rImport,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;
};

class SwXMLBlockContext : public SvXMLImportContext
{
public:
    SwXMLBlockContext( SwXMLBlockListImport& rImport,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList );
};

class SwXMLTextBlockDocumentContext : public SvXMLImportContext
{
private:
    SwXMLTextBlockImport& m_rLocalRef;

public:
    SwXMLTextBlockDocumentContext( SwXMLTextBlockImport& rImport );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;
};

class SwXMLTextBlockBodyContext : public SvXMLImportContext
{
private:
    SwXMLTextBlockImport& m_rLocalRef;

public:
    SwXMLTextBlockBodyContext( SwXMLTextBlockImport& rImport );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32, const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;
};

class SwXMLTextBlockTextContext : public SvXMLImportContext
{
private:
    SwXMLTextBlockImport& m_rLocalRef;

public:
    SwXMLTextBlockTextContext( SwXMLTextBlockImport& rImport );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;
};

class SwXMLTextBlockParContext : public SvXMLImportContext
{
private:
    SwXMLTextBlockImport& m_rLocalRef;

public:
    SwXMLTextBlockParContext( SwXMLTextBlockImport & rImport );

    virtual void SAL_CALL characters( const OUString & aChars ) override;

    virtual ~SwXMLTextBlockParContext() override;
};

}

SwXMLTextBlockTokenHandler::SwXMLTextBlockTokenHandler()
{
}

SwXMLTextBlockTokenHandler::~SwXMLTextBlockTokenHandler()
{
}

sal_Int32 SAL_CALL SwXMLTextBlockTokenHandler::getTokenFromUTF8( const Sequence< sal_Int8 >& Identifier )
{
    return getTokenDirect( reinterpret_cast< const char* >( Identifier.getConstArray() ), Identifier.getLength() );
}

Sequence< sal_Int8 > SAL_CALL SwXMLTextBlockTokenHandler::getUTF8Identifier( sal_Int32 )
{
    return Sequence< sal_Int8 >();
}

sal_Int32 SwXMLTextBlockTokenHandler::getTokenDirect( const char *pTag, sal_Int32 nLength ) const
{
    if( !nLength )
        nLength = strlen( pTag );
    const struct xmltoken* pToken = TextBlockTokens::in_word_set( pTag, nLength );
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

SwXMLBlockListTokenHandler::SwXMLBlockListTokenHandler()
{
}

SwXMLBlockListTokenHandler::~SwXMLBlockListTokenHandler()
{
}

sal_Int32 SAL_CALL SwXMLBlockListTokenHandler::getTokenFromUTF8( const Sequence< sal_Int8 >& Identifier )
{
    return getTokenDirect( reinterpret_cast< const char* >( Identifier.getConstArray() ), Identifier.getLength() );
}

Sequence< sal_Int8 > SAL_CALL SwXMLBlockListTokenHandler::getUTF8Identifier( sal_Int32 )
{
    return Sequence< sal_Int8 >();
}

sal_Int32 SwXMLBlockListTokenHandler::getTokenDirect( const char *pTag, sal_Int32 nLength ) const
{
    if( !nLength )
        nLength = strlen( pTag );
    const struct xmltoken* pToken = BlockListTokens::in_word_set( pTag, nLength );
    return pToken ? pToken->nToken : XML_TOKEN_INVALID;
}

SwXMLBlockListContext::SwXMLBlockListContext(
    SwXMLBlockListImport& rImport, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
    : SvXMLImportContext(rImport)
    , m_rLocalRef(rImport)
{
    if( xAttrList.is() && xAttrList->hasAttribute( SwXMLBlockListToken::LIST_NAME ) )
        rImport.getBlockList().SetName( xAttrList->getValue( SwXMLBlockListToken::LIST_NAME ) );
}

uno::Reference< ::xml::sax::XFastContextHandler > SAL_CALL
SwXMLBlockListContext::createFastChildContext( sal_Int32 Element,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    if ( Element == SwXMLBlockListToken::BLOCK )
        return new SwXMLBlockContext(m_rLocalRef, xAttrList);
    return nullptr;
}

SwXMLBlockContext::SwXMLBlockContext(
    SwXMLBlockListImport& rImport,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList ) :
    SvXMLImportContext( rImport )
{
    static const CharClass & rCC = GetAppCharClass();
    OUString aShort, aLong, aPackageName;
    bool bTextOnly = false;
    if( xAttrList.is() )
    {
        if( xAttrList->hasAttribute( SwXMLBlockListToken::ABBREVIATED_NAME ) )
            aShort = rCC.uppercase( xAttrList->getValue( SwXMLBlockListToken::ABBREVIATED_NAME ) );
        if( xAttrList->hasAttribute( SwXMLBlockListToken::NAME ) )
            aLong = xAttrList->getValue( SwXMLBlockListToken::NAME );
        if( xAttrList->hasAttribute( SwXMLBlockListToken::PACKAGE_NAME ) )
            aPackageName = xAttrList->getValue( SwXMLBlockListToken::PACKAGE_NAME );
        if( xAttrList->hasAttribute( SwXMLBlockListToken::UNFORMATTED_TEXT ) )
        {
            OUString rAttrValue( xAttrList->getValue( SwXMLBlockListToken::UNFORMATTED_TEXT ) );
            if( IsXMLToken( rAttrValue, XML_TRUE ) )
                bTextOnly = true;
        }
    }
    if (aShort.isEmpty() || aLong.isEmpty() || aPackageName.isEmpty())
        return;
    rImport.getBlockList().AddName( aShort, aLong, aPackageName, bTextOnly);
}

SwXMLTextBlockDocumentContext::SwXMLTextBlockDocumentContext(SwXMLTextBlockImport& rImport)
    : SvXMLImportContext(rImport)
    , m_rLocalRef(rImport)
{
}

uno::Reference< ::xml::sax::XFastContextHandler > SAL_CALL
SwXMLTextBlockDocumentContext::createFastChildContext( sal_Int32 Element,
    const uno::Reference< xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    if ( Element == SwXMLTextBlockToken::OFFICE_BODY )
        return new SwXMLTextBlockBodyContext(m_rLocalRef);
    return nullptr;
}

SwXMLTextBlockTextContext::SwXMLTextBlockTextContext(SwXMLTextBlockImport& rImport)
    : SvXMLImportContext(rImport)
    , m_rLocalRef(rImport)
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
SwXMLTextBlockTextContext::createFastChildContext( sal_Int32 Element,
    const uno::Reference< xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    if ( Element == SwXMLTextBlockToken::TEXT_P )
        return new SwXMLTextBlockParContext(m_rLocalRef);
    return nullptr;
}

SwXMLTextBlockBodyContext::SwXMLTextBlockBodyContext(SwXMLTextBlockImport& rImport)
    : SvXMLImportContext(rImport)
    , m_rLocalRef(rImport)
{
}

uno::Reference < xml::sax::XFastContextHandler > SAL_CALL
SwXMLTextBlockBodyContext::createFastChildContext( sal_Int32 Element,
    const uno::Reference< xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    if( Element == SwXMLTextBlockToken::OFFICE_TEXT )
        return new SwXMLTextBlockTextContext(m_rLocalRef);
    else if( Element == SwXMLTextBlockToken::TEXT_P )
        return new SwXMLTextBlockParContext(m_rLocalRef);
    return nullptr;
}

SwXMLTextBlockParContext::SwXMLTextBlockParContext(SwXMLTextBlockImport& rImport)
    : SvXMLImportContext(rImport)
    , m_rLocalRef(rImport)
{
}

void SAL_CALL SwXMLTextBlockParContext::characters( const OUString & aChars )
{
    m_rLocalRef.m_rText += aChars;
}

SwXMLTextBlockParContext::~SwXMLTextBlockParContext()
{
    if (m_rLocalRef.m_bTextOnly)
        m_rLocalRef.m_rText += "\015";
    else
    {
        if (!m_rLocalRef.m_rText.endsWith(" "))
            m_rLocalRef.m_rText += " ";
    }
}

// SwXMLBlockListImport //////////////////////////////
SwXMLBlockListImport::SwXMLBlockListImport(
    const uno::Reference< uno::XComponentContext >& rContext,
    SwXMLTextBlocks &rBlocks )
:   SvXMLImport( rContext, "", SvXMLImportFlags::NONE ),
    m_rBlockList (rBlocks)
{
}

SwXMLBlockListImport::~SwXMLBlockListImport()
    throw ()
{
}

SvXMLImportContext* SwXMLBlockListImport::CreateFastContext( sal_Int32 Element,
    const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    if( Element == SwXMLBlockListToken::BLOCK_LIST )
        return new SwXMLBlockListContext( *this, xAttrList );
    return nullptr;
}

SwXMLTextBlockImport::SwXMLTextBlockImport(
    const uno::Reference< uno::XComponentContext >& rContext,
    OUString & rNewText,
    bool bNewTextOnly )
:   SvXMLImport(rContext, "", SvXMLImportFlags::ALL ),
    m_bTextOnly ( bNewTextOnly ),
    m_rText ( rNewText )
{
}

SwXMLTextBlockImport::~SwXMLTextBlockImport()
    throw()
{
}

SvXMLImportContext* SwXMLTextBlockImport::CreateFastContext( sal_Int32 Element,
    const uno::Reference< xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    if( Element == SwXMLTextBlockToken::OFFICE_DOCUMENT ||
        Element == SwXMLTextBlockToken::OFFICE_DOCUMENT_CONTENT )
        return new SwXMLTextBlockDocumentContext( *this );
    return nullptr;
}

void SAL_CALL SwXMLTextBlockImport::endDocument()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
