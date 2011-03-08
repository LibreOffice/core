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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <tools/debug.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <sfx2/docfile.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmloff/xmlnmspe.hxx"
#include "xmloff/nmspmap.hxx"

#include "xmloff/xmltoken.hxx"
#include "xmloff/xmlmetae.hxx"
#include "xmloff/DashStyle.hxx"
#include "xmloff/GradientStyle.hxx"
#include "xmloff/HatchStyle.hxx"
#include "xmloff/ImageStyle.hxx"
#include "xmloff/MarkerStyle.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include "xmlgrhlp.hxx"

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

sal_Bool SvxXMLXTableExportComponent::save( const OUString& rURL, const uno::Reference<container::XNameContainer >& xTable ) throw()
{
    uno::Reference < embed::XStorage > xStorage;
    SfxMedium* pMedium = NULL;
    sal_Bool bRet = sal_False;

    uno::Reference< XGraphicObjectResolver >    xGrfResolver;
    SvXMLGraphicHelper* pGraphicHelper = 0;

    try
    {
        do
        {
            uno::Reference < io::XOutputStream > xOut;
            uno::Reference < io::XStream > xStream;

            sal_Bool bNeedStorage = xTable->getElementType() == ::getCppuType((const OUString*)0);

            uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );
            if( !xServiceFactory.is() )
            {
                OSL_FAIL( "got no service manager" );
                return FALSE;
            }

            uno::Reference< uno::XInterface > xWriter( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" ) ) ) );
            if( !xWriter.is() )
            {
                OSL_FAIL( "com.sun.star.xml.sax.Writer service missing" );
                return FALSE;
            }

            uno::Reference<xml::sax::XDocumentHandler>  xHandler( xWriter, uno::UNO_QUERY );

            if( bNeedStorage )
            {
                xStorage =
                  ::comphelper::OStorageHelper::GetStorageFromURL( rURL, embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE );

                if( !xStorage.is() )
                {
                    OSL_FAIL( "no storage!" );
                    break;
                }

                OUString sMetaName( RTL_CONSTASCII_USTRINGPARAM( "Content.xml" ) );
                xStream = xStorage->openStreamElement( sMetaName, embed::ElementModes::WRITE );
                pGraphicHelper = SvXMLGraphicHelper::Create( xStorage, GRAPHICHELPER_MODE_WRITE );
                xGrfResolver = pGraphicHelper;
                xOut = xStream->getOutputStream();
            }
            else
            {
                pMedium = new SfxMedium( rURL, STREAM_WRITE | STREAM_TRUNC, TRUE );
                pMedium->IsRemote();

                SvStream* pStream = pMedium->GetOutStream();
                if( NULL == pStream )
                {
                    OSL_FAIL( "no output stream!" );
                    break;
                }

                xOut = new utl::OOutputStreamWrapper( *pStream );
            }

            uno::Reference<io::XActiveDataSource> xMetaSrc( xWriter, uno::UNO_QUERY );
            xMetaSrc->setOutputStream( xOut );

            const OUString aName;

            // #110680#
            // SvxXMLXTableExportComponent aExporter( aName, xHandler, xTable, xGrfResolver );
            SvxXMLXTableExportComponent aExporter( xServiceFactory, aName, xHandler, xTable, xGrfResolver );

            bRet = aExporter.exportTable();

        }
        while( 0 );

        if( pGraphicHelper )
            SvXMLGraphicHelper::Destroy( pGraphicHelper );

        if( xStorage.is() )
        {
            uno::Reference< XTransactedObject > xTrans( xStorage, UNO_QUERY );
            if( xTrans.is() )
                xTrans->commit();

            uno::Reference< XComponent > xComp( xStorage, UNO_QUERY );
            if( xComp.is() )
                xStorage->dispose();
        }
    }
    catch( uno::Exception& )
    {
        bRet = sal_False;
    }

    if( pMedium )
    {
        pMedium->Commit();
        delete pMedium;
    }

    return bRet;
}

sal_Bool SvxXMLXTableExportComponent::exportTable() throw()
{
    sal_Bool bRet = sal_False;

    try
    {
        GetDocHandler()->startDocument();

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

            bRet = sal_True;
        }
        while(0);

        GetDocHandler()->endDocument();
    }
    catch( Exception const& )
    {
        bRet = sal_False;
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
    mrExport.GetMM100UnitConverter().convertColor( aOut, nColor );
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
