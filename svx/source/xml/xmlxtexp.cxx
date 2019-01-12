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

#include <tools/urlobj.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

#include <sax/tools/converter.hxx>
#include <sfx2/docfile.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlmetae.hxx>
#include <xmloff/DashStyle.hxx>
#include <xmloff/GradientStyle.hxx>
#include <xmloff/HatchStyle.hxx>
#include <xmloff/ImageStyle.hxx>
#include <xmloff/MarkerStyle.hxx>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <svx/xmlgrhlp.hxx>

#include <xmlxtexp.hxx>

#include <comphelper/storagehelper.hxx>
#include <memory>

using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::document;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;
using namespace ::xmloff::token;
using namespace cppu;

using com::sun::star::embed::XTransactedObject;

class SvxXMLTableEntryExporter
{
public:
    explicit SvxXMLTableEntryExporter( SvXMLExport& rExport ) : mrExport( rExport ) {}
    virtual ~SvxXMLTableEntryExporter();

    virtual void exportEntry( const OUString& rStrName, const Any& rValue ) = 0;

protected:
    SvXMLExport& mrExport;
};

class SvxXMLColorEntryExporter : public SvxXMLTableEntryExporter
{
public:
    explicit SvxXMLColorEntryExporter( SvXMLExport& rExport );

    virtual void exportEntry( const OUString& rStrName, const Any& rValue ) override;
};

class SvxXMLLineEndEntryExporter : public SvxXMLTableEntryExporter
{
public:
    explicit SvxXMLLineEndEntryExporter( SvXMLExport& rExport );

    virtual void exportEntry( const OUString& rStrName, const Any& rValue ) override;
private:
    XMLMarkerStyleExport maMarkerStyle;
};

class SvxXMLDashEntryExporter : public SvxXMLTableEntryExporter
{
public:
    explicit SvxXMLDashEntryExporter( SvXMLExport& rExport );

    virtual void exportEntry( const OUString& rStrName, const Any& rValue ) override;

private:
    XMLDashStyleExport maDashStyle;
};

class SvxXMLHatchEntryExporter : public SvxXMLTableEntryExporter
{
public:
    explicit SvxXMLHatchEntryExporter( SvXMLExport& rExport );

    virtual void exportEntry( const OUString& rStrName, const Any& rValue ) override;
private:
    XMLHatchStyleExport maHatchStyle;
};

class SvxXMLGradientEntryExporter : public SvxXMLTableEntryExporter
{
public:
    explicit SvxXMLGradientEntryExporter( SvXMLExport& rExport );

    virtual void exportEntry( const OUString& rStrName, const Any& rValue ) override;
private:
    XMLGradientStyleExport maGradientStyle;
};

class SvxXMLBitmapEntryExporter : public SvxXMLTableEntryExporter
{
public:
    explicit SvxXMLBitmapEntryExporter( SvXMLExport& rExport );

    virtual void exportEntry( const OUString& rStrName, const Any& rValue ) override;
};


SvxXMLXTableExportComponent::SvxXMLXTableExportComponent(
    const css::uno::Reference< css::uno::XComponentContext >& rContext,
    const OUString& rFileName,
    const uno::Reference<xml::sax::XDocumentHandler> & rHandler,
    const uno::Reference<container::XNameContainer >& xTable,
    uno::Reference<document::XGraphicStorageHandler> const & xGraphicStorageHandler)
:   SvXMLExport(rContext, "", rFileName, rHandler, nullptr, FieldUnit::MM_100TH, SvXMLExportFlags::NONE),
    mxTable( xTable )
{

    GetNamespaceMap_().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_DRAW), GetXMLToken(XML_N_DRAW), XML_NAMESPACE_DRAW );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_SVG), GetXMLToken(XML_N_SVG),  XML_NAMESPACE_SVG );
    SetGraphicStorageHandler(xGraphicStorageHandler);
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
        xProps->setPropertyValue("MediaType",  uno::makeAny( OUString( "text/xml" ) ) );

        // use stock encryption
        xProps->setPropertyValue("UseCommonStoragePasswordEncryption", uno::makeAny( true ) );
    } catch ( const uno::Exception & )
    {
        OSL_FAIL( "exception setting stream metadata" );
    }
}

static void createStorageStream( uno::Reference < io::XOutputStream > *xOut,
                                 rtl::Reference<SvXMLGraphicHelper>&   rxGraphicHelper,
                                 const uno::Reference < embed::XStorage >& xSubStorage )
{
    uno::Reference < io::XStream > xStream;
    xStream = xSubStorage->openStreamElement(
                        "Content.xml",
                        embed::ElementModes::WRITE );
    rxGraphicHelper = SvXMLGraphicHelper::Create( xSubStorage, SvXMLGraphicHelperMode::Write );
    initializeStreamMetadata( xStream );
    *xOut = xStream->getOutputStream();
}

bool SvxXMLXTableExportComponent::save(
        const OUString& rURL,
        const uno::Reference<container::XNameContainer >& xTable,
        const uno::Reference<embed::XStorage >& xStorage,
        OUString *pOptName )
{
    bool bRet = false;
    std::unique_ptr<SfxMedium> pMedium;
    rtl::Reference<SvXMLGraphicHelper> xGraphicHelper;
    sal_Int32 eCreate = embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE;

    INetURLObject aURLObj( rURL );
    bool bToStorage = aURLObj.GetProtocol() == INetProtocol::NotValid; // a relative path

    bool bSaveAsStorage = xTable->getElementType() == cppu::UnoType<awt::XBitmap>::get();

    if( pOptName )
        *pOptName = rURL;

    try
    {
        uno::Reference< uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

        uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create( xContext );

        uno::Reference < io::XStream > xStream;
        uno::Reference < io::XOutputStream > xOut;
        uno::Reference<embed::XStorage > xSubStorage;
        uno::Reference<XGraphicStorageHandler> xGraphicStorageHandler;

        uno::Reference<xml::sax::XDocumentHandler> xHandler( xWriter, uno::UNO_QUERY );

        if( !bToStorage || !xStorage.is() )
        { // local URL -> SfxMedium route
            if( bSaveAsStorage )
                xSubStorage = ::comphelper::OStorageHelper::GetStorageFromURL( rURL, eCreate );
            else
            {
                pMedium.reset(new SfxMedium( rURL, StreamMode::WRITE | StreamMode::TRUNC ));

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
            OUString aPath = rURL;

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
                aPath += ".xml";
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
            createStorageStream( &xOut, xGraphicHelper, xSubStorage );
        if( !xOut.is() )
            return false;

        uno::Reference<io::XActiveDataSource> xMetaSrc( xWriter, uno::UNO_QUERY );
        xMetaSrc->setOutputStream( xOut );
        if( xGraphicHelper.is() )
            xGraphicStorageHandler = xGraphicHelper.get();

        // Finally do the export
        const OUString aName;
        rtl::Reference< SvxXMLXTableExportComponent > xExporter( new SvxXMLXTableExportComponent( xContext, aName, xHandler, xTable, xGraphicStorageHandler ) );
        bRet = xExporter->exportTable();

        if( xGraphicHelper )
            xGraphicHelper->dispose();
        xGraphicHelper.clear();

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
        pMedium->Commit();

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
            std::unique_ptr<SvxXMLTableEntryExporter> pExporter;

            if( aExportType == cppu::UnoType<sal_Int32>::get() )
            {
                pExporter.reset(new SvxXMLColorEntryExporter(*this));
                pEleName = "color-table";
            }
            else if( aExportType == cppu::UnoType< drawing::PolyPolygonBezierCoords >::get() )
            {
                pExporter.reset(new SvxXMLLineEndEntryExporter(*this));
                pEleName = "marker-table";
            }
            else if( aExportType == cppu::UnoType< drawing::LineDash >::get() )
            {
                pExporter.reset(new SvxXMLDashEntryExporter(*this));
                pEleName = "dash-table";
            }
            else if( aExportType == cppu::UnoType< drawing::Hatch >::get() )
            {
                pExporter.reset(new SvxXMLHatchEntryExporter(*this));
                pEleName = "hatch-table";
            }
            else if( aExportType == cppu::UnoType< awt::Gradient >::get() )
            {
                pExporter.reset(new SvxXMLGradientEntryExporter(*this));
                pEleName = "gradient-table";
            }
            else if( aExportType == cppu::UnoType<awt::XBitmap>::get())
            {
                pExporter.reset(new SvxXMLBitmapEntryExporter(*this));
                pEleName = "bitmap-table";
            }
            else
            {
                OSL_FAIL( "unknown type for export");
                break;
            }

            SvXMLElementExport aElem( *this, XML_NAMESPACE_OOO, pEleName, true, true );

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
        while(false);

        GetDocHandler()->endDocument();
    }
    catch( Exception const& )
    {
        bRet = false;
    }

    return bRet;
}

// methods without content:
void SvxXMLXTableExportComponent::ExportAutoStyles_() {}
void SvxXMLXTableExportComponent::ExportMasterStyles_() {}
void SvxXMLXTableExportComponent::ExportContent_() {}


SvxXMLTableEntryExporter::~SvxXMLTableEntryExporter()
{
}


SvxXMLColorEntryExporter::SvxXMLColorEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport )
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

    SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW, XML_COLOR, true, true );
}


SvxXMLLineEndEntryExporter::SvxXMLLineEndEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport ), maMarkerStyle( rExport )
{
}

void SvxXMLLineEndEntryExporter::exportEntry( const OUString& rStrName, const Any& rValue )
{
    maMarkerStyle.exportXML( rStrName, rValue );
}


SvxXMLDashEntryExporter::SvxXMLDashEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport ), maDashStyle( rExport )
{
}

void SvxXMLDashEntryExporter::exportEntry( const OUString& rStrName, const Any& rValue )
{
    maDashStyle.exportXML( rStrName, rValue );
}


SvxXMLHatchEntryExporter::SvxXMLHatchEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport ), maHatchStyle( rExport )
{
}

void SvxXMLHatchEntryExporter::exportEntry( const OUString& rStrName, const Any& rValue )
{
    maHatchStyle.exportXML( rStrName, rValue );
}


SvxXMLGradientEntryExporter::SvxXMLGradientEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport ), maGradientStyle( rExport )
{
}

void SvxXMLGradientEntryExporter::exportEntry( const OUString& rStrName, const Any& rValue )
{
    maGradientStyle.exportXML( rStrName, rValue );
}


SvxXMLBitmapEntryExporter::SvxXMLBitmapEntryExporter( SvXMLExport& rExport )
: SvxXMLTableEntryExporter( rExport )
{
}

void SvxXMLBitmapEntryExporter::exportEntry( const OUString& rStrName, const Any& rValue )
{
    XMLImageStyle::exportXML(rStrName, rValue, mrExport);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
