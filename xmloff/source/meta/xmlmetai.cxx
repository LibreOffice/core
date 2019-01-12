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
#include <rtl/character.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>

using namespace com::sun::star;
using namespace ::xmloff::token;

/// builds a DOM tree from SAX events, by forwarding to SAXDocumentBuilder
class XMLDocumentBuilderContext : public SvXMLImportContext
{
private:
    css::uno::Reference< css::xml::dom::XSAXDocumentBuilder2> mxDocBuilder;

public:
    XMLDocumentBuilderContext(SvXMLImport& rImport, sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
        const css::uno::Reference< css::xml::dom::XSAXDocumentBuilder2>& rDocBuilder);

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

XMLDocumentBuilderContext::XMLDocumentBuilderContext(SvXMLImport& rImport,
        sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>&,
        const uno::Reference<xml::dom::XSAXDocumentBuilder2>& rDocBuilder) :
    SvXMLImportContext( rImport ),
    mxDocBuilder(rDocBuilder)
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
    return new XMLDocumentBuilderContext( GetImport(), nElement, xAttrList, mxDocBuilder );
}

static void
lcl_initDocumentProperties(SvXMLImport & rImport,
        uno::Reference<xml::dom::XSAXDocumentBuilder2> const& xDocBuilder,
        uno::Reference<document::XDocumentProperties> const& xDocProps)
{
    uno::Sequence< uno::Any > aSeq(1);
    aSeq[0] <<= xDocBuilder->getDocument();
    uno::Reference< lang::XInitialization > const xInit(xDocProps,
        uno::UNO_QUERY_THROW);
    try {
        xInit->initialize(aSeq);
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
            "SvXMLMetaDocumentContext::initDocumentProperties: "
            "properties init exception",
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
            xPath->eval(xDoc.get(), "string(/office:document-meta/office:meta/meta:generator)"),
            uno::UNO_SET_THROW);
        OUString const value(xObj->getString());
        SvXMLMetaDocumentContext::setBuildId(value, rImport.getImportInfo());
    } catch (const uno::RuntimeException&) {
        throw;
    } catch (const uno::Exception&) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw lang::WrappedTargetRuntimeException(
            "SvXMLMetaDocumentContext::initGenerator: exception",
            rImport, anyEx);
    }
}

SvXMLMetaDocumentContext::SvXMLMetaDocumentContext(SvXMLImport& rImport,
            const uno::Reference<document::XDocumentProperties>& xDocProps) :
    SvXMLImportContext( rImport ),
    mxDocProps(xDocProps),
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

void SAL_CALL SvXMLMetaDocumentContext::startFastElement( sal_Int32 nElement,
            const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    mxDocBuilder->startDocument();
    // hardcode office:document-meta (necessary in case of flat file ODF)
    mxDocBuilder->startFastElement( ( nElement & NMSP_MASK ) | XML_DOCUMENT_META, xAttrList );
}

void SAL_CALL SvXMLMetaDocumentContext::endFastElement( sal_Int32 nElement )
{
    // hardcode office:document-meta (necessary in case of flat file ODF)
    mxDocBuilder->endFastElement( ( nElement & NMSP_MASK ) | XML_DOCUMENT_META );
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

void SAL_CALL SvXMLMetaDocumentContext::characters( const OUString& /*rChars*/ )
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL SvXMLMetaDocumentContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    if ( nElement == ( NAMESPACE_TOKEN( XML_NAMESPACE_OFFICE ) | XML_META ) )
        return new XMLDocumentBuilderContext(
                GetImport(), nElement, xAttrList, mxDocBuilder);
    else
        return new SvXMLImportContext( GetImport() );
}

void SvXMLMetaDocumentContext::setBuildId(OUString const& i_rBuildId, const uno::Reference<beans::XPropertySet>& xImportInfo )
{
    OUString sBuildId;
    // skip to second product
    sal_Int32 nBegin = i_rBuildId.indexOf( ' ' );
    if ( nBegin != -1 )
    {
        // skip to build information
        nBegin = i_rBuildId.indexOf( '/', nBegin );
        if ( nBegin != -1 )
        {
            sal_Int32 nEnd = i_rBuildId.indexOf( 'm', nBegin );
            if ( nEnd != -1 )
            {
                OUStringBuffer sBuffer(
                    i_rBuildId.copy( nBegin+1, nEnd-nBegin-1 ) );
                const OUString sBuildCompare(
                     "$Build-"  );
                nBegin = i_rBuildId.indexOf( sBuildCompare, nEnd );
                if ( nBegin != -1 )
                {
                    sBuffer.append( '$' );
                    sBuffer.append( std::u16string_view(i_rBuildId).substr(
                        nBegin + sBuildCompare.getLength()) );
                    sBuildId = sBuffer.makeStringAndClear();
                }
            }
        }
    }

    if ( sBuildId.isEmpty() )
    {
        if (    i_rBuildId.startsWith("StarOffice 7")
            ||  i_rBuildId.startsWith("StarSuite 7")
            ||  i_rBuildId.startsWith("StarOffice 6")
            ||  i_rBuildId.startsWith("StarSuite 6")
            ||  i_rBuildId.startsWith("OpenOffice.org 1"))
        {
            sBuildId = "645$8687";
        }
        else if (i_rBuildId.startsWith("NeoOffice/2"))
        {
            sBuildId = "680$9134"; // fake NeoOffice as OpenOffice.org 2.2 release
        }
    }

    // "LibreOffice_project" was hard-coded since LO 3.3.0
    // see utl::DocInfoHelper::GetGeneratorString()
    if (i_rBuildId.indexOf("LibreOffice_project/") != -1)
    {
        OUStringBuffer sNumber;
        auto const firstSlash = i_rBuildId.indexOf("/");
        assert(firstSlash != -1);
        for (sal_Int32 i = firstSlash + 1; i < i_rBuildId.getLength(); ++i)
        {
            if (rtl::isAsciiDigit(i_rBuildId[i]))
            {
                sNumber.append(i_rBuildId[i]);
            }
            else if ('.' != i_rBuildId[i])
            {
                break;
            }
        }
        if (!sNumber.isEmpty())
        {
            sBuildId += ";" + sNumber.makeStringAndClear();
        }
    }

    if ( !sBuildId.isEmpty() ) try
    {
        if( xImportInfo.is() )
        {
            const OUString aPropName("BuildId");
            uno::Reference< beans::XPropertySetInfo > xSetInfo(
                xImportInfo->getPropertySetInfo());
            if( xSetInfo.is() && xSetInfo->hasPropertyByName( aPropName ) )
                xImportInfo->setPropertyValue( aPropName, uno::makeAny( sBuildId ) );
        }
    }
    catch(const uno::Exception&)
    {
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
