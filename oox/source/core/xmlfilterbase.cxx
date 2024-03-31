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

#include <oox/core/xmlfilterbase.hxx>

#include <cstdio>
#include <string_view>

#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/Pair.hpp>
#include <com/sun/star/embed/XRelationshipAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <o3tl/any.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/docinfohelper.hxx>
#include <unotools/securityoptions.hxx>
#include <sax/fshelper.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <oox/core/fastparser.hxx>
#include <oox/core/fragmenthandler.hxx>
#include <oox/core/recordparser.hxx>
#include <oox/core/relationshandler.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/helper/zipstorage.hxx>
#include <oox/ole/olestorage.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/relationship.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XOOXMLDocumentPropertiesImporter.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <comphelper/processfactory.hxx>
#include <oox/core/filterdetect.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/ofopxmlhelper.hxx>

#include <oox/crypto/DocumentEncryption.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <sax/tools/converter.hxx>
#include <oox/token/namespacemap.hxx>
#include <editeng/unoprnms.hxx>
#include <o3tl/sorted_vector.hxx>

using ::com::sun::star::xml::dom::DocumentBuilder;
using ::com::sun::star::xml::dom::XDocument;
using ::com::sun::star::xml::dom::XDocumentBuilder;

namespace oox::core {

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
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

const Sequence< beans::Pair< OUString, sal_Int32 > >& NamespaceIds()
{
    static const Sequence< beans::Pair< OUString, sal_Int32 > > SINGLETON
        {
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
            {"http://schemas.microsoft.com/office/drawing/2012/chart",
             NMSP_c15},
            {"http://schemas.microsoft.com/office/spreadsheetml/2015/revision2",
             NMSP_xr2},
            {"http://schemas.microsoft.com/office/drawing/2017/decorative", NMSP_adec},
            {"http://schemas.microsoft.com/office/drawing/2016/SVG/main", NMSP_asvg},
        };
    return SINGLETON;
};

void registerNamespaces( FastParser& rParser )
{
    const Sequence< beans::Pair<OUString, sal_Int32> >& ids = NamespaceIds();

    // Filter out duplicates: a namespace can have multiple URLs, think of
    // strict vs transitional.
    o3tl::sorted_vector<sal_Int32> aSet;
    aSet.reserve(ids.getLength());
    for (const auto& rId : ids)
        aSet.insert(rId.Second);

    for (auto const& elem : aSet)
        rParser.registerNamespace(elem);
}

} // namespace

struct XmlFilterBaseImpl
{
    typedef RefMap< OUString, Relations > RelationsMap;

    FastParser                     maFastParser;
    RelationsMap                   maRelationsMap;
    const NamespaceMap&            mrNamespaceMap;
    NamedShapePairs* mpDiagramFontHeights = nullptr;

    /// @throws RuntimeException
    explicit            XmlFilterBaseImpl();
};

constexpr OUString gaBinSuffix( u".bin"_ustr );

XmlFilterBaseImpl::XmlFilterBaseImpl() :
    mrNamespaceMap(StaticNamespaceMap())
{
    // register XML namespaces
    registerNamespaces(maFastParser);
}

XmlFilterBase::XmlFilterBase( const Reference< XComponentContext >& rxContext ) :
    FilterBase( rxContext ),
    mxImpl( new XmlFilterBaseImpl ),
    mnRelId( 1 ),
    mnMaxDocId( 0 ),
    mbMSO2007(false),
    mbMSO(false),
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
    mxImpl->maFastParser.clearDocumentHandler();
}

std::shared_ptr<::oox::drawingml::Theme> XmlFilterBase::getCurrentThemePtr() const
{
    // default returns empty ptr
    return std::shared_ptr<::oox::drawingml::Theme>();
}

void XmlFilterBase::checkDocumentProperties(const Reference<XDocumentProperties>& xDocProps)
{
    mbMSO2007 = mbMSO = false;
    if (!xDocProps->getGenerator().startsWithIgnoreAsciiCase("Microsoft"))
        return;
    mbMSO = true;

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

void XmlFilterBase::putPropertiesToDocumentGrabBag(const css::uno::Reference<css::lang::XComponent>& xDstDoc,
                                                   const comphelper::SequenceAsHashMap& rProperties)
{
    try
    {
        uno::Reference<beans::XPropertySet> xDocProps(xDstDoc, uno::UNO_QUERY);
        if (xDocProps.is())
        {
            uno::Reference<beans::XPropertySetInfo> xPropsInfo = xDocProps->getPropertySetInfo();

            static constexpr OUString aGrabBagPropName = u"InteropGrabBag"_ustr;
            if (xPropsInfo.is() && xPropsInfo->hasPropertyByName(aGrabBagPropName))
            {
                // get existing grab bag
                comphelper::SequenceAsHashMap aGrabBag(xDocProps->getPropertyValue(aGrabBagPropName));

                // put the new items
                aGrabBag.update(rProperties);

                // put it back to the document
                xDocProps->setPropertyValue(aGrabBagPropName, uno::Any(aGrabBag.getAsConstPropertyValueList()));
            }
        }
    }
    catch (const uno::Exception&)
    {
        SAL_WARN("oox","Failed to save documents grab bag");
    }
}

void XmlFilterBase::importDocumentProperties()
{
    MediaDescriptor aMediaDesc( getMediaDescriptor() );
    Reference< XInputStream > xInputStream;
    Reference< XComponentContext > xContext = getComponentContext();
    rtl::Reference< ::oox::core::FilterDetect > xDetector( new ::oox::core::FilterDetect( xContext ) );
    xInputStream = xDetector->extractUnencryptedPackage( aMediaDesc );
    Reference< XComponent > xModel = getModel();
    const bool repairPackage = aMediaDesc.getUnpackedValueOrDefault("RepairPackage", false);
    Reference< XStorage > xDocumentStorage (
        ::comphelper::OStorageHelper::GetStorageOfFormatFromInputStream(
            OFOPXML_STORAGE_FORMAT_STRING, xInputStream, {}, repairPackage));
    Reference< XInterface > xTemp = xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.document.OOXMLDocumentPropertiesImporter",
            xContext);
    Reference< XOOXMLDocumentPropertiesImporter > xImporter( xTemp, UNO_QUERY );
    Reference< XDocumentPropertiesSupplier > xPropSupplier( xModel, UNO_QUERY);
    Reference< XDocumentProperties > xDocProps = xPropSupplier->getDocumentProperties();
    xImporter->importProperties( xDocumentStorage, xDocProps );
    checkDocumentProperties(xDocProps);

    importCustomFragments(xDocumentStorage);
}

FastParser* XmlFilterBase::createParser()
{
    FastParser* pParser = new FastParser;
    registerNamespaces(*pParser);
    return pParser;
}

namespace {

OUString getTransitionalRelationshipOfficeDocType(std::u16string_view rPart)
{
    return OUString::Concat("http://schemas.openxmlformats.org/officeDocument/2006/relationships/")
        + rPart;
}

OUString getStrictRelationshipOfficeDocType(std::u16string_view rPart)
{
    return OUString::Concat("http://purl.oclc.org/ooxml/officeDocument/relationships/") + rPart;
}

}

OUString XmlFilterBase::getFragmentPathFromFirstTypeFromOfficeDoc( std::u16string_view rPart )
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
    FastParser aParser;
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
    if (aFragmentPath.endsWith(gaBinSuffix))
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
            aSource.mxInStream = std::make_shared<BinaryXInputStream>( xInStrm, true );
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
    if( !rxHandler.is() )
        return false;

    // try to import XML stream
    try
    {
        /*  Try to open the fragment stream (may fail, do not throw/assert).
            Using the virtual function openFragmentStream() allows a document
            handler to create specialized input streams, e.g. VML streams that
            have to preprocess the raw input data. */
        Reference< XInputStream > xInStrm = rxHandler->openFragmentStream();
        /*  tdf#100084 Check again the aFragmentPath route with lowercase file name
            TODO: complete handling of case-insensitive file paths */
        if ( !xInStrm.is() )
        {
            sal_Int32 nPathLen = aFragmentPath.lastIndexOf('/') + 1;
            OUString fileName = aFragmentPath.copy(nPathLen);
            OUString sLowerCaseFileName = fileName.toAsciiLowerCase();
            if ( fileName != sLowerCaseFileName )
            {
                aFragmentPath = aFragmentPath.subView(0, nPathLen) + sLowerCaseFileName;
                xInStrm = openInputStream(aFragmentPath);
            }
        }

        // own try/catch block for showing parser failure assertion with fragment path
        if( xInStrm.is() ) try
        {
            rParser.setDocumentHandler(rxHandler);
            rParser.parseStream(xInStrm, aFragmentPath);
            return true;
        }
        catch( Exception& )
        {
            OSL_FAIL( OStringBuffer( "XmlFilterBase::importFragment - XML parser failed in fragment '"  +
                     OUStringToOString( aFragmentPath, RTL_TEXTENCODING_ASCII_US ) + "'" ).getStr() );
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
    if (aFragmentPath.endsWith(gaBinSuffix))
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
    if( !rxHandler.is() )
        return false;

    // try to import XML stream
    try
    {
        rxSerializer->fastSerialize( rxHandler,
                                     mxImpl->maFastParser.getTokenHandler(),
                                     Sequence< StringPair >(),
                                     NamespaceIds() );
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
        rxRelations = std::make_shared<Relations>( rFragmentPath );
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
    const bool bWriteHeader = rMediaType.indexOf( "vml" ) < 0 || rMediaType.indexOf( "+xml" ) >= 0;
    return std::make_shared<FastSerializerHelper>( openFragmentStream( rStreamName, rMediaType ), bWriteHeader );
}

namespace {

OUString lclAddRelation( const Reference< XRelationshipAccess >& rRelations, sal_Int32 nId, const OUString& rType, std::u16string_view rTarget, bool bExternal )
{
    OUString sId = "rId" + OUString::number( nId );

    Sequence< StringPair > aEntry( bExternal ? 3 : 2 );
    auto pEntry = aEntry.getArray();
    pEntry[0].First = "Type";
    pEntry[0].Second = rType;
    pEntry[1].First = "Target";
    pEntry[1].Second = INetURLObject::decode(rTarget, INetURLObject::DecodeMechanism::ToIUri, RTL_TEXTENCODING_UTF8);
    if( bExternal )
    {
        pEntry[2].First = "TargetMode";
        pEntry[2].Second = "External";
    }
    rRelations->insertRelationshipByID( sId, aEntry, true );

    return sId;
}

} // namespace

OUString XmlFilterBase::addRelation( const OUString& rType, std::u16string_view rTarget )
{
    Reference< XRelationshipAccess > xRelations( getStorage()->getXStorage(), UNO_QUERY );
    if( xRelations.is() )
        return lclAddRelation( xRelations, mnRelId ++, rType, rTarget, false/*bExternal*/ );

    return OUString();
}

OUString XmlFilterBase::addRelation( const Reference< XOutputStream >& rOutputStream, const OUString& rType, std::u16string_view rTarget, bool bExternal )
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
writeElement( const FSHelperPtr& pDoc, sal_Int32 nXmlElement, std::u16string_view sValue )
{
    pDoc->startElement(nXmlElement);
    pDoc->writeEscaped( sValue );
    pDoc->endElement( nXmlElement );
}

static void
writeElement( const FSHelperPtr& pDoc, sal_Int32 nXmlElement, const sal_Int32 nValue )
{
    pDoc->startElement(nXmlElement);
    pDoc->write( nValue );
    pDoc->endElement( nXmlElement );
}

static void
writeElement( const FSHelperPtr& pDoc, sal_Int32 nXmlElement, const util::DateTime& rTime )
{
    if( rTime.Year == 0 )
        return;

    if ( ( nXmlElement >> 16 ) != XML_dcterms )
        pDoc->startElement(nXmlElement);
    else
        pDoc->startElement(nXmlElement, FSNS(XML_xsi, XML_type), "dcterms:W3CDTF");

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
    if( !aItems.hasElements() )
        return;

    OUStringBuffer sRep;
    // tdf#143175 - join elements including a delimiter using a standard iterator
    ::comphelper::intersperse(aItems.begin(), aItems.end(),
                              ::comphelper::OUStringBufferAppender(sRep), OUString(" "));

    writeElement( pDoc, nXmlElement, sRep );
}

static void
writeElement( const FSHelperPtr& pDoc, sal_Int32 nXmlElement, const LanguageTag& rLanguageTag )
{
    // dc:language, Dublin Core recommends "such as RFC 4646", which is BCP 47
    // and obsoleted by RFC 5646, see
    // http://dublincore.org/documents/dcmi-terms/#terms-language
    // http://dublincore.org/documents/dcmi-terms/#elements-language
    writeElement( pDoc, nXmlElement, rLanguageTag.getBcp47MS() );
}

static void
writeCoreProperties( XmlFilterBase& rSelf, const Reference< XDocumentProperties >& xProperties )
{
    bool bRemovePersonalInfo
        = SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo);
    bool bRemoveUserInfo
        = bRemovePersonalInfo
          && !SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnKeepDocUserInfo);

    OUString sValue;
    if( rSelf.getVersion() == oox::core::ISOIEC_29500_2008  )
    {
        // The lowercase "officedocument" is intentional and according to the spec
        // (although most other places are written "officeDocument")
        sValue = "http://schemas.openxmlformats.org/officedocument/2006/relationships/metadata/core-properties";
    }
    else
        sValue = "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties";

    rSelf.addRelation( sValue, u"docProps/core.xml" );
    FSHelperPtr pCoreProps = rSelf.openFragmentStreamWithSerializer(
            "docProps/core.xml",
            "application/vnd.openxmlformats-package.core-properties+xml" );
    pCoreProps->startElementNS( XML_cp, XML_coreProperties,
        FSNS(XML_xmlns, XML_cp),       rSelf.getNamespaceURL(OOX_NS(packageMetaCorePr)),
        FSNS(XML_xmlns, XML_dc),       rSelf.getNamespaceURL(OOX_NS(dc)),
        FSNS(XML_xmlns, XML_dcterms),  rSelf.getNamespaceURL(OOX_NS(dcTerms)),
        FSNS(XML_xmlns, XML_dcmitype), rSelf.getNamespaceURL(OOX_NS(dcmiType)),
        FSNS(XML_xmlns, XML_xsi),      rSelf.getNamespaceURL(OOX_NS(xsi)));

    uno::Reference<beans::XPropertyAccess> xUserDefinedProperties(xProperties->getUserDefinedProperties(), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aUserDefinedProperties(xUserDefinedProperties->getPropertyValues());
    comphelper::SequenceAsHashMap::iterator it;

    it = aUserDefinedProperties.find("OOXMLCorePropertyCategory");
    if (it != aUserDefinedProperties.end())
    {
        OUString aValue;
        if (it->second >>= aValue)
            writeElement( pCoreProps, FSNS( XML_cp, XML_category ), aValue );
    }

    it = aUserDefinedProperties.find("OOXMLCorePropertyContentStatus");
    if (it != aUserDefinedProperties.end())
    {
        OUString aValue;
        if (it->second >>= aValue)
            writeElement( pCoreProps, FSNS( XML_cp, XML_contentStatus ), aValue );
    }

    it = aUserDefinedProperties.find("OOXMLCorePropertyContentType");
    if (it != aUserDefinedProperties.end())
    {
        OUString aValue;
        if (it->second >>= aValue)
            writeElement( pCoreProps, FSNS( XML_cp, XML_contentType ), aValue );
    }
    if (!bRemoveUserInfo)
    {
        writeElement(pCoreProps, FSNS(XML_dcterms, XML_created), xProperties->getCreationDate());
        writeElement(pCoreProps, FSNS(XML_dc, XML_creator), xProperties->getAuthor());
    }
    writeElement( pCoreProps, FSNS( XML_dc, XML_description ),      xProperties->getDescription() );

    it = aUserDefinedProperties.find("OOXMLCorePropertyIdentifier");
    if (it != aUserDefinedProperties.end())
    {
        OUString aValue;
        if (it->second >>= aValue)
            writeElement( pCoreProps, FSNS( XML_dc, XML_identifier ), aValue );
    }
    writeElement( pCoreProps, FSNS( XML_cp, XML_keywords ),         xProperties->getKeywords() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_language ),         LanguageTag( xProperties->getLanguage()) );

    if (!bRemoveUserInfo)
    {
        writeElement(pCoreProps, FSNS(XML_cp, XML_lastModifiedBy), xProperties->getModifiedBy());
        writeElement(pCoreProps, FSNS(XML_cp, XML_lastPrinted), xProperties->getPrintDate());
        writeElement(pCoreProps, FSNS(XML_dcterms, XML_modified),
                     xProperties->getModificationDate());
    }
    if (!bRemovePersonalInfo)
    {
        writeElement(pCoreProps, FSNS(XML_cp, XML_revision), xProperties->getEditingCycles());
    }
    writeElement( pCoreProps, FSNS( XML_dc, XML_subject ),          xProperties->getSubject() );
    writeElement( pCoreProps, FSNS( XML_dc, XML_title ),            xProperties->getTitle() );

    it = aUserDefinedProperties.find("OOXMLCorePropertyVersion");
    if (it != aUserDefinedProperties.end())
    {
        OUString aValue;
        if (it->second >>= aValue)
            writeElement( pCoreProps, FSNS( XML_cp, XML_version ), aValue );
    }

    pCoreProps->endElementNS( XML_cp, XML_coreProperties );

    pCoreProps->endDocument();
}

static void
writeAppProperties( XmlFilterBase& rSelf, const Reference< XDocumentProperties >& xProperties )
{
    bool bRemovePersonalInfo
        = SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo);
    bool bRemoveUserInfo
        = bRemovePersonalInfo
          && !SvtSecurityOptions::IsOptionSet(SvtSecurityOptions::EOption::DocWarnKeepDocUserInfo);
    rSelf.addRelation(
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties",
            u"docProps/app.xml" );
    FSHelperPtr pAppProps = rSelf.openFragmentStreamWithSerializer(
            "docProps/app.xml",
            "application/vnd.openxmlformats-officedocument.extended-properties+xml" );
    pAppProps->startElement( XML_Properties,
            XML_xmlns,               rSelf.getNamespaceURL(OOX_NS(officeExtPr)),
            FSNS(XML_xmlns, XML_vt), rSelf.getNamespaceURL(OOX_NS(officeDocPropsVT)));

    uno::Reference<beans::XPropertyAccess> xUserDefinedProperties(xProperties->getUserDefinedProperties(), uno::UNO_QUERY);
    comphelper::SequenceAsHashMap aUserDefinedProperties(xUserDefinedProperties->getPropertyValues());
    comphelper::SequenceAsHashMap::iterator it;

    if (!bRemovePersonalInfo)
        writeElement(pAppProps, XML_Template, xProperties->getTemplateName());

    it = aUserDefinedProperties.find("Manager");
    if (it != aUserDefinedProperties.end())
    {
        OUString aValue;
        if (it->second >>= aValue)
            writeElement( pAppProps, XML_Manager,       aValue );
    }

#ifdef OOXTODO
    writeElement( pAppProps, XML_PresentationFormat,    "presentation format" );
    writeElement( pAppProps, XML_Lines,                 "lines" );
    writeElement( pAppProps, XML_Slides,                "slides" );
    writeElement( pAppProps, XML_Notes,                 "notes" );
#endif  /* def OOXTODO */
    // EditingDuration is in seconds, TotalTime is in minutes.
    if (!bRemovePersonalInfo)
        writeElement(pAppProps, XML_TotalTime, xProperties->getEditingDuration() / 60);
#ifdef OOXTODO
    writeElement( pAppProps, XML_HiddenSlides,          "hidden slides" );
    writeElement( pAppProps, XML_MMClips,               "mm clips" );
    writeElement( pAppProps, XML_ScaleCrop,             "scale crop" );
    writeElement( pAppProps, XML_HeadingPairs,          "heading pairs" );
    writeElement( pAppProps, XML_TitlesOfParts,         "titles of parts" );
    writeElement( pAppProps, XML_LinksUpToDate,         "links up-to-date" );
    writeElement( pAppProps, XML_SharedDoc,             "shared doc" );
    writeElement( pAppProps, XML_HLinks,                "hlinks" );
    writeElement( pAppProps, XML_HyperlinksChanged,     "hyperlinks changed" );
    writeElement( pAppProps, XML_DigSig,                "digital signature" );
#endif  /* def OOXTODO */
    writeElement( pAppProps, XML_Application,           utl::DocInfoHelper::GetGeneratorString() );

    it = aUserDefinedProperties.find("HyperlinkBase");
    if (it != aUserDefinedProperties.end())
    {
        OUString aValue;
        if (it->second >>= aValue)
            writeElement( pAppProps, XML_HyperlinkBase, aValue );
    }
    // AppVersion specifies the version of the application which produced document
    // It is strictly connected with MS Office versions:
    //     * 12:  [Office 2007]  [LO < 7.0]
    //     * 14:  [Office 2010]
    //     * 15:  [Office 2013/2016/2019]  [LO >= 7.0]
    // The LibreOffice is application on 2013/2016/2019 level
    writeElement( pAppProps, XML_AppVersion, u"15.0000" );

    // OOXTODO Calculate DocSecurity value based on security (password, read-only etc.)
    it = aUserDefinedProperties.find("DocSecurity");
    if (it != aUserDefinedProperties.end())
    {
        sal_Int32 nValue;
        if (it->second >>= nValue)
            writeElement( pAppProps, XML_DocSecurity, nValue );
    }

    comphelper::SequenceAsHashMap aStats = xProperties->getDocumentStatistics();
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

    it = aUserDefinedProperties.find("Company");
    if (it != aUserDefinedProperties.end() && !bRemoveUserInfo)
    {
        OUString aValue;
        if (it->second >>= aValue)
            writeElement(pAppProps, XML_Company, aValue);
    }

    pAppProps->endElement( XML_Properties );

    pAppProps->endDocument();
}

static void
writeCustomProperties( XmlFilterBase& rSelf, const Reference< XDocumentProperties >& xProperties, bool bSecurityOptOpenReadOnly )
{
    uno::Reference<beans::XPropertyAccess> xUserDefinedProperties( xProperties->getUserDefinedProperties(), uno::UNO_QUERY );
    auto aprop = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(xUserDefinedProperties->getPropertyValues());
    sal_Int32 nbCustomProperties = aprop.size();
    // tdf#89791 : if no custom properties, no need to add docProps/custom.x
    // tdf#107690: except the case of read-only documents, because that
    // is handled by the _MarkAsFinal custom property in MSO.
    if (!nbCustomProperties && !bSecurityOptOpenReadOnly)
        return;

    if (bSecurityOptOpenReadOnly)
    {
        PropertyValue aPropertyValue;
        // MSO custom property for read-only documents
        aPropertyValue.Name = "_MarkAsFinal";
        aPropertyValue.Value <<= true;
        aprop.push_back(aPropertyValue);
    }

    rSelf.addRelation(
            "http://schemas.openxmlformats.org/officeDocument/2006/relationships/custom-properties",
            u"docProps/custom.xml" );
    FSHelperPtr pAppProps = rSelf.openFragmentStreamWithSerializer(
            "docProps/custom.xml",
            "application/vnd.openxmlformats-officedocument.custom-properties+xml" );
    pAppProps->startElement( XML_Properties,
            XML_xmlns,               rSelf.getNamespaceURL(OOX_NS(officeCustomPr)),
            FSNS(XML_xmlns, XML_vt), rSelf.getNamespaceURL(OOX_NS(officeDocPropsVT)));

    size_t nIndex = 0;
    for (const auto& rProp : aprop)
    {
        if ( !rProp.Name.isEmpty() )
        {
            // Skip storing these values in Custom Properties as it will be stored in Core/Extended Properties
            if (( rProp.Name == "OOXMLCorePropertyCategory" ) || // stored in cp:category
                ( rProp.Name == "OOXMLCorePropertyContentStatus" ) || // stored in cp:contentStatus
                ( rProp.Name == "OOXMLCorePropertyContentType" ) || // stored in cp:contentType
                ( rProp.Name == "OOXMLCorePropertyIdentifier" ) || // stored in dc:identifier
                ( rProp.Name == "OOXMLCorePropertyVersion" ) || // stored in cp:version
                ( rProp.Name == "HyperlinkBase" ) || // stored in Extended File Properties
                ( rProp.Name == "AppVersion" ) || // stored in Extended File Properties
                ( rProp.Name == "DocSecurity" ) || // stored in Extended File Properties
                ( rProp.Name == "Manager" ) || // stored in Extended File Properties
                ( rProp.Name == "Company" )) // stored in Extended File Properties
                continue;

            // pid starts from 2 not from 1 as MS supports pid from 2
            pAppProps->startElement( XML_property ,
                XML_fmtid,  "{D5CDD505-2E9C-101B-9397-08002B2CF9AE}",
                XML_pid,    OString::number(nIndex + 2),
                XML_name,   rProp.Name);

            switch ( rProp.Value.getValueTypeClass() )
            {
                case TypeClass_STRING:
                {
                    OUString aValue;
                    rProp.Value >>= aValue;
                    writeElement( pAppProps, FSNS( XML_vt, XML_lpwstr ), aValue );
                }
                break;
                case TypeClass_BOOLEAN:
                {
                    bool val = *o3tl::forceAccess<bool>(rProp.Value);
                    writeElement( pAppProps, FSNS( XML_vt, XML_bool ), val ? 1 : 0);
                }
                break;
                case TypeClass_DOUBLE:
                {
                    double num = {}; // spurious -Werror=maybe-uninitialized
                    if ( rProp.Value >>= num )
                    {
                        // r8 - 8-byte real number
                        writeElement( pAppProps, FSNS( XML_vt, XML_r8 ), OUString::number(num) );
                    }
                }
                break;
                default:
                {
                    double num = {}; // spurious -Werror=maybe-uninitialized
                    util::Date aDate;
                    util::Duration aDuration;
                    util::DateTime aDateTime;
                    if ( rProp.Value >>= num )
                    {
                        // i4 - 4-byte signed integer
                        writeElement( pAppProps, FSNS( XML_vt, XML_i4 ), num );
                    }
                    else if ( rProp.Value >>= aDate )
                    {
                        aDateTime = util::DateTime( 0, 0 , 0, 0, aDate.Day, aDate.Month, aDate.Year, true );
                        writeElement( pAppProps, FSNS( XML_vt, XML_filetime ), aDateTime);
                    }
                    else if ( rProp.Value >>= aDuration )
                    {
                        OUStringBuffer buf;
                        ::sax::Converter::convertDuration( buf, aDuration );
                        OUString aDurationStr = buf.makeStringAndClear();
                        writeElement( pAppProps, FSNS( XML_vt, XML_lpwstr ), aDurationStr );
                    }
                    else if ( rProp.Value >>= aDateTime )
                            writeElement( pAppProps, FSNS( XML_vt, XML_filetime ), aDateTime );
                    else
                        //no other options
                        OSL_FAIL( "XMLFilterBase::writeCustomProperties unsupported value type!" );
                 }
                 break;
            }
            pAppProps->endElement( XML_property );
        }
        ++nIndex;
    }
    pAppProps->endElement( XML_Properties );

    pAppProps->endDocument();
}

void XmlFilterBase::exportDocumentProperties( const Reference< XDocumentProperties >& xProperties, bool bSecurityOptOpenReadOnly )
{
    if( xProperties.is() )
    {
        writeCoreProperties( *this, xProperties );
        writeAppProperties( *this, xProperties );
        writeCustomProperties( *this, xProperties, bSecurityOptOpenReadOnly );
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
    const Sequence< NamedValue > aMediaEncData = rMediaDescriptor.getUnpackedValueOrDefault(
                                        MediaDescriptor::PROP_ENCRYPTIONDATA,
                                        Sequence< NamedValue >() );

    if (aMediaEncData.getLength() == 0)
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

    const Sequence< NamedValue > aMediaEncData = rMediaDescriptor.getUnpackedValueOrDefault(
                                        MediaDescriptor::PROP_ENCRYPTIONDATA,
                                        Sequence< NamedValue >() );

    if (aMediaEncData.getLength())
    {
        commitStorage();

        Reference< XStream> xDocumentStream (FilterBase::implGetOutputStream(rMediaDescriptor));
        oox::ole::OleStorage aOleStorage( getComponentContext(), xDocumentStream, true );
        crypto::DocumentEncryption encryptor( getComponentContext(), getMainDocumentStream(), aOleStorage, aMediaEncData );
        bRet = encryptor.encrypt();
        if (bRet)
            aOleStorage.commit();
    }

    return bRet;
}

// private --------------------------------------------------------------------

StorageRef XmlFilterBase::implCreateStorage( const Reference< XInputStream >& rxInStream ) const
{
    return std::make_shared<ZipStorage>(
        getComponentContext(), rxInStream,
        getMediaDescriptor().getUnpackedValueOrDefault("RepairPackage", false));
}

StorageRef XmlFilterBase::implCreateStorage( const Reference< XStream >& rxOutStream ) const
{
    return std::make_shared<ZipStorage>( getComponentContext(), rxOutStream );
}

bool XmlFilterBase::isMSO2007Document() const
{
    return mbMSO2007;
}

bool XmlFilterBase::isMSODocument() const
{
    return mbMSO;
}

void XmlFilterBase::setMissingExtDrawing()
{
    mbMissingExtDrawing = true;
}

void XmlFilterBase::setDiagramFontHeights(NamedShapePairs* pDiagramFontHeights)
{
    mxImpl->mpDiagramFontHeights = pDiagramFontHeights;
}

NamedShapePairs* XmlFilterBase::getDiagramFontHeights() { return mxImpl->mpDiagramFontHeights; }

OUString XmlFilterBase::getNamespaceURL(sal_Int32 nNSID) const
{
    auto itr = mxImpl->mrNamespaceMap.maTransitionalNamespaceMap.find(nNSID);
    if (itr == mxImpl->mrNamespaceMap.maTransitionalNamespaceMap.end())
    {
        SAL_WARN("oox", "missing namespace in the namespace map for : " << nNSID);
        return OUString();
    }

    return itr->second;
}

void XmlFilterBase::importCustomFragments(css::uno::Reference<css::embed::XStorage> const & xDocumentStorage)
{
    Reference<XRelationshipAccess> xRelations(xDocumentStorage, UNO_QUERY);
    if (!xRelations.is())
        return;

    const uno::Sequence<uno::Sequence<beans::StringPair>> aSeqs = xRelations->getAllRelationships();

    std::vector<StreamDataSequence> aCustomFragments;
    std::vector<OUString> aCustomFragmentTypes;
    std::vector<OUString> aCustomFragmentTargets;
    for (const uno::Sequence<beans::StringPair>& aSeq : aSeqs)
    {
        OUString sType;
        OUString sTarget;
        for (const beans::StringPair& aPair : aSeq)
        {
            if (aPair.First == "Target")
                sTarget = aPair.Second;
            else if (aPair.First == "Type")
                sType = aPair.Second;
        }

        // Preserve non-standard (i.e. custom) entries.
        if (!sType.match("http://schemas.openxmlformats.org") // OOXML/ECMA Transitional
            && !sType.match("http://purl.oclc.org")) // OOXML Strict
        {
            StreamDataSequence aDataSeq;
            if (importBinaryData(aDataSeq, sTarget))
            {
                aCustomFragments.emplace_back(aDataSeq);
                aCustomFragmentTypes.emplace_back(sType);
                aCustomFragmentTargets.emplace_back(sTarget);
            }
        }
    }

    // Adding the saved custom xml DOM
    comphelper::SequenceAsHashMap aGrabBagProperties;
    aGrabBagProperties["OOXCustomFragments"] <<= comphelper::containerToSequence(aCustomFragments);
    aGrabBagProperties["OOXCustomFragmentTypes"] <<= comphelper::containerToSequence(aCustomFragmentTypes);
    aGrabBagProperties["OOXCustomFragmentTargets"] <<= comphelper::containerToSequence(aCustomFragmentTargets);

    std::vector<uno::Reference<xml::dom::XDocument>> aCustomXmlDomList;
    std::vector<uno::Reference<xml::dom::XDocument>> aCustomXmlDomPropsList;
    //FIXME: Ideally, we should get these the relations, but it seems that is not consistently set.
    // In some cases it's stored in the workbook relationships, which is unexpected. So we discover them directly.
    for (int i = 1; ; ++i)
    {
        Reference<XDocument> xCustDoc = importFragment("customXml/item" + OUString::number(i) + ".xml");
        Reference<XDocument> xCustDocProps = importFragment("customXml/itemProps" + OUString::number(i) + ".xml");
        if (xCustDoc && xCustDocProps)
        {
            aCustomXmlDomList.emplace_back(xCustDoc);
            aCustomXmlDomPropsList.emplace_back(xCustDocProps);
        }
        else
            break;
    }

    // Adding the saved custom xml DOM
    aGrabBagProperties["OOXCustomXml"] <<= comphelper::containerToSequence(aCustomXmlDomList);
    aGrabBagProperties["OOXCustomXmlProps"] <<= comphelper::containerToSequence(aCustomXmlDomPropsList);

    // Save the [Content_Types].xml after parsing.
    uno::Sequence<uno::Sequence<beans::StringPair>> aContentTypeInfo;
    uno::Reference<io::XInputStream> xInputStream = openInputStream("[Content_Types].xml");
    if (xInputStream.is())
        aContentTypeInfo = comphelper::OFOPXMLHelper::ReadContentTypeSequence(xInputStream, getComponentContext());

    aGrabBagProperties["OOXContentTypes"] <<= aContentTypeInfo;

    Reference<XComponent> xModel = getModel();
    oox::core::XmlFilterBase::putPropertiesToDocumentGrabBag(xModel, aGrabBagProperties);
}

void XmlFilterBase::exportCustomFragments()
{
    Reference<XComponent> xModel = getModel();
    uno::Reference<beans::XPropertySet> xPropSet(xModel, uno::UNO_QUERY_THROW);

    uno::Reference<beans::XPropertySetInfo> xPropSetInfo = xPropSet->getPropertySetInfo();
    if (!xPropSetInfo->hasPropertyByName(UNO_NAME_MISC_OBJ_INTEROPGRABBAG))
        return;

    uno::Sequence<uno::Reference<xml::dom::XDocument>> customXmlDomlist;
    uno::Sequence<uno::Reference<xml::dom::XDocument>> customXmlDomPropslist;
    uno::Sequence<StreamDataSequence> customFragments;
    uno::Sequence<OUString> customFragmentTypes;
    uno::Sequence<OUString> customFragmentTargets;
    uno::Sequence<uno::Sequence<beans::StringPair>> aContentTypes;

    uno::Sequence<beans::PropertyValue> propList;
    xPropSet->getPropertyValue(UNO_NAME_MISC_OBJ_INTEROPGRABBAG) >>= propList;
    for (const auto& rProp : propList)
    {
        const OUString propName = rProp.Name;
        if (propName == "OOXCustomXml")
        {
            rProp.Value >>= customXmlDomlist;
        }
        else if (propName == "OOXCustomXmlProps")
        {
            rProp.Value >>= customXmlDomPropslist;
        }
        else if (propName == "OOXCustomFragments")
        {
            rProp.Value >>= customFragments;
        }
        else if (propName == "OOXCustomFragmentTypes")
        {
            rProp.Value >>= customFragmentTypes;
        }
        else if (propName == "OOXCustomFragmentTargets")
        {
            rProp.Value >>= customFragmentTargets;
        }
        else if (propName == "OOXContentTypes")
        {
            rProp.Value >>= aContentTypes;
        }
    }

    // Expect customXmlDomPropslist.getLength() == customXmlDomlist.getLength().
    for (sal_Int32 j = 0; j < customXmlDomlist.getLength(); j++)
    {
        uno::Reference<xml::dom::XDocument> customXmlDom = customXmlDomlist[j];
        uno::Reference<xml::dom::XDocument> customXmlDomProps = customXmlDomPropslist[j];
        const OUString fragmentPath = "customXml/item" + OUString::number(j+1) + ".xml";
        if (customXmlDom.is())
        {
            addRelation(oox::getRelationship(Relationship::CUSTOMXML), Concat2View("../" + fragmentPath));

            uno::Reference<xml::sax::XSAXSerializable> serializer(customXmlDom, uno::UNO_QUERY);
            uno::Reference<xml::sax::XWriter> writer = xml::sax::Writer::create(comphelper::getProcessComponentContext());
            writer->setOutputStream(openFragmentStream(fragmentPath, "application/xml"));
            serializer->serialize(uno::Reference<xml::sax::XDocumentHandler>(writer, uno::UNO_QUERY_THROW),
                                  uno::Sequence<beans::StringPair>());
        }

        if (customXmlDomProps.is())
        {
            uno::Reference<xml::sax::XSAXSerializable> serializer(customXmlDomProps, uno::UNO_QUERY);
            uno::Reference<xml::sax::XWriter> writer = xml::sax::Writer::create(comphelper::getProcessComponentContext());
            writer->setOutputStream(openFragmentStream("customXml/itemProps"+OUString::number(j+1)+".xml",
                                    "application/vnd.openxmlformats-officedocument.customXmlProperties+xml"));
            serializer->serialize(uno::Reference<xml::sax::XDocumentHandler>(writer, uno::UNO_QUERY_THROW),
                                  uno::Sequence<beans::StringPair>());

            // Adding itemprops's relationship entry to item.xml.rels file
            addRelation(openFragmentStream(fragmentPath, "application/xml"),
                        oox::getRelationship(Relationship::CUSTOMXMLPROPS),
                        Concat2View("itemProps"+OUString::number(j+1)+".xml"));
        }
    }

    // Expect customFragments.getLength() == customFragmentTypes.getLength() == customFragmentTargets.getLength().
    for (sal_Int32 j = 0; j < customFragments.getLength(); j++)
    {
        addRelation(customFragmentTypes[j], customFragmentTargets[j]);
        const OUString aFilename = customFragmentTargets[j];
        Reference<XOutputStream> xOutStream = openOutputStream(aFilename);
        if (xOutStream.is())
        {
            xOutStream->writeBytes(customFragments[j]);
            uno::Reference<XPropertySet> xProps(xOutStream, uno::UNO_QUERY);
            if (xProps.is())
            {
                const OUString aType = comphelper::OFOPXMLHelper::GetContentTypeByName(aContentTypes, aFilename);
                const OUString aContentType = (aType.getLength() ? aType : OUString("application/octet-stream"));
                xProps->setPropertyValue("MediaType", uno::Any(aContentType));
            }
        }
    }
}

} // namespace oox::core

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
