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

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/xml/dom/XSAXDocumentBuilder.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>

#include <tools/debug.hxx>

#include <xmloff/xmlmetai.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"


using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using namespace com::sun::star;
using namespace ::xmloff::token;


//===========================================================================

/// builds a DOM tree from SAX events, by forwarding to SAXDocumentBuilder
class XMLDocumentBuilderContext : public SvXMLImportContext
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XDocumentHandler> mxDocBuilder;

public:
    XMLDocumentBuilderContext(SvXMLImport& rImport, sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XDocumentHandler>& rDocBuilder);

    virtual ~XMLDocumentBuilderContext();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
        const rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList>& xAttrList );

    virtual void StartElement( const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

    virtual void Characters( const ::rtl::OUString& rChars );

    virtual void EndElement();
};

XMLDocumentBuilderContext::XMLDocumentBuilderContext(SvXMLImport& rImport,
        sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
        const uno::Reference<xml::sax::XAttributeList>&,
        const uno::Reference<xml::sax::XDocumentHandler>& rDocBuilder) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mxDocBuilder(rDocBuilder)
{
}

XMLDocumentBuilderContext::~XMLDocumentBuilderContext()
{
}

SvXMLImportContext *
XMLDocumentBuilderContext::CreateChildContext( sal_uInt16 nPrefix,
    const rtl::OUString& rLocalName,
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

void XMLDocumentBuilderContext::Characters( const ::rtl::OUString& rChars )
{
    mxDocBuilder->characters(rChars);
}

void XMLDocumentBuilderContext::EndElement()
{
    mxDocBuilder->endElement(
      GetImport().GetNamespaceMap().GetQNameByKey(GetPrefix(), GetLocalName()));
}


//===========================================================================

SvXMLMetaDocumentContext::SvXMLMetaDocumentContext(SvXMLImport& rImport,
            sal_uInt16 nPrfx, const rtl::OUString& rLName,
            const uno::Reference<document::XDocumentProperties>& xDocProps,
            const uno::Reference<xml::sax::XDocumentHandler>& xDocBuilder) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    mxDocProps(xDocProps),
    mxDocBuilder(xDocBuilder)
{
    DBG_ASSERT(xDocProps.is(), "SvXMLMetaDocumentContext: no document props");
    DBG_ASSERT(xDocBuilder.is(), "SvXMLMetaDocumentContext: no document hdlr");
    // here are no attributes
}

SvXMLMetaDocumentContext::~SvXMLMetaDocumentContext()
{
}

SvXMLImportContext *SvXMLMetaDocumentContext::CreateChildContext(
             sal_uInt16 nPrefix, const rtl::OUString& rLocalName,
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
    initDocumentProperties();
}

void SvXMLMetaDocumentContext::initDocumentProperties()
{
    uno::Sequence< uno::Any > aSeq(1);
    uno::Reference< xml::dom::XSAXDocumentBuilder > xDB (mxDocBuilder,
        uno::UNO_QUERY_THROW);
    aSeq[0] <<= xDB->getDocument();
    uno::Reference< lang::XInitialization > xInit(mxDocProps,
        uno::UNO_QUERY_THROW);
    try {
        xInit->initialize(aSeq);
        GetImport().SetStatistics(mxDocProps->getDocumentStatistics());
        // convert all URLs from relative to absolute
        mxDocProps->setTemplateURL(GetImport().GetAbsoluteReference(
            mxDocProps->getTemplateURL()));
        mxDocProps->setAutoloadURL(GetImport().GetAbsoluteReference(
            mxDocProps->getAutoloadURL()));
        setBuildId(mxDocProps->getGenerator());
    } catch (uno::RuntimeException) {
        throw;
    } catch (uno::Exception & e) {
        throw lang::WrappedTargetRuntimeException(
            ::rtl::OUString::createFromAscii(
                "SvXMLMetaDocumentContext::initDocumentProperties: "
                "properties init exception"),
            GetImport(), makeAny(e));
    }
}

void SvXMLMetaDocumentContext::setBuildId(const ::rtl::OUString & i_rBuildId)
{
    SvXMLMetaDocumentContext::setBuildId( i_rBuildId, GetImport().getImportInfo() );
}

//static
void SvXMLMetaDocumentContext::setBuildId(::rtl::OUString const& i_rBuildId, const uno::Reference<beans::XPropertySet>& xImportInfo )
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
                    RTL_CONSTASCII_USTRINGPARAM( "$Build-" ) );
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

    if ( sBuildId.getLength() == 0 )
    {
        if ((i_rBuildId.compareToAscii(
                RTL_CONSTASCII_STRINGPARAM("StarOffice 7") ) == 0) ||
            (i_rBuildId.compareToAscii(
                RTL_CONSTASCII_STRINGPARAM("StarSuite 7") ) == 0)  ||
            (i_rBuildId.compareToAscii(
                RTL_CONSTASCII_STRINGPARAM("OpenOffice.org 1") ) == 0))
        {
            sBuildId = OUString::createFromAscii( "645$8687" );
        }
        if ((i_rBuildId.compareToAscii( RTL_CONSTASCII_STRINGPARAM("NeoOffice/2") ) == 0) )
        {
            sBuildId = OUString::createFromAscii( "680$9134" ); // fake NeoOffice as OpenOffice.org 2.2 release
        }
    }

    if ( sBuildId.getLength() ) try
    {
        if( xImportInfo.is() )
        {
            const OUString aPropName(RTL_CONSTASCII_USTRINGPARAM("BuildId"));
            uno::Reference< beans::XPropertySetInfo > xSetInfo(
                xImportInfo->getPropertySetInfo());
            if( xSetInfo.is() && xSetInfo->hasPropertyByName( aPropName ) )
                xImportInfo->setPropertyValue( aPropName, uno::makeAny( sBuildId ) );
        }
    }
    catch( uno::Exception& )
    {
    }
}

