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
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <comphelper/base64.hxx>

#include <vector>
#include <com/sun/star/i18n/XForbiddenCharacters.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/formula/SymbolDescriptor.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/document/NamedPropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <unotools/configmgr.hxx>
#include "xmlenums.hxx"

using namespace com::sun::star;
using namespace ::xmloff::token;

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

class XMLConfigBaseContext : public SvXMLImportContext
{
protected:
    XMLMyList                   maProps;
    beans::PropertyValue        maProp;
    css::uno::Any&              mrAny;
    XMLConfigBaseContext*       mpBaseContext;
public:
    XMLConfigBaseContext(SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                    css::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext);

    void AddPropertyValue() { maProps.push_back(maProp); }
};

class XMLConfigItemContext : public SvXMLImportContext
{
    OUString               msType;
    OUString               msValue;
    uno::Sequence<sal_Int8>     maDecoded;
    css::uno::Any&         mrAny;
    const OUString         mrItemName;
    XMLConfigBaseContext*  mpBaseContext;

public:
    XMLConfigItemContext(SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                                    css::uno::Any& rAny,
                                    const OUString& rItemName,
                                    XMLConfigBaseContext* pBaseContext);

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
    virtual void Characters( const OUString& rChars ) override;

    virtual void EndElement() override;

    void ManipulateConfigItem();
};

class XMLConfigItemSetContext : public XMLConfigBaseContext
{
public:
    XMLConfigItemSetContext(SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                                    css::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext);

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class XMLConfigItemMapNamedContext : public XMLConfigBaseContext
{
public:
    XMLConfigItemMapNamedContext(SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                                    css::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext);

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

class XMLConfigItemMapIndexedContext : public XMLConfigBaseContext
{
private:
    OUString const maConfigItemName;

public:
    XMLConfigItemMapIndexedContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                                    css::uno::Any& rAny,
                                    const OUString& rConfigItemName,
                                    XMLConfigBaseContext* pBaseContext);

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;

    virtual void EndElement() override;
};

static SvXMLImportContext *CreateSettingsContext(SvXMLImport& rImport, sal_uInt16 p_nPrefix,
                        const OUString& rLocalName,
                        const uno::Reference<xml::sax::XAttributeList>& xAttrList,
                        beans::PropertyValue& rProp, XMLConfigBaseContext* pBaseContext)
{
    SvXMLImportContext *pContext = nullptr;

    rProp.Name.clear();
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_CONFIG)
        {
            if (IsXMLToken(aLocalName, XML_NAME))
                rProp.Name = sValue;
        }
    }

    if (p_nPrefix == XML_NAMESPACE_CONFIG)
    {
        if (IsXMLToken(rLocalName, XML_CONFIG_ITEM))
            pContext = new XMLConfigItemContext(rImport, p_nPrefix, rLocalName, xAttrList, rProp.Value, rProp.Name, pBaseContext);
        else if((IsXMLToken(rLocalName, XML_CONFIG_ITEM_SET)) ||
                (IsXMLToken(rLocalName, XML_CONFIG_ITEM_MAP_ENTRY)) )
            pContext = new XMLConfigItemSetContext(rImport, p_nPrefix, rLocalName, xAttrList, rProp.Value, pBaseContext);
        else if(IsXMLToken(rLocalName, XML_CONFIG_ITEM_MAP_NAMED))
            pContext = new XMLConfigItemMapNamedContext(rImport, p_nPrefix, rLocalName, xAttrList, rProp.Value, pBaseContext);
        else if(IsXMLToken(rLocalName, XML_CONFIG_ITEM_MAP_INDEXED))
            pContext = new XMLConfigItemMapIndexedContext(rImport, p_nPrefix, rLocalName, xAttrList, rProp.Value, rProp.Name, pBaseContext);
    }

    if( !pContext )
        pContext = new SvXMLImportContext( rImport, p_nPrefix, rLocalName );

    return pContext;
}

namespace
{
    struct SettingsGroup
    {
        OUString const sGroupName;
        uno::Any        aSettings;

        SettingsGroup( const OUString& _rGroupName, const uno::Any& _rSettings )
            :sGroupName( _rGroupName )
            ,aSettings( _rSettings )
        {
        }
    };
}

struct XMLDocumentSettingsContext_Data
{
    css::uno::Any                   aViewProps;
    css::uno::Any                   aConfigProps;
    ::std::vector< SettingsGroup >  aDocSpecificSettings;
};

XMLDocumentSettingsContext::XMLDocumentSettingsContext(SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                    const uno::Reference<xml::sax::XAttributeList>& )
    : SvXMLImportContext( rImport, nPrfx, rLName )
    , m_pData( new XMLDocumentSettingsContext_Data )
{
    // here are no attributes
}

XMLDocumentSettingsContext::~XMLDocumentSettingsContext()
{
}

SvXMLImportContextRef XMLDocumentSettingsContext::CreateChildContext( sal_uInt16 p_nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    OUString sName;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_CONFIG)
        {
            if (IsXMLToken(aLocalName, XML_NAME))
                sName = sValue;
        }
    }

    if (p_nPrefix == XML_NAMESPACE_CONFIG)
    {
        if (IsXMLToken(rLocalName, XML_CONFIG_ITEM_SET))
        {
            OUString aLocalConfigName;
            sal_uInt16 nConfigPrefix =
                GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sName, &aLocalConfigName );

            if( XML_NAMESPACE_OOO == nConfigPrefix )
            {
                if (IsXMLToken(aLocalConfigName, XML_VIEW_SETTINGS))
                    pContext = new XMLConfigItemSetContext(GetImport(),
                                        p_nPrefix, rLocalName, xAttrList,
                                        m_pData->aViewProps, nullptr);
                else if (IsXMLToken(aLocalConfigName,
                                                XML_CONFIGURATION_SETTINGS))
                    pContext = new XMLConfigItemSetContext(GetImport(),
                                        p_nPrefix, rLocalName, xAttrList,
                                        m_pData->aConfigProps, nullptr);
                else
                {
                    m_pData->aDocSpecificSettings.emplace_back( aLocalConfigName, uno::Any() );

                    pContext = new XMLConfigItemSetContext(GetImport(),
                                        p_nPrefix, rLocalName, xAttrList,
                                        m_pData->aDocSpecificSettings.back().aSettings, nullptr);
                }
            }
        }
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );

    return pContext;
}

void XMLDocumentSettingsContext::EndElement()
{
    uno::Sequence<beans::PropertyValue> aSeqViewProps;
    if (m_pData->aViewProps >>= aSeqViewProps)
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
    if ( m_pData->aConfigProps >>= aSeqConfigProps )
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

    for (auto const& settings : m_pData->aDocSpecificSettings)
    {
        uno::Sequence< beans::PropertyValue > aDocSettings;
        OSL_VERIFY( settings.aSettings >>= aDocSettings );
        GetImport().SetDocumentSpecificSettings( settings.sGroupName, aDocSettings );
    }
}

XMLConfigBaseContext::XMLConfigBaseContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName, css::uno::Any& rTempAny,
        XMLConfigBaseContext* pTempBaseContext)
    : SvXMLImportContext( rImport, nPrfx, rLName ),
    maProps( rImport.GetComponentContext() ),
    maProp(),
    mrAny(rTempAny),
    mpBaseContext(pTempBaseContext)
{
}

XMLConfigItemSetContext::XMLConfigItemSetContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>&,
                                    css::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext)
    : XMLConfigBaseContext( rImport, nPrfx, rLName, rAny, pBaseContext )
{
    // here are no attributes
}

SvXMLImportContextRef XMLConfigItemSetContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    return CreateSettingsContext(GetImport(), nPrefix, rLocalName, xAttrList, maProp, this);
}

void XMLConfigItemSetContext::EndElement()
{
    mrAny <<= maProps.GetSequence();
    if (mpBaseContext)
        mpBaseContext->AddPropertyValue();
}

XMLConfigItemContext::XMLConfigItemContext(SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
                                    css::uno::Any& rTempAny,
                                    const OUString& rTempItemName,
                                    XMLConfigBaseContext* pTempBaseContext)
    : SvXMLImportContext(rImport, nPrfx, rLName),
    mrAny(rTempAny),
    mrItemName(rTempItemName),
    mpBaseContext(pTempBaseContext)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );

        if (nPrefix == XML_NAMESPACE_CONFIG)
        {
            if (IsXMLToken(aLocalName, XML_TYPE))
                msType = sValue;
        }
    }
}

SvXMLImportContextRef XMLConfigItemContext::CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference< css::xml::sax::XAttributeList>& )
{
    SvXMLImportContext* pContext = new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
    return pContext;
}

void XMLConfigItemContext::Characters( const OUString& rChars )
{
    if (IsXMLToken(msType, XML_BASE64BINARY))
    {
        OUString sTrimmedChars( rChars.trim() );
        if( !sTrimmedChars.isEmpty() )
        {
            OUString sChars;
            if( !msValue.isEmpty() )
            {
                sChars = msValue;
                sChars += sTrimmedChars;
                msValue.clear();
            }
            else
            {
                sChars = sTrimmedChars;
            }
            uno::Sequence<sal_Int8> aBuffer((sChars.getLength() / 4) * 3 );
            sal_Int32 const nCharsDecoded =
                ::comphelper::Base64::decodeSomeChars( aBuffer, sChars );
            sal_uInt32 nStartPos(maDecoded.getLength());
            sal_uInt32 nCount(aBuffer.getLength());
            maDecoded.realloc(nStartPos + nCount);
            sal_Int8* pDecoded = maDecoded.getArray();
            sal_Int8* pBuffer = aBuffer.getArray();
            for (sal_uInt32 i = 0; i < nCount; i++, pBuffer++)
                pDecoded[nStartPos + i] = *pBuffer;
            if( nCharsDecoded != sChars.getLength() )
                msValue = sChars.copy( nCharsDecoded );
        }
    }
    else
        msValue += rChars;
}

void XMLConfigItemContext::EndElement()
{
    if (mpBaseContext)
    {
        if (IsXMLToken(msType, XML_BOOLEAN))
        {
            bool bValue(false);
            if (IsXMLToken(msValue, XML_TRUE))
                bValue = true;
            mrAny <<= bValue;
        }
        else if (IsXMLToken(msType, XML_BYTE))
        {
            sal_Int32 nValue(0);
            ::sax::Converter::convertNumber(nValue, msValue);
            mrAny <<= static_cast<sal_Int8>(nValue);
        }
        else if (IsXMLToken(msType, XML_SHORT))
        {
            sal_Int32 nValue(0);
            ::sax::Converter::convertNumber(nValue, msValue);
            mrAny <<= static_cast<sal_Int16>(nValue);
        }
        else if (IsXMLToken(msType, XML_INT))
        {
            sal_Int32 nValue(0);
            ::sax::Converter::convertNumber(nValue, msValue);
            mrAny <<= nValue;
        }
        else if (IsXMLToken(msType, XML_LONG))
        {
            sal_Int64 nValue(msValue.toInt64());
            mrAny <<= nValue;
        }
        else if (IsXMLToken(msType, XML_DOUBLE))
        {
            double fValue(0.0);
            ::sax::Converter::convertDouble(fValue, msValue);
            mrAny <<= fValue;
        }
        else if (IsXMLToken(msType, XML_STRING))
        {
            mrAny <<= msValue;
        }
        else if (IsXMLToken(msType, XML_DATETIME))
        {
            util::DateTime aDateTime;
            ::sax::Converter::parseDateTime(aDateTime, msValue);
            mrAny <<= aDateTime;
        }
        else if (IsXMLToken(msType, XML_BASE64BINARY))
        {
            mrAny <<= maDecoded;
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
            uno::Reference< util::XStringSubstitution > xStringSubsitution( util::PathSubstitution::create(xContext) );

            OUString aURL;
            mrAny >>= aURL;
            aURL = xStringSubsitution->substituteVariables( aURL, false );
            mrAny <<= aURL;
        }
        catch( uno::Exception& )
        {
        }
    }
}

XMLConfigItemMapNamedContext::XMLConfigItemMapNamedContext(SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>&,
                                    css::uno::Any& rAny,
                                    XMLConfigBaseContext* pBaseContext)
    : XMLConfigBaseContext(rImport, nPrfx, rLName, rAny, pBaseContext)
{
}

SvXMLImportContextRef XMLConfigItemMapNamedContext::CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    return CreateSettingsContext(GetImport(), nPrefix, rLocalName, xAttrList, maProp, this);
}

void XMLConfigItemMapNamedContext::EndElement()
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

XMLConfigItemMapIndexedContext::XMLConfigItemMapIndexedContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
                                    const OUString& rLName,
                                    const css::uno::Reference< css::xml::sax::XAttributeList>&,
                                    css::uno::Any& rAny,
                                    const OUString& rConfigItemName,
                                    XMLConfigBaseContext* pBaseContext)
    : XMLConfigBaseContext(rImport, nPrfx, rLName, rAny, pBaseContext),
      maConfigItemName( rConfigItemName )
{
}

SvXMLImportContextRef XMLConfigItemMapIndexedContext::CreateChildContext( sal_uInt16 nPrefix,
                                                    const OUString& rLocalName,
                                                    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    return CreateSettingsContext(GetImport(), nPrefix, rLocalName, xAttrList, maProp, this);
}

void XMLConfigItemMapIndexedContext::EndElement()
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

                uno::Reference< container::XIndexAccess > xIndex( maProps.GetIndexContainer(), uno::UNO_QUERY );

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
                            catch (uno::Exception const& e)
                            {
                                SAL_WARN("xmloff.core",
                                    "Exception while importing forbidden characters: " << e);
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
            uno::Reference< container::XIndexAccess > xIndex( maProps.GetIndexContainer(), uno::UNO_QUERY );

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
