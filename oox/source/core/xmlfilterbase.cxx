/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "oox/core/xmlfilterbase.hxx"

#include <cstdio>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <unotools/mediadescriptor.hxx>
#include <sax/fshelper.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <i18nlangtag/languagetag.hxx>
#include "oox/core/fastparser.hxx"
#include "oox/core/filterdetect.hxx"
#include "oox/core/fragmenthandler.hxx"
#include "oox/core/recordparser.hxx"
#include "oox/core/relationshandler.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/zipstorage.hxx"
#include "oox/token/properties.hxx"
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XOOXMLDocumentPropertiesImporter.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <comphelper/processfactory.hxx>
#include <oox/core/filterdetect.hxx>
#include <comphelper/storagehelper.hxx>

#include <oox/crypto/DocumentEncryption.hxx>

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
        static const char* const namespaceURIs[] = {
            "http:
            "http:
            "http:
            "http:
            "http:
            "http:
            "http:
            "urn:schemas-microsoft-com:vml",
            "urn:schemas-microsoft-com:office:office",
            "urn:schemas-microsoft-com:office:word",
            "urn:schemas-microsoft-com:office:excel",
            "urn:schemas-microsoft-com:office:powerpoint",
            "http:
            "http:
            "http:
            "http:
            "http:
            "http:
            "http:
            "http:
            "http:
        };

        static const sal_Int32 namespaceIds[] = {
            NMSP_xml,
            NMSP_packageRel,
            NMSP_officeRel,
            NMSP_dml,
            NMSP_dmlDiagram,
            NMSP_dmlChart,
            NMSP_dmlChartDr,
            NMSP_dmlSpreadDr,
            NMSP_vml,
            NMSP_vmlOffice,
            NMSP_vmlWord,
            NMSP_vmlExcel,
            NMSP_vmlPowerpoint,
            NMSP_xls,
            NMSP_ppt,
            NMSP_ax,
            NMSP_xm,
            NMSP_mce,
            NMSP_mceTest,
            NMSP_dsp,
            NMSP_xlsExtLst
        };

        Sequence< beans::Pair< OUString, sal_Int32 > > aRet(STATIC_ARRAY_SIZE(namespaceIds));
        for( sal_Int32 i=0; i<aRet.getLength(); ++i )
            aRet[i] = make_Pair(
                OUString::createFromAscii(namespaceURIs[i]),
                namespaceIds[i]);
        return aRet;
    }
};

void registerNamespaces( FastParser& rParser )
{
    const Sequence< beans::Pair<OUString, sal_Int32> > ids = NamespaceIds::get();
    for (sal_Int32 i = 0; i < ids.getLength(); ++i)
        rParser.registerNamespace(ids[i].Second);
}

} 

struct XmlFilterBaseImpl
{
    typedef RefMap< OUString, Relations > RelationsMap;

    FastParser                     maFastParser;
    const OUString                 maBinSuffix;
    const OUString                 maVmlSuffix;
    RelationsMap                   maRelationsMap;
    TextFieldStack                 maTextFieldStack;

    explicit            XmlFilterBaseImpl( const Reference< XComponentContext >& rxContext ) throw( RuntimeException );
};

XmlFilterBaseImpl::XmlFilterBaseImpl( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    maFastParser( rxContext ),
    maBinSuffix( ".bin" ),
    maVmlSuffix( ".vml" )
{
    
    registerNamespaces(maFastParser);
}

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



void XmlFilterBase::importDocumentProperties()
{
    Reference< XMultiServiceFactory > xFactory( getComponentContext()->getServiceManager(), UNO_QUERY );
    MediaDescriptor aMediaDesc( getMediaDescriptor() );
    Reference< XInputStream > xInputStream;
    Reference< XComponentContext > xContext = getComponentContext();
    ::oox::core::FilterDetect aDetector( xContext );
    xInputStream = aDetector.extractUnencryptedPackage( aMediaDesc );
    Reference< XComponent > xModel( getModel(), UNO_QUERY );
    Reference< XStorage > xDocumentStorage (
            ::comphelper::OStorageHelper::GetStorageOfFormatFromInputStream( OFOPXML_STORAGE_FORMAT_STRING, xInputStream ) );
    Reference< XInterface > xTemp = xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.document.OOXMLDocumentPropertiesImporter",
            xContext);
    Reference< XOOXMLDocumentPropertiesImporter > xImporter( xTemp, UNO_QUERY );
    Reference< XDocumentPropertiesSupplier > xPropSupplier( xModel, UNO_QUERY);
    xImporter->importProperties( xDocumentStorage, xPropSupplier->getDocumentProperties() );
}

FastParser* XmlFilterBase::createParser() const
{
    FastParser* pParser = new FastParser(getComponentContext());
    registerNamespaces(*pParser);
    return pParser;
}

OUString XmlFilterBase::getFragmentPathFromFirstType( const OUString& rType )
{
    
    return importRelations( OUString() )->getFragmentPathFromFirstType( rType );
}

bool XmlFilterBase::importFragment( const rtl::Reference<FragmentHandler>& rxHandler )
{
    return importFragment(rxHandler, mxImpl->maFastParser);
}

bool XmlFilterBase::importFragment( const rtl::Reference<FragmentHandler>& rxHandler, FastParser& rParser )
{
    OSL_ENSURE( rxHandler.is(), "XmlFilterBase::importFragment - missing fragment handler" );
    if( !rxHandler.is() )
        return false;

    
    OUString aFragmentPath = rxHandler->getFragmentPath();
    OSL_ENSURE( !aFragmentPath.isEmpty(), "XmlFilterBase::importFragment - missing fragment path" );
    if( aFragmentPath.isEmpty() )
        return false;

    
    if( lclHasSuffix( aFragmentPath, mxImpl->maBinSuffix ) )
    {
        try
        {
            
            Reference< XInputStream > xInStrm( openInputStream( aFragmentPath ), UNO_SET_THROW );

            
            RecordParser aParser;
            aParser.setFragmentHandler( rxHandler );

            
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

    
    Reference< XFastDocumentHandler > xDocHandler( rxHandler.get() );
    if( !xDocHandler.is() )
        return false;

    
    try
    {
        /*  Try to open the fragment stream (may fail, do not throw/assert).
            Using the virtual function openFragmentStream() allows a document
            handler to create specialized input streams, e.g. VML streams that
            have to preprocess the raw input data. */
        Reference< XInputStream > xInStrm = rxHandler->openFragmentStream();

        
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

OUString XmlFilterBase::getNamespaceURL( const OUString& rPrefix )
{
    return mxImpl->maFastParser.getNamespaceURL( rPrefix );
}

bool XmlFilterBase::hasNamespaceURL( const OUString& rPrefix ) const
{
    return mxImpl->maFastParser.hasNamespaceURL(rPrefix);
}

sal_Int32 XmlFilterBase::getNamespaceId( const OUString& rUrl )
{
     return mxImpl->maFastParser.getNamespaceId( rUrl );
}

Reference<XDocument> XmlFilterBase::importFragment( const OUString& aFragmentPath )
{
    Reference<XDocument> xRet;

    
    OSL_ENSURE( !aFragmentPath.isEmpty(), "XmlFilterBase::importFragment - empty fragment path" );
    if( aFragmentPath.isEmpty() )
        return xRet;

    
    Reference< XInputStream > xInStrm = openInputStream( aFragmentPath );
    if( !xInStrm.is() )
        return xRet;

    
    sal_Int32 nBinSuffixPos = aFragmentPath.getLength() - mxImpl->maBinSuffix.getLength();
    if( (nBinSuffixPos >= 0) && aFragmentPath.match( mxImpl->maBinSuffix, nBinSuffixPos ) )
        return xRet;

    
    try
    {
        
        Reference<XDocumentBuilder> xDomBuilder( DocumentBuilder::create( getComponentContext() ) );

        
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
    
    RelationsRef& rxRelations = mxImpl->maRelationsMap[ rFragmentPath ];
    if( !rxRelations )
    {
        
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
    aEntry[0].First = "Type";
    aEntry[0].Second = rType;
    aEntry[1].First = "Target";
    aEntry[1].Second = rTarget;
    if( bExternal )
    {
        aEntry[2].First = "TargetMode";
        aEntry[2].Second = "External";
    }
    xRelations->insertRelationshipByID( sId, aEntry, sal_True );

    return sId;
}

} 

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
    if( sValue.isEmpty() )
        return;
    pDoc->startElement( nXmlElement, FSEND );
    pDoc->writeEscaped( sValue );
    pDoc->endElement( nXmlElement );
}

static void
writeElement( FSHelperPtr pDoc, sal_Int32 nXmlElement, const sal_Int32 nValue )
{
    pDoc->startElement( nXmlElement, FSEND );
    pDoc->write( OUString::number( nValue ) );
    pDoc->endElement( nXmlElement );
}

static void
writeElement( FSHelperPtr pDoc, sal_Int32 nXmlElement, const util::DateTime& rTime )
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
writeElement( FSHelperPtr pDoc, sal_Int32 nXmlElement, Sequence< OUString > aItems )
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
writeElement( FSHelperPtr pDoc, sal_Int32 nXmlElement, const LanguageTag& rLanguageTag )
{
    
    
    
    
    writeElement( pDoc, nXmlElement, rLanguageTag.getBcp47() );
}

static void
writeCoreProperties( XmlFilterBase& rSelf, Reference< XDocumentProperties > xProperties )
{
    OUString sValue;
    if( rSelf.getVersion() == oox::core::ISOIEC_29500_2008  )
        sValue = "http:
    else
        sValue = "http:

    rSelf.addRelation( sValue, "docProps/core.xml" );
    FSHelperPtr pCoreProps = rSelf.openFragmentStreamWithSerializer(
            "docProps/core.xml",
            "application/vnd.openxmlformats-package.core-properties+xml" );
    pCoreProps->startElementNS( XML_cp, XML_coreProperties,
            FSNS( XML_xmlns, XML_cp ),          "http:
            FSNS( XML_xmlns, XML_dc ),          "http:
            FSNS( XML_xmlns, XML_dcterms ),     "http:
            FSNS( XML_xmlns, XML_dcmitype ),    "http:
            FSNS( XML_xmlns, XML_xsi ),         "http:
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
writeAppProperties( XmlFilterBase& rSelf, Reference< XDocumentProperties > xProperties )
{
    rSelf.addRelation(
            "http:
            "docProps/app.xml" );
    FSHelperPtr pAppProps = rSelf.openFragmentStreamWithSerializer(
            "docProps/app.xml",
            "application/vnd.openxmlformats-officedocument.extended-properties+xml" );
    pAppProps->startElement( XML_Properties,
            XML_xmlns,                  "http:
            FSNS( XML_xmlns, XML_vt ),  "http:
            FSEND );

    writeElement( pAppProps, XML_Template,              xProperties->getTemplateName() );
#ifdef OOXTODO
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
#ifdef OOXTODO
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
#ifdef OOXTODO
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
        OSL_TRACE( "# Document Statistics:" );
        for( sal_Int32 i = 0, end = aStats.getLength(); i < end; ++i )
        {
            ::com::sun::star::uno::Any aValue = aStats[ i ].Value;
            OUString sValue;
            bool bHaveString = aValue >>= sValue;
            OSL_TRACE ("#\t%s=%s [%s]\n",
                    OUStringToOString( aStats[ i ].Name, RTL_TEXTENCODING_UTF8 ).getStr(),
                    bHaveString
                        ? OUStringToOString( sValue, RTL_TEXTENCODING_UTF8 ).getStr()
                        : "<unconvertable>",
                    OUStringToOString( aValue.getValueTypeName(), RTL_TEXTENCODING_UTF8 ).getStr());
        }
    }
    return *this;
}



Reference< XInputStream > XmlFilterBase::implGetInputStream( MediaDescriptor& rMediaDesc ) const
{
    /*  Get the input stream directly from the media descriptor, or decrypt the
        package again. The latter is needed e.g. when the document is reloaded.
        All this is implemented in the detector service. */
    FilterDetect aDetector( getComponentContext() );
    return aDetector.extractUnencryptedPackage( rMediaDesc );
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
    else 
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



StorageRef XmlFilterBase::implCreateStorage( const Reference< XInputStream >& rxInStream ) const
{
    return StorageRef( new ZipStorage( getComponentContext(), rxInStream ) );
}

StorageRef XmlFilterBase::implCreateStorage( const Reference< XStream >& rxOutStream ) const
{
    return StorageRef( new ZipStorage( getComponentContext(), rxOutStream ) );
}



} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
