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

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/xml/dom/SAXDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XSAXDocumentBuilder2.hpp>
#include <com/sun/star/xml/xpath/XPathAPI.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <comphelper/processfactory.hxx>
#include <xmloff/xmlmetai.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"

using namespace com::sun::star;
using namespace ::xmloff::token;

/// builds a DOM tree from SAX events, by forwarding to SAXDocumentBuilder
class XMLDocumentBuilderContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::dom::XSAXDocumentBuilder2> mxDocBuilder;

public:
    XMLDocumentBuilderContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::dom::XSAXDocumentBuilder2>& rDocBuilder);

    virtual ~XMLDocumentBuilderContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void Characters( const OUString& rChars );

    virtual void EndElement();
};

XMLDocumentBuilderContext::XMLDocumentBuilderContext(SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const uno::Reference<xml::sax::XAttributeList>&,
        const uno::Reference<xml::dom::XSAXDocumentBuilder2>& rDocBuilder) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mxDocBuilder(rDocBuilder)
{
}

XMLDocumentBuilderContext::~XMLDocumentBuilderContext()
{
}

SvXMLImportContext *
XMLDocumentBuilderContext::CreateChildContext( sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& rAttrs)
{
    return new XMLDocumentBuilderContext(
                GetImport(), nPrefix, rLocalName, rAttrs, mxDocBuilder);
}

void XMLDocumentBuilderContext::StartElement(
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    mxDocBuilder->startElement(
      GetImport().GetNamespaceMap().GetQNameByKey(GetPrefix(), GetLocalName()),
      xAttrList);
}

void XMLDocumentBuilderContext::Characters( const OUString& rChars )
{
    mxDocBuilder->characters(rChars);
}

void XMLDocumentBuilderContext::EndElement()
{
    mxDocBuilder->endElement(
      GetImport().GetNamespaceMap().GetQNameByKey(GetPrefix(), GetLocalName()));
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
    } catch (const uno::Exception& e) {
        throw lang::WrappedTargetRuntimeException(
            OUString(
                "SvXMLMetaDocumentContext::initDocumentProperties: "
                "properties init exception"),
            rImport, makeAny(e));
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

        OUString const expr( "string(/office:document-meta/office:meta/meta:generator)");
        uno::Reference< xml::xpath::XXPathObject > const xObj(
            xPath->eval(xDoc.get(), expr), uno::UNO_SET_THROW);
        OUString const value(xObj->getString());
        SvXMLMetaDocumentContext::setBuildId(value, rImport.getImportInfo());
    } catch (const uno::RuntimeException&) {
        throw;
    } catch (const uno::Exception& e) {
        throw lang::WrappedTargetRuntimeException(
            OUString(
                                "SvXMLMetaDocumentContext::initGenerator: exception"),
            rImport, makeAny(e));
    }
}

SvXMLMetaDocumentContext::SvXMLMetaDocumentContext(SvXMLImport& rImport,
            sal_uInt16 nPrfx, const OUString& rLName,
            const uno::Reference<document::XDocumentProperties>& xDocProps) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
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

SvXMLImportContext *SvXMLMetaDocumentContext::CreateChildContext(
             sal_uInt16 nPrefix, const OUString& rLocalName,
             const uno::Reference<xml::sax::XAttributeList>& rAttrs)
{
    if (  (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken(rLocalName, XML_META) )
    {
        return new XMLDocumentBuilderContext(
                GetImport(), nPrefix, rLocalName, rAttrs, mxDocBuilder);
    }
    else
    {
        return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
}

void SvXMLMetaDocumentContext::StartElement(
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    mxDocBuilder->startDocument();
    // hardcode office:document-meta (necessary in case of flat file ODF)
    mxDocBuilder->startElement(
        GetImport().GetNamespaceMap().GetQNameByKey(GetPrefix(),
            GetXMLToken(XML_DOCUMENT_META)), xAttrList);

}

void SvXMLMetaDocumentContext::EndElement()
{
    // hardcode office:document-meta (necessary in case of flat file ODF)
    mxDocBuilder->endElement(
        GetImport().GetNamespaceMap().GetQNameByKey(GetPrefix(),
            GetXMLToken(XML_DOCUMENT_META)));
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
                    sBuffer.append( (sal_Unicode)'$' );
                    sBuffer.append( i_rBuildId.copy(
                        nBegin + sBuildCompare.getLength() ) );
                    sBuildId = sBuffer.makeStringAndClear();
                }
            }
        }
    }

    if ( sBuildId.isEmpty() )
    {
        if (    i_rBuildId.startsWith("StarOffice 7")
            ||  i_rBuildId.startsWith("StarSuite 7")
            ||  i_rBuildId.startsWith("OpenOffice.org 1"))
        {
            sBuildId = OUString("645$8687");
        }
        else if (i_rBuildId.startsWith("NeoOffice/2"))
        {
            sBuildId = OUString("680$9134"); // fake NeoOffice as OpenOffice.org 2.2 release
        }
    }

    if (i_rBuildId.startsWith("LibreOffice/"))
    {
        OUStringBuffer sNumber;
        for (sal_Int32 i = sizeof("LibreOffice/") - 1;
                i < i_rBuildId.getLength(); ++i)
        {
            if (isdigit(i_rBuildId[i]))
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
            sBuildId += (";" + sNumber.makeStringAndClear());
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
