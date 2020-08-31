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

#include <officecfg/Office/Common.hxx>
#include <sax/tools/converter.hxx>

#include <com/sun/star/util/PathSubstitution.hpp>
#include <com/sun/star/util/XStringSubstitution.hpp>
#include <xmloff/DocumentSettingsContext.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <comphelper/base64.hxx>

#include <vector>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/formula/SymbolDescriptor.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/document/NamedPropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include <unotools/configmgr.hxx>
#include "xmlenums.hxx"

using namespace com::sun::star;
using namespace ::xmloff::token;

namespace {

class XMLMyList
{
    std::vector<beans::PropertyValue> aProps;
    sal_uInt32                        nCount;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

public:
    explicit XMLMyList(const uno::Reference<uno::XComponentContext>& rxContext);

    void push_back(beans::PropertyValue const & aProp) { aProps.push_back(aProp); nCount++; }
    uno::Sequence<beans::PropertyValue> GetSequence();
    uno::Reference<container::XNameContainer> GetNameContainer();
    uno::Reference<container::XIndexContainer> GetIndexContainer();
};

}

XMLMyList::XMLMyList(const uno::Reference<uno::XComponentContext>& rxContext)
:   nCount(0),
    m_xContext(rxContext)
{
    assert(m_xContext.is());
}

uno::Sequence<beans::PropertyValue> XMLMyList::GetSequence()
{
    uno::Sequence<beans::PropertyValue> aSeq;
    if(nCount)
    {
        assert(nCount == aProps.size());
        aSeq.realloc(nCount);
        beans::PropertyValue* pProps = aSeq.getArray();
        for (auto const& prop : aProps)
        {
            *pProps = prop;
            ++pProps;
        }
    }
    return aSeq;
}

uno::Reference<container::XNameContainer> XMLMyList::GetNameContainer()
{
    uno::Reference<container::XNameContainer> xNameContainer = document::NamedPropertyValues::create(m_xContext);
    for (auto const& prop : aProps)
    {
        xNameContainer->insertByName(prop.Name, prop.Value);
    }

    return xNameContainer;
}

uno::Reference<container::XIndexContainer> XMLMyList::GetIndexContainer()
{
    uno::Reference<container::XIndexContainer> xIndexContainer = document::IndexedPropertyValues::create(m_xContext);
    sal_uInt32 i(0);
    for (auto const& prop : aProps)
    {
        xIndexContainer->insertByIndex(i, prop.Value);
        ++i;
    }

    return xIndexContainer;
}

namespace {

class XMLConfigBaseContext : public SvXMLImportContext
{
protected:
    XMLMyList                   maProps;
    beans::PropertyValue        maProp;
    css::uno::Any&              mrAny;
    XMLConfigBaseContext*       mpBaseContext;
public:
    XMLConfigBaseContext(SvXMLImport& rImport,
                                    css::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext);

    void AddPropertyValue() { maProps.push_back(maProp); }

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
                const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
};

class XMLConfigItemContext : public SvXMLImportContext
{
    OUString               msType;
    css::uno::Any&         mrAny;
    const OUString         mrItemName;
    XMLConfigBaseContext*  mpBaseContext;
    OUStringBuffer         maCharBuffer;

public:
    XMLConfigItemContext(SvXMLImport& rImport,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
                                    css::uno::Any& rAny,
                                    const OUString& rItemName,
                                    XMLConfigBaseContext* pBaseContext);

    virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
                const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}

    virtual void SAL_CALL characters( const OUString& rChars ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    void ManipulateConfigItem();
};

class XMLConfigItemSetContext : public XMLConfigBaseContext
{
public:
    XMLConfigItemSetContext(SvXMLImport& rImport,
                                    css::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class XMLConfigItemMapNamedContext : public XMLConfigBaseContext
{
public:
    XMLConfigItemMapNamedContext(SvXMLImport& rImport,
                                    css::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

class XMLConfigItemMapIndexedContext : public XMLConfigBaseContext
{
private:
    OUString maConfigItemName;

public:
    XMLConfigItemMapIndexedContext(SvXMLImport& rImport,
                                    css::uno::Any& rAny,
                                    const OUString& rConfigItemName,
                                    XMLConfigBaseContext* pBaseContext);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

}

static SvXMLImportContext *CreateSettingsContext(SvXMLImport& rImport, sal_Int32 nElement,
                        const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                        beans::PropertyValue& rProp, XMLConfigBaseContext* pBaseContext)
{
    SvXMLImportContext *pContext = nullptr;

    rProp.Name.clear();
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if (aIter.getToken() == XML_ELEMENT(CONFIG, XML_NAME))
            rProp.Name = aIter.toString();
    }

    if (nElement == XML_ELEMENT(CONFIG, XML_CONFIG_ITEM))
        pContext = new XMLConfigItemContext(rImport, xAttrList, rProp.Value, rProp.Name, pBaseContext);
    else if(nElement == XML_ELEMENT(CONFIG, XML_CONFIG_ITEM_SET) ||
            nElement == XML_ELEMENT(CONFIG, XML_CONFIG_ITEM_MAP_ENTRY) )
        pContext = new XMLConfigItemSetContext(rImport, rProp.Value, pBaseContext);
    else if(nElement == XML_ELEMENT(CONFIG, XML_CONFIG_ITEM_MAP_NAMED))
        pContext = new XMLConfigItemMapNamedContext(rImport, rProp.Value, pBaseContext);
    else if(nElement == XML_ELEMENT(CONFIG, XML_CONFIG_ITEM_MAP_INDEXED))
        pContext = new XMLConfigItemMapIndexedContext(rImport, rProp.Value, rProp.Name, pBaseContext);

    return pContext;
}

XMLDocumentSettingsContext::XMLDocumentSettingsContext(SvXMLImport& rImport)
    : SvXMLImportContext( rImport )
{
    // here are no attributes
}

XMLDocumentSettingsContext::~XMLDocumentSettingsContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler >  XMLDocumentSettingsContext::createFastChildContext(
            sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    OUString sName;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if (aIter.getToken() == XML_ELEMENT(CONFIG, XML_NAME))
            sName = aIter.toString();
    }

    if (nElement == XML_ELEMENT(CONFIG, XML_CONFIG_ITEM_SET))
    {
        OUString aLocalConfigName;
        sal_uInt16 nConfigPrefix =
            GetImport().GetNamespaceMap().GetKeyByAttrValueQName(
                                        sName, &aLocalConfigName );

        if( XML_NAMESPACE_OOO == nConfigPrefix )
        {
            if (IsXMLToken(aLocalConfigName, XML_VIEW_SETTINGS))
                pContext = new XMLConfigItemSetContext(GetImport(),
                                    maViewProps, nullptr);
            else if (IsXMLToken(aLocalConfigName,
                                            XML_CONFIGURATION_SETTINGS))
                pContext = new XMLConfigItemSetContext(GetImport(),
                                    maConfigProps, nullptr);
            else
            {
                maDocSpecificSettings.push_back( {aLocalConfigName, uno::Any()} );

                pContext = new XMLConfigItemSetContext(GetImport(),
                                    maDocSpecificSettings.back().aSettings, nullptr);
            }
        }
    }

    return pContext;
}

void XMLDocumentSettingsContext::endFastElement(sal_Int32 )
{
    uno::Sequence<beans::PropertyValue> aSeqViewProps;
    if (maViewProps >>= aSeqViewProps)
    {
        GetImport().SetViewSettings(aSeqViewProps);
        sal_Int32 i(aSeqViewProps.getLength() - 1);
        bool bFound(false);
        while((i >= 0) && !bFound)
        {
            if (aSeqViewProps[i].Name == "Views")
            {
                bFound = true;
                uno::Reference<container::XIndexAccess> xIndexAccess;
                if (aSeqViewProps[i].Value >>= xIndexAccess)
                {
                    uno::Reference<document::XViewDataSupplier> xViewDataSupplier(GetImport().GetModel(), uno::UNO_QUERY);
                    if (xViewDataSupplier.is())
                        xViewDataSupplier->setViewData(xIndexAccess);
                }
            }
            else
                i--;
        }
    }

    uno::Sequence<beans::PropertyValue> aSeqConfigProps;
    if ( maConfigProps >>= aSeqConfigProps )
    {
        if (!utl::ConfigManager::IsFuzzing() && !officecfg::Office::Common::Save::Document::LoadPrinter::get())
        {
            sal_Int32 i = aSeqConfigProps.getLength() - 1;
            int nFound = 0;

            while ( ( i >= 0 ) && nFound < 2 )
            {
                OUString sProp( aSeqConfigProps[i].Name );

                if ( sProp == "PrinterName" )
                {
                    aSeqConfigProps[i].Value <<= OUString();
                    nFound++;
                }
                else if ( sProp == "PrinterSetup" )
                {
                    uno::Sequence< sal_Int8 > aEmpty;
                    aSeqConfigProps[i].Value <<= aEmpty;
                    nFound++;
                }

                i--;
            }
        }

        GetImport().SetConfigurationSettings( aSeqConfigProps );
    }

    for (auto const& settings : maDocSpecificSettings)
    {
        uno::Sequence< beans::PropertyValue > aDocSettings;
        OSL_VERIFY( settings.aSettings >>= aDocSettings );
        GetImport().SetDocumentSpecificSettings( settings.sGroupName, aDocSettings );
    }
}

XMLConfigBaseContext::XMLConfigBaseContext(SvXMLImport& rImport,
        css::uno::Any& rTempAny,
        XMLConfigBaseContext* pTempBaseContext)
    : SvXMLImportContext( rImport ),
    maProps( rImport.GetComponentContext() ),
    maProp(),
    mrAny(rTempAny),
    mpBaseContext(pTempBaseContext)
{
}

XMLConfigItemSetContext::XMLConfigItemSetContext(SvXMLImport& rImport,
                                    css::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext)
    : XMLConfigBaseContext( rImport, rAny, pBaseContext )
{
    // here are no attributes
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLConfigItemSetContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    return CreateSettingsContext(GetImport(), nElement, xAttrList, maProp, this);
}

void XMLConfigItemSetContext::endFastElement(sal_Int32 )
{
    mrAny <<= maProps.GetSequence();
    if (mpBaseContext)
        mpBaseContext->AddPropertyValue();
}

XMLConfigItemContext::XMLConfigItemContext(SvXMLImport& rImport,
                                    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
                                    css::uno::Any& rTempAny,
                                    const OUString& rTempItemName,
                                    XMLConfigBaseContext* pTempBaseContext)
    : SvXMLImportContext(rImport),
    mrAny(rTempAny),
    mrItemName(rTempItemName),
    mpBaseContext(pTempBaseContext)
{
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        if (aIter.getToken() == XML_ELEMENT(CONFIG, XML_TYPE))
            msType = aIter.toString();
    }
}

void XMLConfigItemContext::characters( const OUString& rChars )
{
    maCharBuffer.append(rChars);
}

void XMLConfigItemContext::endFastElement(sal_Int32 )
{
    OUString sValue;
    uno::Sequence<sal_Int8> aDecoded;
    if (IsXMLToken(msType, XML_BASE64BINARY))
    {
        OUString sChars = maCharBuffer.makeStringAndClear().trim();
        if( !sChars.isEmpty() )
            ::comphelper::Base64::decodeSomeChars( aDecoded, sChars );
    }
    else
        sValue = maCharBuffer.makeStringAndClear();

    if (mpBaseContext)
    {
        if (IsXMLToken(msType, XML_BOOLEAN))
        {
            bool bValue(false);
            if (IsXMLToken(sValue, XML_TRUE))
                bValue = true;
            mrAny <<= bValue;
        }
        else if (IsXMLToken(msType, XML_BYTE))
        {
            sal_Int32 nValue(0);
            ::sax::Converter::convertNumber(nValue, sValue);
            mrAny <<= static_cast<sal_Int8>(nValue);
        }
        else if (IsXMLToken(msType, XML_SHORT))
        {
            sal_Int32 nValue(0);
            ::sax::Converter::convertNumber(nValue, sValue);
            mrAny <<= static_cast<sal_Int16>(nValue);
        }
        else if (IsXMLToken(msType, XML_INT))
        {
            sal_Int32 nValue(0);
            ::sax::Converter::convertNumber(nValue, sValue);
            mrAny <<= nValue;
        }
        else if (IsXMLToken(msType, XML_LONG))
        {
            sal_Int64 nValue(sValue.toInt64());
            mrAny <<= nValue;
        }
        else if (IsXMLToken(msType, XML_DOUBLE))
        {
            double fValue(0.0);
            ::sax::Converter::convertDouble(fValue, sValue);
            mrAny <<= fValue;
        }
        else if (IsXMLToken(msType, XML_STRING))
        {
            mrAny <<= sValue;
        }
        else if (IsXMLToken(msType, XML_DATETIME))
        {
            util::DateTime aDateTime;
            ::sax::Converter::parseDateTime(aDateTime, sValue);
            mrAny <<= aDateTime;
        }
        else if (IsXMLToken(msType, XML_BASE64BINARY))
        {
            mrAny <<= aDecoded;
        }
        else {
            SAL_INFO("xmloff.core",
                    "XMLConfigItemContext: unknown type: " << msType);
        }

        ManipulateConfigItem();

        mpBaseContext->AddPropertyValue();
    }
    else {
        assert(false && "no BaseContext");
    }
}

/** There are some instances where there is a mismatch between API and
 * XML mapping of a setting. In this case, this method allows us to
 * manipulate the values accordingly. */
void XMLConfigItemContext::ManipulateConfigItem()
{
    if( mrItemName == "PrinterIndependentLayout" )
    {
        OUString sValue;
        mrAny >>= sValue;

        sal_Int16 nTmp = document::PrinterIndependentLayout::HIGH_RESOLUTION;

        if( sValue == "enabled" || sValue == "low-resolution" )
        {
            nTmp = document::PrinterIndependentLayout::LOW_RESOLUTION;
        }
        else if ( sValue == "disabled" )
        {
            nTmp = document::PrinterIndependentLayout::DISABLED;
        }
        // else: default to high_resolution

        mrAny <<= nTmp;
    }
    else if( (mrItemName == "ColorTableURL") || (mrItemName == "LineEndTableURL") || (mrItemName == "HatchTableURL")
          || (mrItemName == "DashTableURL") || (mrItemName == "GradientTableURL") || (mrItemName == "BitmapTableURL") )
    {
        try
        {
            uno::Reference< uno::XComponentContext > xContext( GetImport().GetComponentContext() );
            uno::Reference< util::XStringSubstitution > xStringSubstitution( util::PathSubstitution::create(xContext) );

            OUString aURL;
            mrAny >>= aURL;
            aURL = xStringSubstitution->substituteVariables( aURL, false );
            mrAny <<= aURL;
        }
        catch( uno::Exception& )
        {
        }
    }
}

XMLConfigItemMapNamedContext::XMLConfigItemMapNamedContext(SvXMLImport& rImport,
                                    css::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext)
    : XMLConfigBaseContext(rImport, rAny, pBaseContext)
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLConfigItemMapNamedContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    return CreateSettingsContext(GetImport(), nElement, xAttrList, maProp, this);
}

void XMLConfigItemMapNamedContext::endFastElement(sal_Int32 )
{
    if (mpBaseContext)
    {
        mrAny <<= maProps.GetNameContainer();
        mpBaseContext->AddPropertyValue();
    }
    else {
        assert(false && "no BaseContext");
    }
}

XMLConfigItemMapIndexedContext::XMLConfigItemMapIndexedContext(SvXMLImport& rImport,
                                    css::uno::Any& rAny,
                                    const OUString& rConfigItemName,
                                    XMLConfigBaseContext* pBaseContext)
    : XMLConfigBaseContext(rImport, rAny, pBaseContext),
      maConfigItemName( rConfigItemName )
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLConfigItemMapIndexedContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    return CreateSettingsContext(GetImport(), nElement, xAttrList, maProp, this);
}

void XMLConfigItemMapIndexedContext::endFastElement(sal_Int32 )
{
    if (mpBaseContext)
    {
        if ( maConfigItemName == "ForbiddenCharacters" )
        {
            uno::Reference< i18n::XForbiddenCharacters > xForbChars;

            // get the forbidden characters from the document
            uno::Reference< lang::XMultiServiceFactory > xFac( GetImport().GetModel(), uno::UNO_QUERY );
            if( xFac.is() )
            {
                uno::Reference< beans::XPropertySet > xProps( xFac->createInstance( "com.sun.star.document.Settings" ), uno::UNO_QUERY );
                if( xProps.is() && xProps->getPropertySetInfo()->hasPropertyByName( maConfigItemName ) )
                {
                    xProps->getPropertyValue( maConfigItemName ) >>= xForbChars;
                }
            }

            if( xForbChars.is() )
            {

                uno::Reference< container::XIndexAccess > xIndex = maProps.GetIndexContainer();

                const sal_Int32 nCount = xIndex->getCount();
                uno::Sequence < beans::PropertyValue > aProps;
                for (sal_Int32 i = 0; i < nCount; i++)
                {
                    if ((xIndex->getByIndex( i ) >>= aProps) && (aProps.getLength() == XML_FORBIDDEN_CHARACTER_MAX ) )
                    {
                        /* FIXME-BCP47: this stupid and counterpart in
                         * xmloff/source/core/SettingsExportHelper.cxx
                         * XMLSettingsExportHelper::exportForbiddenCharacters()
                         * */

                        beans::PropertyValue *pForChar = aProps.getArray();
                        i18n::ForbiddenCharacters aForbid;
                        lang::Locale aLocale;
                        bool bHaveLanguage = false, bHaveCountry = false, bHaveVariant = false,
                             bHaveBegin = false, bHaveEnd = false;

                        for ( sal_Int32 j = 0 ; j < XML_FORBIDDEN_CHARACTER_MAX ; j++ )
                        {
                            if (pForChar->Name == "Language")
                            {
                                pForChar->Value >>= aLocale.Language;
                                bHaveLanguage = true;
                            }
                            else if (pForChar->Name == "Country")
                            {
                                pForChar->Value >>= aLocale.Country;
                                bHaveCountry = true;
                            }
                            else if (pForChar->Name == "Variant")
                            {
                                pForChar->Value >>= aLocale.Variant;
                                bHaveVariant = true;
                            }
                            else if (pForChar->Name == "BeginLine")
                            {
                                pForChar->Value >>= aForbid.beginLine;
                                bHaveBegin = true;
                            }
                            else if (pForChar->Name == "EndLine")
                            {
                                pForChar->Value >>= aForbid.endLine;
                                bHaveEnd = true;
                            }
                            pForChar++;
                        }

                        if ( bHaveLanguage && bHaveCountry && bHaveVariant && bHaveBegin && bHaveEnd )
                        {
                            try
                            {
                                xForbChars->setForbiddenCharacters( aLocale, aForbid );
                            }
                            catch (uno::Exception const&)
                            {
                                TOOLS_WARN_EXCEPTION("xmloff.core",
                                    "Exception while importing forbidden characters");
                            }
                        }
                    }
                }
            }
            else
            {
                SAL_WARN("xmloff.core", "could not get the XForbiddenCharacters from document!");
                mrAny <<= maProps.GetIndexContainer();
            }
        }
        else if ( maConfigItemName == "Symbols" )
        {
            uno::Reference< container::XIndexAccess > xIndex = maProps.GetIndexContainer();

            const sal_Int32 nCount = xIndex->getCount();
            uno::Sequence < beans::PropertyValue > aProps;
            uno::Sequence < formula::SymbolDescriptor > aSymbolList ( nCount );

            formula::SymbolDescriptor *pDescriptor = aSymbolList.getArray();

            sal_Int16 nNumFullEntries = 0;

            for ( sal_Int32 i = 0; i < nCount; i++ )
            {
                if ((xIndex->getByIndex( i ) >>= aProps) && (aProps.getLength() == XML_SYMBOL_DESCRIPTOR_MAX ) )
                {
                    bool bHaveName = false, bHaveExportName = false, bHaveCharSet = false,
                         bHaveFontName = false, bHaveFamily = false, bHavePitch = false,
                         bHaveWeight = false, bHaveItalic = false, bHaveSymbolSet = false,
                         bHaveCharacter = false;
                    beans::PropertyValue *pSymbol = aProps.getArray();

                    for ( sal_Int32 j = 0 ; j < XML_SYMBOL_DESCRIPTOR_MAX ; j++ )
                    {
                        if (pSymbol->Name == "Name")
                        {
                            pSymbol->Value >>= pDescriptor[nNumFullEntries].sName;
                            bHaveName = true;
                        }
                        else if (pSymbol->Name == "ExportName")
                        {
                            pSymbol->Value >>= pDescriptor[nNumFullEntries].sExportName;
                            bHaveExportName = true;
                        }
                        else if (pSymbol->Name == "FontName")
                        {
                            pSymbol->Value >>= pDescriptor[nNumFullEntries].sFontName;
                            bHaveFontName = true;
                        }
                        else if (pSymbol->Name == "CharSet")
                        {
                            pSymbol->Value >>= pDescriptor[nNumFullEntries].nCharSet;
                            bHaveCharSet = true;
                        }
                        else if (pSymbol->Name == "Family")
                        {
                            pSymbol->Value >>= pDescriptor[nNumFullEntries].nFamily;
                            bHaveFamily = true;
                        }
                        else if (pSymbol->Name == "Pitch")
                        {
                            pSymbol->Value >>= pDescriptor[nNumFullEntries].nPitch;
                            bHavePitch = true;
                        }
                        else if (pSymbol->Name == "Weight")
                        {
                            pSymbol->Value >>= pDescriptor[nNumFullEntries].nWeight;
                            bHaveWeight = true;
                        }
                        else if (pSymbol->Name == "Italic")
                        {
                            pSymbol->Value >>= pDescriptor[nNumFullEntries].nItalic;
                            bHaveItalic = true;
                        }
                        else if (pSymbol->Name == "SymbolSet")
                        {
                            pSymbol->Value >>= pDescriptor[nNumFullEntries].sSymbolSet;
                            bHaveSymbolSet = true;
                        }
                        else if (pSymbol->Name == "Character")
                        {
                            pSymbol->Value >>= pDescriptor[nNumFullEntries].nCharacter;
                            bHaveCharacter = true;
                        }
                        pSymbol++;
                    }
                    if ( bHaveName && bHaveExportName && bHaveCharSet && bHaveFontName && bHaveCharacter
                         && bHaveFamily && bHavePitch && bHaveWeight && bHaveItalic && bHaveSymbolSet)
                        nNumFullEntries++;
                }
            }
            aSymbolList.realloc (nNumFullEntries);
            mrAny <<= aSymbolList;
        }
        else
        {
            mrAny <<= maProps.GetIndexContainer();
        }
        mpBaseContext->AddPropertyValue();
    }
    else {
        assert(false && "no BaseContext");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
