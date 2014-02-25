/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MWAWImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/uno/Reference.h>
#include <cppuhelper/supportsservice.hxx>

#include <xmloff/attrlist.hxx>
#include <ucbhelper/content.hxx>

#include <libmwaw/libmwaw.hxx>
#include <libodfgen/libodfgen.hxx>

#include "common/DocumentHandler.hxx"
#include "common/WPXSvStream.hxx"
#include "MWAWImportFilter.hxx"

using namespace ::com::sun::star::uno;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XFilter;
using com::sun::star::document::XExtendedFilterDetection;
using com::sun::star::ucb::XCommandEnvironment;

using com::sun::star::io::XInputStream;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;

static bool handleEmbeddedMWAWObject(const WPXBinaryData &data, OdfDocumentHandler *pHandler,  const OdfStreamType streamType)
{
    OdgGenerator exporter(pHandler, streamType);
    return MWAWDocument::decodeGraphic(data, &exporter);
}

sal_Bool SAL_CALL MWAWImportFilter::importImpl( const Sequence< ::com::sun::star::beans::PropertyValue > &aDescriptor )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "MWAWImportFilter::importImpl");

    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue *pValue = aDescriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= xInputStream;
    }
    if ( !xInputStream.is() )
    {
        OSL_ASSERT( false );
        return sal_False;
    }

    // An XML import service: what we push sax messages to..
    Reference < XDocumentHandler > xInternalHandler(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.comp.Writer.XMLOasisImporter", mxContext),
        css::uno::UNO_QUERY_THROW);

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDoc);

    // OO Document Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    DocumentHandler xHandler(xInternalHandler);

    WPXSvInputStream input( xInputStream );

    OdtGenerator collector(&xHandler, ODF_FLAT_XML);
    collector.registerEmbeddedObjectHandler("image/mwaw-odg", &handleEmbeddedMWAWObject);
    if (MWAWDocument::MWAW_R_OK == MWAWDocument::parse(&input, &collector))
        return sal_True;
    return sal_False;
}

sal_Bool SAL_CALL MWAWImportFilter::filter( const Sequence< ::com::sun::star::beans::PropertyValue > &aDescriptor )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "MWAWImportFilter::filter");
    return importImpl ( aDescriptor );
}
void SAL_CALL MWAWImportFilter::cancel(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "MWAWImportFilter::cancel");
}

// XImporter
void SAL_CALL MWAWImportFilter::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent > &xDoc )
throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "MWAWImportFilter::getTargetDocument");
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL MWAWImportFilter::detect( com::sun::star::uno::Sequence< PropertyValue > &Descriptor )
throw( com::sun::star::uno::RuntimeException, std::exception )
{
    SAL_INFO("writerperfect", "MWAWImportFilter::detect");

    MWAWDocument::Confidence confidence = MWAWDocument::MWAW_C_NONE;
    MWAWDocument::Type docType = MWAWDocument::MWAW_T_UNKNOWN;
    MWAWDocument::Kind docKind = MWAWDocument::MWAW_K_UNKNOWN;
    OUString sTypeName;
    sal_Int32 nLength = Descriptor.getLength();
    sal_Int32 location = nLength;
    const PropertyValue *pValue = Descriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "TypeName" )
            location=i;
        else if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= xInputStream;
    }

    if (!xInputStream.is())
        return OUString();

    WPXSvInputStream input( xInputStream );

    confidence = MWAWDocument::isFileFormatSupported(&input, docType, docKind);

    if (confidence == MWAWDocument::MWAW_C_EXCELLENT)
    {
        if ( docKind == MWAWDocument::MWAW_K_TEXT )
        {
            switch (docType)
            {
            case MWAWDocument::MWAW_T_ACTA:
                sTypeName = "writer_Mac_Acta";
                break;
            case MWAWDocument::MWAW_T_BEAGLEWORKS:
                sTypeName = "writer_Beagle_Works";
                break;
            case MWAWDocument::MWAW_T_CLARISWORKS:
                sTypeName = "writer_ClarisWorks";
                break;
            case MWAWDocument::MWAW_T_DOCMAKER:
                sTypeName = "writer_DocMaker";
                break;
            case MWAWDocument::MWAW_T_EDOC:
                sTypeName = "writer_eDoc_Document";
                break;
            case MWAWDocument::MWAW_T_GREATWORKS:
                sTypeName = "writer_Great_Works";
                break;
            case MWAWDocument::MWAW_T_FULLWRITE:
                sTypeName = "writer_FullWrite_Professional";
                break;
            case MWAWDocument::MWAW_T_HANMACWORDJ:
                sTypeName = "writer_HanMac_Word_J";
                break;
            case MWAWDocument::MWAW_T_HANMACWORDK:
                sTypeName = "writer_HanMac_Word_K";
                break;
            case MWAWDocument::MWAW_T_LIGHTWAYTEXT:
                sTypeName = "writer_LightWayText";
                break;
            case MWAWDocument::MWAW_T_MACDOC:
                sTypeName = "writer_MacDoc";
                break;
            case MWAWDocument::MWAW_T_MARINERWRITE:
                sTypeName = "writer_Mariner_Write";
                break;
            case MWAWDocument::MWAW_T_MINDWRITE:
                sTypeName = "writer_MindWrite";
                break;
            case MWAWDocument::MWAW_T_MACWRITE:
                sTypeName = "writer_MacWrite";
                break;
            case MWAWDocument::MWAW_T_MACWRITEPRO:
                sTypeName = "writer_MacWritePro";
                break;
            case MWAWDocument::MWAW_T_MICROSOFTWORD:
                sTypeName = "writer_Mac_Word";
                break;
            case MWAWDocument::MWAW_T_MICROSOFTWORKS:
                sTypeName = "writer_Mac_Works";
                break;
            case MWAWDocument::MWAW_T_MORE:
                sTypeName = "writer_Mac_More";
                break;
            case MWAWDocument::MWAW_T_NISUSWRITER:
                sTypeName = "writer_Nisus_Writer";
                break;
            case MWAWDocument::MWAW_T_TEACHTEXT:
                sTypeName = "writer_TeachText";
                break;
            case MWAWDocument::MWAW_T_TEXEDIT:
                sTypeName = "writer_TexEdit";
                break;
            case MWAWDocument::MWAW_T_WRITENOW:
                sTypeName = "writer_WriteNow";
                break;
            case MWAWDocument::MWAW_T_WRITERPLUS:
                sTypeName = "writer_WriterPlus";
                break;
            case MWAWDocument::MWAW_T_ZWRITE:
                sTypeName = "writer_ZWrite";
                break;

            case MWAWDocument::MWAW_T_FRAMEMAKER:
            case MWAWDocument::MWAW_T_MACDRAW:
            case MWAWDocument::MWAW_T_MACPAINT:
            case MWAWDocument::MWAW_T_PAGEMAKER:
            case MWAWDocument::MWAW_T_READYSETGO:
            case MWAWDocument::MWAW_T_RAGTIME:
            case MWAWDocument::MWAW_T_XPRESS:
            case MWAWDocument::MWAW_T_RESERVED1:
            case MWAWDocument::MWAW_T_RESERVED2:
            case MWAWDocument::MWAW_T_RESERVED3:
            case MWAWDocument::MWAW_T_RESERVED4:
            case MWAWDocument::MWAW_T_RESERVED5:
            case MWAWDocument::MWAW_T_RESERVED6:
            case MWAWDocument::MWAW_T_RESERVED7:
            case MWAWDocument::MWAW_T_RESERVED8:
            case MWAWDocument::MWAW_T_RESERVED9:
            case MWAWDocument::MWAW_T_UNKNOWN:
            default:
                break;
            }
        }
    }

    if (!sTypeName.isEmpty())
    {
        if ( location == nLength )
        {
            Descriptor.realloc(nLength+1);
            Descriptor[location].Name = "TypeName";
        }

        Descriptor[location].Value <<=sTypeName;
    }

    return sTypeName;
}


// XInitialization
void SAL_CALL MWAWImportFilter::initialize( const Sequence< Any > &aArguments )
throw (Exception, RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "MWAWImportFilter::initialize");
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        const PropertyValue *pValue = aAnySeq.getConstArray();
        nLength = aAnySeq.getLength();
        for ( sal_Int32 i = 0 ; i < nLength; i++)
        {
            if ( pValue[i].Name == "Type" )
            {
                pValue[i].Value >>= msFilterName;
                break;
            }
        }
    }
}
OUString MWAWImportFilter_getImplementationName ()
throw (RuntimeException)
{
    return OUString (  "com.sun.star.comp.Writer.MWAWImportFilter"  );
}

Sequence< OUString > SAL_CALL MWAWImportFilter_getSupportedServiceNames(  )
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > SAL_CALL MWAWImportFilter_createInstance( const Reference< XComponentContext > &rContext)
throw( Exception )
{
    return (cppu::OWeakObject *) new MWAWImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL MWAWImportFilter::getImplementationName(  )
throw (RuntimeException, std::exception)
{
    return MWAWImportFilter_getImplementationName();
}
sal_Bool SAL_CALL MWAWImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}
Sequence< OUString > SAL_CALL MWAWImportFilter::getSupportedServiceNames(  )
throw (RuntimeException, std::exception)
{
    return MWAWImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
