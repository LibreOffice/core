/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "oox/core/xmlfilterbase.hxx"

#include <cstdio>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <comphelper/mediadescriptor.hxx>
#include <sax/fshelper.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include "oox/core/fastparser.hxx"
#include "oox/core/filterdetect.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/core/recordparser.hxx"
#include "oox/core/relationshandler.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/zipstorage.hxx"
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XOOXMLDocumentPropertiesImporter.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <oox/core/filterdetect.hxx>
#include <comphelper/storagehelper.hxx>
using ::com::sun::star::uno::XComponentContext;
using ::com::sun::star::document::XOOXMLDocumentPropertiesImporter;
using ::com::sun::star::document::XDocumentPropertiesSupplier;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::lang::XComponent;

namespace oox {
namespace core {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;

using ::comphelper::MediaDescriptor;
using ::rtl::OStringBuffer;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::sax_fastparser::FSHelperPtr;
using ::sax_fastparser::FastSerializerHelper;





// ============================================================================

namespace {

bool lclHasSuffix( const OUString& rFragmentPath, const OUString& rSuffix )
{
    sal_Int32 nSuffixPos = rFragmentPath.getLength() - rSuffix.getLength();
    return (nSuffixPos >= 0) && rFragmentPath.match( rSuffix, nSuffixPos );
}

} // namespace

// ============================================================================

struct XmlFilterBaseImpl
{
    typedef RefMap< OUString, Relations > RelationsMap;

    FastParser          maFastParser;
    const OUString      maBinSuffix;
    const OUString      maVmlSuffix;
    RelationsMap        maRelationsMap;
    TextFieldStack      maTextFieldStack;

    explicit            XmlFilterBaseImpl( const Reference< XComponentContext >& rxContext ) throw( RuntimeException );
};

// ----------------------------------------------------------------------------

XmlFilterBaseImpl::XmlFilterBaseImpl( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    maFastParser( rxContext ),
    maBinSuffix( CREATE_OUSTRING( ".bin" ) ),
    maVmlSuffix( CREATE_OUSTRING( ".vml" ) )
{
    // register XML namespaces
    maFastParser.registerNamespace( NMSP_xml );
    maFastParser.registerNamespace( NMSP_packageRel );
    maFastParser.registerNamespace( NMSP_officeRel );

    maFastParser.registerNamespace( NMSP_dml );
    maFastParser.registerNamespace( NMSP_dmlDiagram );
    maFastParser.registerNamespace( NMSP_dmlChart );
    maFastParser.registerNamespace( NMSP_dmlChartDr );
    maFastParser.registerNamespace( NMSP_dmlSpreadDr );

    maFastParser.registerNamespace( NMSP_vml );
    maFastParser.registerNamespace( NMSP_vmlOffice );
    maFastParser.registerNamespace( NMSP_vmlWord );
    maFastParser.registerNamespace( NMSP_vmlExcel );
    maFastParser.registerNamespace( NMSP_vmlPowerpoint );

    maFastParser.registerNamespace( NMSP_xls );
    maFastParser.registerNamespace( NMSP_ppt );

    maFastParser.registerNamespace( NMSP_ax );
    maFastParser.registerNamespace( NMSP_xm );
    maFastParser.registerNamespace( NMSP_mce );
    maFastParser.registerNamespace( NMSP_mceTest );
}


static Reference< XComponentContext > lcl_getComponentContext(Reference< XMultiServiceFactory > aFactory)
{
    Reference< XComponentContext > xContext;
    try
    {
        Reference< XPropertySet > xFactProp( aFactory, UNO_QUERY );
        if( xFactProp.is() )
            xFactProp->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("DefaultContext")) ) >>= xContext;
    }
    catch( Exception& )
    {}

    return xContext;
}

// ============================================================================

// ============================================================================

XmlFilterBase::XmlFilterBase( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    FilterBase( rxContext ),
    mxImpl( new XmlFilterBaseImpl( rxContext ) ),
    mnRelId( 1 ),
    mnMaxDocId( 0 )
{
}

XmlFilterBase::~XmlFilterBase()
{
}

// ----------------------------------------------------------------------------

void XmlFilterBase::importDocumentProperties() throw()
{
    Reference< XMultiServiceFactory > xFactory( getServiceFactory(), UNO_QUERY );
    MediaDescriptor aMediaDesc( getMediaDescriptor() );
    Reference< XInputStream > xInputStream;
    Reference< XComponentContext > xContext = lcl_getComponentContext(getServiceFactory());
    ::oox::core::FilterDetect aDetector( xContext );
    xInputStream = aDetector.extractUnencryptedPackage( aMediaDesc );
    Reference< XComponent > xModel( getModel(), UNO_QUERY );
    Reference< XStorage > xDocumentStorage (
            ::comphelper::OStorageHelper::GetStorageOfFormatFromInputStream( OFOPXML_STORAGE_FORMAT_STRING, xInputStream ) );
    Reference< XInterface > xTemp = xContext->getServiceManager()->createInstanceWithContext(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.OOXMLDocumentPropertiesImporter")),
            xContext);
    Reference< XOOXMLDocumentPropertiesImporter > xImporter( xTemp, UNO_QUERY );
    Reference< XDocumentPropertiesSupplier > xPropSupplier( xModel, UNO_QUERY);
    xImporter->importProperties( xDocumentStorage, xPropSupplier->getDocumentProperties() );
}

OUString XmlFilterBase::getFragmentPathFromFirstType( const OUString& rType )
{
    // importRelations() caches the relations map for subsequence calls
    return importRelations( OUString() )->getFragmentPathFromFirstType( rType );
}

bool XmlFilterBase::importFragment( const ::rtl::Reference< FragmentHandler >& rxHandler )
{
    OSL_ENSURE( rxHandler.is(), "XmlFilterBase::importFragment - missing fragment handler" );
    if( !rxHandler.is() )
        return false;

    // fragment handler must contain path to fragment stream
    OUString aFragmentPath = rxHandler->getFragmentPath();
    OSL_ENSURE( aFragmentPath.getLength() > 0, "XmlFilterBase::importFragment - missing fragment path" );
    if( aFragmentPath.getLength() == 0 )
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
            mxImpl->maFastParser.setDocumentHandler( xDocHandler );
            mxImpl->maFastParser.parseStream( xInStrm, aFragmentPath );
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

OUString XmlFilterBase::getNamespaceURL( const OUString& rPrefix )
{
    return mxImpl->maFastParser.getNamespaceURL( rPrefix );
}

sal_Int32 XmlFilterBase::getNamespaceId( const OUString& rUrl )
{
     return mxImpl->maFastParser.getNamespaceId( rUrl );
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
    bool bWriteHeader = true;
    if( rMediaType.indexOfAsciiL( "vml", 3 ) >= 0 &&
        rMediaType.indexOfAsciiL( "+xml", 4 ) < 0 )
        bWriteHeader = false;
    return FSHelperPtr( new FastSerializerHelper( openFragmentStream( rStreamName, rMediaType ), bWriteHeader ) );
}

TextFieldStack& XmlFilterBase::getTextFieldStack() const
{
    return mxImpl->maTextFieldStack;
}

namespace {

OUString lclAddRelation( const Reference< XRelationshipAccess > xRelations, sal_Int32 nId, const OUString& rType, const OUString& rTarget, bool bExternal )
{
    OUString sId = OUStringBuffer().appendAscii( "rId" ).append( nId ).makeStringAndClear();

    Sequence< StringPair > aEntry( bExternal ? 3 : 2 );
    aEntry[0].First = CREATE_OUSTRING( "Type" );
    aEntry[0].Second = rType;
    aEntry[1].First = CREATE_OUSTRING( "Target" );
    aEntry[1].Second = rTarget;
    if( bExternal )
    {
        aEntry[2].First = CREATE_OUSTRING( "TargetMode" );
        aEntry[2].Second = CREATE_OUSTRING( "External" );
    }
    xRelations->insertRelationshipByID( sId, aEntry, sal_True );

    return sId;
}

} // namespace

OUString XmlFilterBase::addRelation( const OUString& rType, const OUString& rTarget, bool bExternal )
{
    Reference< XRelationshipAccess > xRelations( getStorage()->getXStorage(), UNO_QUERY );
    if( xRelations.is() )
        return lclAddRelation( xRelations, mnRelId ++, rType, rTarget, bExternal );

    return OUString();
}

OUString XmlFilterBase::addRelation( const Reference< XOutputStream > xOutputStream, const OUString& rType, const OUString& rTarget, bool bExternal )
{
    sal_Int32 nId = 0;

    PropertySet aPropSet( xOutputStream );
    if( aPropSet.is() )
        aPropSet.getProperty( nId, PROP_RelId );
    else
        nId = mnRelId++;

    Reference< XRelationshipAccess > xRelations( xOutputStream, UNO_QUERY );
    if( xRelations.is() )
        return lclAddRelation( xRelations, nId, rType, rTarget, bExternal );

    return OUString();
}

static void
writeElement( FSHelperPtr pDoc, sal_Int32 nXmlElement, const OUString& sValue )
{
    if( sValue.getLength() == 0 )
        return;
    pDoc->startElement( nXmlElement, FSEND );
    pDoc->write( sValue );
    pDoc->endElement( nXmlElement );
}

static void
writeElement( FSHelperPtr pDoc, sal_Int32 nXmlElement, const sal_Int32 nValue )
{
    pDoc->startElement( nXmlElement, FSEND );
    pDoc->write( OUString::valueOf( nValue ) );
    pDoc->endElement( nXmlElement );
}

static void
writeElement( FSHelperPtr pDoc, sal_Int32 nXmlElement, const DateTime& rTime )
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
    snprintf( pStr, sizeof( pStr ), "%d-%02d-%02dT%02d:%02d:%02d.%02dZ",
            rTime.Year, rTime.Month, rTime.Day,
            rTime.Hours, rTime.Minutes, rTime.Seconds,
            rTime.HundredthSeconds );

    pDoc->write( pStr );

    pDoc->endElement( nXmlElement );
}

static void
writeElement( FSHelperPtr pDoc, sal_Int32 nXmlElement, Sequence< rtl::OUString > aItems )
{
    if( aItems.getLength() == 0 )
        return;

    OUStringBuffer sRep;
    sRep.append( aItems[ 0 ] );

    for( sal_Int32 i = 1, end = aItems.getLength(); i < end; ++i )
    {
        sRep.appendAscii( " " ).append( aItems[ i ] );
    }

    writeElement( pDoc, nXmlElement, sRep.makeStringAndClear() );
}

static void
writeElement( FSHelperPtr pDoc, sal_Int32 nXmlElement, const Locale& rLocale )
{
    // TODO: what to do with .Country and .Variant
    writeElement( pDoc, nXmlElement, rLocale.Language );
}

static void
writeCoreProperties( XmlFilterBase& rSelf, Reference< XDocumentProperties > xProperties )
{
    OUString sValue;
    if( rSelf.getVersion() == oox::core::ISOIEC_29500_2008  )
        sValue = CREATE_OUSTRING( "http://schemas.openxmlformats.org/officedocument/2006/relationships/metadata/core-properties" );
    else
        sValue = CREATE_OUSTRING( "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" );

    rSelf.addRelation( sValue, CREATE_OUSTRING( "docProps/core.xml" ) );
    FSHelperPtr pCoreProps = rSelf.openFragmentStreamWithSerializer(
            CREATE_OUSTRING( "docProps/core.xml" ),
            CREATE_OUSTRING( "application/vnd.openxmlformats-package.core-properties+xml" ) );
    pCoreProps->startElementNS( XML_cp, XML_coreProperties,
            FSNS( XML_xmlns, XML_cp ),          "http://schemas.openxmlformats.org/package/2006/metadata/core-properties",
            FSNS( XML_xmlns, XML_dc ),          "http://purl.org/dc/elements/1.1/",
            FSNS( XML_xmlns, XML_dcterms ),     "http://purl.org/dc/terms/",
            FSNS( XML_xmlns, XML_dcmitype ),    "http://purl.org/dc/dcmitype/",
            FSNS( XML_xmlns, XML_xsi ),         "http://www.w3.org/2001/XMLSchema-instance",
            FSEND );

#if OOXTODO
    writeElement( pCoreProps, FSNS( XML_cp, XML_category ),         "category" );
    writeElement( pCoreProps, FSNS( XML_cp, XML_contentStatus ),    "status" );
    writeElement( pCoreProps, FSNS( XML_cp, XML_contentType ),      "contentType" );
#endif  /* def OOXTODO */
    writeElement( pCoreProps, FSNS( XML_dcterms, XML_created ),     xProperties->getCreationDate() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_creator ),          xProperties->getAuthor() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_description ),      xProperties->getDescription() );
#if OOXTODO
    writeElement( pCoreProps, FSNS( XML_dc, XML_identifier ),       "ident" );
#endif  /* def OOXTODO */
    writeElement( pCoreProps, FSNS( XML_cp, XML_keywords ),         xProperties->getKeywords() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_language ),         xProperties->getLanguage() );
    writeElement( pCoreProps, FSNS( XML_cp, XML_lastModifiedBy ),   xProperties->getModifiedBy() );
    writeElement( pCoreProps, FSNS( XML_cp, XML_lastPrinted ),      xProperties->getPrintDate() );
    writeElement( pCoreProps, FSNS( XML_dcterms, XML_modified ),    xProperties->getModificationDate() );
    writeElement( pCoreProps, FSNS( XML_cp, XML_revision ),         xProperties->getEditingCycles() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_subject ),          xProperties->getSubject() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_title ),            xProperties->getTitle() );
#if OOXTODO
    writeElement( pCoreProps, FSNS( XML_cp, XML_version ),          "version" );
#endif  /* def OOXTODO */

    pCoreProps->endElementNS( XML_cp, XML_coreProperties );
}

static void
writeAppProperties( XmlFilterBase& rSelf, Reference< XDocumentProperties > xProperties )
{
    rSelf.addRelation(
            CREATE_OUSTRING( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" ),
            CREATE_OUSTRING( "docProps/app.xml" ) );
    FSHelperPtr pAppProps = rSelf.openFragmentStreamWithSerializer(
            CREATE_OUSTRING( "docProps/app.xml" ),
            CREATE_OUSTRING( "application/vnd.openxmlformats-officedocument.extended-properties+xml" ) );
    pAppProps->startElement( XML_Properties,
            XML_xmlns,                  "http://schemas.openxmlformats.org/officeDocument/2006/extended-properties",
            FSNS( XML_xmlns, XML_vt ),  "http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes",
            FSEND );

    writeElement( pAppProps, XML_Template,              xProperties->getTemplateName() );
#if OOXTODO
    writeElement( pAppProps, XML_Manager,               "manager" );
    writeElement( pAppProps, XML_Company,               "company" );
    writeElement( pAppProps, XML_Pages,                 "pages" );
    writeElement( pAppProps, XML_Words,                 "words" );
    writeElement( pAppProps, XML_Characters,            "characters" );
    writeElement( pAppProps, XML_PresentationFormat,    "presentation format" );
    writeElement( pAppProps, XML_Lines,                 "lines" );
    writeElement( pAppProps, XML_Paragraphs,            "paragraphs" );
    writeElement( pAppProps, XML_Slides,                "slides" );
    writeElement( pAppProps, XML_Notes,                 "notes" );
#endif  /* def OOXTODO */
    writeElement( pAppProps, XML_TotalTime,             xProperties->getEditingDuration() );
#if OOXTODO
    writeElement( pAppProps, XML_HiddenSlides,          "hidden slides" );
    writeElement( pAppProps, XML_MMClips,               "mm clips" );
    writeElement( pAppProps, XML_ScaleCrop,             "scale crop" );
    writeElement( pAppProps, XML_HeadingPairs,          "heading pairs" );
    writeElement( pAppProps, XML_TitlesOfParts,         "titles of parts" );
    writeElement( pAppProps, XML_LinksUpToDate,         "links up-to-date" );
    writeElement( pAppProps, XML_CharactersWithSpaces,  "characters with spaces" );
    writeElement( pAppProps, XML_SharedDoc,             "shared doc" );
    writeElement( pAppProps, XML_HyperlinkBase,         "hyperlink base" );
    writeElement( pAppProps, XML_HLinks,                "hlinks" );
    writeElement( pAppProps, XML_HyperlinksChanged,     "hyperlinks changed" );
    writeElement( pAppProps, XML_DigSig,                "digital signature" );
#endif  /* def OOXTODO */
    writeElement( pAppProps, XML_Application,           xProperties->getGenerator() );
#if OOXTODO
    writeElement( pAppProps, XML_AppVersion,            "app version" );
    writeElement( pAppProps, XML_DocSecurity,           "doc security" );
#endif  /* def OOXTODO */
    pAppProps->endElement( XML_Properties );
}

XmlFilterBase& XmlFilterBase::exportDocumentProperties( Reference< XDocumentProperties > xProperties )
{
    if( xProperties.is() )
    {
        writeCoreProperties( *this, xProperties );
        writeAppProperties( *this, xProperties );
        Sequence< ::com::sun::star::beans::NamedValue > aStats = xProperties->getDocumentStatistics();
        printf( "# Document Statistics:\n" );
        for( sal_Int32 i = 0, end = aStats.getLength(); i < end; ++i )
        {
            ::com::sun::star::uno::Any aValue = aStats[ i ].Value;
            ::rtl::OUString sValue;
            bool bHaveString = aValue >>= sValue;
            printf ("#\t%s=%s [%s]\n",
                    OUStringToOString( aStats[ i ].Name, RTL_TEXTENCODING_UTF8 ).getStr(),
                    bHaveString
                        ? OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr()
                        : "<unconvertable>",
                    OUStringToOString( aValue.getValueTypeName(), RTL_TEXTENCODING_UTF8 ).getStr());
        }
    }
    return *this;
}

// protected ------------------------------------------------------------------

Reference< XInputStream > XmlFilterBase::implGetInputStream( MediaDescriptor& rMediaDesc ) const
{
    /*  Get the input stream directly from the media descriptor, or decrypt the
        package again. The latter is needed e.g. when the document is reloaded.
        All this is implemented in the detector service. */
    FilterDetect aDetector( getComponentContext() );
    return aDetector.extractUnencryptedPackage( rMediaDesc );
}

// private --------------------------------------------------------------------

StorageRef XmlFilterBase::implCreateStorage( const Reference< XInputStream >& rxInStream ) const
{
    return StorageRef( new ZipStorage( getServiceFactory(), rxInStream ) );
}

StorageRef XmlFilterBase::implCreateStorage( const Reference< XStream >& rxOutStream ) const
{
    return StorageRef( new ZipStorage( getServiceFactory(), rxOutStream ) );
}

// ============================================================================

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
