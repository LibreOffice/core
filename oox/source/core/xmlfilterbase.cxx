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

#include "oox/core/xmlfilterbase.hxx"

#include <cstdio>
#include <set>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <o3tl/any.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/docinfohelper.hxx>
#include <sax/fshelper.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <osl/diagnose.h>
#include <i18nlangtag/languagetag.hxx>
#include "oox/core/fastparser.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/core/recordparser.hxx"
#include "oox/core/relationshandler.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/zipstorage.hxx"
#include <oox/ole/olestorage.hxx>
#include <oox/token/namespaces.hxx>
#include "oox/token/properties.hxx"
#include <oox/token/tokens.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XOOXMLDocumentPropertiesImporter.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <comphelper/processfactory.hxx>
#include <oox/core/filterdetect.hxx>
#include <comphelper/storagehelper.hxx>

#include <oox/crypto/DocumentEncryption.hxx>
#include <tools/date.hxx>
#include <tools/datetime.hxx>
#include <com/sun/star/util/Duration.hpp>
#include <sax/tools/converter.hxx>

using ::com::sun::star::xml::dom::DocumentBuilder;
using ::com::sun::star::xml::dom::XDocument;
using ::com::sun::star::xml::dom::XDocumentBuilder;

namespace oox {
namespace core {

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

using utl::MediaDescriptor;
using ::sax_fastparser::FSHelperPtr;
using ::sax_fastparser::FastSerializerHelper;

namespace {

bool lclHasSuffix( const OUString& rFragmentPath, const OUString& rSuffix )
{
    sal_Int32 nSuffixPos = rFragmentPath.getLength() - rSuffix.getLength();
    return (nSuffixPos >= 0) && rFragmentPath.match( rSuffix, nSuffixPos );
}

struct NamespaceIds: public rtl::StaticWithInit<
    Sequence< beans::Pair< OUString, sal_Int32 > >,
    NamespaceIds>
{
    Sequence< beans::Pair< OUString, sal_Int32 > > operator()()
    {
        return css::uno::Sequence<css::beans::Pair<OUString, sal_Int32>>{
            {"http://www.w3.org/XML/1998/namespace", NMSP_xml},
            {"http://schemas.openxmlformats.org/package/2006/relationships",
             NMSP_packageRel},
            {"http://schemas.openxmlformats.org/officeDocument/2006/relationships",
             NMSP_officeRel},
            {"http://purl.oclc.org/ooxml/officeDocument/relationships",
             NMSP_officeRel},
            {"http://schemas.openxmlformats.org/drawingml/2006/main", NMSP_dml},
            {"http://purl.oclc.org/ooxml/drawingml/main", NMSP_dml},
            {"http://schemas.openxmlformats.org/drawingml/2006/diagram",
             NMSP_dmlDiagram},
            {"http://purl.oclc.org/ooxml/drawingml/diagram", NMSP_dmlDiagram},
            {"http://schemas.openxmlformats.org/drawingml/2006/chart",
             NMSP_dmlChart},
            {"http://schemas.openxmlformats.org/drawingml/2006/chartDrawing",
             NMSP_dmlChartDr},
            {"urn:schemas-microsoft-com:vml", NMSP_vml},
            {"urn:schemas-microsoft-com:office:office", NMSP_vmlOffice},
            {"urn:schemas-microsoft-com:office:word", NMSP_vmlWord},
            {"urn:schemas-microsoft-com:office:excel", NMSP_vmlExcel},
            {"urn:schemas-microsoft-com:office:powerpoint", NMSP_vmlPowerpoint},
            {"http://schemas.microsoft.com/office/2006/activeX", NMSP_ax},
            {"http://schemas.openxmlformats.org/spreadsheetml/2006/main",
             NMSP_xls},
            {"http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing",
             NMSP_xm},
            {"http://schemas.microsoft.com/office/excel/2006/main",
             NMSP_dmlSpreadDr},
            {"http://schemas.openxmlformats.org/presentationml/2006/main",
             NMSP_ppt},
            {"http://schemas.openxmlformats.org/markup-compatibility/2006",
             NMSP_mce},
            {"http://schemas.openxmlformats.org/spreadsheetml/2006/main/v2",
             NMSP_mceTest},
            {"http://schemas.openxmlformats.org/officeDocument/2006/math",
             NMSP_officeMath},
            {"http://schemas.microsoft.com/office/drawing/2008/diagram",
             NMSP_dsp},
            {"http://schemas.microsoft.com/office/spreadsheetml/2009/9/main",
             NMSP_xls14Lst},
            {"http://schemas.libreoffice.org/", NMSP_loext},
            {"http://schemas.microsoft.com/office/drawing/2010/main",
             NMSP_a14},
            {"http://schemas.microsoft.com/office/powerpoint/2010/main",
             NMSP_p14},
            {"http://schemas.microsoft.com/office/powerpoint/2012/main",
             NMSP_p15},
            {"http://schemas.microsoft.com/office/spreadsheetml/2011/1/ac",
             NMSP_x12ac},
        };
    }
};

void registerNamespaces( FastParser& rParser )
{
    const Sequence< beans::Pair<OUString, sal_Int32> > ids = NamespaceIds::get();

    // Filter out duplicates: a namespace can have multiple URLs, think of
    // strict vs transitional.
    std::set<sal_Int32> aSet;
    for (sal_Int32 i = 0; i < ids.getLength(); ++i)
        aSet.insert(ids[i].Second);

    for (std::set<sal_Int32>::iterator it = aSet.begin(); it != aSet.end(); ++it)
        rParser.registerNamespace(*it);
}

} // namespace

struct XmlFilterBaseImpl
{
    typedef RefMap< OUString, Relations > RelationsMap;

    Reference<XComponentContext>   mxContext;
    FastParser                     maFastParser;
    const OUString                 maBinSuffix;
    RelationsMap                   maRelationsMap;
    TextFieldStack                 maTextFieldStack;

    explicit            XmlFilterBaseImpl( const Reference< XComponentContext >& rxContext ) throw( RuntimeException );
    ~XmlFilterBaseImpl();
};

XmlFilterBaseImpl::XmlFilterBaseImpl( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    mxContext(rxContext),
    maFastParser( rxContext ),
    maBinSuffix( ".bin" )
{
    // register XML namespaces
    registerNamespaces(maFastParser);
}

XmlFilterBaseImpl::~XmlFilterBaseImpl()
{
}

XmlFilterBase::XmlFilterBase( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    FilterBase( rxContext ),
    mxImpl( new XmlFilterBaseImpl( rxContext ) ),
    mnRelId( 1 ),
    mnMaxDocId( 0 ),
    mbMSO2007(false),
    mbMissingExtDrawing(false)
{
}

XmlFilterBase::~XmlFilterBase()
{
    // #i118640# Reset the DocumentHandler at the FastSaxParser manually; this is
    // needed since the mechanism is that instances of FragmentHandler execute
    // their stuff (creating objects, setting attributes, ...) on being destroyed.
    // They get destroyed by setting a new DocumentHandler. This also happens in
    // the following implicit destruction chain of ~XmlFilterBaseImpl, but in that
    // case it's member RelationsMap maRelationsMap will be destroyed, but maybe
    // still be used by ~FragmentHandler -> crash.
    mxImpl->maFastParser.setDocumentHandler( nullptr );
}

void XmlFilterBase::checkDocumentProperties(const Reference<XDocumentProperties>& xDocProps)
{
    mbMSO2007 = false;
    if (!xDocProps->getGenerator().startsWithIgnoreAsciiCase("Microsoft"))
        return;

    uno::Reference<beans::XPropertyAccess> xUserDefProps(xDocProps->getUserDefinedProperties(), uno::UNO_QUERY);
    if (!xUserDefProps.is())
        return;

    comphelper::SequenceAsHashMap aUserDefinedProperties(xUserDefProps->getPropertyValues());
    comphelper::SequenceAsHashMap::iterator it = aUserDefinedProperties.find("AppVersion");
    if (it == aUserDefinedProperties.end())
        return;

    OUString aValue;
    if (!(it->second >>= aValue))
        return;

    if (!aValue.startsWithIgnoreAsciiCase("12."))
        return;

    SAL_INFO("oox", "a MSO 2007 document");
    mbMSO2007 = true;
}

void XmlFilterBase::importDocumentProperties()
{
    Reference< XMultiServiceFactory > xFactory( getComponentContext()->getServiceManager(), UNO_QUERY );
    MediaDescriptor aMediaDesc( getMediaDescriptor() );
    Reference< XInputStream > xInputStream;
    Reference< XComponentContext > xContext = getComponentContext();
    rtl::Reference< ::oox::core::FilterDetect > xDetector( new ::oox::core::FilterDetect( xContext ) );
    xInputStream = xDetector->extractUnencryptedPackage( aMediaDesc );
    Reference< XComponent > xModel( getModel(), UNO_QUERY );
    Reference< XStorage > xDocumentStorage (
            ::comphelper::OStorageHelper::GetStorageOfFormatFromInputStream( OFOPXML_STORAGE_FORMAT_STRING, xInputStream ) );
    Reference< XInterface > xTemp = xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.document.OOXMLDocumentPropertiesImporter",
            xContext);
    Reference< XOOXMLDocumentPropertiesImporter > xImporter( xTemp, UNO_QUERY );
    Reference< XDocumentPropertiesSupplier > xPropSupplier( xModel, UNO_QUERY);
    Reference< XDocumentProperties > xDocProps = xPropSupplier->getDocumentProperties();
    xImporter->importProperties( xDocumentStorage, xDocProps );
    checkDocumentProperties(xDocProps);
}

FastParser* XmlFilterBase::createParser() const
{
    FastParser* pParser = new FastParser(getComponentContext());
    registerNamespaces(*pParser);
    return pParser;
}

namespace {

OUString getTransitionalRelationshipOfficeDocType(const OUString& rPart)
{
    return "http://schemas.openxmlformats.org/officeDocument/2006/relationships/" + rPart;
}

OUString getStrictRelationshipOfficeDocType(const OUString& rPart)
{
    return "http://purl.oclc.org/ooxml/officeDocument/relationships/" + rPart;
}

}

OUString XmlFilterBase::getFragmentPathFromFirstTypeFromOfficeDoc( const OUString& rPart )
{
    // importRelations() caches the relations map for subsequence calls
    const OUString aTransitionalRelationshipType = getTransitionalRelationshipOfficeDocType(rPart);
    OUString aFragment = importRelations( OUString() )->getFragmentPathFromFirstType( aTransitionalRelationshipType );
    if(aFragment.isEmpty())
    {
        const OUString aStrictRelationshipType = getStrictRelationshipOfficeDocType(rPart);
        aFragment = importRelations( OUString() )->getFragmentPathFromFirstType( aStrictRelationshipType );
    }

    return aFragment;
}

bool XmlFilterBase::importFragment( const rtl::Reference<FragmentHandler>& rxHandler )
{
    FastParser aParser(mxImpl->mxContext);
    registerNamespaces(aParser);
    return importFragment(rxHandler, aParser);
}

bool XmlFilterBase::importFragment( const rtl::Reference<FragmentHandler>& rxHandler, FastParser& rParser )
{
    OSL_ENSURE( rxHandler.is(), "XmlFilterBase::importFragment - missing fragment handler" );
    if( !rxHandler.is() )
        return false;

    // fragment handler must contain path to fragment stream
    OUString aFragmentPath = rxHandler->getFragmentPath();
    OSL_ENSURE( !aFragmentPath.isEmpty(), "XmlFilterBase::importFragment - missing fragment path" );
    if( aFragmentPath.isEmpty() )
        return false;

    // try to import binary streams (fragment extension must be '.bin')
    if( lclHasSuffix( aFragmentPath, mxImpl->maBinSuffix ) )
    {
        try
        {
            // try to open the fragment stream (this may fail - do not assert)
            Reference< XInputStream > xInStrm( openInputStream( aFragmentPath ), UNO_SET_THROW );

            // create the record parser
            RecordParser aParser;
            aParser.setFragmentHandler( rxHandler );

            // create the input source and parse the stream
            RecordInputSource aSource;
            aSource.mxInStream.reset( new BinaryXInputStream( xInStrm, true ) );
            aSource.maSystemId = aFragmentPath;
            aParser.parseStream( aSource );
            return true;
        }
        catch( Exception& )
        {
        }
        return false;
    }

    // get the XFastDocumentHandler interface from the fragment handler
    Reference< XFastDocumentHandler > xDocHandler( rxHandler.get() );
    if( !xDocHandler.is() )
        return false;

    // try to import XML stream
    try
    {
        /*  Try to open the fragment stream (may fail, do not throw/assert).
            Using the virtual function openFragmentStream() allows a document
            handler to create specialized input streams, e.g. VML streams that
            have to preprocess the raw input data. */
        Reference< XInputStream > xInStrm = rxHandler->openFragmentStream();

        // own try/catch block for showing parser failure assertion with fragment path
        if( xInStrm.is() ) try
        {
            rParser.setDocumentHandler(xDocHandler);
            rParser.parseStream(xInStrm, aFragmentPath);
            return true;
        }
        catch( Exception& )
        {
            OSL_FAIL( OStringBuffer( "XmlFilterBase::importFragment - XML parser failed in fragment '" ).
                append( OUStringToOString( aFragmentPath, RTL_TEXTENCODING_ASCII_US ) ).append( '\'' ).getStr() );
        }
    }
    catch( Exception& )
    {
    }
    return false;
}

Reference<XDocument> XmlFilterBase::importFragment( const OUString& aFragmentPath )
{
    Reference<XDocument> xRet;

    // path to fragment stream valid?
    OSL_ENSURE( !aFragmentPath.isEmpty(), "XmlFilterBase::importFragment - empty fragment path" );
    if( aFragmentPath.isEmpty() )
        return xRet;

    // try to open the fragment stream (this may fail - do not assert)
    Reference< XInputStream > xInStrm = openInputStream( aFragmentPath );
    if( !xInStrm.is() )
        return xRet;

    // binary streams (fragment extension is '.bin') currently not supported
    sal_Int32 nBinSuffixPos = aFragmentPath.getLength() - mxImpl->maBinSuffix.getLength();
    if( (nBinSuffixPos >= 0) && aFragmentPath.match( mxImpl->maBinSuffix, nBinSuffixPos ) )
        return xRet;

    // try to import XML stream
    try
    {
        // create the dom parser
        Reference<XDocumentBuilder> xDomBuilder( DocumentBuilder::create( getComponentContext() ) );

        // create DOM from fragment
        xRet = xDomBuilder->parse(xInStrm);
    }
    catch( Exception& )
    {
    }

    return xRet;
}

bool XmlFilterBase::importFragment( const ::rtl::Reference< FragmentHandler >& rxHandler,
                                    const Reference< XFastSAXSerializable >& rxSerializer )
{
    Reference< XFastDocumentHandler > xDocHandler( rxHandler.get() );
    if( !xDocHandler.is() )
        return false;

    // try to import XML stream
    try
    {
        rxSerializer->fastSerialize( xDocHandler,
                                     mxImpl->maFastParser.getTokenHandler(),
                                     Sequence< StringPair >(),
                                     NamespaceIds::get() );
        return true;
    }
    catch( Exception& )
    {}

    return false;
}

RelationsRef XmlFilterBase::importRelations( const OUString& rFragmentPath )
{
    // try to find cached relations
    RelationsRef& rxRelations = mxImpl->maRelationsMap[ rFragmentPath ];
    if( !rxRelations )
    {
        // import and cache relations
        rxRelations.reset( new Relations( rFragmentPath ) );
        importFragment( new RelationsFragment( *this, rxRelations ) );
    }
    return rxRelations;
}

Reference< XOutputStream > XmlFilterBase::openFragmentStream( const OUString& rStreamName, const OUString& rMediaType )
{
    Reference< XOutputStream > xOutputStream = openOutputStream( rStreamName );
    PropertySet aPropSet( xOutputStream );
    aPropSet.setProperty( PROP_MediaType, rMediaType );
    return xOutputStream;
}

FSHelperPtr XmlFilterBase::openFragmentStreamWithSerializer( const OUString& rStreamName, const OUString& rMediaType )
{
    bool bWriteHeader = rMediaType.indexOf( "vml" ) < 0 || rMediaType.indexOf( "+xml" ) >= 0;
    return std::make_shared<FastSerializerHelper>( openFragmentStream( rStreamName, rMediaType ), bWriteHeader );
}

TextFieldStack& XmlFilterBase::getTextFieldStack() const
{
    return mxImpl->maTextFieldStack;
}

namespace {

OUString lclAddRelation( const Reference< XRelationshipAccess >& rRelations, sal_Int32 nId, const OUString& rType, const OUString& rTarget, bool bExternal )
{
    OUString sId = OUStringBuffer().append( "rId" ).append( nId ).makeStringAndClear();

    Sequence< StringPair > aEntry( bExternal ? 3 : 2 );
    aEntry[0].First = "Type";
    aEntry[0].Second = rType;
    aEntry[1].First = "Target";
    aEntry[1].Second = rTarget;
    if( bExternal )
    {
        aEntry[2].First = "TargetMode";
        aEntry[2].Second = "External";
    }
    rRelations->insertRelationshipByID( sId, aEntry, true );

    return sId;
}

} // namespace

OUString XmlFilterBase::addRelation( const OUString& rType, const OUString& rTarget )
{
    Reference< XRelationshipAccess > xRelations( getStorage()->getXStorage(), UNO_QUERY );
    if( xRelations.is() )
        return lclAddRelation( xRelations, mnRelId ++, rType, rTarget, false/*bExternal*/ );

    return OUString();
}

OUString XmlFilterBase::addRelation( const Reference< XOutputStream >& rOutputStream, const OUString& rType, const OUString& rTarget, bool bExternal )
{
    sal_Int32 nId = 0;

    PropertySet aPropSet( rOutputStream );
    if( aPropSet.is() )
        aPropSet.getProperty( nId, PROP_RelId );
    else
        nId = mnRelId++;

    Reference< XRelationshipAccess > xRelations( rOutputStream, UNO_QUERY );
    if( xRelations.is() )
        return lclAddRelation( xRelations, nId, rType, rTarget, bExternal );

    return OUString();
}

static void
writeElement( const FSHelperPtr& pDoc, sal_Int32 nXmlElement, const OUString& sValue )
{
    pDoc->startElement( nXmlElement, FSEND );
    pDoc->writeEscaped( sValue );
    pDoc->endElement( nXmlElement );
}

static void
writeElement( const FSHelperPtr& pDoc, sal_Int32 nXmlElement, const sal_Int32 nValue )
{
    pDoc->startElement( nXmlElement, FSEND );
    pDoc->write( nValue );
    pDoc->endElement( nXmlElement );
}

static void
writeElement( const FSHelperPtr& pDoc, sal_Int32 nXmlElement, const util::DateTime& rTime )
{
    if( rTime.Year == 0 )
        return;

    if ( ( nXmlElement >> 16 ) != XML_dcterms )
        pDoc->startElement( nXmlElement, FSEND );
    else
        pDoc->startElement( nXmlElement,
                FSNS( XML_xsi, XML_type ), "dcterms:W3CDTF",
                FSEND );

    char pStr[200];
    snprintf( pStr, sizeof( pStr ), "%d-%02d-%02dT%02d:%02d:%02dZ",
            rTime.Year, rTime.Month, rTime.Day,
            rTime.Hours, rTime.Minutes, rTime.Seconds );

    pDoc->write( pStr );

    pDoc->endElement( nXmlElement );
}

static void
writeElement( const FSHelperPtr& pDoc, sal_Int32 nXmlElement, const Sequence< OUString >& aItems )
{
    if( aItems.getLength() == 0 )
        return;

    OUStringBuffer sRep;
    sRep.append( aItems[ 0 ] );

    for( sal_Int32 i = 1, end = aItems.getLength(); i < end; ++i )
    {
        sRep.append( " " ).append( aItems[ i ] );
    }

    writeElement( pDoc, nXmlElement, sRep.makeStringAndClear() );
}

static void
writeElement( const FSHelperPtr& pDoc, sal_Int32 nXmlElement, const LanguageTag& rLanguageTag )
{
    // dc:language, Dublin Core recommends "such as RFC 4646", which is BCP 47
    // and obsoleted by RFC 5646, see
    // http://dublincore.org/documents/dcmi-terms/#terms-language
    // http://dublincore.org/documents/dcmi-terms/#elements-language
    writeElement( pDoc, nXmlElement, rLanguageTag.getBcp47() );
}

static void
writeCoreProperties( XmlFilterBase& rSelf, const Reference< XDocumentProperties >& xProperties )
{
    OUString sValue;
    if( rSelf.getVersion() == oox::core::ISOIEC_29500_2008  )
        sValue = "http://schemas.openxmlformats.org/officedocument/2006/relationships/metadata/core-properties";
    else
        sValue = "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties";

    rSelf.addRelation( sValue, "docProps/core.xml" );
    FSHelperPtr pCoreProps = rSelf.openFragmentStreamWithSerializer(
            "docProps/core.xml",
            "application/vnd.openxmlformats-package.core-properties+xml" );
    pCoreProps->startElementNS( XML_cp, XML_coreProperties,
            FSNS( XML_xmlns, XML_cp ),          "http://schemas.openxmlformats.org/package/2006/metadata/core-properties",
            FSNS( XML_xmlns, XML_dc ),          "http://purl.org/dc/elements/1.1/",
            FSNS( XML_xmlns, XML_dcterms ),     "http://purl.org/dc/terms/",
            FSNS( XML_xmlns, XML_dcmitype ),    "http://purl.org/dc/dcmitype/",
            FSNS( XML_xmlns, XML_xsi ),         "http://www.w3.org/2001/XMLSchema-instance",
            FSEND );

#ifdef OOXTODO
    writeElement( pCoreProps, FSNS( XML_cp, XML_category ),         "category" );
    writeElement( pCoreProps, FSNS( XML_cp, XML_contentStatus ),    "status" );
    writeElement( pCoreProps, FSNS( XML_cp, XML_contentType ),      "contentType" );
#endif  /* def OOXTODO */
    writeElement( pCoreProps, FSNS( XML_dcterms, XML_created ),     xProperties->getCreationDate() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_creator ),          xProperties->getAuthor() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_description ),      xProperties->getDescription() );
#ifdef OOXTODO
    writeElement( pCoreProps, FSNS( XML_dc, XML_identifier ),       "ident" );
#endif  /* def OOXTODO */
    writeElement( pCoreProps, FSNS( XML_cp, XML_keywords ),         xProperties->getKeywords() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_language ),         LanguageTag( xProperties->getLanguage()) );
    writeElement( pCoreProps, FSNS( XML_cp, XML_lastModifiedBy ),   xProperties->getModifiedBy() );
    writeElement( pCoreProps, FSNS( XML_cp, XML_lastPrinted ),      xProperties->getPrintDate() );
    writeElement( pCoreProps, FSNS( XML_dcterms, XML_modified ),    xProperties->getModificationDate() );
    writeElement( pCoreProps, FSNS( XML_cp, XML_revision ),         xProperties->getEditingCycles() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_subject ),          xProperties->getSubject() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_title ),            xProperties->getTitle() );
#ifdef OOXTODO
    writeElement( pCoreProps, FSNS( XML_cp, XML_version ),          "version" );
#endif  /* def OOXTODO */

    pCoreProps->endElementNS( XML_cp, XML_coreProperties );
}

static void
writeAppProperties( XmlFilterBase& rSelf, const Reference< XDocumentProperties >& xProperties )
{
    rSelf.addRelation(
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties",
            "docProps/app.xml" );
    FSHelperPtr pAppProps = rSelf.openFragmentStreamWithSerializer(
            "docProps/app.xml",
            "application/vnd.openxmlformats-officedocument.extended-properties+xml" );
    pAppProps->startElement( XML_Properties,
            XML_xmlns,                  "http://schemas.openxmlformats.org/officeDocument/2006/extended-properties",
            FSNS( XML_xmlns, XML_vt ),  "http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes",
            FSEND );

    writeElement( pAppProps, XML_Template,              xProperties->getTemplateName() );
#ifdef OOXTODO
    writeElement( pAppProps, XML_Manager,               "manager" );
    writeElement( pAppProps, XML_PresentationFormat,    "presentation format" );
    writeElement( pAppProps, XML_Lines,                 "lines" );
    writeElement( pAppProps, XML_Slides,                "slides" );
    writeElement( pAppProps, XML_Notes,                 "notes" );
#endif  /* def OOXTODO */
    // EditingDuration is in seconds, TotalTime is in minutes.
    writeElement( pAppProps, XML_TotalTime,             xProperties->getEditingDuration() / 60 );
#ifdef OOXTODO
    writeElement( pAppProps, XML_HiddenSlides,          "hidden slides" );
    writeElement( pAppProps, XML_MMClips,               "mm clips" );
    writeElement( pAppProps, XML_ScaleCrop,             "scale crop" );
    writeElement( pAppProps, XML_HeadingPairs,          "heading pairs" );
    writeElement( pAppProps, XML_TitlesOfParts,         "titles of parts" );
    writeElement( pAppProps, XML_LinksUpToDate,         "links up-to-date" );
    writeElement( pAppProps, XML_SharedDoc,             "shared doc" );
    writeElement( pAppProps, XML_HyperlinkBase,         "hyperlink base" );
    writeElement( pAppProps, XML_HLinks,                "hlinks" );
    writeElement( pAppProps, XML_HyperlinksChanged,     "hyperlinks changed" );
    writeElement( pAppProps, XML_DigSig,                "digital signature" );
#endif  /* def OOXTODO */
    writeElement( pAppProps, XML_Application,           utl::DocInfoHelper::GetGeneratorString() );
#ifdef OOXTODO
    writeElement( pAppProps, XML_AppVersion,            "app version" );
    writeElement( pAppProps, XML_DocSecurity,           "doc security" );
#endif  /* def OOXTODO */

    comphelper::SequenceAsHashMap aStats = xProperties->getDocumentStatistics();
    comphelper::SequenceAsHashMap::iterator it;
    sal_Int32 nValue = 0;

    it = aStats.find("PageCount");
    if (it != aStats.end())
    {
            if (it->second >>= nValue)
                writeElement(pAppProps, XML_Pages, nValue);
    }

    it = aStats.find("WordCount");
    if (it != aStats.end())
    {
            if (it->second >>= nValue)
                writeElement(pAppProps, XML_Words, nValue);
    }

    it = aStats.find("NonWhitespaceCharacterCount");
    if (it != aStats.end())
    {
            if (it->second >>= nValue)
                writeElement(pAppProps, XML_Characters, nValue);
    }

    it = aStats.find("CharacterCount");
    if (it != aStats.end())
    {
            if (it->second >>= nValue)
                writeElement(pAppProps, XML_CharactersWithSpaces, nValue);
    }

    it = aStats.find("ParagraphCount");
    if (it != aStats.end())
    {
            if (it->second >>= nValue)
                writeElement(pAppProps, XML_Paragraphs, nValue);
    }

    uno::Reference<beans::XPropertyAccess> xUserDefinedProperties(xProperties->getUserDefinedProperties(), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aUserDefinedProperties(xUserDefinedProperties->getPropertyValues());
    it = aUserDefinedProperties.find("Company");
    if (it != aUserDefinedProperties.end())
    {
        OUString aValue;
        if (it->second >>= aValue)
            writeElement(pAppProps, XML_Company, aValue);
    }

    pAppProps->endElement( XML_Properties );
}

static void
writeCustomProperties( XmlFilterBase& rSelf, const Reference< XDocumentProperties >& xProperties )
{
    uno::Reference<beans::XPropertyAccess> xUserDefinedProperties( xProperties->getUserDefinedProperties(), uno::UNO_QUERY );
    Sequence< PropertyValue > aprop( xUserDefinedProperties->getPropertyValues() );
    sal_Int32 nbCustomProperties = aprop.getLength();
    // tdf#89791 : if no custom properties, no need to add docProps/custom.x
    if (!nbCustomProperties)
        return;

    rSelf.addRelation(
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/custom-properties",
            "docProps/custom.xml" );
    FSHelperPtr pAppProps = rSelf.openFragmentStreamWithSerializer(
            "docProps/custom.xml",
            "application/vnd.openxmlformats-officedocument.custom-properties+xml" );
    pAppProps->startElement( XML_Properties,
            XML_xmlns,                  "http://schemas.openxmlformats.org/officeDocument/2006/custom-properties",
            FSNS( XML_xmlns, XML_vt ),  "http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes",
            FSEND );

    for ( sal_Int32 n = 0; n < nbCustomProperties; ++n )
    {
        if ( !aprop[n].Name.isEmpty() )
        {
            OString aName = OUStringToOString( aprop[n].Name, RTL_TEXTENCODING_ASCII_US );
            // pid starts from 2 not from 1 as MS supports pid from 2
            pAppProps->startElement( XML_property ,
                XML_fmtid,  "{D5CDD505-2E9C-101B-9397-08002B2CF9AE}",
                XML_pid,    OString::number(n + 2),
                XML_name,   aName,
                FSEND);

            switch ( ( aprop[n].Value ).getValueTypeClass() )
            {
                case TypeClass_STRING:
                {
                    OUString aValue;
                    aprop[n].Value >>= aValue;
                     writeElement( pAppProps, FSNS( XML_vt, XML_lpwstr ), aValue );
                }
                break;
                case TypeClass_BOOLEAN:
                {
                    bool val = *o3tl::forceAccess<bool>(aprop[n].Value);
                    writeElement( pAppProps, FSNS( XML_vt, XML_bool ), val ? 1 : 0);
                }
                break;
                default:
                {
                    double num;
                    util::Date aDate;
                    util::Duration aDuration;
                    util::DateTime aDateTime;
                    if ( ( aprop[n].Value ) >>= num )
                    {
                        writeElement( pAppProps, FSNS( XML_vt, XML_i4 ), num );
                    }
                    else if ( ( aprop[n].Value ) >>= aDate )
                    {
                        aDateTime = util::DateTime( 0, 0 , 0, 0, aDate.Year, aDate.Month, aDate.Day, true );
                        writeElement( pAppProps, FSNS( XML_vt, XML_filetime ), aDateTime);
                    }
                    else if ( ( aprop[n].Value ) >>= aDuration )
                    {
                        OUStringBuffer buf;
                        ::sax::Converter::convertDuration( buf, aDuration );
                        OUString aDurationStr = buf.makeStringAndClear();
                        writeElement( pAppProps, FSNS( XML_vt, XML_lpwstr ), aDurationStr );
                    }
                    else if ( ( aprop[n].Value ) >>= aDateTime )
                            writeElement( pAppProps, FSNS( XML_vt, XML_filetime ), aDateTime );
                    else
                        //no other options
                        OSL_FAIL( "XMLFilterBase::writeCustomProperties unsupported value type!" );
                 }
                 break;
            }
            pAppProps->endElement( XML_property );
        }
    }
    pAppProps->endElement( XML_Properties );
}

void XmlFilterBase::exportDocumentProperties( const Reference< XDocumentProperties >& xProperties )
{
    if( xProperties.is() )
    {
        writeCoreProperties( *this, xProperties );
        writeAppProperties( *this, xProperties );
        writeCustomProperties( *this, xProperties );
    }
}

// protected ------------------------------------------------------------------

Reference< XInputStream > XmlFilterBase::implGetInputStream( MediaDescriptor& rMediaDesc ) const
{
    /*  Get the input stream directly from the media descriptor, or decrypt the
        package again. The latter is needed e.g. when the document is reloaded.
        All this is implemented in the detector service. */
    rtl::Reference< FilterDetect > xDetector( new FilterDetect( getComponentContext() ) );
    return xDetector->extractUnencryptedPackage( rMediaDesc );
}

Reference<XStream> XmlFilterBase::implGetOutputStream( MediaDescriptor& rMediaDescriptor ) const
{
    Sequence< NamedValue > aMediaEncData;
    aMediaEncData = rMediaDescriptor.getUnpackedValueOrDefault(
                                        MediaDescriptor::PROP_ENCRYPTIONDATA(),
                                        Sequence< NamedValue >() );

    OUString aPassword;
    for (int i=0; i<aMediaEncData.getLength(); i++)
    {
        if (aMediaEncData[i].Name == "OOXPassword")
        {
            Any& any = aMediaEncData[i].Value;
            any >>= aPassword;
            break;
        }
    }
    if (aPassword.isEmpty())
    {
        return FilterBase::implGetOutputStream( rMediaDescriptor );
    }
    else // We need to encrypt the stream so create a memory stream
    {
        Reference< XComponentContext > xContext = getComponentContext();
        return Reference< XStream > (
                    xContext->getServiceManager()->createInstanceWithContext("com.sun.star.comp.MemoryStream", xContext),
                    uno::UNO_QUERY_THROW );
    }
}

bool XmlFilterBase::implFinalizeExport( MediaDescriptor& rMediaDescriptor )
{
    bool bRet = true;

    Sequence< NamedValue > aMediaEncData;
    aMediaEncData = rMediaDescriptor.getUnpackedValueOrDefault(
                                        MediaDescriptor::PROP_ENCRYPTIONDATA(),
                                        Sequence< NamedValue >() );

    OUString aPassword;

    for (int i=0; i<aMediaEncData.getLength(); i++)
    {
        if (aMediaEncData[i].Name == "OOXPassword")
        {
            Any& any = aMediaEncData[i].Value;
            any >>= aPassword;
            break;
        }
    }

    if (!aPassword.isEmpty())
    {
        commitStorage();

        Reference< XStream> xDocumentStream (FilterBase::implGetOutputStream(rMediaDescriptor));
        oox::ole::OleStorage aOleStorage( getComponentContext(), xDocumentStream, true );
        DocumentEncryption encryptor(getMainDocumentStream(), aOleStorage, aPassword);
        bRet = encryptor.encrypt();
        if (bRet)
            aOleStorage.commit();
    }

    return bRet;
}

// private --------------------------------------------------------------------

StorageRef XmlFilterBase::implCreateStorage( const Reference< XInputStream >& rxInStream ) const
{
    return StorageRef( new ZipStorage( getComponentContext(), rxInStream ) );
}

StorageRef XmlFilterBase::implCreateStorage( const Reference< XStream >& rxOutStream ) const
{
    return StorageRef( new ZipStorage( getComponentContext(), rxOutStream ) );
}

bool XmlFilterBase::isMSO2007Document() const
{
    return mbMSO2007;
}

void XmlFilterBase::setMissingExtDrawing()
{
    mbMissingExtDrawing = true;
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
