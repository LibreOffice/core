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

#include <tools/urlobj.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

#include <sax/tools/converter.hxx>
#include <sfx2/docfile.hxx>
#include <rtl/ustrbuf.hxx>
#include "xmloff/xmlnmspe.hxx"
#include "xmloff/nmspmap.hxx"

#include "xmloff/xmltoken.hxx"
#include "xmloff/xmlmetae.hxx"
#include "xmloff/DashStyle.hxx"
#include "xmloff/GradientStyle.hxx"
#include "xmloff/HatchStyle.hxx"
#include "xmloff/ImageStyle.hxx"
#include "xmloff/MarkerStyle.hxx"
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include "svx/xmlgrhlp.hxx"

#include "xmlxtexp.hxx"

#include <comphelper/storagehelper.hxx>

using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;
using namespace ::xmloff::token;
using namespace ::rtl;
using namespace cppu;

using com::sun::star::embed::XTransactedObject;

class SvxXMLTableEntryExporter
{
public:
    SvxXMLTableEntryExporter( SvXMLExport& rExport ) : mrExport( rExport ) {}
    virtual ~SvxXMLTableEntryExporter();

    virtual void exportEntry( const OUString& rStrName, const Any& rValue ) = 0;

protected:
    SvXMLExport& mrExport;
};

class SvxXMLColorEntryExporter : public SvxXMLTableEntryExporter
{
public:
    SvxXMLColorEntryExporter( SvXMLExport& rExport );
    virtual ~SvxXMLColorEntryExporter();

    virtual void exportEntry( const OUString& rStrName, const Any& rValue );
};

class SvxXMLLineEndEntryExporter : public SvxXMLTableEntryExporter
{
public:
    SvxXMLLineEndEntryExporter( SvXMLExport& rExport );
    SvxXMLLineEndEntryExporter();
    virtual ~SvxXMLLineEndEntryExporter();

    virtual void exportEntry( const OUString& rStrName, const Any& rValue );
private:
    XMLMarkerStyleExport maMarkerStyle;
};

class SvxXMLDashEntryExporter : public SvxXMLTableEntryExporter
{
public:
    SvxXMLDashEntryExporter( SvXMLExport& rExport );
    virtual ~SvxXMLDashEntryExporter();

    virtual void exportEntry( const OUString& rStrName, const Any& rValue );

private:
    XMLDashStyleExport maDashStyle;
};

class SvxXMLHatchEntryExporter : public SvxXMLTableEntryExporter
{
public:
    SvxXMLHatchEntryExporter( SvXMLExport& rExport );
    virtual ~SvxXMLHatchEntryExporter();

    virtual void exportEntry( const OUString& rStrName, const Any& rValue );
private:
    XMLHatchStyleExport maHatchStyle;
};

class SvxXMLGradientEntryExporter : public SvxXMLTableEntryExporter
{
public:
    SvxXMLGradientEntryExporter( SvXMLExport& rExport );
    virtual ~SvxXMLGradientEntryExporter();

    virtual void exportEntry( const OUString& rStrName, const Any& rValue );
private:
    XMLGradientStyleExport maGradientStyle;
};

class SvxXMLBitmapEntryExporter : public SvxXMLTableEntryExporter
{
public:
    SvxXMLBitmapEntryExporter( SvXMLExport& rExport );
    virtual ~SvxXMLBitmapEntryExporter();

    virtual void exportEntry( const OUString& rStrName, const Any& rValue );

private:
    XMLImageStyle maImageStyle;
};

///////////////////////////////////////////////////////////////////////

// #110680#
SvxXMLXTableExportComponent::SvxXMLXTableExportComponent(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const OUString& rFileName,
    const uno::Reference<xml::sax::XDocumentHandler> & rHandler,
    const uno::Reference<container::XNameContainer >& xTable,
    uno::Reference<document::XGraphicObjectResolver >& xGrfResolver )
:   SvXMLExport( xServiceFactory, rFileName, rHandler, NULL, MAP_100TH_MM),
    mxTable( xTable )
{

    _GetNamespaceMap().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_DRAW), GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_SVG), GetXMLToken(XML_N_SVG),  XML_NAMESPACE_SVG );
    SetGraphicResolver( xGrfResolver );
    setExportFlags( 0 );
}

SvxXMLXTableExportComponent::~SvxXMLXTableExportComponent()
{
}

static void initializeStreamMetadata( const uno::Reference< uno::XInterface > &xOut )
{
    uno::Reference< beans::XPropertySet > xProps( xOut, uno::UNO_QUERY );
    if( !xProps.is() )
    {
        OSL_FAIL( "Missing stream metadata interface" );
        return;
    }

    try
    {
        xProps->setPropertyValue(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ),
            uno::makeAny( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "text/xml" ) ) ) );

        // use stock encryption
        xProps->setPropertyValue(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UseCommonStoragePasswordEncryption" ) ),
            uno::makeAny( sal_True ) );
    } catch ( const uno::Exception & )
    {
        OSL_FAIL( "exception setting stream metadata" );
    }
}

static void createStorageStream( uno::Reference < io::XOutputStream > *xOut,
                                 SvXMLGraphicHelper                  **ppGraphicHelper,
                                 uno::Reference < embed::XStorage >    xSubStorage )
{
    uno::Reference < io::XStream > xStream;
    xStream = xSubStorage->openStreamElement(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Content.xml" ) ),
                        embed::ElementModes::WRITE );
    *ppGraphicHelper = SvXMLGraphicHelper::Create( xSubStorage, GRAPHICHELPER_MODE_WRITE );
    initializeStreamMetadata( xStream );
    *xOut = xStream->getOutputStream();
}

bool SvxXMLXTableExportComponent::save(
        const OUString& rURL,
        const uno::Reference<container::XNameContainer >& xTable,
        const uno::Reference<embed::XStorage >& xStorage,
        rtl::OUString *pOptName ) throw()
{
    bool bRet = false;
    SfxMedium* pMedium = NULL;
    SvXMLGraphicHelper* pGraphicHelper = NULL;
    sal_Int32 eCreate = embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE;

    INetURLObject aURLObj( rURL );
    bool bToStorage = aURLObj.GetProtocol() == INET_PROT_NOT_VALID; // a relative path

    sal_Bool bSaveAsStorage = xTable->getElementType() == ::getCppuType((const OUString*)0);

    if( pOptName )
        *pOptName = rURL;

    try
    {
        uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );
        uno::Reference< uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
        if( !xServiceFactory.is() )
        {
            OSL_FAIL( "got no service manager" );
            return false;
        }

        uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create( xContext );

        uno::Reference < io::XStream > xStream;
        uno::Reference < io::XOutputStream > xOut;
        uno::Reference<embed::XStorage > xSubStorage;
        uno::Reference< XGraphicObjectResolver > xGrfResolver;

        uno::Reference<xml::sax::XDocumentHandler> xHandler( xWriter, uno::UNO_QUERY );

        if( !bToStorage || !xStorage.is() )
        { // local URL -> SfxMedium route
            if( bSaveAsStorage )
                xSubStorage = ::comphelper::OStorageHelper::GetStorageFromURL( rURL, eCreate );
            else
            {
                pMedium = new SfxMedium( rURL, STREAM_WRITE | STREAM_TRUNC );
                pMedium->IsRemote();

                SvStream* pStream = pMedium->GetOutStream();
                if( !pStream )
                {
                    OSL_FAIL( "no output stream!" );
                    return false;
                }

                xOut = new utl::OOutputStreamWrapper( *pStream );
            }
        }
        else // save into the xSubStorage
        {
            rtl::OUString aPath = rURL;

            if( bSaveAsStorage )
            {
                try {
                    xSubStorage = xStorage->openStorageElement( aPath, eCreate );
                } catch (uno::Exception &) {
                    OSL_FAIL( "no output storage!" );
                    return false;
                }
            }
            else
            {
                aPath += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".xml" ) );
                try {
                    xStream = xStorage->openStreamElement( aPath, eCreate );
                    if( !xStream.is() )
                        return false;
                    initializeStreamMetadata( xStream );
                    xOut = xStream->getOutputStream();
                } catch (uno::Exception &) {
                    OSL_FAIL( "no output stream!" );
                    return false;
                }
                if( pOptName )
                    *pOptName = aPath;
            }
        }

        if( !xOut.is() && xSubStorage.is() )
            createStorageStream( &xOut, &pGraphicHelper, xSubStorage );
        if( !xOut.is() )
            return false;

        uno::Reference<io::XActiveDataSource> xMetaSrc( xWriter, uno::UNO_QUERY );
        xMetaSrc->setOutputStream( xOut );
        if( pGraphicHelper )
            xGrfResolver = pGraphicHelper;

        // Finally do the export
        const OUString aName;
        SvxXMLXTableExportComponent aExporter( xServiceFactory, aName, xHandler, xTable, xGrfResolver );
        bRet = aExporter.exportTable();

        if( pGraphicHelper )
            SvXMLGraphicHelper::Destroy( pGraphicHelper );

        if( xSubStorage.is() )
        {
            uno::Reference< XTransactedObject > xTrans( xSubStorage, UNO_QUERY );
            if( xTrans.is() )
                xTrans->commit();

            uno::Reference< XComponent > xComp( xSubStorage, UNO_QUERY );
            if( xComp.is() )
                xSubStorage->dispose();
        }
    }
    catch( uno::Exception& )
    {
        bRet = false;
    }

    if( pMedium )
    {
        pMedium->Commit();
        delete pMedium;
    }

    return bRet;
}

bool SvxXMLXTableExportComponent::exportTable() throw()
{
    bool bRet = false;

    try
    {
        GetDocHandler()->startDocument();

        addChaffWhenEncryptedStorage();

        // export namespaces
        sal_uInt16 nPos = GetNamespaceMap().GetFirstKey();
        while( USHRT_MAX != nPos )
        {
            GetAttrList().AddAttribute( GetNamespaceMap().GetAttrNameByKey( nPos ), GetNamespaceMap().GetNameByKey( nPos ) );
            nPos = GetNamespaceMap().GetNextKey( nPos );
        }

        do
        {
            if( !mxTable.is() )
                break;

            char const* pEleName;
            Type aExportType = mxTable->getElementType();
            SvxXMLTableEntryExporter* pExporter = NULL;

            if( aExportType == ::getCppuType((const sal_Int32*)0) )
            {
                pExporter = new SvxXMLColorEntryExporter(*this);
                pEleName = "color-table";
            }
            else if( aExportType == ::getCppuType((const drawing::PolyPolygonBezierCoords*)0) )
            {
                pExporter = new SvxXMLLineEndEntryExporter(*this);
                pEleName = "marker-table";
            }
            else if( aExportType == ::getCppuType((const drawing::LineDash*)0) )
            {
                pExporter = new SvxXMLDashEntryExporter(*this);
                pEleName = "dash-table";
            }
            else if( aExportType == ::getCppuType((const drawing::Hatch*)0) )
            {
                pExporter = new SvxXMLHatchEntryExporter(*this);
                pEleName = "hatch-table";
            }
            else if( aExportType == ::getCppuType((const awt::Gradient*)0))
            {
                pExporter = new SvxXMLGradientEntryExporter(*this);
                pEleName = "gradient-table";
            }
            else if( aExportType == ::getCppuType((const OUString*)0))
            {
                pExporter = new SvxXMLBitmapEntryExporter(*this);
                pEleName = "bitmap-table";
            }
            else
            {
                OSL_FAIL( "unknown type for export");
                break;
            }

            SvXMLElementExport aElem( *this, XML_NAMESPACE_OOO, pEleName, sal_True, sal_True );

            Sequence< OUString > aNames = mxTable->getElementNames();
            const sal_Int32 nCount = aNames.getLength();
            const OUString* pNames = aNames.getConstArray();
            Any aAny;

            sal_Int32 nIndex;
            for( nIndex = 0; nIndex < nCount; nIndex++, pNames++ )
            {
                aAny = mxTable->getByName( *pNames );
                pExporter->exportEntry( *pNames, aAny );
            }

            bRet = true;
        }
        while(0);

        GetDocHandler()->endDocument();
    }
    catch( Exception const& )
    {
        bRet = false;
    }

    return bRet;
}

// methods without content:
void SvxXMLXTableExportComponent::_ExportAutoStyles() {}
void SvxXMLXTableExportComponent::_ExportMasterStyles() {}
void SvxXMLXTableExportComponent::_ExportContent() {}

///////////////////////////////////////////////////////////////////////

SvxXMLTableEntryExporter::~SvxXMLTableEntryExporter()
{
}

///////////////////////////////////////////////////////////////////////

SvxXMLColorEntryExporter::SvxXMLColorEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport )
{
}

SvxXMLColorEntryExporter::~SvxXMLColorEntryExporter()
{
}

void SvxXMLColorEntryExporter::exportEntry( const OUString& rStrName, const Any& rValue )
{
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, rStrName );

    sal_Int32 nColor = 0;
    rValue >>= nColor;

    OUStringBuffer aOut;
    ::sax::Converter::convertColor( aOut, nColor );
    mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_COLOR, aOut.makeStringAndClear() );

    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW, XML_COLOR, sal_True, sal_True );
}

///////////////////////////////////////////////////////////////////////

SvxXMLLineEndEntryExporter::SvxXMLLineEndEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport ), maMarkerStyle( rExport )
{
}

SvxXMLLineEndEntryExporter::~SvxXMLLineEndEntryExporter()
{
}

void SvxXMLLineEndEntryExporter::exportEntry( const OUString& rStrName, const Any& rValue )
{
    maMarkerStyle.exportXML( rStrName, rValue );
}

///////////////////////////////////////////////////////////////////////

SvxXMLDashEntryExporter::SvxXMLDashEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport ), maDashStyle( rExport )
{
}

SvxXMLDashEntryExporter::~SvxXMLDashEntryExporter()
{
}

void SvxXMLDashEntryExporter::exportEntry( const OUString& rStrName, const Any& rValue )
{
    maDashStyle.exportXML( rStrName, rValue );
}

///////////////////////////////////////////////////////////////////////

SvxXMLHatchEntryExporter::SvxXMLHatchEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport ), maHatchStyle( rExport )
{
}

SvxXMLHatchEntryExporter::~SvxXMLHatchEntryExporter()
{
}

void SvxXMLHatchEntryExporter::exportEntry( const OUString& rStrName, const Any& rValue )
{
    maHatchStyle.exportXML( rStrName, rValue );
}

///////////////////////////////////////////////////////////////////////

SvxXMLGradientEntryExporter::SvxXMLGradientEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport ), maGradientStyle( rExport )
{
}

SvxXMLGradientEntryExporter::~SvxXMLGradientEntryExporter()
{
}

void SvxXMLGradientEntryExporter::exportEntry( const OUString& rStrName, const Any& rValue )
{
    maGradientStyle.exportXML( rStrName, rValue );
}

///////////////////////////////////////////////////////////////////////

SvxXMLBitmapEntryExporter::SvxXMLBitmapEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport )
{
}

SvxXMLBitmapEntryExporter::~SvxXMLBitmapEntryExporter()
{
}

void SvxXMLBitmapEntryExporter::exportEntry( const OUString& rStrName, const Any& rValue )
{
    maImageStyle.exportXML( rStrName, rValue, mrExport );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
