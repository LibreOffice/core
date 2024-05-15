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

#include <string_view>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/xml/dom/SAXDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XSAXDocumentBuilder2.hpp>
#include <com/sun/star/xml/xpath/XPathAPI.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <utility>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>

using namespace com::sun::star;
using namespace ::xmloff::token;

namespace {

/// builds a DOM tree from SAX events, by forwarding to SAXDocumentBuilder
class XMLDocumentBuilderContext : public SvXMLImportContext
{
private:
    css::uno::Reference< css::xml::dom::XSAXDocumentBuilder2> mxDocBuilder;
    SvXMLMetaDocumentContext *const m_pTopLevel;

public:
    XMLDocumentBuilderContext(SvXMLImport& rImport, sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
        css::uno::Reference<css::xml::dom::XSAXDocumentBuilder2> xDocBuilder,
        SvXMLMetaDocumentContext * pTopLevel);

    virtual void SAL_CALL characters( const OUString& aChars ) override;

    virtual void SAL_CALL startFastElement( sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    virtual void SAL_CALL endFastElement( sal_Int32 nElement ) override;

    virtual void SAL_CALL startUnknownElement( const OUString& Namespace, const OUString& Name,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& Attribs ) override;

    virtual void SAL_CALL endUnknownElement( const OUString& Namespace, const OUString& Name ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

};

}

XMLDocumentBuilderContext::XMLDocumentBuilderContext(SvXMLImport& rImport,
        sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>&,
        uno::Reference<xml::dom::XSAXDocumentBuilder2> xDocBuilder,
        SvXMLMetaDocumentContext *const pTopLevel)
    : SvXMLImportContext(rImport)
    , mxDocBuilder(std::move(xDocBuilder))
    , m_pTopLevel(pTopLevel)
{
}

void SAL_CALL XMLDocumentBuilderContext::startFastElement( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList >& xAttribs )
{
    mxDocBuilder->startFastElement(nElement, xAttribs);
}

void SAL_CALL XMLDocumentBuilderContext::endFastElement( sal_Int32 nElement )
{
    mxDocBuilder->endFastElement(nElement);
    if (m_pTopLevel)
    {
        // call this here because in the flat ODF case the top-level
        // endFastElement is called only at the very end of the document,
        // which is too late to init BuildId
        m_pTopLevel->FinishMetaElement();
    }
}

void SAL_CALL XMLDocumentBuilderContext::startUnknownElement( const OUString& rNamespace,
        const OUString& rName, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    mxDocBuilder->startUnknownElement(rNamespace, rName, xAttrList);
}

void SAL_CALL XMLDocumentBuilderContext::endUnknownElement( const OUString& rNamespace, const OUString& rName )
{
    mxDocBuilder->endUnknownElement(rNamespace, rName);
}

void SAL_CALL XMLDocumentBuilderContext::characters( const OUString& rChars )
{
    mxDocBuilder->characters(rChars);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL XMLDocumentBuilderContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    return new XMLDocumentBuilderContext(GetImport(), nElement, xAttrList, mxDocBuilder, nullptr);
}

static void
lcl_initDocumentProperties(SvXMLImport & rImport,
        uno::Reference<xml::dom::XSAXDocumentBuilder2> const& xDocBuilder,
        uno::Reference<document::XDocumentProperties> const& xDocProps)
{
    uno::Reference< lang::XInitialization > const xInit(xDocProps,
        uno::UNO_QUERY_THROW);
    try {
        xInit->initialize({ uno::Any(xDocBuilder->getDocument()) });
        rImport.SetStatistics(xDocProps->getDocumentStatistics());
        // convert all URLs from relative to absolute
        xDocProps->setTemplateURL(rImport.GetAbsoluteReference(
            xDocProps->getTemplateURL()));
        xDocProps->setAutoloadURL(rImport.GetAbsoluteReference(
            xDocProps->getAutoloadURL()));
        SvXMLMetaDocumentContext::setBuildId(
            xDocProps->getGenerator(), rImport.getImportInfo());
    } catch (const uno::RuntimeException&) {
        throw;
    } catch (const uno::Exception&) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(
            u"SvXMLMetaDocumentContext::initDocumentProperties: "
            "properties init exception"_ustr,
            rImport, anyEx);
    }
}

static void
lcl_initGenerator(SvXMLImport & rImport,
        uno::Reference<xml::dom::XSAXDocumentBuilder2> const& xDocBuilder)
{
    uno::Reference< xml::dom::XDocument > const xDoc(xDocBuilder->getDocument(),
        uno::UNO_SET_THROW);
    try {
        uno::Reference< xml::xpath::XXPathAPI > const xPath = xml::xpath::XPathAPI::create(
            rImport.GetComponentContext() );
        xPath->registerNS(GetXMLToken(XML_NP_OFFICE),GetXMLToken(XML_N_OFFICE));
        xPath->registerNS(GetXMLToken(XML_NP_META), GetXMLToken(XML_N_META));

        uno::Reference< xml::xpath::XXPathObject > const xObj(
            xPath->eval(xDoc, u"string(/office:document-meta/office:meta/meta:generator)"_ustr),
            uno::UNO_SET_THROW);
        OUString const value(xObj->getString());
        SvXMLMetaDocumentContext::setBuildId(value, rImport.getImportInfo());
    } catch (const uno::RuntimeException&) {
        throw;
    } catch (const uno::Exception&) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(
            u"SvXMLMetaDocumentContext::initGenerator: exception"_ustr,
            rImport, anyEx);
    }
}

SvXMLMetaDocumentContext::SvXMLMetaDocumentContext(SvXMLImport& rImport,
            uno::Reference<document::XDocumentProperties> xDocProps) :
    SvXMLImportContext( rImport ),
    mxDocProps(std::move(xDocProps)),
    mxDocBuilder(
        xml::dom::SAXDocumentBuilder::create(
            comphelper::getProcessComponentContext()))
{
// #i103539#: must always read meta.xml for generator, xDocProps unwanted then
//    OSL_ENSURE(xDocProps.is(), "SvXMLMetaDocumentContext: no document props");
}

SvXMLMetaDocumentContext::~SvXMLMetaDocumentContext()
{
}

void SAL_CALL SvXMLMetaDocumentContext::startFastElement(sal_Int32 /*nElement*/,
            const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    mxDocBuilder->startDocument();
    // hardcode office:document-meta (necessary in case of flat file ODF)
    mxDocBuilder->startFastElement(XML_ELEMENT(OFFICE, XML_DOCUMENT_META), xAttrList);
}

void SvXMLMetaDocumentContext::FinishMetaElement()
{
    // hardcode office:document-meta (necessary in case of flat file ODF)
    mxDocBuilder->endFastElement(XML_ELEMENT(OFFICE, XML_DOCUMENT_META));
    mxDocBuilder->endDocument();
    if (mxDocProps.is())
    {
        lcl_initDocumentProperties(GetImport(), mxDocBuilder, mxDocProps);
    }
    else
    {
        lcl_initGenerator(GetImport(), mxDocBuilder);
    }
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SvXMLMetaDocumentContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    if ( nElement == XML_ELEMENT(OFFICE, XML_META) )
        return new XMLDocumentBuilderContext(
                GetImport(), nElement, xAttrList, mxDocBuilder, this);
    return nullptr;
}

void SvXMLMetaDocumentContext::setBuildId(std::u16string_view i_rBuildId, const uno::Reference<beans::XPropertySet>& xImportInfo )
{
    OUString sBuildId;
    // skip to second product
    size_t nBegin = i_rBuildId.find( ' ' );
    if ( nBegin != std::u16string_view::npos )
    {
        // skip to build information
        nBegin = i_rBuildId.find( '/', nBegin );
        if ( nBegin != std::u16string_view::npos )
        {
            size_t nEnd = i_rBuildId.find( 'm', nBegin );
            if ( nEnd != std::u16string_view::npos )
            {
                OUStringBuffer sBuffer(
                    i_rBuildId.substr( nBegin+1, nEnd-nBegin-1 ) );
                static constexpr OUString sBuildCompare(
                     u"$Build-"_ustr  );
                nBegin = i_rBuildId.find( sBuildCompare, nEnd );
                if ( nBegin != std::u16string_view::npos )
                {
                    sBuffer.append( '$' );
                    sBuffer.append( i_rBuildId.substr(nBegin + sBuildCompare.getLength()) );
                    sBuildId = sBuffer.makeStringAndClear();
                }
            }
        }
    }

    if ( sBuildId.isEmpty() )
    {
        if (    o3tl::starts_with(i_rBuildId, u"StarOffice 7")
            ||  o3tl::starts_with(i_rBuildId, u"StarSuite 7")
            ||  o3tl::starts_with(i_rBuildId, u"StarOffice 6")
            ||  o3tl::starts_with(i_rBuildId, u"StarSuite 6")
            ||  o3tl::starts_with(i_rBuildId, u"OpenOffice.org 1"))
        {
            sBuildId = "645$8687";
        }
        else if (o3tl::starts_with(i_rBuildId, u"NeoOffice/2"))
        {
            sBuildId = "680$9134"; // fake NeoOffice as OpenOffice.org 2.2 release
        }
    }

    // "LibreOffice_project" was hard-coded since LO 3.3.0
    // see utl::DocInfoHelper::GetGeneratorString()
    if (i_rBuildId.find(u"LibreOffice_project/") != std::u16string_view::npos)
    {
        OUStringBuffer sNumber;
        size_t const firstSlash = i_rBuildId.find('/');
        assert(firstSlash != std::u16string_view::npos);
        for (size_t i = firstSlash + 1; i < i_rBuildId.size(); ++i)
        {
            if (rtl::isAsciiDigit(i_rBuildId[i]) || '.' == i_rBuildId[i])
            {
                sNumber.append(i_rBuildId[i]);
            }
            else
            {
                break;
            }
        }
        if (!sNumber.isEmpty())
        {
            sBuildId += ";" + sNumber;
        }
    }

    if ( sBuildId.isEmpty() )
        return;

    try
    {
        if( xImportInfo.is() )
        {
            static constexpr OUString aPropName(u"BuildId"_ustr);
            uno::Reference< beans::XPropertySetInfo > xSetInfo(
                xImportInfo->getPropertySetInfo());
            if( xSetInfo.is() && xSetInfo->hasPropertyByName( aPropName ) )
                xImportInfo->setPropertyValue( aPropName, uno::Any( sBuildId ) );
        }
    }
    catch(const uno::Exception&)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
