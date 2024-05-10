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

#include <tools/debug.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/ConversionPropertyType.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <unotools/streamwrap.hxx>

#include "convdic.hxx"
#include "convdicxml.hxx"
#include <linguistic/misc.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;


constexpr OUStringLiteral XML_NAMESPACE_TCD_STRING = u"http://openoffice.org/2003/text-conversion-dictionary";
constexpr OUString CONV_TYPE_HANGUL_HANJA = u"Hangul / Hanja"_ustr;
constexpr OUString CONV_TYPE_SCHINESE_TCHINESE = u"Chinese simplified / Chinese traditional"_ustr;


static OUString ConversionTypeToText( sal_Int16 nConversionType )
{
    OUString aRes;
    if (nConversionType == ConversionDictionaryType::HANGUL_HANJA)
        aRes = CONV_TYPE_HANGUL_HANJA;
    else if (nConversionType == ConversionDictionaryType::SCHINESE_TCHINESE)
        aRes = CONV_TYPE_SCHINESE_TCHINESE;
    return aRes;
}

static sal_Int16 GetConversionTypeFromText( std::u16string_view rText )
{
    sal_Int16 nRes = -1;
    if (rText == CONV_TYPE_HANGUL_HANJA)
        nRes = ConversionDictionaryType::HANGUL_HANJA;
    else if (rText == CONV_TYPE_SCHINESE_TCHINESE)
        nRes = ConversionDictionaryType::SCHINESE_TCHINESE;
    return nRes;
}

namespace {

class ConvDicXMLImportContext :
    public SvXMLImportContext
{
public:
    ConvDicXMLImportContext( ConvDicXMLImport &rImport ) :
        SvXMLImportContext( rImport )
    {
    }

    ConvDicXMLImport & GetConvDicImport()
    {
        return static_cast<ConvDicXMLImport &>(GetImport());
    }

    // SvXMLImportContext
    virtual css::uno::Reference<XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;
};


class ConvDicXMLDictionaryContext_Impl :
    public ConvDicXMLImportContext
{
    LanguageType nLanguage;
    sal_Int16    nConversionType;

public:
    ConvDicXMLDictionaryContext_Impl( ConvDicXMLImport &rImport ) :
        ConvDicXMLImportContext( rImport ),
        nLanguage(LANGUAGE_NONE), nConversionType(-1)
    {
    }

    // SvXMLImportContext
    virtual void SAL_CALL startFastElement( sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
    virtual css::uno::Reference<XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;
};


class ConvDicXMLEntryTextContext_Impl :
    public ConvDicXMLImportContext
{
    OUString    aLeftText;

public:
    ConvDicXMLEntryTextContext_Impl( ConvDicXMLImport &rImport ) :
        ConvDicXMLImportContext( rImport )
    {
    }

    // SvXMLImportContext
    virtual void SAL_CALL startFastElement( sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;
    virtual css::uno::Reference<XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 Element, const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList ) override;

    const OUString &    GetLeftText() const { return aLeftText; }
};


class ConvDicXMLRightTextContext_Impl :
    public ConvDicXMLImportContext
{
    OUString aRightText;
    ConvDicXMLEntryTextContext_Impl &rEntryContext;

public:
    ConvDicXMLRightTextContext_Impl(
            ConvDicXMLImport &rImport,
            ConvDicXMLEntryTextContext_Impl &rParentContext ) :
        ConvDicXMLImportContext( rImport ),
        rEntryContext( rParentContext )
    {
    }

    // SvXMLImportContext
    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;
    virtual void SAL_CALL characters( const OUString &rChars ) override;
};

}

//void ConvDicXMLImportContext::characters(const OUString & /*rChars*/)
//{
    /*
    Whitespace occurring within the content of token elements is "trimmed"
    from the ends (i.e. all whitespace at the beginning and end of the
    content is removed), and "collapsed" internally (i.e. each sequence of
    1 or more whitespace characters is replaced with one blank character).
    */
    //collapsing not done yet!

//}

css::uno::Reference<XFastContextHandler> ConvDicXMLImportContext::createFastChildContext(
        sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    if ( Element == ConvDicXMLToken::TEXT_CONVERSION_DICTIONARY )
        return new ConvDicXMLDictionaryContext_Impl( GetConvDicImport() );
    return nullptr;
}


void ConvDicXMLDictionaryContext_Impl::startFastElement( sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& rxAttrList )
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( rxAttrList ))
    {
        switch (aIter.getToken())
        {
            case XML_NAMESPACE_TCD | XML_LANG:
                nLanguage = LanguageTag::convertToLanguageType( aIter.toString() );
            break;
            case XML_NAMESPACE_TCD | XML_CONVERSION_TYPE:
                nConversionType = GetConversionTypeFromText( aIter.toString() );
            break;
            default:
                ;
        }
    }
    GetConvDicImport().SetLanguage( nLanguage );
    GetConvDicImport().SetConversionType( nConversionType );

}

css::uno::Reference<XFastContextHandler> ConvDicXMLDictionaryContext_Impl::createFastChildContext(
        sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    if ( Element == ConvDicXMLToken::ENTRY )
        return new ConvDicXMLEntryTextContext_Impl( GetConvDicImport() );
    return nullptr;
}

css::uno::Reference<XFastContextHandler> ConvDicXMLEntryTextContext_Impl::createFastChildContext(
        sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    if ( Element == ConvDicXMLToken::RIGHT_TEXT )
        return new ConvDicXMLRightTextContext_Impl( GetConvDicImport(), *this );
    return nullptr;
}

void ConvDicXMLEntryTextContext_Impl::startFastElement(
    sal_Int32 /*Element*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& rxAttrList )
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( rxAttrList ))
    {
        switch (aIter.getToken())
        {
            case XML_NAMESPACE_TCD | XML_LEFT_TEXT:
                aLeftText = aIter.toString();
                break;
            default:
                ;
        }
    }
}


void ConvDicXMLRightTextContext_Impl::characters( const OUString &rChars )
{
    aRightText += rChars;
}

void ConvDicXMLRightTextContext_Impl::endFastElement( sal_Int32 /*nElement*/ )
{
    ConvDic *pDic = GetConvDicImport().GetDic();
    if (pDic)
        pDic->AddEntry( rEntryContext.GetLeftText(), aRightText );
}


bool ConvDicXMLExport::Export()
{
    uno::Reference< document::XExporter > xExporter( this );
    uno::Reference< document::XFilter > xFilter( xExporter, UNO_QUERY );
    xFilter->filter( {} );      // calls exportDoc implicitly

    return bSuccess;
}


ErrCode ConvDicXMLExport::exportDoc( enum ::xmloff::token::XMLTokenEnum /*eClass*/ )
{
    GetNamespaceMap_().Add( u"tcd"_ustr,
            XML_NAMESPACE_TCD_STRING, XML_NAMESPACE_TCD );

    GetDocHandler()->startDocument();

    // Add xmlns line and some other arguments
    AddAttribute( GetNamespaceMap_().GetAttrNameByKey( XML_NAMESPACE_TCD ),
                  GetNamespaceMap_().GetNameByKey( XML_NAMESPACE_TCD ) );
    AddAttribute( XML_NAMESPACE_TCD, u"package"_ustr, u"org.openoffice.Office"_ustr );

    OUString aIsoLang( LanguageTag::convertToBcp47( rDic.nLanguage ) );
    AddAttribute( XML_NAMESPACE_TCD, u"lang"_ustr, aIsoLang );
    OUString aConvType( ConversionTypeToText( rDic.nConversionType ) );
    AddAttribute( XML_NAMESPACE_TCD, u"conversion-type"_ustr, aConvType );

    //!! block necessary in order to have SvXMLElementExport d-tor called
    //!! before the call to endDocument
    {
        SvXMLElementExport aRoot( *this, XML_NAMESPACE_TCD, u"text-conversion-dictionary"_ustr, true, true );
        ExportContent_();
    }

    GetDocHandler()->endDocument();

    bSuccess = true;
    return ERRCODE_NONE;
}


void ConvDicXMLExport::ExportContent_()
{
    // acquire sorted list of all keys
    std::set<OUString>   aKeySet;
    for (auto const& elem : rDic.aFromLeft)
        aKeySet.insert( elem.first );

    for (const OUString& aLeftText : aKeySet)
    {
        AddAttribute( XML_NAMESPACE_TCD, u"left-text"_ustr, aLeftText );
        if (rDic.pConvPropType) // property-type list available?
        {
            sal_Int16 nPropertyType = -1;
            PropTypeMap::iterator aIt2 = rDic.pConvPropType->find( aLeftText );
            if (aIt2 != rDic.pConvPropType->end())
                nPropertyType = (*aIt2).second;
            DBG_ASSERT( nPropertyType, "property-type not found" );
            if (nPropertyType == -1)
                nPropertyType = ConversionPropertyType::NOT_DEFINED;
            AddAttribute( XML_NAMESPACE_TCD, u"property-type"_ustr, OUString::number(  nPropertyType ) );
        }
        SvXMLElementExport aEntryMain( *this, XML_NAMESPACE_TCD,
                u"entry"_ustr , true, true );

        std::pair< ConvMap::iterator, ConvMap::iterator > aRange =
                rDic.aFromLeft.equal_range(aLeftText);
        for (auto aIt = aRange.first;  aIt != aRange.second;  ++aIt)
        {
            DBG_ASSERT( aLeftText == (*aIt).first, "key <-> entry mismatch" );
            OUString aRightText( (*aIt).second );
            SvXMLElementExport aEntryRightText( *this, XML_NAMESPACE_TCD,
                    u"right-text"_ustr , true, false );
            Characters( aRightText );
        }
    }
}

    //!!  see comment for pDic member
ConvDicXMLImport::ConvDicXMLImport( ConvDic *pConvDic ) :
    SvXMLImport ( comphelper::getProcessComponentContext(), u"com.sun.star.lingu2.ConvDicXMLImport"_ustr, SvXMLImportFlags::ALL ),
    pDic        ( pConvDic ), nLanguage(LANGUAGE_NONE), nConversionType(-1)
{
    GetNamespaceMap().Add( GetXMLToken(XML_NP_TCD), GetXMLToken(XML_N_TCD), XML_NAMESPACE_TCD);
}

SvXMLImportContext * ConvDicXMLImport::CreateFastContext(
        sal_Int32 Element,
        const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*xAttrList*/ )
{
    if( Element == ConvDicXMLToken::TEXT_CONVERSION_DICTIONARY )
        return new ConvDicXMLDictionaryContext_Impl( *this );
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
